// scheduler_main.cpp
#define USE_NCURSES_LOGGING

#include "Scheduler.h"
#include "TaskQueue.h"
#include "Globals.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ncurses.h>
#include <unordered_map>
#include <exception>

// This function is defined in ncursesGUI.cpp.
extern void ncursesGUI(Scheduler* scheduler);

#define SCHEDULER_PORT 5000
#define FLOOR_PORT 5001
#define BUFFER_SIZE 1024

// Helper function: returns the wrap-around distance using C-scan logic.
int wrapDistance(int currentFloor, int requestedFloor, int totalFloors) {
    return (requestedFloor - currentFloor + totalFloors) % totalFloors;
}

// Helper function: selects an elevator based on a C-scan algorithm.
// It computes a "distance" for each elevator and chooses the one with the smallest distance.
int selectElevatorForTask(const Task& task, const std::unordered_map<int, ElevatorStatusData>& statuses, int totalFloors = 22) {
    int bestElevator = -1;
    int bestDistance = totalFloors + 100; // Start with a large number.
    for (const auto &entry : statuses) {
        int eid = entry.first;
        const ElevatorStatusData &esd = entry.second;
        int distance = 0;
        if (esd.status == STANDBY) {
            // If idle, simply use the wrap-around distance.
            distance = wrapDistance(esd.currentFloor, task.floorNumber, totalFloors);
        } else {
            // If the elevator is moving, check if it is moving in the same direction.
            if (esd.travelDirection == task.direction) {
                if (task.floorNumber >= esd.currentFloor)
                    distance = task.floorNumber - esd.currentFloor;
                else
                    distance = (totalFloors - esd.currentFloor) + task.floorNumber;
            } else {
                // Add a penalty if moving in the opposite direction.
                distance = wrapDistance(esd.currentFloor, task.floorNumber, totalFloors) + totalFloors;
            }
        }
        if (distance < bestDistance) {
            bestDistance = distance;
            bestElevator = eid;
        }
    }
    return bestElevator;
}

// Helper function: constructs an elevator update message string.
std::string constructUpdateMsg(const ElevatorStatusData &data) {
    std::string msg = "ELEVATOR_UPDATE," + std::to_string(data.elevatorID) + "," +
                      std::to_string(data.destination) + "," +
                      std::string(data.travelDirection == UP ? "UP" : "DOWN") + "," +
                      std::to_string(data.currentFloor) + ",";
    msg += std::string(data.status == STANDBY ? "STANDBY" :
                       (data.status == IN_TRANSIT ? "IN_TRANSIT" : "REACHED")) + ",";
    msg += std::string(data.doorStatus == DOOR_CLOSED ? "CLOSED" :
                       (data.doorStatus == DOOR_OPENING ? "OPENING" :
                        (data.doorStatus == DOOR_OPEN ? "OPEN" : "CLOSING")));
    return msg;
}

