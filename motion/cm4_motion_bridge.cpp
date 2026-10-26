#include <cmath>
#include "cm4_motion_bridge.hpp"

#include "motion_proto.hpp"
#include "motion/validation.hpp"

#include <cstring>

namespace robot_cm4 {
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
}

bool Cm4MotionBridge::submitRobotCommand(const ZSS::New::Robot_Command& proto,
                                         motion::TimeUs receive_us) {
    std::lock_guard<std::mutex> lock(command_mutex_);
    motion::MotionCommandV2 command;
    if (!proto.has_motion_command()) return false;
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
    if (!config_sent_) return buildConfigFrame(now_us, frame);
    return buildCommandFrame(now_us, frame);
}

bool Cm4MotionBridge::processFeedbackFrame(const std::uint8_t* frame,
                                           motion::TimeUs now_us) {
    motion::MotionFeedbackV2 feedback;
    if (!motion::decodeFeedbackFrame(frame, &feedback)) {
        ++protocol_errors_;
        return false;
    }
    std::lock_guard<std::mutex> lock(feedback_mutex_);
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
    }
    return true;
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
    {
        std::lock_guard<std::mutex> lock(feedback_mutex_);
        feedback = feedback_;
        valid = has_feedback_;
    }
    if (!valid) return;
    feedbackToProto(feedback, status->mutable_motion_feedback());
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
