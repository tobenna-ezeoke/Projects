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
    // Map of elevatorID to its latest status.
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
    
    // Return a copy of all elevator statuses (for the ncurses GUI)
    std::unordered_map<int, ElevatorStatusData> getElevatorStatuses();
};

#endif // SCHEDULER_H