// UDP listener: receives messages from Floor and Elevator processes.
void udpListener(Scheduler* scheduler, int sock) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in senderAddr;
    socklen_t senderLen = sizeof(senderAddr);
    while (true) {
        int recvLen = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr*)&senderAddr, &senderLen);
        if (recvLen > 0) {
            buffer[recvLen] = '\0';
            std::string msg(buffer);
            DEBUG_LOG_COLOR("Scheduler: Received UDP message", 2);
            // Process a floor request.
            if (msg.find("FLOOR_REQUEST") == 0) {
                // Expected format: FLOOR_REQUEST,<timestamp>,<floor>,<direction>
                size_t pos1 = msg.find(",");
                size_t pos2 = msg.find(",", pos1 + 1);
                size_t pos3 = msg.find(",", pos2 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
                    int floor = std::stoi(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                    std::string directionStr = msg.substr(pos3 + 1);
                    DEBUG_LOG_COLOR(("Scheduler: Received floor request for floor " + std::to_string(floor) +
                                     " with direction " + directionStr).c_str(), 3);
                    Task floorTask(FLOOR_CALL, floor, (directionStr == "UP" ? UP : DOWN), -1);
                    scheduler->addElevatorRequest(floorTask);
                    DEBUG_LOG_COLOR(("Scheduler: Task queue size after floor request: " + std::to_string(scheduler->getQueueSize())).c_str(), 3);
                }
            }
            // Process an elevator update.
            else if (msg.find("ELEVATOR_UPDATE") == 0) {
                // Expected format: ELEVATOR_UPDATE,<elevatorID>,<destination>,<direction>,<currentFloor>,<status>,<doorStatus>
                size_t pos1 = msg.find(",");
                size_t pos2 = msg.find(",", pos1 + 1);
                size_t pos3 = msg.find(",", pos2 + 1);
                size_t pos4 = msg.find(",", pos3 + 1);
                size_t pos5 = msg.find(",", pos4 + 1);
                size_t pos6 = msg.find(",", pos5 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos &&
                    pos4 != std::string::npos && pos5 != std::string::npos && pos6 != std::string::npos) {
                    int elevatorID = std::stoi(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    int destination = std::stoi(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                    std::string dirStr = msg.substr(pos3 + 1, pos4 - pos3 - 1);
                    Direction dir = (dirStr == "UP") ? UP : DOWN;
                    int currentFloor = std::stoi(msg.substr(pos4 + 1, pos5 - pos4 - 1));
                    std::string statusStr = msg.substr(pos5 + 1, pos6 - pos5 - 1);
                    ElevatorStatus status = (statusStr == "STANDBY") ? STANDBY :
                                              (statusStr == "IN_TRANSIT" ? IN_TRANSIT : REACHED);
                    std::string doorStr = msg.substr(pos6 + 1);
                    DoorStatus door = (doorStr == "CLOSED") ? DOOR_CLOSED :
                                      (doorStr == "OPENING" ? DOOR_OPENING :
                                      (doorStr == "OPEN" ? DOOR_OPEN : DOOR_CLOSING));
                    ElevatorStatusData data(elevatorID, destination, dir, currentFloor, status, door);
                    DEBUG_LOG_COLOR(("Scheduler: Recording elevator update from elevator " + std::to_string(elevatorID)).c_str(), 4);
                    scheduler->recordElevatorUpdate(data);
                    
                    // Forward the update to the Floor process.
                    std::string statusMsg = "ELEVATOR_STATUS," + std::to_string(elevatorID) + "," +
                                            std::to_string(destination) + "," + (dir == UP ? "UP" : "DOWN") + "," +
                                            std::to_string(currentFloor) + "," +
                                            (status == STANDBY ? "STANDBY" : (status == IN_TRANSIT ? "IN_TRANSIT" : "REACHED")) + "," +
                                            (door == DOOR_CLOSED ? "CLOSED" :
                                             (door == DOOR_OPENING ? "OPENING" : (door == DOOR_OPEN ? "OPEN" : "CLOSING")));
                    struct sockaddr_in floorAddr;
                    memset(&floorAddr, 0, sizeof(floorAddr));
                    floorAddr.sin_family = AF_INET;
                    floorAddr.sin_port = htons(FLOOR_PORT);
                    inet_pton(AF_INET, "127.0.0.1", &floorAddr.sin_addr);
                    sendto(sock, statusMsg.c_str(), statusMsg.size(), 0,
                           (struct sockaddr*)&floorAddr, sizeof(floorAddr));
                }
            }
        }
    }
}

// Updated taskAssigner using a C-scan algorithm and non-blocking task fetch.
void taskAssigner(Scheduler* scheduler, int sock) {
    while(true) {
         auto statuses = scheduler->getElevatorStatuses();
         DEBUG_LOG_COLOR(("Scheduler: Current elevator statuses count: " + std::to_string(statuses.size())).c_str(), 2);
         
         size_t queueSize = scheduler->getQueueSize();
         DEBUG_LOG_COLOR(("Scheduler: Task queue size: " + std::to_string(queueSize)).c_str(), 2);
         
         // While there is at least one task in the queue, try to assign it.
         while(queueSize > 0) {
             Task task;
             try {
                 task = scheduler->fetchNextTask();
                 DEBUG_LOG_COLOR(("Scheduler: Fetched task for floor " + std::to_string(task.floorNumber)).c_str(), 3);
             } catch(const std::exception &e) {
                 DEBUG_LOG_COLOR("Scheduler: No task available in non-blocking fetch", 2);
                 break;
             }
             
             // Select an elevator using our C-scan algorithm.
             int chosenElevator = selectElevatorForTask(task, statuses, 22);
             if(chosenElevator == -1) {
                 DEBUG_LOG_COLOR("Scheduler: No elevator available for assignment; re-adding task.", 4);
                 scheduler->addElevatorRequest(task);
                 break;
             }
             DEBUG_LOG_COLOR(("Scheduler: Selected elevator " + std::to_string(chosenElevator) +
                              " for floor " + std::to_string(task.floorNumber)).c_str(), 3);
             
             int elevatorPort = 5001 + chosenElevator;
             struct sockaddr_in elevAddr;
             memset(&elevAddr, 0, sizeof(elevAddr));
             elevAddr.sin_family = AF_INET;
             elevAddr.sin_port = htons(elevatorPort);
             inet_pton(AF_INET, "127.0.0.1", &elevAddr.sin_addr);
             std::string assignMsg = "TASK_ASSIGN," + std::to_string(task.floorNumber) + "," +
                                     (task.direction == UP ? "UP" : "DOWN");
             DEBUG_LOG_COLOR(("Scheduler: Sending TASK_ASSIGN to elevator " + std::to_string(chosenElevator) +
                              " on port " + std::to_string(elevatorPort) + " with message: " + assignMsg).c_str(), 5);
             int ret = sendto(sock, assignMsg.c_str(), assignMsg.size(), 0,
                               (struct sockaddr*)&elevAddr, sizeof(elevAddr));
             if(ret < 0) {
                 DEBUG_LOG_COLOR(("Scheduler: Failed to send TASK_ASSIGN to elevator " + std::to_string(chosenElevator) +
                                  ", re-adding task to queue.").c_str(), 4);
                 scheduler->addElevatorRequest(task);
                 break;
             }
             queueSize = scheduler->getQueueSize();
         }
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    TaskQueue taskQueue;
    Scheduler scheduler(&taskQueue);
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        DEBUG_LOG_COLOR("Failed to create UDP socket.", 4);
        return 1;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SCHEDULER_PORT);
    
    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        DEBUG_LOG_COLOR("Failed to bind UDP socket.", 4);
        return 1;
    }
    
    std::thread listenerThread(udpListener, &scheduler, sock);
    std::thread assignerThread(taskAssigner, &scheduler, sock);
    
    // Run the ncurses GUI.
    ncursesGUI(&scheduler);
    
    listenerThread.join();
    assignerThread.join();
    close(sock);
    return 0;
}
