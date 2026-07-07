#include "Elevator.h"
#include "Globals.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <iostream>
#include <cstdlib>

using namespace std;

// Global flag for fault injection (simulate a timeout fault).
// bool faultInjectedTimeout = false;

// Constructor initializes the elevator with the given ID, number of floors, and references 
// to the scheduler and task queue. It also sets initial states and default values.
Elevator::Elevator(int id, int maxFloors, Scheduler* sched, TaskQueue* tasks)
    : elevatorID(id), currentLevel(0), totalLevels(maxFloors), direction(UP), targetFloor(0),
      status(STANDBY), doorStatus(DOOR_CLOSED), scheduler(sched), taskQueue(tasks),
      currentLoad(0), maxCapacity(10), faultInjectedTimeout(false), faultDoorTimeout(false) {
    ostringstream oss;
    oss << "=== [Elevator " << elevatorID << "] Initialized with " << totalLevels << " floors. ===";
    cout << oss.str() << endl;
}

// Handles elevator requests by checking its status and interacting with the scheduler.
void Elevator::handleRequests() {
    informScheduler(); // Send status update to scheduler
    
    switch (status) {
        case STANDBY: { // Elevator is waiting for requests
            {
                ostringstream oss;
                oss << ">>> [Elevator " << elevatorID << "] Status: STANDBY. Waiting for a request...";
                cout << oss.str() << endl;
            }
            if (scheduler) {
                // Fetch a task assigned to this elevator
                Task task = scheduler->fetchTaskForElevator(elevatorID);
                {
                    ostringstream oss;
                    oss << ">>> [Elevator " << elevatorID << "] Request received: Floor " 
                        << task.floorNumber << " (" << (task.direction == UP ? "UP" : "DOWN") << ") assigned to me.";
                    cout << oss.str() << endl;
                }
                processRequest(task);  // Process the task
                updateStatus(IN_TRANSIT); // Set elevator in transit
                moveElevator(); // Start moving towards the target floor
            } else {
                cout << "Elevator: Scheduler pointer is null in handleRequests." << endl;
            }
            break;
        }
        case IN_TRANSIT: { // Elevator is moving
            cout << ">>> [Elevator " << elevatorID << "] Status: IN_TRANSIT. Currently moving..." << endl;
            break;
        }
        case REACHED: { // Elevator has reached the target floor
            cout << ">>> [Elevator " << elevatorID << "] Status: REACHED. Arrived at destination. Returning to STANDBY." << endl;
            updateStatus(STANDBY);
            break;
        }
        case FAULT: { // A fault has occurred, shutting down the elevator
            cout << "Elevator: Fault detected. Shutting down." << endl;
            exit(1);
            break;
        }
    }
}

// Processes a new task and sets the target floor if the task is assigned to this elevator.
void Elevator::processRequest(const Task& task) {
    ostringstream oss;
    oss << "+++ [Elevator " << elevatorID << "] Processing request: Floor " 
        << task.floorNumber << ", Direction: " << (task.direction == UP ? "UP" : "DOWN") << " +++";
    cout << oss.str() << endl;
    
    if (task.elevatorID == elevatorID) {
        direction = task.direction;
        targetFloor = task.floorNumber;
        currentLevel = getCurrentLevel();
        ostringstream oss2;
        oss2 << "+++ [Elevator " << elevatorID << "] Assigned to Floor " << targetFloor << " +++";
        cout << oss2.str() << endl;
    } else {
        ostringstream oss3;
        oss3 << "!!! [Elevator " << elevatorID << "] Not assigned to process request for Floor " 
             << task.floorNumber << " !!!";
        cout << oss3.str() << endl;
    }
}

// Retrieves and returns the current status of the elevator, including its position and load.
ElevatorStatusData Elevator::retrieveElevatorStatus() const {
    return ElevatorStatusData(elevatorID, targetFloor, direction, currentLevel, status, doorStatus, currentLoad, maxCapacity, localTaskQueue.size());
}

// Returns the current floor of the elevator.
int Elevator::getCurrentLevel() const {
    return currentLevel;
}

// Updates the door status of the elevator.
void Elevator::setDoorStatus(DoorStatus ds) {
    doorStatus = ds;
}

