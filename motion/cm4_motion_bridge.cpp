#include <cmath>
#include "cm4_motion_bridge.hpp"

#include "motion_proto.hpp"
#include "motion/validation.hpp"

#include <cstring>

namespace robot_cm4 {
namespace {
// A retransmitted execution event must never extend the Medusa/Athena status
// pulse beyond the window stamped by the FIRST observation of that event.
constexpr motion::TimeUs kExecutionPulseWindowUs = 100000;
}  // namespace

void Cm4MotionBridge::configure(const motion::MotionConfigV2& config,
                                std::uint32_t session_id,
                                motion::TimeUs now_us) {
    config_ = config;
    session_id_ = session_id != 0 ? session_id
                                  : static_cast<std::uint32_t>(now_us ^ (now_us >> 32));
    disabled_sequence_ = 1;
    command_sequences_ = motion::SequenceGate{};
    command_ = motion::MotionCommandV2{};
    has_command_ = false;
    has_vision_ = false;
    last_attached_vision_sequence_ = 0;
    last_attached_vision_capture_ = 0;
    feedback_ = motion::MotionFeedbackV2{};
    has_feedback_ = false;
    last_feedback_receive_us_ = 0;
    last_feedback_frame_us_ = 0;
    spi_timed_out_ = true;
    commands_submitted_ = 0;
    protocol_errors_ = 0;
    spi_timeouts_ = 0;
    vision_attached_ = 0;
    feedback_frames_ = 0;
    config_frames_sent_ = 0;
    config_sent_ = false;
    command_time_map_.reset();
    feedback_time_map_.reset();
    command_receive_us_ = now_us;
    // A CM4 restart starts a new session: keep the explicitly enabled match
    // mode but require a fresh capability handshake and drop old ACKs/pulses.
    const bool match_enabled = match_execution_.enabled();
    {
        std::lock_guard<std::mutex> lock(command_mutex_);
        pending_ack_count_ = 0;
        execution_acks_forwarded_ = 0;
    }
    {
        std::lock_guard<std::mutex> lock(feedback_mutex_);
        current_world_session_ = 0;
        last_execution_feedback_ = motion::KickExecutionFeedback{};
        pulse_valid_ = false;
        pulse_start_us_ = 0;
        flat_kick_ms_ = 0;
        chip_kick_ms_ = 0;
        execution_feedback_frames_ = 0;
        execution_events_ = 0;
        execution_duplicates_ = 0;
        execution_old_generation_ = 0;
    }
    match_execution_.configure(match_enabled, now_us);
}

void Cm4MotionBridge::setMatchExecutionEnabled(bool enabled,
                                                std::uint32_t world_session_id,
                                                motion::TimeUs now_us) {
    match_world_session_id_ = world_session_id;
    if (enabled == match_execution_.enabled()) return;
    match_execution_.configure(enabled, now_us);
    std::lock_guard<std::mutex> command_lock(command_mutex_);
    pending_ack_count_ = 0;
    std::lock_guard<std::mutex> feedback_lock(feedback_mutex_);
    current_world_session_ = 0;
    last_execution_feedback_ = motion::KickExecutionFeedback{};
    pulse_valid_ = false;
    pulse_start_us_ = 0;
    flat_kick_ms_ = 0;
    chip_kick_ms_ = 0;
}

bool Cm4MotionBridge::submitRobotCommand(const ZSS::New::Robot_Command& proto,
                                         motion::TimeUs receive_us) {
    std::lock_guard<std::mutex> lock(command_mutex_);
    motion::MotionCommandV2 command;
    if (!proto.has_motion_command()) return false;
    // Execution ACKs ride the ordinary command chain but clear board state
    // only; they are queued for the next command frame even when the motion
    // command itself is rejected.  They never refresh command age/validity.
    if (match_execution_.enabled() && proto.motion_command().has_execution_ack()) {
        motion::KickExecutionAck ack =
            executionAckFromProto(proto.motion_command().execution_ack());
        if (ack.kick_sequence != 0) pushPendingAck(ack);
    }
    {
        command = commandFromProto(proto.motion_command());
        if (!motion::validCommand(command)) return false;
        if (!command_sequences_.accepts(command.session_id, command.sequence)) return false;
        if (has_command_ && command.session_id != command_.session_id) {
            command_time_map_.reset();
            has_vision_ = false;
        }
        if (command.generated_time_us == 0) {
            command.generated_time_us = receive_us;
        } else {
            command_time_map_.observe(command.generated_time_us, receive_us);
            command.generated_time_us = command_time_map_.toLocal(command.generated_time_us);
        }
        if (command.session_id == 0) command.session_id = session_id_;
    }
    if (command.valid_for_us == 0) {
        command.valid_for_us = motion::secondsToUs(config_.command_timeout_s);
    }
    command.received_time_us = receive_us;

    motion::VisionObservationV2 vision;
    bool has_vision = false;
    if (proto.has_vision_observation() && proto.vision_observation().valid()) {
        vision = visionFromProto(proto.vision_observation());
        const motion::TimeUs remote_capture = vision.capture_time_us;
        if (remote_capture != 0) {
            vision.capture_time_us = command_time_map_.toLocal(remote_capture);
        } else {
            vision.capture_time_us = receive_us;
        }
        vision.receive_time_us = receive_us;
        has_vision = true;
    }

    {
        command_ = command;
        command_sequences_.accept(command.session_id, command.sequence);
        has_command_ = true;
        command_receive_us_ = receive_us;
        if (has_vision) {
            if (!has_vision_ || vision.sequence == 0 ||
                vision.capture_time_us >= vision_.capture_time_us ||
                (vision.capture_time_us == vision_.capture_time_us &&
                 vision.sequence >= vision_.sequence)) {
                vision_ = vision;
                has_vision_ = true;
            }
        }
        ++commands_submitted_;
    }
    return true;
}

bool Cm4MotionBridge::submitVision(const motion::VisionObservationV2& observation,
                                   motion::TimeUs now_us) {
    if (!observation.valid) return false;
    motion::VisionObservationV2 copy = observation;
    if (copy.capture_time_us == 0) copy.capture_time_us = now_us;
    copy.receive_time_us = now_us;
    {
        std::lock_guard<std::mutex> lock(command_mutex_);
        if (has_vision_ && copy.capture_time_us < vision_.capture_time_us) {
            return false;
        }
        vision_ = copy;
        has_vision_ = true;
    }
    return true;
}

void Cm4MotionBridge::buildDisabledCommand(motion::TimeUs now_us,
                                           motion::MotionCommandV2* command) {
    *command = motion::MotionCommandV2{};
    // A safety frame uses its own session so it can never make a later real
    // command look stale just because the disabled stream advanced further.
    command->session_id = session_id_ ^ 0x5AFE0000u;
    command->sequence = disabled_sequence_++;
    command->generated_time_us = now_us;
    command->valid_for_us = 100000;
    command->translation_mode = motion::TranslationMode::kStop;
    command->yaw_mode = motion::YawMode::kHold;
    command->enable = false;
    command->limits = config_.limits;
    command->trajectory_epoch = 0;
    command->dribble_mode = 0;
}

bool Cm4MotionBridge::buildCommandFrame(motion::TimeUs now_us, std::uint8_t* frame) {
    motion::MotionCommandV2 command;
    motion::VisionObservationV2 vision;
    bool attach_vision = false;
    {
        std::lock_guard<std::mutex> lock(command_mutex_);
        // Preserve the original sequence and generation time even after
        // expiry. A synthetic new session would reopen replay/kick gates.
        if (has_command_) {
            command = command_;
        } else {
            buildDisabledCommand(now_us, &command);
        }
        if (has_vision_ && vision_.valid && now_us >= vision_.capture_time_us &&
            now_us - vision_.capture_time_us <= motion::secondsToUs(0.08f)) {
            if (vision_.sequence == 0 ||
                vision_.sequence != last_attached_vision_sequence_ ||
                vision_.capture_time_us != last_attached_vision_capture_) {
                vision = vision_;
                attach_vision = true;
                last_attached_vision_sequence_ = vision_.sequence;
                last_attached_vision_capture_ = vision_.capture_time_us;
                ++vision_attached_;
            }
        }
        if (match_execution_.enabled()) {
            // Capability declaration is retransmitted on every command frame
            // until the STM32 observes it; the exchange is idempotent.
            command.match_capability =
                match_execution_.declaration(match_world_session_id_);
            motion::KickExecutionAck ack;
            if (popPendingAck(&ack)) {
                command.execution_ack = ack;
                ++execution_acks_forwarded_;
            }
        }
    }
    if (frame == nullptr) return false;
    return motion::encodeCommandFrame(command, attach_vision ? &vision : nullptr, frame);
}

bool Cm4MotionBridge::buildConfigFrame(motion::TimeUs now_us, std::uint8_t* frame) {
    if (frame == nullptr) return false;
    const bool ok = motion::encodeConfigFrame(config_, session_id_ ^ 0xC0F10000u,
                                              config_frames_sent_ + 1, now_us, 100000,
                                              frame);
    if (ok) {
        ++config_frames_sent_;
        config_sent_ = true;
    }
    return ok;
}

bool Cm4MotionBridge::buildFrame(motion::TimeUs now_us, std::uint8_t* frame) {
    if (match_execution_.enabled()) match_execution_.poll(now_us);
    if (!config_sent_) return buildConfigFrame(now_us, frame);
    return buildCommandFrame(now_us, frame);
}

bool Cm4MotionBridge::processFeedbackFrame(const std::uint8_t* frame,
                                           motion::TimeUs now_us) {
    motion::MotionFeedbackV2 feedback;
    motion::MatchExecutionExtension extension;
    const bool match_enabled = match_execution_.enabled();
    if (match_enabled) {
        // Extension-aware decode: a malformed tail invalidates the whole frame.
        if (!motion::decodeFeedbackFrame(frame, &feedback, &extension)) {
            ++protocol_errors_;
            return false;
        }
    } else if (!motion::decodeFeedbackFrame(frame, &feedback)) {
        ++protocol_errors_;
        return false;
    }
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    match_execution_.poll(now_us);
    const motion::TimeUs source_time = feedback.feedback_time_us;
    if (has_feedback_ && source_time <= last_feedback_frame_us_) return false;
    if (feedback.feedback_time_us != 0) {
        feedback_time_map_.observe(feedback.feedback_time_us, now_us);
        feedback.fusion_time_us = feedback_time_map_.toLocal(feedback.fusion_time_us);
        feedback.prediction_time_us =
            feedback_time_map_.toLocal(feedback.prediction_time_us);
        feedback.vision_capture_time_us =
            feedback_time_map_.toLocal(feedback.vision_capture_time_us);
        feedback.feedback_time_us = feedback_time_map_.toLocal(feedback.feedback_time_us);
    }
    {
        feedback_ = feedback;
        has_feedback_ = true;
        last_feedback_receive_us_ = now_us;
        last_feedback_frame_us_ = source_time;
        spi_timed_out_ = false;
        ++feedback_frames_;
        if (match_enabled) observeFeedbackExtension(extension, now_us);
    }
    return true;
}

void Cm4MotionBridge::observeFeedbackExtension(
    const motion::MatchExecutionExtension& extension, motion::TimeUs now_us) {
    // Caller holds feedback_mutex_.
    if (extension.capability.present) {
        match_execution_.observePeerCapabilities(extension.capability.capability_bits,
                                                 now_us);
        if (current_world_session_ == 0 &&
            extension.capability.world_session_id != 0) {
            current_world_session_ = extension.capability.world_session_id;
        }
    }
    // Elapsed status-pulse time is derived from the FIRST observation of an
    // executed event, so retransmissions can never extend it.
    flat_kick_ms_ = 0;
    chip_kick_ms_ = 0;
    if (pulse_valid_ && now_us >= pulse_start_us_) {
        const motion::TimeUs elapsed = now_us - pulse_start_us_;
        if (elapsed <= kExecutionPulseWindowUs) {
            const std::int32_t ms =
                static_cast<std::int32_t>(elapsed / 1000) + 1;
            if (pulse_event_type_ == motion::KickEventType::kFlat) {
                flat_kick_ms_ = ms;
            } else if (pulse_event_type_ == motion::KickEventType::kChip) {
                chip_kick_ms_ = ms;
            }
        }
    }
    if (!extension.feedback.present) return;
    ++execution_feedback_frames_;
    const motion::KickExecutionFeedback& event = extension.feedback;
    const bool same_event = pulse_valid_ &&
        event.world_session_id == pulse_world_session_ &&
        event.producer_session_id == pulse_producer_session_ &&
        event.kick_sequence == pulse_kick_sequence_;
    if (same_event) {
        // Duplicate feedback: forward the event identity (Medusa deduplicates
        // and ACKs again) but do not restamp or extend the status pulse.
        ++execution_duplicates_;
        last_execution_feedback_ = event;
        return;
    }
    pulse_valid_ = true;
    pulse_world_session_ = event.world_session_id;
    pulse_producer_session_ = event.producer_session_id;
    pulse_kick_sequence_ = event.kick_sequence;
    pulse_event_type_ = motion::KickEventType::kNone;
    pulse_start_us_ = 0;
    if (current_world_session_ == 0) current_world_session_ = event.world_session_id;
    if (event.state != motion::KickExecutionState::kExecuted) {
        // rejected/expired/deduplicated events only complete the audit trail.
        last_execution_feedback_ = event;
        return;
    }
    if (event.world_session_id != current_world_session_ ||
        event.event_type == motion::KickEventType::kNone) {
        // Old session/placement event: audit only, never a new status pulse.
        ++execution_old_generation_;
        last_execution_feedback_ = event;
        return;
    }
    pulse_event_type_ = event.event_type;
    pulse_start_us_ = now_us;
    flat_kick_ms_ = event.event_type == motion::KickEventType::kFlat ? 1 : 0;
    chip_kick_ms_ = event.event_type == motion::KickEventType::kChip ? 1 : 0;
    ++execution_events_;
    last_execution_feedback_ = event;
}

void Cm4MotionBridge::pushPendingAck(const motion::KickExecutionAck& ack) {
    // Caller holds command_mutex_.  Identical ACKs already waiting are
    // idempotent; a full queue drops the OLDEST audit ACK (the execution event
    // keeps retransmitting, so Medusa will re-send it).
    for (std::size_t i = 0; i < pending_ack_count_; ++i) {
        const auto& queued = pending_acks_[i];
        if (queued.world_session_id == ack.world_session_id &&
            queued.producer_session_id == ack.producer_session_id &&
            queued.kick_sequence == ack.kick_sequence) {
            return;
        }
    }
    if (pending_ack_count_ >= pending_acks_.size()) {
        for (std::size_t i = 1; i < pending_ack_count_; ++i) {
            pending_acks_[i - 1] = pending_acks_[i];
        }
        --pending_ack_count_;
    }
    pending_acks_[pending_ack_count_++] = ack;
}

bool Cm4MotionBridge::popPendingAck(motion::KickExecutionAck* ack) {
    // Caller holds command_mutex_.
    if (pending_ack_count_ == 0) return false;
    *ack = pending_acks_[0];
    for (std::size_t i = 1; i < pending_ack_count_; ++i) {
        pending_acks_[i - 1] = pending_acks_[i];
    }
    --pending_ack_count_;
    pending_acks_[pending_ack_count_] = motion::KickExecutionAck{};
    return true;
}

motion::KickExecutionFeedback Cm4MotionBridge::lastExecutionFeedback() const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    return last_execution_feedback_;
}

