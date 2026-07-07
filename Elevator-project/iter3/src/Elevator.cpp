#include "Elevator.h"
#include "Globals.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <iostream>

using namespace std;

Elevator::Elevator(int id, int maxFloors, Scheduler* sched, TaskQueue* tasks)
    : elevatorID(id), currentLevel(0), totalLevels(maxFloors), direction(UP), targetFloor(0),
      status(STANDBY), doorStatus(DOOR_CLOSED), scheduler(sched), taskQueue(tasks) {
    {
        ostringstream oss;
        oss << "=== [Elevator " << elevatorID 
            << "] Initialized with " << totalLevels << " floors. ===";
        DEBUG_LOG_COLOR(oss.str(), 5);
    }
}

void Elevator::handleRequests() {
    informScheduler();
    switch (status) {
        case STANDBY: {
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID 
                    << "] Status: STANDBY. Waiting for a request...";
                DEBUG_LOG_COLOR(oss.str(), 2);
            }
            if (scheduler) {
                Task task = scheduler->fetchTaskForElevator(elevatorID);
                {
                    ostringstream oss;
                    oss << ">>> [Elevator " << elevatorID 
                        << "] Request received: Floor " << task.floorNumber 
                        << " (" << (task.direction == UP ? "UP" : "DOWN") << ") assigned to me.";
                    DEBUG_LOG_COLOR(oss.str(), 3);
                }
                processRequest(task);
                updateStatus(IN_TRANSIT);
                moveElevator();
            } else {
                DEBUG_LOG_COLOR("Elevator: Scheduler pointer is null in handleRequests.", 4);
            }
            break;
        }
        case IN_TRANSIT: {
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID 
                    << "] Status: IN_TRANSIT. Currently moving...";
                DEBUG_LOG_COLOR(oss.str(), 2);
            }
            break;
        }
        case REACHED: {
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID 
                    << "] Status: REACHED. Arrived at destination. Returning to STANDBY.";
                DEBUG_LOG_COLOR(oss.str(), 1);
            }
            updateStatus(STANDBY);
            break;
        }
    }
}

void Elevator::processRequest(const Task& task) {
    {
        ostringstream oss;
        oss << "+++ [Elevator " << elevatorID << "] Processing request: Floor " 
            << task.floorNumber << ", Direction: " 
            << (task.direction == UP ? "UP" : "DOWN") << " +++";
        DEBUG_LOG_COLOR(oss.str(), 3);
    }
    if (task.elevatorID == elevatorID) {
        direction = task.direction;
        targetFloor = task.floorNumber;
        currentLevel = getCurrentLevel();
        {
            ostringstream oss;
            oss << "+++ [Elevator " << elevatorID << "] Assigned to Floor " 
                << targetFloor << " +++";
            DEBUG_LOG_COLOR(oss.str(), 3);
        }
    } else {
        {
            ostringstream oss;
            oss << "!!! [Elevator " << elevatorID << "] Not assigned to process request for Floor " 
                << task.floorNumber << " !!!";
            DEBUG_LOG_COLOR(oss.str(), 4);
        }
    }
}

ElevatorStatusData Elevator::retrieveElevatorStatus() const {
    return ElevatorStatusData(elevatorID, targetFloor, direction, currentLevel, status, doorStatus);
}

int Elevator::getCurrentLevel() const {
    return currentLevel;
}

void Elevator::informScheduler() {
    if (scheduler) {
        {
            ostringstream oss;
            oss << "--- [Elevator " << elevatorID 
                << "] Sending update to Scheduler ---";
            DEBUG_LOG_COLOR(oss.str(), 5);
        }
        ElevatorStatusData data = retrieveElevatorStatus();
        {
            ostringstream oss;
            oss << "    ID: " << data.elevatorID 
                << " | Destination: " << data.destination
                << " | Direction: " << (data.travelDirection == UP ? "UP" : "DOWN")
                << " | Current Floor: " << data.currentFloor 
                << " | Status: " 
                << (data.status == STANDBY ? "STANDBY" : (data.status == IN_TRANSIT ? "IN_TRANSIT" : "REACHED"))
                << " | Door: " 
                << (data.doorStatus == DOOR_CLOSED ? "CLOSED" :
                    (data.doorStatus == DOOR_OPENING ? "OPENING" :
                    (data.doorStatus == DOOR_OPEN ? "OPEN" : "CLOSING")));
            DEBUG_LOG_COLOR(oss.str(), 5);
        }
        scheduler->recordElevatorUpdate(data);
    } else {
        DEBUG_LOG_COLOR("Elevator: Scheduler pointer is null. Skipping informScheduler.", 2);
    }
}

