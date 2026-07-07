#ifndef FLOOR_H
#define FLOOR_H

#include "Globals.h"
#include "Scheduler.h"
#include "TaskQueue.h"
#include <chrono>
#include <string>

class Floor {
private:
    Task floorTask;
    std::chrono::system_clock::time_point parseTime(const std::string& timestamp);
    void sendElevatorRequest(const Task& task);
    Scheduler* scheduler;

public:
    Floor(Scheduler* sched);
    void pressCallButton();
    void updateDisplay();
};

#endif // FLOOR_H
