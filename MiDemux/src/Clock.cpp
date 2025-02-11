#include "Clock.h"

#include <cstring>

const double RESOLUTION = 27000000.0; // 27MHz

SystemClock::SystemClock(void)
    :time_(0)
{

}

SystemClock::~SystemClock(void)
{
}

void SystemClock::setTime(uint64_t time)
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    /*std::cout.precision(12);
    std::cout << time << std::endl;*/
    time_ = time;
}

uint64_t SystemClock::time()
{
    std::lock_guard<std::mutex> autoLock(mutex_);
    return time_;
}


PCRClock::PCRClock(void)
{

}

PCRClock::~PCRClock(void)
{
}

void PCRClock::setTime(uint8_t* time)
{
    memcpy(time_, time, 6);
}

double PCRClock::timeInSeconds() const
{
    return (double)(time() / RESOLUTION);
}

uint64_t PCRClock::time() const
{
    uint64_t pcr = baseTime() * 300 + extTime();
    return pcr;
}

uint64_t PCRClock::baseTime() const
{
    uint64_t pcr_base = ((uint64_t)time_[0] << (33 - 8)) |
        ((uint64_t)time_[1] << (33 - 16)) |
        ((uint64_t)time_[2] << (33 - 24)) |
        ((uint64_t)time_[3] << (33 - 32));

    return pcr_base;
}

uint16_t PCRClock::extTime() const
{
    uint16_t pcr_ext = time_[4] & 0x01 << 9;
    pcr_ext = pcr_ext | time_[5];

    return pcr_ext;
}

