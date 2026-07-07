#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "Globals.h"
#include "Scheduler.h"
#include "TaskQueue.h"
#include <vector>

// Structure to hold a task along with its associated passenger count.
struct ElevatorTask {
    Task task;
    int passengerCount;
};

class Elevator {
private:
    int elevatorID;
    int currentLevel;
    int totalLevels;
    Direction direction;
    int targetFloor;
    ElevatorStatus status;
    DoorStatus doorStatus;
    Scheduler* scheduler;
    TaskQueue* taskQueue;

    // Capacity/load simulation.
    int currentLoad;
    int maxCapacity;
    
    // Internal (local) task queue.
    std::vector<ElevatorTask> localTaskQueue;

public:
    Elevator(int id, int maxFloors, Scheduler* sched, TaskQueue* tasks);
    std::atomic<bool> faultInjectedTimeout;
    std::atomic<bool> faultDoorTimeout;
    // Core methods.
    void handleRequests();
    void processRequest(const Task& task);
    void moveElevator();
    ElevatorStatus checkStatus() const;
    void updateStatus(ElevatorStatus newStatus);
    void informScheduler();
    int getCurrentLevel() const;
    ElevatorStatusData retrieveElevatorStatus() const;

    // New setters/getters.
    void setDoorStatus(DoorStatus ds);
    void setDestination(int dest);
    void setCurrentLoad(int load) { currentLoad = load; }
    void setMaxCapacity(int cap) { maxCapacity = cap; }
    int getCurrentLoad() const { return currentLoad; }
    int getMaxCapacity() const { return maxCapacity; }
    int getID() const { return elevatorID; }

    // Floor manipulation helpers.
    void incrementFloor() { currentLevel++; }
    void decrementFloor() { currentLevel--; }

    // Methods to manage the internal task queue.
    void addTaskToQueue(const ElevatorTask& et) { localTaskQueue.push_back(et); }
    void clearTaskQueue() { localTaskQueue.clear(); }
    const std::vector<ElevatorTask>& getLocalTaskQueue() const { return localTaskQueue; }
};

#endif // ELEVATOR_H
