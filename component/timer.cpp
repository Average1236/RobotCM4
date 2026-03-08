#include "timer.h"

timer::timer(int signId, timer_t timerId, void (*callback)(union sigval))
{
    signId_ = signId;
    timerId_ = timerId;

    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent)); // 清零初始化
    evp.sigev_value.sival_int = signId;       // 也是标识定时器的，这和timerid有什么区别？回调函数可以获得
    evp.sigev_notify = SIGEV_THREAD;          // 线程通知的方式，派驻新线程
    evp.sigev_notify_function = callback;     // 线程函数地址

    if (timer_create(CLOCK_REALTIME, &evp, &timerId) == -1)
    {
        perror("fail to timer_create");
        exit(-1);
    }
}

void timer::stopTimer(void)
{
    timer_delete(timerId_);
}

void timer::startTimer(struct itimerspec it)
{
    if (timer_settime(timerId_, 0, &it, NULL) == -1)
    {
        perror("fail to timer_settime");
        exit(-1);
    }
}