std::uint32_t Cm4MotionBridge::executionFeedbackFrames() const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    return execution_feedback_frames_;
}

std::uint32_t Cm4MotionBridge::executionEvents() const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    return execution_events_;
}

std::uint32_t Cm4MotionBridge::executionDuplicates() const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    return execution_duplicates_;
}

std::uint32_t Cm4MotionBridge::executionOldGeneration() const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    return execution_old_generation_;
}

std::uint32_t Cm4MotionBridge::executionAcksForwarded() const {
    std::lock_guard<std::mutex> lock(command_mutex_);
    return execution_acks_forwarded_;
}

bool Cm4MotionBridge::exchange(SpiTransport& transport, motion::TimeUs now_us) {
    std::uint8_t tx[motion::kSpiFrameSize] = {0};
    std::uint8_t rx[motion::kSpiFrameSize] = {0};
    if (!buildFrame(now_us, tx)) {
        ++protocol_errors_;
        return false;
    }
    if (!transport.transfer(tx, rx, motion::kSpiFrameSize)) {
        noteSpiTimeout(now_us);
        return false;
    }
    if (!processFeedbackFrame(rx, now_us)) {
        noteSpiTimeout(now_us);
        return false;
    }
    return true;
}

void Cm4MotionBridge::noteSpiTimeout(motion::TimeUs now_us) {
    {
        std::lock_guard<std::mutex> lock(feedback_mutex_);
        if (!spi_timed_out_) ++spi_timeouts_;
        spi_timed_out_ = true;
        last_feedback_receive_us_ = now_us;
    }
}

