#define USE_NCURSES_LOGGING

#include "Scheduler.h"
#include "TaskQueue.h"
#include "Globals.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ncurses.h>
#include <unordered_map>
#include <exception>

extern void ncursesGUI(Scheduler* scheduler);

#define SCHEDULER_PORT 5000
#define FLOOR_PORT 5001
#define BUFFER_SIZE 1024
#define TOTAL_FLOORS 22

// Helper: returns the wrap-around distance using C-scan logic.
int wrapDistance(int currentFloor, int requestedFloor, int totalFloors) {
    return (requestedFloor - currentFloor + totalFloors) % totalFloors;
}

// Helper: selects an elevator based on C-scan, ignoring those in FAULT state.
int selectElevatorForTask(const Task& task, const std::unordered_map<int, ElevatorStatusData>& statuses, int totalFloors = TOTAL_FLOORS) {
    int bestElevator = -1;
    int bestDistance = totalFloors + 100;
    for (const auto& entry : statuses) {
        int eid = entry.first;
        const ElevatorStatusData& esd = entry.second;
        if (esd.status == FAULT) continue;  // Skip faulty elevators.
        int distance = 0;
        if (esd.status == STANDBY) {
            distance = wrapDistance(esd.currentFloor, task.floorNumber, totalFloors);
        } else {
            if (esd.travelDirection == task.direction) {
                if (task.floorNumber >= esd.currentFloor)
                    distance = task.floorNumber - esd.currentFloor;
                else
                    distance = (totalFloors - esd.currentFloor) + task.floorNumber;
            } else {
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

// Helper: constructs an elevator update message string.
std::string constructUpdateMsg(const ElevatorStatusData& data) {
    std::ostringstream oss;
    oss << data.elevatorID << ","
        << data.destination << ","
        << (data.travelDirection == UP ? "UP" : "DOWN") << ","
        << data.currentFloor << ","
        << (data.status == STANDBY ? "STANDBY" :
            (data.status == IN_TRANSIT ? "IN_TRANSIT" : "REACHED")) << ","
        << (data.doorStatus == DOOR_CLOSED ? "CLOSED" :
            (data.doorStatus == DOOR_OPENING ? "OPENING" :
            (data.doorStatus == DOOR_OPEN ? "OPEN" :
            (data.doorStatus == DOOR_CLOSING ? "CLOSING" : "STUCK")))) << ","
        << data.currentLoad << ","
        << data.maxCapacity << ","
        << data.queueSize;
    return oss.str();
}

// UDP listener: receives messages and processes them.
// In the UDP listener function in scheduler_main.cpp:
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
            // DEBUG_LOG_COLOR("Scheduler: Received UDP message", 2);
            // Process a FLOOR_REQUEST message.
            if (msg.find("FLOOR_REQUEST") == 0) {
                size_t pos1 = msg.find(",");
                size_t pos2 = msg.find(",", pos1 + 1);
                size_t pos3 = msg.find(",", pos2 + 1);
                size_t pos4 = msg.find(",", pos3 + 1);
                std::string faultFlag = "none";
                if (pos4 != std::string::npos)
                    faultFlag = msg.substr(pos4 + 1);
                int floor = std::stoi(msg.substr(pos2 + 1, pos3 - pos2 - 1));
                std::string directionStr = msg.substr(pos3 + 1, pos4 - pos3 - 1);
                if (faultFlag != "none") {
                    std::ostringstream oss;
                    oss << "Scheduler: Fault injection detected: " << faultFlag << " for floor " << floor;
                    DEBUG_LOG_COLOR(oss.str(), 4);
                }
                Task floorTask(FLOOR_CALL, floor, (directionStr == "UP" ? UP : DOWN), -1, faultFlag);
                scheduler->addElevatorRequest(floorTask);
                DEBUG_LOG_COLOR(("Scheduler: Global Task Queue Size: " + std::to_string(scheduler->getQueueSize())).c_str(), 3);
            }
            // Process a CAPACITY_FULL message.
            else if (msg.find("CAPACITY_FULL") == 0) {
                size_t pos1 = msg.find(",");
                size_t pos2 = msg.find(",", pos1 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    int floor = std::stoi(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    std::string directionStr = msg.substr(pos2 + 1);
                    std::ostringstream oss;
                    oss << "Scheduler: Received CAPACITY_FULL for floor " << floor << " direction " << directionStr;
                    DEBUG_LOG_COLOR(oss.str(), 4);
                    Task floorTask(FLOOR_CALL, floor, (directionStr == "UP" ? UP : DOWN), -1);
                    scheduler->addElevatorRequest(floorTask);
                }
            }
            // Process a TASK_RETURN message.
            else if (msg.find("TASK_RETURN") == 0) {
                // Expected format: TASK_RETURN,<floor>,<direction>
                size_t pos1 = msg.find(",");
                size_t pos2 = msg.find(",", pos1 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    int floor = std::stoi(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    std::string directionStr = msg.substr(pos2 + 1);
                    std::ostringstream oss;
                    oss << "Scheduler: Received TASK_RETURN for floor " << floor << " direction " << directionStr;
                    DEBUG_LOG_COLOR(oss.str(), 4);
                    Task returnTask(FLOOR_CALL, floor, (directionStr == "UP" ? UP : DOWN), -1);
                    scheduler->addElevatorRequest(returnTask);
                }
            }
            // Process an ELEVATOR_UPDATE message.
            else if (msg.find("ELEVATOR_UPDATE") == 0) {
                const std::string prefix = "ELEVATOR_UPDATE,";
                std::string dataMsg = msg.substr(prefix.length());
                size_t pos1 = dataMsg.find(",");
                size_t pos2 = dataMsg.find(",", pos1 + 1);
                size_t pos3 = dataMsg.find(",", pos2 + 1);
                size_t pos4 = dataMsg.find(",", pos3 + 1);
                size_t pos5 = dataMsg.find(",", pos4 + 1);
                size_t pos6 = dataMsg.find(",", pos5 + 1);
                size_t pos7 = dataMsg.find(",", pos6 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos &&
                    pos4 != std::string::npos && pos5 != std::string::npos && pos6 != std::string::npos &&
                    pos7 != std::string::npos) {
                    int elevatorID = std::stoi(dataMsg.substr(0, pos1));
                    int destination = std::stoi(dataMsg.substr(pos1 + 1, pos2 - pos1 - 1));
                    std::string dirStr = dataMsg.substr(pos2 + 1, pos3 - pos2 - 1);
                    Direction dir = (dirStr == "UP") ? UP : DOWN;
                    int currentFloor = std::stoi(dataMsg.substr(pos3 + 1, pos4 - pos3 - 1));
                    std::string statusStr = dataMsg.substr(pos4 + 1, pos5 - pos4 - 1);
                    ElevatorStatus status = (statusStr == "STANDBY") ? STANDBY :
                    (statusStr == "IN_TRANSIT") ? IN_TRANSIT :
                    (statusStr == "REACHED") ? REACHED :
                    (statusStr == "PFAULT") ? PFAULT : FAULT;

                    std::string doorStr = dataMsg.substr(pos5 + 1, pos6 - pos5 - 1);
                    DoorStatus door = (doorStr == "STUCK") ? DOOR_STUCK :
                                      (doorStr == "CLOSED") ? DOOR_CLOSED :
                                      (doorStr == "OPENING") ? DOOR_OPENING :
                                      (doorStr == "OPEN") ? DOOR_OPEN : DOOR_CLOSING;
                    int currentLoad = std::stoi(dataMsg.substr(pos6 + 1, pos7 - pos6 - 1));
                    int maxCapacity = std::stoi(dataMsg.substr(pos7 + 1));
                    
                    ElevatorStatusData data(elevatorID, destination, dir, currentFloor, status, door, currentLoad, maxCapacity);
                    scheduler->recordElevatorUpdate(data);
                    
                    // Forward update to Floor.
                    std::string statusMsg = "ELEVATOR_STATUS," + constructUpdateMsg(data);
                    struct sockaddr_in floorAddr;
                    memset(&floorAddr, 0, sizeof(floorAddr));
                    floorAddr.sin_family = AF_INET;
                    floorAddr.sin_port = htons(FLOOR_PORT);
                    inet_pton(AF_INET, "127.0.0.1", &floorAddr.sin_addr);
                    sendto(sock, statusMsg.c_str(), statusMsg.size(), 0, (struct sockaddr*)&floorAddr, sizeof(floorAddr));
                }
            }
        }
    }
}


// Task assigner: repeatedly fetches tasks and sends TASK_ASSIGN messages.
// If sendto fails, re-add the task to the global queue.
void taskAssigner(Scheduler* scheduler, int sock) {
    while (true) {
        auto statuses = scheduler->getElevatorStatuses();
        // DEBUG_LOG_COLOR(("Scheduler: Elevator statuses count: " + std::to_string(statuses.size())).c_str(), 2);
        size_t queueSize = scheduler->getQueueSize();
        // DEBUG_LOG_COLOR(("Scheduler: Global Task Queue Size: " + std::to_string(queueSize)).c_str(), 2);
        while (queueSize > 0) {
            Task task;
            try {
                task = scheduler->fetchNextTask();
                DEBUG_LOG_COLOR(("Scheduler: Fetched task for floor " + std::to_string(task.floorNumber)).c_str(), 3);
            }
            catch (const std::exception& e) {
                DEBUG_LOG_COLOR("Scheduler: No task available in non-blocking fetch", 2);
                break;
            }
            int chosenElevator = selectElevatorForTask(task, statuses, TOTAL_FLOORS);
            if (chosenElevator == -1) {
                DEBUG_LOG_COLOR("Scheduler: No elevator available; re-adding task.", 4);
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

            DEBUG_LOG_COLOR("fault flag is" + (task.faultFlag),5);
            if (task.faultFlag != "none" && !task.faultFlag.empty()) {
                assignMsg += "," + task.faultFlag;
                DEBUG_LOG_COLOR("Fault detected and injected!!",5);
            }
            
            
            DEBUG_LOG_COLOR(("Scheduler: Sending TASK_ASSIGN to elevator " + std::to_string(chosenElevator) +
                " on port " + std::to_string(elevatorPort) + " with message: " + assignMsg).c_str(), 5);
            int ret = sendto(sock, assignMsg.c_str(), assignMsg.size(), 0, (struct sockaddr*)&elevAddr, sizeof(elevAddr));
            if (ret < 0) {
                DEBUG_LOG_COLOR(("Scheduler: Failed to send TASK_ASSIGN to elevator " + std::to_string(chosenElevator) +
                    ", re-adding task.").c_str(), 4);
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
    if (sock < 0) {
        DEBUG_LOG_COLOR("Failed to create UDP socket.", 4);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SCHEDULER_PORT);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        DEBUG_LOG_COLOR("Failed to bind UDP socket.", 4);
        return 1;
    }

    std::thread listenerThread(udpListener, &scheduler, sock);
    std::thread assignerThread(taskAssigner, &scheduler, sock);

    // Launch the ncurses GUI.
    ncursesGUI(&scheduler);

    listenerThread.join();
    assignerThread.join();
    close(sock);
    return 0;
}
