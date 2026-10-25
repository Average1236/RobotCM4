#ifndef ROBOT_CM4_MOTION_PROTO_HPP
#define ROBOT_CM4_MOTION_PROTO_HPP

// Wire conversion between the shared motion types and ZSS::New protocol v2.
// Used by the CM4 production bridge, the host integration runner and Kun2.

#include "motion/types.hpp"
#include "zss_cmd.pb.h"

namespace robot_cm4 {

inline void limitsToProto(const motion::CommandLimits& limits,
                          ZSS::New::MotionLimitsV2* proto) {
    proto->set_max_linear_speed_m_s(limits.max_linear_speed_m_s);
    proto->set_max_linear_accel_m_s2(limits.max_linear_accel_m_s2);
    proto->set_max_linear_decel_m_s2(limits.max_linear_decel_m_s2);
    proto->set_max_yaw_speed_rad_s(limits.max_yaw_speed_rad_s);
    proto->set_max_yaw_accel_rad_s2(limits.max_yaw_accel_rad_s2);
    proto->set_max_centripetal_accel_m_s2(limits.max_centripetal_accel_m_s2);
    proto->set_max_wheel_torque_nm(limits.max_wheel_torque_nm);
    proto->set_max_torque_rate_nm_per_s(limits.max_torque_rate_nm_per_s);
    proto->set_position_correction_gain(limits.position_correction_gain);
    proto->set_position_correction_limit_m_s(limits.position_correction_limit_m_s);
    proto->set_yaw_correction_gain(limits.yaw_correction_gain);
    proto->set_yaw_rate_gain(limits.yaw_rate_gain);
    proto->set_wheel_speed_kp(limits.wheel_speed_kp);
    proto->set_wheel_speed_ki(limits.wheel_speed_ki);
    proto->set_wheel_speed_backcalc(limits.wheel_speed_backcalc);
}

inline motion::CommandLimits limitsFromProto(const ZSS::New::MotionLimitsV2& proto,
                                             motion::CommandLimits fallback = {}) {
    motion::CommandLimits limits = fallback;
    if (proto.max_linear_speed_m_s() > 0.0f) {
        limits.max_linear_speed_m_s = proto.max_linear_speed_m_s();
    }
    if (proto.max_linear_accel_m_s2() > 0.0f) {
        limits.max_linear_accel_m_s2 = proto.max_linear_accel_m_s2();
    }
    if (proto.max_linear_decel_m_s2() > 0.0f) {
        limits.max_linear_decel_m_s2 = proto.max_linear_decel_m_s2();
    }
    if (proto.max_yaw_speed_rad_s() > 0.0f) {
        limits.max_yaw_speed_rad_s = proto.max_yaw_speed_rad_s();
    }
    if (proto.max_yaw_accel_rad_s2() > 0.0f) {
        limits.max_yaw_accel_rad_s2 = proto.max_yaw_accel_rad_s2();
    }
    if (proto.max_centripetal_accel_m_s2() > 0.0f) {
        limits.max_centripetal_accel_m_s2 = proto.max_centripetal_accel_m_s2();
    }
    if (proto.max_wheel_torque_nm() > 0.0f) {
        limits.max_wheel_torque_nm = proto.max_wheel_torque_nm();
    }
    if (proto.max_torque_rate_nm_per_s() > 0.0f) {
        limits.max_torque_rate_nm_per_s = proto.max_torque_rate_nm_per_s();
    }
    if (proto.position_correction_gain() > 0.0f) {
        limits.position_correction_gain = proto.position_correction_gain();
    }
    if (proto.position_correction_limit_m_s() > 0.0f) {
        limits.position_correction_limit_m_s = proto.position_correction_limit_m_s();
    }
    if (proto.yaw_correction_gain() > 0.0f) {
        limits.yaw_correction_gain = proto.yaw_correction_gain();
    }
    if (proto.yaw_rate_gain() >= 0.0f) limits.yaw_rate_gain = proto.yaw_rate_gain();
    if (proto.wheel_speed_kp() > 0.0f) limits.wheel_speed_kp = proto.wheel_speed_kp();
    if (proto.wheel_speed_ki() > 0.0f) limits.wheel_speed_ki = proto.wheel_speed_ki();
    if (proto.wheel_speed_backcalc() > 0.0f) {
        limits.wheel_speed_backcalc = proto.wheel_speed_backcalc();
    }
    return limits;
}

inline void commandToProto(const motion::MotionCommandV2& command,
                           ZSS::New::MotionCommandV2* proto) {
    proto->set_backend(command.backend == motion::Backend::kLegacy
                           ? ZSS::New::MOTION_BACKEND_LEGACY
                           : ZSS::New::MOTION_BACKEND_NEW);
    proto->set_translation_mode(static_cast<ZSS::New::MotionTranslationMode>(
        static_cast<int>(command.translation_mode)));
    proto->set_yaw_mode(
        static_cast<ZSS::New::MotionYawMode>(static_cast<int>(command.yaw_mode)));
    proto->set_session_id(command.session_id);
    proto->set_sequence(command.sequence);
    proto->set_trajectory_epoch(command.trajectory_epoch);
    proto->set_generated_time_us(command.generated_time_us);
    proto->set_valid_for_us(static_cast<std::uint32_t>(command.valid_for_us));
    auto* pose = proto->mutable_target_pose();
    pose->set_x(command.target_pose.position.x);
    pose->set_y(command.target_pose.position.y);
    pose->set_yaw(command.target_pose.yaw);
    auto* velocity = proto->mutable_target_velocity();
    velocity->set_vx(command.target_velocity.linear.x);
    velocity->set_vy(command.target_velocity.linear.y);
    velocity->set_wz(command.target_velocity.angular);
    limitsToProto(command.limits, proto->mutable_limits());
    proto->set_enable(command.enable);
    proto->set_kick_requested(command.kick_requested);
    proto->set_chip_mode(command.chip_mode);
    proto->set_kick_discharge_time_us(command.kick_discharge_time_us);
    proto->set_kick_sequence(command.kick_sequence);
    proto->set_kick_valid_for_us(command.kick_valid_for_us);
    proto->set_dribble_mode(command.dribble_mode);
    proto->set_dribble_power(command.dribble_power);
    proto->set_dribble_velocity_rps(command.dribble_velocity_rps);
    proto->set_dribble_torque_ff_nm(command.dribble_torque_ff_nm);
}

inline motion::MotionCommandV2 commandFromProto(const ZSS::New::MotionCommandV2& proto) {
    motion::MotionCommandV2 command;
    command.backend = proto.backend() == ZSS::New::MOTION_BACKEND_LEGACY
                          ? motion::Backend::kLegacy
                          : motion::Backend::kNew;
    command.translation_mode =
        static_cast<motion::TranslationMode>(proto.translation_mode());
    command.yaw_mode = static_cast<motion::YawMode>(proto.yaw_mode());
    command.session_id = proto.session_id();
    command.sequence = proto.sequence();
    command.trajectory_epoch = proto.trajectory_epoch();
    command.generated_time_us = proto.generated_time_us();
    command.valid_for_us = proto.valid_for_us();
    if (command.valid_for_us == 0) command.valid_for_us = 100000;
    command.target_pose.position.x = proto.target_pose().x();
    command.target_pose.position.y = proto.target_pose().y();
    command.target_pose.yaw = proto.target_pose().yaw();
    command.target_velocity.linear.x = proto.target_velocity().vx();
    command.target_velocity.linear.y = proto.target_velocity().vy();
    command.target_velocity.angular = proto.target_velocity().wz();
    command.limits = limitsFromProto(proto.limits());
    command.enable = proto.enable();
    command.kick_requested = proto.kick_requested();
    command.chip_mode = proto.chip_mode();
    command.kick_discharge_time_us = proto.kick_discharge_time_us();
    command.kick_sequence = proto.kick_sequence();
    command.kick_valid_for_us = proto.kick_valid_for_us();
    command.dribble_mode = proto.dribble_mode();
    command.dribble_power = proto.dribble_power();
    command.dribble_velocity_rps = proto.dribble_velocity_rps();
    command.dribble_torque_ff_nm = proto.dribble_torque_ff_nm();
    return command;
}

inline void visionToProto(const motion::VisionObservationV2& vision,
                          ZSS::New::VisionObservationV2* proto) {
    proto->set_capture_time_us(vision.capture_time_us);
    proto->set_sequence(vision.sequence);
    auto* pose = proto->mutable_pose();
    pose->set_x(vision.pose.position.x);
    pose->set_y(vision.pose.position.y);
    pose->set_yaw(vision.pose.yaw);
    proto->set_position_std_m(vision.position_std_m);
    proto->set_yaw_std_rad(vision.yaw_std_rad);
    proto->set_quality(static_cast<ZSS::New::MotionLocalizationQuality>(
        static_cast<int>(vision.quality)));
    proto->set_valid(vision.valid);
}

inline motion::VisionObservationV2 visionFromProto(
    const ZSS::New::VisionObservationV2& proto) {
    motion::VisionObservationV2 vision;
    vision.capture_time_us = proto.capture_time_us();
    vision.sequence = proto.sequence();
    vision.pose.position.x = proto.pose().x();
    vision.pose.position.y = proto.pose().y();
    vision.pose.yaw = proto.pose().yaw();
    vision.position_std_m = proto.position_std_m();
    vision.yaw_std_rad = proto.yaw_std_rad();
    vision.quality = static_cast<motion::LocalizationQuality>(proto.quality());
    vision.valid = proto.valid();
    return vision;
}

inline void configToProto(const motion::MotionConfigV2& config,
                          ZSS::New::MotionConfigV2* proto) {
    proto->set_mass_kg(config.robot.mass_kg);
    proto->set_inertia_kg_m2(config.robot.inertia_kg_m2);
    proto->set_wheel_radius_m(config.robot.wheel_radius_m);
    proto->set_wheel_center_distance_m(config.robot.wheel_center_distance_m);
    proto->set_wheel_inertia_kg_m2(config.robot.wheel_inertia_kg_m2);
    proto->set_wheel_friction_torque_nm(config.robot.wheel_friction_torque_nm);
    proto->set_wheel_viscous_nm_per_rad_s(config.robot.wheel_viscous_nm_per_rad_s);
    proto->set_efficiency(config.robot.efficiency);
    proto->set_max_wheel_torque_nm(config.robot.max_wheel_torque_nm);
    proto->set_max_wheel_speed_rad_s(config.robot.max_wheel_speed_rad_s);
    proto->set_max_wheel_accel_rad_s2(config.robot.max_wheel_accel_rad_s2);
    proto->set_torque_rate_nm_per_s(config.robot.torque_rate_nm_per_s);
    for (std::size_t i = 0; i < motion::kWheelCount; ++i) {
        proto->add_wheel_axis_angle_rad(config.robot.wheels[i].axis_angle_rad);
        proto->add_wheel_yaw_sign(config.robot.wheels[i].yaw_sign);
    }
    limitsToProto(config.limits, proto->mutable_limits());
    proto->set_fusion_delay_s(config.fusion_delay_s);
    proto->set_history_span_s(config.history_span_s);
    proto->set_command_timeout_s(config.command_timeout_s);
    proto->set_spi_timeout_s(config.spi_timeout_s);
    proto->set_vision_degrade_s(config.vision_degrade_s);
    proto->set_vision_invalid_s(config.vision_invalid_s);
    proto->set_config_version(config.config_version);
}

inline motion::MotionConfigV2 configFromProto(const ZSS::New::MotionConfigV2& proto) {
    motion::MotionConfigV2 config;
    config.robot.mass_kg = proto.mass_kg();
    config.robot.inertia_kg_m2 = proto.inertia_kg_m2();
    config.robot.wheel_radius_m = proto.wheel_radius_m();
    config.robot.wheel_center_distance_m = proto.wheel_center_distance_m();
    config.robot.wheel_inertia_kg_m2 = proto.wheel_inertia_kg_m2();
    config.robot.wheel_friction_torque_nm = proto.wheel_friction_torque_nm();
    config.robot.wheel_viscous_nm_per_rad_s = proto.wheel_viscous_nm_per_rad_s();
    config.robot.efficiency = proto.efficiency();
    config.robot.max_wheel_torque_nm = proto.max_wheel_torque_nm();
    config.robot.max_wheel_speed_rad_s = proto.max_wheel_speed_rad_s();
    config.robot.max_wheel_accel_rad_s2 = proto.max_wheel_accel_rad_s2();
    config.robot.torque_rate_nm_per_s = proto.torque_rate_nm_per_s();
    for (int i = 0; i < proto.wheel_axis_angle_rad_size() && i < 4; ++i) {
        config.robot.wheels[i].axis_angle_rad = proto.wheel_axis_angle_rad(i);
        config.robot.wheels[i].rotation_radius_m = config.robot.wheel_center_distance_m;
    }
    for (int i = 0; i < proto.wheel_yaw_sign_size() && i < 4; ++i) {
        config.robot.wheels[i].yaw_sign = proto.wheel_yaw_sign(i);
    }
    config.limits = limitsFromProto(proto.limits());
    config.fusion_delay_s = proto.fusion_delay_s();
    config.history_span_s = proto.history_span_s();
    config.command_timeout_s = proto.command_timeout_s();
    config.spi_timeout_s = proto.spi_timeout_s();
    config.vision_degrade_s = proto.vision_degrade_s();
    config.vision_invalid_s = proto.vision_invalid_s();
    config.config_version = proto.config_version();
    return config;
}

inline void feedbackToProto(const motion::MotionFeedbackV2& feedback,
                            ZSS::New::MotionFeedbackV2* proto) {
    proto->set_backend(feedback.backend == motion::Backend::kLegacy
                           ? ZSS::New::MOTION_BACKEND_LEGACY
                           : ZSS::New::MOTION_BACKEND_NEW);
    proto->set_safety(static_cast<ZSS::New::MotionSafetyState>(
        static_cast<int>(feedback.safety)));
    proto->set_localization(static_cast<ZSS::New::MotionLocalizationQuality>(
        static_cast<int>(feedback.localization)));
    proto->set_session_id(feedback.session_id);
    proto->set_executed_sequence(feedback.executed_sequence);
    proto->set_trajectory_epoch(feedback.trajectory_epoch);
    proto->set_active_command_age_us(feedback.active_command_age_us);
    proto->set_fusion_time_us(feedback.fusion_time_us);
    proto->set_prediction_time_us(feedback.prediction_time_us);
    proto->set_feedback_time_us(feedback.feedback_time_us);
    proto->set_vision_capture_time_us(feedback.vision_capture_time_us);
    proto->set_vision_age_us(static_cast<std::uint32_t>(feedback.vision_age_us));
    auto* pose = proto->mutable_estimated_pose();
    pose->set_x(feedback.estimated_pose.position.x);
    pose->set_y(feedback.estimated_pose.position.y);
    pose->set_yaw(feedback.estimated_pose.yaw);
    auto* velocity = proto->mutable_estimated_velocity();
    velocity->set_vx(feedback.estimated_velocity.linear.x);
    velocity->set_vy(feedback.estimated_velocity.linear.y);
    velocity->set_wz(feedback.estimated_velocity.angular);
    proto->set_position_sigma_m(feedback.position_sigma_m);
    proto->set_yaw_sigma_rad(feedback.yaw_sigma_rad);
    auto* ref_pose = proto->mutable_reference_pose();
    ref_pose->set_x(feedback.reference_pose.position.x);
    ref_pose->set_y(feedback.reference_pose.position.y);
    ref_pose->set_yaw(feedback.reference_pose.yaw);
    auto* ref_velocity = proto->mutable_reference_velocity();
    ref_velocity->set_vx(feedback.reference_velocity.linear.x);
    ref_velocity->set_vy(feedback.reference_velocity.linear.y);
    ref_velocity->set_wz(feedback.reference_velocity.angular);
    auto* ref_accel = proto->mutable_reference_accel();
    ref_accel->set_ax(feedback.reference_accel.linear.x);
    ref_accel->set_ay(feedback.reference_accel.linear.y);
    ref_accel->set_alpha(feedback.reference_accel.angular);
    proto->set_trajectory_active(feedback.trajectory_active);
    proto->set_gyro_bias_z_rad_s(feedback.gyro_bias_z_rad_s);
    proto->set_accel_bias_x_m_s2(feedback.accel_bias_x_m_s2);
    proto->set_accel_bias_y_m_s2(feedback.accel_bias_y_m_s2);
    for (std::size_t i = 0; i < motion::kWheelCount; ++i) {
        proto->add_wheel_speed_target(feedback.wheel_speed_target.rad_s[i]);
        proto->add_wheel_speed_measured(feedback.wheel_speed_measured.rad_s[i]);
        proto->add_feedforward_torque(feedback.feedforward_torque.nm[i]);
        proto->add_feedback_torque(feedback.feedback_torque.nm[i]);
        proto->add_integral_torque(feedback.integral_torque.nm[i]);
        proto->add_applied_torque(feedback.applied_torque.nm[i]);
    }
    proto->set_saturation(feedback.saturation);
    proto->set_wheel_speed_enabled(feedback.wheel_speed_enabled);
    proto->set_vision_fused(feedback.vision_fused);
    proto->set_accelerometer_degraded(feedback.accelerometer_degraded);
    proto->set_wheel_slip_suspected(feedback.wheel_slip_suspected);
    proto->set_late_observations(feedback.late_observations);
    proto->set_rejected_observations(feedback.rejected_observations);
    proto->set_vision_updates(feedback.vision_updates);
    proto->set_estimator_version(feedback.estimator_version);
    proto->set_controller_version(feedback.controller_version);
    proto->set_model_version(feedback.model_version);
    proto->set_protocol_version(feedback.protocol_version);
}

inline motion::MotionFeedbackV2 feedbackFromProto(
    const ZSS::New::MotionFeedbackV2& proto) {
    motion::MotionFeedbackV2 feedback;
    feedback.backend = proto.backend() == ZSS::New::MOTION_BACKEND_LEGACY
                           ? motion::Backend::kLegacy
                           : motion::Backend::kNew;
    feedback.safety = static_cast<motion::SafetyState>(proto.safety());
    feedback.localization =
        static_cast<motion::LocalizationQuality>(proto.localization());
    feedback.session_id = proto.session_id();
    feedback.executed_sequence = proto.executed_sequence();
    feedback.trajectory_epoch = proto.trajectory_epoch();
    feedback.active_command_age_us = proto.active_command_age_us();
    feedback.fusion_time_us = proto.fusion_time_us();
    feedback.prediction_time_us = proto.prediction_time_us();
    feedback.feedback_time_us = proto.feedback_time_us();
    feedback.vision_capture_time_us = proto.vision_capture_time_us();
    feedback.vision_age_us = proto.vision_age_us();
    feedback.estimated_pose.position.x = proto.estimated_pose().x();
    feedback.estimated_pose.position.y = proto.estimated_pose().y();
    feedback.estimated_pose.yaw = proto.estimated_pose().yaw();
    feedback.estimated_velocity.linear.x = proto.estimated_velocity().vx();
    feedback.estimated_velocity.linear.y = proto.estimated_velocity().vy();
    feedback.estimated_velocity.angular = proto.estimated_velocity().wz();
    feedback.position_sigma_m = proto.position_sigma_m();
    feedback.yaw_sigma_rad = proto.yaw_sigma_rad();
    feedback.reference_pose.position.x = proto.reference_pose().x();
    feedback.reference_pose.position.y = proto.reference_pose().y();
    feedback.reference_pose.yaw = proto.reference_pose().yaw();
    feedback.reference_velocity.linear.x = proto.reference_velocity().vx();
    feedback.reference_velocity.linear.y = proto.reference_velocity().vy();
    feedback.reference_velocity.angular = proto.reference_velocity().wz();
    feedback.reference_accel.linear.x = proto.reference_accel().ax();
    feedback.reference_accel.linear.y = proto.reference_accel().ay();
    feedback.reference_accel.angular = proto.reference_accel().alpha();
    feedback.trajectory_active = proto.trajectory_active();
    feedback.gyro_bias_z_rad_s = proto.gyro_bias_z_rad_s();
    feedback.accel_bias_x_m_s2 = proto.accel_bias_x_m_s2();
    feedback.accel_bias_y_m_s2 = proto.accel_bias_y_m_s2();
    for (int i = 0; i < proto.wheel_speed_target_size() && i < 4; ++i) {
        feedback.wheel_speed_target.rad_s[i] = proto.wheel_speed_target(i);
    }
    for (int i = 0; i < proto.wheel_speed_measured_size() && i < 4; ++i) {
        feedback.wheel_speed_measured.rad_s[i] = proto.wheel_speed_measured(i);
    }
    for (int i = 0; i < proto.feedforward_torque_size() && i < 4; ++i) {
        feedback.feedforward_torque.nm[i] = proto.feedforward_torque(i);
    }
    for (int i = 0; i < proto.feedback_torque_size() && i < 4; ++i) {
        feedback.feedback_torque.nm[i] = proto.feedback_torque(i);
    }
    for (int i = 0; i < proto.integral_torque_size() && i < 4; ++i) {
        feedback.integral_torque.nm[i] = proto.integral_torque(i);
    }
    for (int i = 0; i < proto.applied_torque_size() && i < 4; ++i) {
        feedback.applied_torque.nm[i] = proto.applied_torque(i);
    }
    feedback.saturation = static_cast<std::uint16_t>(proto.saturation());
    feedback.wheel_speed_enabled = proto.wheel_speed_enabled();
    feedback.vision_fused = proto.vision_fused();
    feedback.accelerometer_degraded = proto.accelerometer_degraded();
    feedback.wheel_slip_suspected = proto.wheel_slip_suspected();
    feedback.late_observations = proto.late_observations();
    feedback.rejected_observations = proto.rejected_observations();
    feedback.vision_updates = proto.vision_updates();
    feedback.estimator_version = static_cast<std::uint16_t>(proto.estimator_version());
    feedback.controller_version = static_cast<std::uint16_t>(proto.controller_version());
    feedback.model_version = static_cast<std::uint16_t>(proto.model_version());
    feedback.protocol_version = static_cast<std::uint16_t>(proto.protocol_version());
    return feedback;
}

// Old commands (use_imu / velocity_r / cmd_pose) are adapted once, explicitly,
// into the v2 mode vocabulary.  The same numeric field is no longer treated
// as an angle in one path and an angular velocity in another.
inline motion::MotionCommandV2 adaptLegacyCommand(
    const ZSS::New::Robot_Command& proto, const motion::CommandLimits& defaults,
    motion::TimeUs receive_time_us, std::uint32_t session_id) {
    motion::MotionCommandV2 command;
    command.session_id = session_id;
    command.sequence = 0;
    command.generated_time_us = receive_time_us;
    command.valid_for_us = 100000;
    command.limits = defaults;
    command.enable = true;
    command.backend = motion::Backend::kNew;

    const std::int32_t acc_x = proto.xy_max_acc_x();
    const std::int32_t acc_y = proto.xy_max_acc_y();
    if (acc_x > 0) command.limits.max_linear_accel_m_s2 = acc_x / 1000.0f;
    if (acc_y > 0) command.limits.max_linear_accel_m_s2 =
        std::fmin(command.limits.max_linear_accel_m_s2, acc_y / 1000.0f);
    const std::int32_t dec_x = proto.xy_max_dec_x();
    if (dec_x > 0) command.limits.max_linear_decel_m_s2 = dec_x / 1000.0f;
    if (proto.yaw_max_vel() > 0) {
        command.limits.max_yaw_speed_rad_s = proto.yaw_max_vel() / 1000.0f;
    }
    if (proto.yaw_max_acc() > 0) {
        command.limits.max_yaw_accel_rad_s2 = proto.yaw_max_acc() / 1000.0f;
    }

    const auto type = proto.cmd_type();
    if (type == ZSS::New::Robot_Command_CmdType_CMD_POSE) {
        command.translation_mode = motion::TranslationMode::kPose;
        command.yaw_mode = motion::YawMode::kPose;
        command.target_pose.position.x = proto.cmd_pose().target().x() * 0.001f;
        command.target_pose.position.y = proto.cmd_pose().target().y() * 0.001f;
        command.target_pose.yaw = proto.cmd_pose().target().w();
        command.target_velocity.linear.x = proto.cmd_pose().target_v().x() * 0.001f;
        command.target_velocity.linear.y = proto.cmd_pose().target_v().y() * 0.001f;
        command.target_velocity.angular = proto.cmd_pose().target_v().w();
        if (proto.cmd_pose().has_config()) {
            const auto& config = proto.cmd_pose().config();
            if (config.max_vx() > 0) command.limits.max_linear_speed_m_s = config.max_vx() * 0.001f;
            if (config.max_ax() > 0) command.limits.max_linear_accel_m_s2 = config.max_ax() * 0.001f;
            if (config.max_aw() > 0) command.limits.max_yaw_accel_rad_s2 = config.max_aw();
        }
    } else if (type == ZSS::New::Robot_Command_CmdType_CMD_VEL) {
        command.translation_mode = motion::TranslationMode::kBodyVelocity;
        command.target_velocity.linear.x = proto.cmd_vel().velocity_x() / 1000.0f;
        command.target_velocity.linear.y = proto.cmd_vel().velocity_y() / 1000.0f;
        if (proto.cmd_vel().use_imu()) {
            command.yaw_mode = motion::YawMode::kPose;
            command.target_pose.yaw = proto.cmd_vel().imu_theta() / 1000.0f;
        } else {
            command.yaw_mode = motion::YawMode::kAngularVelocity;
            command.target_velocity.angular = proto.cmd_vel().velocity_r() / 1000.0f;
        }
    } else if (type == ZSS::New::Robot_Command_CmdType_STOP) {
        command.translation_mode = motion::TranslationMode::kStop;
        command.yaw_mode = motion::YawMode::kHold;
    } else {
        command.translation_mode = motion::TranslationMode::kStop;
        command.yaw_mode = motion::YawMode::kHold;
    }

    command.kick_requested =
        proto.kick_mode() != ZSS::New::Robot_Command_KickMode_NONE;
    command.chip_mode = proto.kick_mode() == ZSS::New::Robot_Command_KickMode_CHIP;
    command.kick_discharge_time_us = 0;
    if (proto.kick_discharge_time() > 0) {
        command.kick_discharge_time_us =
            static_cast<std::uint16_t>(proto.kick_discharge_time());
    }
    command.kick_sequence = 0;
    command.kick_valid_for_us = 100000;
    command.dribble_mode = static_cast<std::uint8_t>(proto.dribble_mode());
    command.dribble_power = proto.dribble_spin() / 100.0f;
    command.dribble_velocity_rps = proto.dribble_velocity() / 100.0f;
    command.dribble_torque_ff_nm = proto.dribble_torque_ff() / 1000.0f;
    return command;
}

}  // namespace robot_cm4

#endif  // ROBOT_CM4_MOTION_PROTO_HPP
