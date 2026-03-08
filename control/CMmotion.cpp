#include <iostream>
#include <math.h>
#include <stdio.h>
#include <CMmotion.h>
#include <fstream>


using namespace std;

namespace {
const double FRAME_PERIOD = 1.0 / 250;
const double FRAME_RATE = 250;
const double PI = 3.14159265358979323846;
bool DEBUG_NO_ZERO_VEL = false;
const double DEC_FACTOR = 2.0;
const double lowerBoundSpeedLimitRotate = 0;
const double upperBoundSpeedLimitRotate = 300*10;
const double min_max_angle_acc = 10.0;
const double min_max_angle_speed = 6.0;
const double rotateScaleFactor = 1.0;
const double minAngleLimitRotateSpeed = 15.0 / 180.0 * PI;

// TimeConstant
//double TimeConstantAcc = ZSS::ZParamManager::instance()->value("CGotoPositionV2/AccTimeConstant",QVariant(1.0)).toDouble();
//double TimeConstantflat = ZSS::ZParamManager::instance()->value("CGotoPositionV2/FlatTimeConstant",QVariant(1.0)).toDouble();
//double TimeConstantDec = ZSS::ZParamManager::instance()->value("CGotoPositionV2/DecTimeConstant",QVariant(1.0)).toDouble();

int timeItor = 0;
int isX = 1;
}
double Normalize(double angle)
{
        if (fabs(angle) > 10 ){
            cout<<angle<<" Normalize Error!!!!!!!!!!!!!!!!!!!!"<<endl;
            return 0;
        }
        static const double M_2PI = PI * 2;
        // 快速粗调整
        angle -= (int)(angle / M_2PI) * M_2PI;

        // 细调整 (-PI,PI]
        while( angle > PI ) {
            angle -= M_2PI;
        }

        while( angle <= -PI ) {
            angle += M_2PI;
        }

        return angle;
}
CVector Polar2Vector(double m,double angle)
{
    return CVector(m*std::cos(angle),m*std::sin(angle));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void compute_motion_1d(double x0, double v0, double v1, double a_max, double v_max,
/// 		double a_factor, double &traj_accel, double &traj_time)
///
/// @brief	一维状态下计算运动过程，给定初始速度和末速度，最大加速度和最大速度，返回加速时间和过程时间
///
/// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
/// @date		2010-4-2
///
/// @param	x0				初始位置
/// @param	v0				初速度
/// @param	v1				末速度
/// @param	a_max			最大加速度
/// @param	v_max			最大速度
/// @param	a_factor		常数，影响加速度的因子
/// @param	traj_accel		计算得加速度
/// @param	traj_time		计算得加速时间
////////////////////////////////////////////////////////////////////////////////////////////////////



void compute_motion_1d(double x0, double v0, double v1,
                       double a_max, double d_max, double v_max, double a_factor, double vel_factor,
                       double &traj_accel, double &traj_time, double &traj_time_acc, double &traj_time_dec, double &traj_time_flat, planType pT, nonZeroMode mode) {
    if (x0 == 0. && v0 == v1) {
        traj_accel = 0;
        return;
    }

    if(!finite(x0) || !finite(v0) || !finite(v1)) {
        traj_accel = 0;
        return;
    }

    //v 同向且v1>v0 x方向 ， 加速度拉最大
    if(pT == MOVE_X && fabs(v1) > 1e-8
            && fabs(v0) < fabs(v1) && v0 * v1 > 0) {
        traj_accel = copysign(a_max, v1);
        return;
    }


    double decFactor = (pT == ROTATE ? 1.0 : DEC_FACTOR);
    a_max /= (1.0 * a_factor);
    d_max /= (1.0 * a_factor);

    double accel_time_to_v1 = fabs(v1 - v0) / a_max;                                                  // 最大加速度加速到末速度的时间
    double accel_dist_to_v1 = fabs((v1 + v0) / 2.0) * accel_time_to_v1;                               // 单一加速到末速度时的位移
    double decel_time_to_v1 = fabs(v0 - v1) / d_max;                                                  // 最大减速度减速到末速度的时间
    double decel_dist_to_v1 = fabs((v0 + v1) / 2.0) * decel_time_to_v1;                               // 单一减速到末速度时的位移

    // 这个时间很关键，设得较大则定位精度将大大降低 by qxz
    double period; // 一段很小的时间，处理运动到目标点附近时加速度，稳定到点，防止超调
    if(pT == MOVE_X)
        period = 1 / 50.0;
    else if(pT == MOVE_Y)
        period = 1.0 / 80.0;
    else
        period = 1 / 40.0;



    // 计算时间部分
    // 需要先后退再加速非零速到点
    if (fabs(v1) > 1e-8 && v1 * x0 > 0 && mode == ACCURATE) {
        double time_to_accel = fabs(v1) / a_max;
        double x_to_accel = (v1 * v1) / (2.0 * a_max);
        compute_motion_1d(x0 + copysign(x_to_accel, v1), v0, 0, a_max * a_factor, d_max *  a_factor,
                          v_max, a_factor, vel_factor, traj_accel, traj_time, traj_time_acc, traj_time_dec, traj_time_flat, pT, mode);
        traj_time += time_to_accel;
       
        return;
    }
    // 从x0运动到零点
    // 初速和目标点反向 或 初速大于目标速来不及减速到目标速
    // 全程减速
    else if (v0 * x0 > 0 || (fabs(v0) > fabs(v1) && decel_dist_to_v1 > fabs(x0))) {
        // 停下后到达的时间 + 停下所用时间
        double time_to_stop = fabs(v0) / (d_max);                                                       // 停下时间
        double x_to_stop = v0 * v0 / (2.0 * d_max);                                                   // 停止时运动距离
        double time_to_accel = fabs(v1) / a_max;
        double x_to_accel = (v1 * v1) / (2.0 * a_max);

        if (fabs(v1) > 1e-8 && mode == ACCURATE) {
            compute_motion_1d(x0 + copysign(x_to_accel, v1), v0, 0, a_max * a_factor, d_max * a_factor,
                              v_max, a_factor, vel_factor, traj_accel, traj_time, traj_time_acc, traj_time_dec, traj_time_flat, pT, mode);
            traj_time += time_to_accel;
            return;
        }
        else {
            compute_motion_1d(x0 + copysign(x_to_stop, v0), 0, v1, a_max * a_factor, d_max * a_factor,
                              v_max, a_factor, vel_factor, traj_accel, traj_time, traj_time_acc, traj_time_dec, traj_time_flat, pT, mode);    // 递归运算直到跳出这一条件
            traj_time += time_to_stop;                                                                    // 加上路径规划时间
            traj_time_dec += time_to_stop;
            // 减速
            if (time_to_stop < period) {
                if(pT == MOVE_Y)
                    traj_accel = compute_stop(v0, a_max);
                else{
                    //qDebug()<<"time to stop"<<time_to_stop;
                    traj_accel = time_to_stop / period * (- copysign(d_max * a_factor, v0)) + (1.0 - time_to_stop / period) * traj_accel;
                }
            }
            else {
                traj_accel = - copysign(decFactor * d_max * a_factor, v0);
            }
            return;
        }
    }

    if (accel_dist_to_v1 > fabs(x0) && fabs(v0) < fabs(v1)) {
        traj_time_acc = (sqrt(2 * a_max * fabs(x0) + v0 * v0) - fabs(v0)) / a_max;
        traj_time_flat = 0;
        traj_time_dec = 0;
    } else if (decel_dist_to_v1 > fabs(x0) && fabs(v0) > fabs(v1)) {
        traj_time_acc = 0;
        traj_time_flat = 0;
        traj_time_dec = (fabs(v0) - sqrt(v0 * v0 - 2 * d_max * fabs(x0))) / d_max;
    } else {
        double v_max_dist = (v_max * v_max - v0 * v0) / (2 * a_max) + (v_max * v_max - v1 * v1) / (2 * d_max);
        if (v_max_dist > fabs(x0)) {
            double v_m = sqrt((2 * a_max * d_max * fabs(x0) + d_max * v0 * v0 + a_max * v1 * v1) / (a_max + d_max));
            traj_time_acc = (v_m - fabs(v0)) / a_max;
            traj_time_flat = 0;
            traj_time_dec = (v_m - fabs(v1)) / d_max;
        } else {
            traj_time_acc = (v_max - fabs(v0)) / a_max;
            traj_time_flat = (fabs(x0) - v_max_dist) / v_max;
            traj_time_dec = (v_max - fabs(v1)) / d_max;
        }
    }
    // 分配加速度部分

    double a_to_v1_at_x0 = fabs(v0 * v0 - v1 * v1) / (2 * fabs(x0));
    double t_to_v1_at_x0 = (-fabs(v0) + sqrt(v0 * v0 + 2 * fabs(a_to_v1_at_x0) * fabs(x0))) / fabs(a_to_v1_at_x0);
    if (t_to_v1_at_x0 < period && a_to_v1_at_x0 < a_max) {
        traj_accel = - copysign(a_to_v1_at_x0, v0);
        traj_time += t_to_v1_at_x0;
        return;
    }

    if (FRAME_PERIOD * a_max + fabs(v0) > v_max && traj_time_flat > period ) {                           // 匀速运动阶段
        traj_time += traj_time_acc + traj_time_flat + traj_time_dec;
        traj_accel = 0;
    }
    else if (traj_time_acc < vel_factor * period && traj_time_flat <  period && traj_time_dec > 0.0) {                                         // 加速接近结束且需减速
        traj_time += traj_time_acc + traj_time_flat +  traj_time_dec;
        traj_accel =  copysign(d_max * a_factor, -v0);
    }
    else {
        traj_time += traj_time_acc + traj_time_flat + traj_time_dec;
        traj_accel =  copysign(a_max * a_factor, -x0);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void compute_motion_2d(CVector x0, CVector v0, CVector v1, double a_max, double v_max,
/// 		double a_factor, CVector &traj_accel, double &time)
///
/// @brief 在二维下计算运动过程，给定初始速度向量和最终速度向量，以及最大加速度，最大速度
///
/// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
/// @date		2010-4-2
///
/// @param	x0						initial coordinate
/// @param	v0						initial velocities
/// @param	v1						final velocities
/// @param	a_max					maxium of accelerate
/// @param	v_max					maxium of velocities
/// @param	a_factor				factor of accelerate
/// @param [in,out]	traj_accel	the traj accel.
/// @param [in,out]	time		the time.
////////////////////////////////////////////////////////////////////////////////////////////////////

void compute_motion_2d(CVector x0, CVector v0, CVector v1,
                       double a_max, double d_max, double v_max,
                       double a_factor, CVector &traj_accel, double &time, double &time_acc, double &time_dec, double &time_flat, nonZeroMode mode) {
    double time_x = 0, time_x_acc = 0, time_x_dec = 0, time_x_flat = 0;
    double time_y = 0, time_y_acc = 0, time_y_dec = 0, time_y_flat = 0;
    double rotangle = 0;
    double traj_accel_x = 0;
    double traj_accel_y = 0;
    if(v0 * x0 > 0) {           //如果发现正在反方向走，则不再零速到点，防止车冲出去
//        GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(1.0, 0.0), QString("Limit v1").toLatin1());
        v1 = CVector(0.0, 0.0);
    }
    if (v1.mod() == 0 || mode == FAST) {
        rotangle = x0.dir();
    }
    else {
        rotangle = v1.dir();
    }
    x0 = x0.rotate(-rotangle);
    v0 = v0.rotate(-rotangle);
    v1 = v1.rotate(-rotangle); //坐标系转换，转换到末速度方向为x轴的坐标系中

    double velFactorX = 1.0, velFactorY = 1.0;
    velFactorX = (fabs(v1.x()) > 1e-8 ? 2.8 : 1.0);
    velFactorY = (fabs(v1.y()) > 1e-8 ? 2.8 : 1.0);

    if(v1.mod() > 0 && mode == FAST) {
        v1.setVector(copysign(v1.mod(), v1.x()), 0);
//        v_max = v1.mod();
    }

    //calcu ky and ky to make kx^2+ky^2=1+1=2 to satisfy the total max acc
    double kx = 1.0, ky = 1.0;
    if(mode == FAST)
    {
        //if y is big , we should move faster in y
        if(fabs(x0.y()) > 200)
        {
            ky = 1 + 0.3 / (1 + fabs(x0.x())/fabs(x0.y()+1e-8));
            kx = sqrt(2 -ky*ky);
        }else{
            kx = 1.0;
            ky = 1.0;
        }

    }

    timeItor = 0;
    isX = 1;
    compute_motion_1d(x0.x(), v0.x(), v1.x(), kx * a_max, kx * d_max, v_max, a_factor, velFactorX,
                      traj_accel_x, time_x, time_x_acc, time_x_dec, time_x_flat, MOVE_X, mode);
    timeItor = 0;
    isX = -1;
    compute_motion_1d(x0.y(), v0.y(), v1.y(), ky * a_max, ky * d_max, v_max, a_factor, velFactorY,
                      traj_accel_y, time_y, time_y_acc, time_y_dec, time_y_flat, MOVE_Y, mode);//两轴同样的最大速度、加速度独立考虑求两轴运动时间

    if(v1.mod() > 1e-8 && mode == ACCURATE) {
        if (time_x - time_y > FRAME_PERIOD) {
            compute_motion_1d(x0.y(), v0.y(), 0, a_max, d_max, v_max, a_factor, velFactorX,
                              traj_accel_y, time_y, time_y_acc, time_y_dec, time_y_flat, MOVE_X, mode);
        } else if (time_y - time_x > FRAME_PERIOD) {
            compute_motion_1d(x0.x(), v0.x(), 0, a_max, d_max, v_max, a_factor, velFactorY,
                              traj_accel_x, time_x, time_x_acc, time_x_dec, time_x_flat, MOVE_X, mode);
        }
    }


    traj_accel = CVector(traj_accel_x, traj_accel_y);
    if (traj_accel.mod())
        traj_accel = traj_accel.rotate(rotangle);
    if(time_x < 1e-5 || time_x > 500) time_x = 0;
    if(time_y < 1e-5 || time_y > 500) time_y = 0;
    if(time_x < time_y) {
        time = time_y;
        time_acc = time_y_acc;
        time_dec = time_y_dec;
        time_flat = time_y_flat;
    } else {
        time = time_x;
        time_acc = time_x_acc;
        time_dec = time_x_dec;
        time_flat = time_x_flat;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	double compute_stop(double v, double max_a)
///
/// @brief	Calculates the stop.
///
/// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
/// @date		2010-4-2
///
/// @param	v		The v.
/// @param	max_a	The maximum a.
///
/// @return	The calculated stop.
////////////////////////////////////////////////////////////////////////////////////////////////////

double compute_stop(double v, double max_a) {
    if (fabs(v) > max_a * FRAME_PERIOD) return  copysign(max_a, -v); // 如果一帧内不能减速到0，就用最大加速度
    else return -v / FRAME_PERIOD; // 如果能减到0，加速度就用速度除以一帧的时间即可
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	void goto_point_omni( const PlayerPoseT& start, const PlayerPoseT& final,
/// 		const PlayerCapabilityT& capability, const double& accel_factor,
/// 		const double& angle_accel_factor, PlayerPoseT& nextStep)
///
/// @brief	给定初始坐标，结束坐标及小车性能，计算小车可能的轨迹，更新下一个位置的机器人状态
///
/// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
/// @date		2010-4-2
///
/// @param	start				The start.
/// @param	final				The final.
/// @param	capability			The capability.
/// @param	accel_factor		The accel factor.
/// @param	angle_accel_factor	The angle accel factor.
/// @param [in,out]	nextStep	the next step.
////////////////////////////////////////////////////////////////////////////////////////////////////

void goto_point_omni( const PlayerPoseT& start,
                      const PlayerPoseT& final,
                      const PlayerCapabilityT& capability,
                      const double& accel_factor,
                      const double& angle_accel_factor,
                      PlayerPoseT& nextStep,
                      nonZeroMode mode,
                      bool use_dir) {
    CGeoPoint target_pos = final.Pos();
    CVector x = start.Pos() - target_pos;
    CVector v = start.Vel();
    double ang = Normalize(start.Dir() - final.Dir());
    double ang_v = start.RotVel();
    CVector target_vel = final.Vel();
    double max_accel = capability.maxAccel;
    double max_decel = capability.maxDec;
    double max_speed = capability.maxSpeed;

    double max_angle_speed = capability.maxAngularSpeed;
    double max_angle_accel = capability.maxAngularAccel;
    double max_angle_decel = capability.maxAngularDec;
    //GDebugEngine::Instance()->gui_debug_msg(CGeoPoint(0,0),QString("rotvel:%1, rotacc:%2, rotdec:%3").arg(max_angle_speed).arg(max_angle_accel).arg(max_angle_decel).toLatin1());
    CVector a;
    double ang_a, factor_a;
    double time_a, time_a_acc, time_a_dec, time_a_flat, time;
    double time_acc, time_dec, time_flat;
    compute_motion_2d(x, v, target_vel, max_accel, max_decel, max_speed, accel_factor, a, time, time_acc, time_dec, time_flat, mode);
    factor_a = 1;

    double rotateFactor;
    double selfSpeed = start.Vel().mod();

//    if(upperBoundSpeedLimitRotate == lowerBoundSpeedLimitRotate)
//        cout << "Oh shit!!! Error speed limit params!!! ---CMmotion.cpp" << endl;
//    else if(selfSpeed < lowerBoundSpeedLimitRotate)
//        rotateFactor = 1.0;
//    else if(selfSpeed < upperBoundSpeedLimitRotate)
//        rotateFactor = 1 - (selfSpeed - lowerBoundSpeedLimitRotate) / (upperBoundSpeedLimitRotate - lowerBoundSpeedLimitRotate);
//    else
//        rotateFactor = 0.0;
    rotateFactor = 1.0;
    if(capability.maxAngularAccel < min_max_angle_acc)
        max_angle_accel= max_angle_decel = capability.maxAngularAccel;
    else
        max_angle_accel = max_angle_decel = min_max_angle_acc + (capability.maxAngularAccel * rotateScaleFactor - min_max_angle_acc) * rotateFactor;

    if(fabs(ang) > minAngleLimitRotateSpeed) {
        if(capability.maxAngularSpeed < min_max_angle_speed)
            max_angle_speed = capability.maxAngularSpeed;
        else
            max_angle_speed = min_max_angle_speed + (capability.maxAngularSpeed * rotateScaleFactor - min_max_angle_speed) * rotateFactor;
    }
    v = v + a * FRAME_PERIOD;
    // if(!use_dir){
        compute_motion_1d(ang, ang_v, 0.0, max_angle_accel, max_angle_decel, max_angle_speed, angle_accel_factor, 1.0, ang_a, time_a, time_a_acc, time_a_dec, time_a_flat, ROTATE, mode);
        ang_v += ang_a * FRAME_PERIOD;
        double delta_ang = ang_v * FRAME_PERIOD;
        v.rotate(-delta_ang);
        if (ang_v > max_angle_speed) {
            ang_v = max_angle_speed;
        } else if (ang_v < -max_angle_speed) {
            ang_v = -max_angle_speed;
        }
        double next_angle = start.Dir() + ang_v * FRAME_PERIOD;
        nextStep.SetDir(next_angle);
        nextStep.SetRotVel(ang_v);
    // }

    if (v.mod() > max_speed) {
        v = v * max_speed / v.mod();
    }

    CGeoPoint next_pos = start.Pos() + Polar2Vector(v.mod() * FRAME_PERIOD, v.dir());
    nextStep.SetValid(true);
    nextStep.SetPos(next_pos);
    nextStep.SetVel(v);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	double expectedCMPathTime(const PlayerPoseT& start, const CGeoPoint& final,
/// 	double maxAccel, double maxVelocity, double accel_factor)
///
/// @brief	调用 compute_motion_2d
///
/// @author	Song Zhibai( Song Zhibai Zju.edu.cn)
/// @date	2010-4-2
///
/// @param	start			The start.
/// @param	final			The final.
/// @param	maxAccel		The maximum accel.
/// @param	maxVelocity		The maximum velocity.
/// @param	accel_factor	The accel factor.
///
/// @return	.
////////////////////////////////////////////////////////////////////////////////////////////////////

double expectedCMPathTime(const PlayerPoseT& start, const CGeoPoint& final, double maxAccel, double maxVelocity, double accel_factor) {
    CGeoPoint target_pos = final;
    CVector x = start.Pos() - target_pos;
    CVector v = start.Vel();
    CVector target_vel = CVector(0, 0);
    CVector a;
    double time;
    double time_acc, time_dec, time_flat;
    compute_motion_2d(x, v, target_vel,
                      maxAccel,
                      maxAccel,
                      maxVelocity,
                      accel_factor,
                      a, time, time_acc, time_dec, time_flat);
    return time;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @fn	double openSpeedCircle(const PlayerPoseT& start, const double dist2Center, const double dirDiff,
///  const double finalAngle, PlayerPoseT& nextStep)
///
/// @param	start			The start.
/// @param  dist2Center     The rotate radius.
/// @param  dirDiff         The direcition diff between robot angle and circle tangent.
/// @param  finalAngle      The final angle u want the robot finish at.
/// @param  nextStep        The return trajectory.
///
/// @return	.
////////////////////////////////////////////////////////////////////////////////////////////////////

void openSpeedCircle(const PlayerPoseT& start, const double dist2Center, const int rotateMethod, const double finalAngle, PlayerPoseT& nextStep, const double dirDiff) {
    CGeoPoint startPos = start.Pos();
    double startRotVel = start.RotVel();

    double startDir, finalDir;
    if (rotateMethod == 1 || rotateMethod == 4) {
        startDir = Normalize(start.Dir() + dirDiff);
        finalDir = Normalize(finalAngle + dirDiff);
    } else {
        startDir = Normalize(start.Dir() - dirDiff);
        finalDir = Normalize(finalAngle - dirDiff);
    }

    double posDirDiff = abs(Normalize(startDir - finalDir));

    double rotAcc, rotTime, rotAccTime, rotDecTime, rotFlatTime;

    if (rotateMethod == 1 || rotateMethod == 4) {
        compute_motion_1d(posDirDiff, start.RotVel(), 0, 15, 5, 5, 1.5, 1.0, rotAcc, rotTime, rotAccTime, rotDecTime, rotFlatTime, ROTATE);
    } else {
        compute_motion_1d(-posDirDiff, start.RotVel(), 0, 15, 5, 5, 1.5, 1.0, rotAcc, rotTime, rotAccTime, rotDecTime, rotFlatTime, ROTATE);
    }

    double rotVel = startRotVel + rotAcc / FRAME_RATE;
    double velMod = abs(rotVel * dist2Center);
    CVector vel;
    if (rotateMethod == 1 || rotateMethod == 3) {
        vel = Polar2Vector(velMod, startDir);
    } else {
        vel = Polar2Vector(velMod, Normalize(startDir + PI));
    }

    CVector localVel = vel.rotate(- start.Dir());
    CGeoPoint nextPos = startPos + vel / FRAME_RATE;
    double nextDir = startDir + rotVel / FRAME_RATE;

    nextStep.SetValid(true);
    nextStep.SetPos(nextPos);
    nextStep.SetDir(nextDir);
    nextStep.SetVel(localVel);
    nextStep.SetRotVel(rotVel);
}


