#include "Scheduler.h"
#include "Globals.h"
#include <iostream>
#include <climits>
#include <cmath>
#include <sstream>
#include <algorithm>

using namespace std;

Scheduler::Scheduler(TaskQueue* taskQueue) : taskQueue(taskQueue) {}

void Scheduler::addElevatorRequest(const Task& request) {
    std::lock_guard<std::mutex> lock(mutex);
    int bestElevatorID = -1;
    int minDistance = INT_MAX;
    
    // First, try to choose an elevator that is in STANDBY.
    for (const auto& pair : elevatorStatuses) {
        const ElevatorStatusData& elevator = pair.second;
        if (elevator.status == STANDBY) {
            int distance = std::abs(elevator.currentFloor - request.floorNumber);
            if (distance < minDistance) {
                minDistance = distance;
                bestElevatorID = elevator.elevatorID;
            }
        }
    }
    
    // If no elevator is in standby, choose one in a cyclic fallback.
    if (bestElevatorID == -1 && !elevatorStatuses.empty()) {
        std::vector<int> ids;
        for (const auto& pair : elevatorStatuses) {
            ids.push_back(pair.second.elevatorID);
        }
        std::sort(ids.begin(), ids.end());
        bestElevatorID = ids[fallbackIndex % ids.size()];
        fallbackIndex++;  // Advance the fallback index for the next assignment.
    }
    
    // If still no elevator is found (unlikely), default to elevator 1.
    if (bestElevatorID == -1) {
        bestElevatorID = 1;
    }
    
    // Create a new Task that preserves the fault flag from the incoming request.
    Task newTask(FLOOR_CALL, request.floorNumber, request.direction, bestElevatorID, request.faultFlag);
    taskQueue->addTask(newTask);
    cv.notify_all();
}

int Scheduler::getQueueSize() {
    return taskQueue->getSize();
}

Task Scheduler::fetchNextTask() {
    return taskQueue->fetchNextTask();
}

Task Scheduler::fetchTaskForElevator(int elevatorID) {
    return taskQueue->fetchTaskForElevator(elevatorID);
}

void Scheduler::recordElevatorUpdate(ElevatorStatusData update) {
    std::lock_guard<std::mutex> lock(mutex);
    elevatorStatuses[update.elevatorID] = update;
    std::ostringstream oss;
    oss << "+++ [Scheduler] Update received: Elevator " 
        << update.elevatorID 
        << " | Floor: " << update.currentFloor 
        << " | Destination: " << update.destination 
        << " | Status: " << (update.status == STANDBY ? "STANDBY" : 
                             (update.status == IN_TRANSIT ? "IN_TRANSIT" : "REACHED")) 
        << " +++";
    DEBUG_LOG_COLOR(oss.str(), 6);
}

ElevatorStatusData Scheduler::retrieveElevatorUpdate() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!elevatorStatuses.empty()) {
         return elevatorStatuses.begin()->second;
    }
    return ElevatorStatusData(-1, -1, UP, -1, STANDBY);
}

std::unordered_map<int, ElevatorStatusData> Scheduler::getElevatorStatuses() {
    std::lock_guard<std::mutex> lock(mutex);
    return elevatorStatuses;
}

int Scheduler::getQueueSizeForElevator(int elevatorID) {
    return taskQueue->getQueueSizeForElevator(elevatorID);
}