void Elevator::moveElevator() {
    int finalDestination = targetFloor;
    auto transitStart = std::chrono::steady_clock::now();
    
    if (currentLevel == finalDestination) {
        updateStatus(STANDBY);
        {
            ostringstream oss;
            oss << "Warning: Elevator " << elevatorID << " tried to loopback.";
            DEBUG_LOG_COLOR(oss.str(), 4);
        }
        return;
    }
    
    // Loop until the elevator reaches its final destination.
    while (currentLevel != finalDestination) {
        direction = (currentLevel < finalDestination) ? UP : DOWN;
        
        // Move step-by-step. hi
        while (currentLevel != finalDestination) {
            if (std::chrono::steady_clock::now() - transitStart > std::chrono::seconds(25 * ELEVATOR_TRAVEL_TIME)) {
                updateStatus(STANDBY);
                {
                    ostringstream oss;
                    oss << "Warning: Elevator " << elevatorID << " transit timed out. Resetting to STANDBY.";
                    DEBUG_LOG_COLOR(oss.str(), 4);
                }
                return;
            }
            updateStatus(IN_TRANSIT);
            informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_TRAVEL_TIME));
            (direction == UP) ? currentLevel++ : currentLevel--;
            {
                ostringstream oss;
                oss << "=== [Elevator " << elevatorID 
                    << "] Moving: Now at Floor " << currentLevel << " ===";
                DEBUG_LOG_COLOR(oss.str(), 5);
            }
        }
        
        {
            ostringstream oss;
            oss << "### [Elevator " << elevatorID << "] Reached Floor " << currentLevel << " ###";
            DEBUG_LOG_COLOR(oss.str(), 1);
        }
        
        informScheduler();
        if (taskQueue) {
            taskQueue->removeTask(currentLevel, elevatorID);
        }
        
        // Simulate door operations if the target is reached.
        if (currentLevel == finalDestination) {
            updateStatus(REACHED);
            {
                ostringstream oss;
                oss << "### [Elevator " << elevatorID 
                    << "] Target Floor " << currentLevel 
                    << " reached. Simulating door operations. ###";
                DEBUG_LOG_COLOR(oss.str(), 3);
            }
            
            doorStatus = DOOR_OPENING;
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID << "] Doors opening...";
                DEBUG_LOG_COLOR(oss.str(), 3);
            }
            informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_OPEN_TIME));
            
            doorStatus = DOOR_OPEN;
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID << "] Doors open.";
                DEBUG_LOG_COLOR(oss.str(), 3);
            }
            informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_LOADING_TIME));
            
            doorStatus = DOOR_CLOSING;
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID << "] Doors closing...";
                DEBUG_LOG_COLOR(oss.str(), 3);
            }
            informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_CLOSE_TIME));
            
            doorStatus = DOOR_CLOSED;
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID << "] Doors closed.";
                DEBUG_LOG_COLOR(oss.str(), 3);
            }
            status = STANDBY;
            informScheduler();
        }
        
        // If reached the building's end, reverse direction.
        if ((direction == UP && currentLevel == totalLevels - 1) ||
            (direction == DOWN && currentLevel == 0)) {
            direction = (direction == UP) ? DOWN : UP;
            {
                ostringstream oss;
                oss << "*** [Elevator " << elevatorID 
                    << "] Reached building end. Reversing direction. ***";
                DEBUG_LOG_COLOR(oss.str(), 6);
            }
        }
    }
}

ElevatorStatus Elevator::checkStatus() const {
    return status;
}

void Elevator::updateStatus(ElevatorStatus newStatus) {
    status = newStatus;
}
