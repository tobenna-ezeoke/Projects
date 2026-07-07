// elevator_main.cpp
#include "Elevator.h"
#include "Globals.h"
#include "TaskQueue.h" // not used in UDP mode
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <exception>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

#define SCHEDULER_PORT 5000
#define BUFFER_SIZE 1024

// Helper: sends a UDP message to the Scheduler.
void sendElevatorUpdate(int sock, const std::string &msg) {
    struct sockaddr_in schedAddr;
    memset(&schedAddr, 0, sizeof(schedAddr));
    schedAddr.sin_family = AF_INET;
    schedAddr.sin_port = htons(SCHEDULER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &schedAddr.sin_addr);
    sendto(sock, msg.c_str(), msg.size(), 0,
           (struct sockaddr*)&schedAddr, sizeof(schedAddr));
}

// Helper: constructs an update message string from elevator status.
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

// Helper: constructs a "task discarded" message.
std::string constructDiscardMsg(const Task &task) {
    return "TASK_DISCARDED," + std::to_string(task.floorNumber) + "," +
           (task.direction == UP ? "UP" : "DOWN");
}

// Global task queue for this elevator and its synchronization primitives.
std::queue<Task> localTaskQueue;
std::mutex taskQueueMutex;
std::condition_variable taskQueueCV;

// Helper function to log current elevator status.
void logElevatorStatus(const Elevator &elevator) {
    ElevatorStatusData data = elevator.retrieveElevatorStatus();
    std::string statusMsg = "Elevator Status [ID: " + std::to_string(data.elevatorID) +
                            " | Curr: " + std::to_string(data.currentFloor) +
                            " | Dest: " + std::to_string(data.destination) +
                            " | Dir: " + (data.travelDirection == UP ? "UP" : "DOWN") +
                            " | Status: " + (data.status == STANDBY ? "STANDBY" :
                                              (data.status == IN_TRANSIT ? "IN_TRANSIT" : "REACHED")) +
                            " | Door: " + (data.doorStatus == DOOR_CLOSED ? "CLOSED" :
                                            (data.doorStatus == DOOR_OPENING ? "OPENING" :
                                             (data.doorStatus == DOOR_OPEN ? "OPEN" : "CLOSING"))) + "]";
    std::cout << statusMsg << std::endl;
}

// Helper function to log the current local task queue size.
void logTaskQueueSize() {
    std::lock_guard<std::mutex> lock(taskQueueMutex);
    std::string msg = "Local Task Queue Size: " + std::to_string(localTaskQueue.size());
    std::cout << msg << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: elevator_main <elevatorID>" << std::endl;
        return 1;
    }
    int elevatorID = std::atoi(argv[1]);
    std::cout << "Elevator process started with elevatorID: " << elevatorID << std::endl;
    
    // Create Elevator object in UDP mode (scheduler and taskQueue are nullptr).
    Elevator elevator(elevatorID, 22, nullptr, nullptr);
    
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) {
        std::cout << "Elevator: Failed to create UDP socket." << std::endl;
        return 1;
    }
    
    // Bind to a unique port for this elevator (5001 + elevatorID).
    int elevatorPort = 5001 + elevatorID;
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(elevatorPort);
    if (bind(udpSock, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        std::cout << "Elevator: Failed to bind UDP socket on port " << elevatorPort << std::endl;
        return 1;
    }
    
    // Send an initial status update.
    ElevatorStatusData initData = elevator.retrieveElevatorStatus();
    std::string initMsg = constructUpdateMsg(initData);
    std::cout << "Elevator " << elevatorID << " sending initial update: " << initMsg << std::endl;
    sendElevatorUpdate(udpSock, initMsg);
    
    // Global polling thread: continuously sends status updates every 500ms.
    std::atomic<bool> keepPolling{true};
    std::thread globalPoller([&]() {
        while (keepPolling.load()) {
            logElevatorStatus(elevator);
            ElevatorStatusData data = elevator.retrieveElevatorStatus();
            std::string updateMsg = constructUpdateMsg(data);
            sendElevatorUpdate(udpSock, updateMsg);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    globalPoller.detach();
    
    // Worker thread to process tasks from the local task queue.
    std::thread taskWorker([&]() {
        while (true) {
            Task currentTask;
            {
                std::unique_lock<std::mutex> lock(taskQueueMutex);
                taskQueueCV.wait(lock, [&]() { return !localTaskQueue.empty(); });
                currentTask = localTaskQueue.front();
                localTaskQueue.pop();
            }
            logTaskQueueSize();
            // If the task is redundant (already at current floor), discard it.
            if (currentTask.floorNumber == elevator.getCurrentLevel()) {
                std::cout << "Elevator " << elevatorID << ": Discarding task for floor " 
                          << currentTask.floorNumber << " (already reached)" << std::endl;
                std::string discardMsg = constructDiscardMsg(currentTask);
                sendElevatorUpdate(udpSock, discardMsg);
                continue;
            }
            std::cout << "Elevator " << elevatorID << " processing queued task for floor " 
                      << currentTask.floorNumber << std::endl;
            try {
                elevator.processRequest(currentTask);
                elevator.moveElevator();
            } catch (const std::exception &e) {
                std::cout << "Elevator " << elevatorID << ": Exception while processing task: " 
                          << e.what() << std::endl;
                std::string discardMsg = constructDiscardMsg(currentTask);
                sendElevatorUpdate(udpSock, discardMsg);
                continue;
            }
            ElevatorStatusData data = elevator.retrieveElevatorStatus();
            std::string finalMsg = constructUpdateMsg(data);
            std::cout << "Elevator " << elevatorID << " sending final update: " << finalMsg << std::endl;
            sendElevatorUpdate(udpSock, finalMsg);
        }
    });
    taskWorker.detach();
    
    // UDP listener thread: listens for TASK_ASSIGN messages and enqueues tasks.
    std::thread udpListener([&]() {
        char buffer[BUFFER_SIZE];
        struct sockaddr_in senderAddr;
        socklen_t senderLen = sizeof(senderAddr);
        while (true) {
            int recvLen = recvfrom(udpSock, buffer, BUFFER_SIZE - 1, 0,
                                    (struct sockaddr*)&senderAddr, &senderLen);
            if (recvLen > 0) {
                buffer[recvLen] = '\0';
                std::string msg(buffer);
                std::cout << "Elevator " << elevatorID << " received UDP message: " << msg << std::endl;
                if (msg.find("TASK_ASSIGN") == 0) {
                    size_t pos1 = msg.find(",");
                    size_t pos2 = msg.find(",", pos1 + 1);
                    if (pos1 != std::string::npos && pos2 != std::string::npos) {
                        int floor = std::stoi(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                        std::string dirStr = msg.substr(pos2 + 1);
                        std::cout << "Elevator " << elevatorID << " enqueuing task: Floor " 
                                  << floor << " Direction " << dirStr << std::endl;
                        Task newTask(FLOOR_CALL, floor, (dirStr == "UP" ? UP : DOWN), elevatorID);
                        {
                            std::lock_guard<std::mutex> lock(taskQueueMutex);
                            localTaskQueue.push(newTask);
                        }
                        logTaskQueueSize();
                        taskQueueCV.notify_one();
                    }
                }
            }
        }
    });
    
    udpListener.join();
    close(udpSock);
    return 0;
}
