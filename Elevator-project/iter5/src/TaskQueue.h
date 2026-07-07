#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "Globals.h"
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <iostream>
#include <stdexcept>

enum TaskType {
    FLOOR_CALL,
    ELEVATOR_ARRIVAL
};

struct Task {
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    TaskType type;
    int elevatorID; 
    int floorNumber;
    Direction direction;
    std::string faultFlag; // New field for fault injection

    Task() 
      : timestamp(std::chrono::system_clock::now()), type(FLOOR_CALL), elevatorID(-1), floorNumber(-1), direction(UP), faultFlag("") {}

    Task(TaskType taskType, int floor, Direction dir, int elevator = -1, const std::string &fault = "")
        : timestamp(std::chrono::system_clock::now()), type(taskType), elevatorID(elevator), floorNumber(floor), direction(dir), faultFlag(fault) {}
};

struct TaskComparator {
    bool operator()(const Task& t1, const Task& t2) {
        return t1.timestamp > t2.timestamp;
    }
};

class TaskQueue {
private:
    std::priority_queue<Task, std::vector<Task>, TaskComparator> taskQueue;
    std::mutex mutex;
    std::condition_variable cv;
    
public:
    void addTask(const Task& task);
    bool removeTask(int floor, int elevatorID);
    bool hasPendingTasks();
    int getSize();
    Task peekTask();
    Task fetchNextTask();
    Task fetchTaskForElevator(int elevatorID);
    std::priority_queue<Task, std::vector<Task>, TaskComparator>& getQueue();
    int getQueueSizeForElevator(int elevatorID);
};

#endif // TASKQUEUE_H
