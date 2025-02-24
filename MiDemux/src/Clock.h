#pragma once

#include <mutex>

class SystemClock
{
public:
    void setTime(uint64_t time);
    uint64_t time();

public:
    SystemClock(void);
    ~SystemClock(void);
    SystemClock(const SystemClock&) = delete;

private:
    uint64_t time_;

    mutable std::mutex mutex_;
};


class PCRClock
{
public:
    void setTime(uint8_t* time);
    double timeInSeconds() const;
    uint64_t time() const;
    uint64_t baseTime() const;
    uint16_t extTime() const;

public:
    PCRClock(void);
    ~PCRClock(void);
    PCRClock(const PCRClock&) = delete;

private:
    uint8_t time_[6]{};
};