void Cm4MotionBridge::applyFeedbackToStatus(ZSS::New::Robot_Status* status) const {
    if (status == nullptr) return;
    motion::MotionFeedbackV2 feedback;
    bool valid = false;
    bool match_enabled = false;
    std::int32_t flat_kick_ms = 0;
    std::int32_t chip_kick_ms = 0;
    {
        std::lock_guard<std::mutex> lock(feedback_mutex_);
        feedback = feedback_;
        valid = has_feedback_;
        match_enabled = match_execution_.enabled();
        flat_kick_ms = flat_kick_ms_;
        chip_kick_ms = chip_kick_ms_;
    }
    if (!valid) return;
    feedbackToProto(feedback, status->mutable_motion_feedback());
    if (match_enabled) {
        // Status pulses are only triggered by the first `executed` observation
        // and never extended by a retransmitted feedback frame.
        status->set_flat_kick(flat_kick_ms);
        status->set_chip_kick(chip_kick_ms);
    }
    if (feedback.battery_voltage > 0.0f) {
        status->set_battery(static_cast<std::int32_t>(feedback.battery_voltage * 10.0f + 0.5f));
    }
    if (feedback.capacitance_voltage > 0.0f) {
        status->set_capacitance(
            static_cast<std::int32_t>(feedback.capacitance_voltage * 10.0f + 0.5f));
    }
    status->set_infrared(static_cast<std::int32_t>(feedback.infrared_age_ms));
    status->set_have_imu(feedback.have_imu);
    if (feedback.vision_fused) {
        constexpr float kRadToDeg = 57.29577951308232f;
        status->set_imu_theta_z(static_cast<std::int32_t>(
            std::fmod(feedback.estimated_pose.yaw * kRadToDeg, 360.0f) * 100.0f));
        status->set_imu_omega_z(static_cast<std::int32_t>(
            feedback.estimated_velocity.angular * kRadToDeg * 100.0f));
    }
    if (status->wheel_encoder_size() == 0) {
        for (int i = 0; i < 4; ++i) {
            status->add_wheel_encoder(feedback.wheel_encoder[i]);
        }
    }
}