// Sets the target floor for the elevator.
void Elevator::setDestination(int dest) {
    targetFloor = dest;
}

// Sends an update of the elevator's status to the scheduler.
void Elevator::informScheduler() {
    if (scheduler) {
        std::ostringstream oss;
        oss << "--- [Elevator " << elevatorID << "] Sending update to Scheduler ---";
        std::cout << oss.str() << std::endl;
        
        ElevatorStatusData data = retrieveElevatorStatus();
        std::ostringstream oss2;
        oss2 << "    ID: " << data.elevatorID 
             << " | Destination: " << data.destination
             << " | Direction: " << (data.travelDirection == UP ? "UP" : "DOWN")
             << " | Current Floor: " << data.currentFloor 
             << " | Status: " 
             << (data.status == STANDBY ? "STANDBY" :
                 (data.status == IN_TRANSIT ? "IN_TRANSIT" :
                 (data.status == REACHED ? "REACHED" :
                 (data.status == PFAULT ? "PFAULT" : "FAULT"))))
             << " | Door: " 
             << (data.doorStatus == DOOR_CLOSED ? "CLOSED" :
                (data.doorStatus == DOOR_OPENING ? "OPENING" :
                (data.doorStatus == DOOR_OPEN ? "OPEN" :
                (data.doorStatus == DOOR_CLOSING ? "CLOSING" : "STUCK"))))
            
             << " | Load: " << data.currentLoad << "/" << data.maxCapacity
             << " | Queue: " << data.queueSize;
        std::cout << oss2.str() << std::endl;
        
        scheduler->recordElevatorUpdate(data);
    } else {
        std::cout << "Elevator: Scheduler pointer is null. Skipping informScheduler." << std::endl;
    }
}

// Moves the elevator toward the assigned floor, handling faults and timeouts.
void Elevator::moveElevator() {
    int finalDestination = targetFloor;
    auto transitStart = std::chrono::steady_clock::now();
    
    if (currentLevel == finalDestination) {
        updateStatus(STANDBY);
        cout << "Warning: Elevator " << elevatorID << " tried to loopback." << endl;
        return;
    }
    
    while (currentLevel != finalDestination) {
        direction = (currentLevel < finalDestination) ? UP : DOWN;
        while (currentLevel != finalDestination) {
            // Check if transit time exceeds the fault timeout limit.
            if (std::chrono::steady_clock::now() - transitStart > std::chrono::seconds(FAULT_TIMEOUT_SECONDS)) {
                updateStatus(FAULT);
                cout << "Elevator: Transit timeout fault. Shutting down elevator." << endl;
                exit(1);
            }
            // Handle fault injection for debugging/testing.
            if (faultInjectedTimeout) {
                cout << ">>> [Elevator " << elevatorID << "] Fault injected: timeout activated. Stopping movement." << endl;
                return;
            }
            updateStatus(IN_TRANSIT);
            informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_TRAVEL_TIME));
            (direction == UP) ? currentLevel++ : currentLevel--;
            ostringstream oss;
            oss << "=== [Elevator " << elevatorID << "] Moving: Now at Floor " << currentLevel << " ===";
            cout << oss.str() << endl;
        }
        
        // Elevator reached the target floor
        cout << "### [Elevator " << elevatorID << "] Reached Floor " << currentLevel << " ###" << endl;
        informScheduler();
        if (taskQueue) {
            taskQueue->removeTask(currentLevel, elevatorID);
        }
        
        if (currentLevel == finalDestination) {
            updateStatus(REACHED);
            setDoorStatus(DOOR_OPENING);
            cout << ">>> [Elevator " << elevatorID << "] Doors opening..." << endl;
            informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_OPEN_TIME));
            
            setDoorStatus(DOOR_OPEN);
            cout << ">>> [Elevator " << elevatorID << "] Doors open." << endl;
            informScheduler();
        }
    }
}

// Returns the current status of the elevator.
ElevatorStatus Elevator::checkStatus() const {
    return status;
}

// Updates the status of the elevator.
void Elevator::updateStatus(ElevatorStatus newStatus) {
    status = newStatus;
}
