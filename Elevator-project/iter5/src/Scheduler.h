#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Globals.h"
#include "TaskQueue.h"
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <chrono>

class Scheduler {
private:
    std::mutex mutex;
    std::condition_variable cv;
    TaskQueue* taskQueue;
    std::unordered_map<int, ElevatorStatusData> elevatorStatuses;
    int fallbackIndex = 0;

public:
    Scheduler(TaskQueue* taskQueue);
    
    void addElevatorRequest(const Task& request);
    Task fetchNextTask();
    Task fetchTaskForElevator(int elevatorID);
    void recordElevatorUpdate(ElevatorStatusData update);
    int getQueueSize();
    ElevatorStatusData retrieveElevatorUpdate();
    std::unordered_map<int, ElevatorStatusData> getElevatorStatuses();
    
    // New: returns the number of tasks assigned for a given elevator.
    int getQueueSizeForElevator(int elevatorID);
};

#endif // SCHEDULER_H
