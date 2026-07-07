#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "Globals.h"
#include "Scheduler.h"
#include "TaskQueue.h"

class Elevator {
private:
    int elevatorID;
    int currentLevel;
    int totalLevels;
    Direction direction;
    int targetFloor;
    ElevatorStatus status;
    DoorStatus doorStatus; // New member for door simulation
    Scheduler* scheduler;
    TaskQueue* taskQueue;

public:
    Elevator(int id, int maxFloors, Scheduler* sched, TaskQueue* tasks);

    void handleRequests();
    void processRequest(const Task& task);
    void moveElevator();
    ElevatorStatus checkStatus() const;
    void updateStatus(ElevatorStatus newStatus);
    void informScheduler();
    int getCurrentLevel() const;
    ElevatorStatusData retrieveElevatorStatus() const;
};

#endif // ELEVATOR_H