motion::MotionCommandV2 Cm4MotionBridge::lastCommand() const {
    std::lock_guard<std::mutex> lock(command_mutex_);
    return command_;
}

motion::MotionFeedbackV2 Cm4MotionBridge::lastFeedback() const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    return feedback_;
}

bool Cm4MotionBridge::hasCommand() const {
    std::lock_guard<std::mutex> lock(command_mutex_);
    return has_command_;
}

bool Cm4MotionBridge::feedbackFresh(motion::TimeUs now_us) const {
    std::lock_guard<std::mutex> lock(feedback_mutex_);
    if (!has_feedback_ || spi_timed_out_) return false;
    return now_us >= last_feedback_receive_us_ &&
           now_us - last_feedback_receive_us_ <=
               motion::secondsToUs(config_.spi_timeout_s);
}

std::uint32_t Cm4MotionBridge::commandsSubmitted() const { return commands_submitted_; }
std::uint32_t Cm4MotionBridge::protocolErrors() const { return protocol_errors_; }
std::uint32_t Cm4MotionBridge::spiTimeouts() const { return spi_timeouts_; }
std::uint32_t Cm4MotionBridge::visionAttached() const { return vision_attached_; }
std::uint32_t Cm4MotionBridge::feedbackFrames() const { return feedback_frames_; }

}  // namespace robot_cm4
