#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

class timer
{
public:
    timer(int signId, timer_t timerId, void (*callback)(union sigval));
    void stopTimer(void);
    void startTimer(struct itimerspec it);

private:
    int signId_;
    timer_t timerId_;
};

#endif