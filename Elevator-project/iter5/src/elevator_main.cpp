#define USE_NCURSES_LOGGING

#include "Elevator.h"
#include "Globals.h"
#include "TaskQueue.h" // Not used in UDP mode
#include <thread>
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <atomic>

#define SCHEDULER_PORT 5000
#define BUFFER_SIZE 1024
#define UPDATE_INTERVAL_MS 100
// With 5 seconds per floor, use a timeout of 30 seconds.

// #define FAULT_TIMEOUT 30  

// Simulation parameters.
const int MAX_CAPACITY = 10;
std::atomic<int> currentLoad{ 0 };  // Global simulated load

// Generates a random passenger group size between 1 and 5.
int generatePassengerCount() {
    return (rand() % 5) + 1;
}

// Helper: trim whitespace.
std::string trim(const std::string& s) {
    std::string result = s;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), result.end());
    return result;
}

// Helper: sends a UDP message to the Scheduler.
void sendElevatorUpdate(int sock, const std::string& msg) {
    struct sockaddr_in schedAddr;
    memset(&schedAddr, 0, sizeof(schedAddr));
    schedAddr.sin_family = AF_INET;
    schedAddr.sin_port = htons(SCHEDULER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &schedAddr.sin_addr);
    int sent = sendto(sock, msg.c_str(), msg.size(), 0,
        (struct sockaddr*)&schedAddr, sizeof(schedAddr));
    if (sent < 0) {
        std::cerr << "[Elevator] Error sending update: " << strerror(errno) << std::endl;
    } else {
        // std::cout << "[Elevator] Sent update: " << msg << std::endl;
    }
}

// Helper: constructs an update message string (including load, capacity, and local task queue size).
std::string constructUpdateMsg(const ElevatorStatusData& data, int currentLoad, int maxCapacity) {
    std::ostringstream oss;
    oss << data.elevatorID << ","
        << data.destination << ","
        << (data.travelDirection == UP ? "UP" : "DOWN") << ","
        << data.currentFloor << ","
        << (data.status == STANDBY ? "STANDBY" :
            (data.status == IN_TRANSIT ? "IN_TRANSIT" :
            (data.status == REACHED ? "REACHED" :
            (data.status == PFAULT ? "PFAULT" : "FAULT")))) << ","
            << (data.doorStatus == DOOR_CLOSED ? "CLOSED" :
                (data.doorStatus == DOOR_OPENING ? "OPENING" :
                (data.doorStatus == DOOR_OPEN ? "OPEN" :
                (data.doorStatus == DOOR_CLOSING ? "CLOSING" : "STUCK")))) << ","
            
        << currentLoad << ","
        << maxCapacity << ","
        << data.queueSize;
    return oss.str();
}


// Forward declaration for simulateMovement.
void simulateMovement(Elevator& elevator, int udpSock);

// Helper: sends a TASK_RETURN message for each pending task in the elevator's internal queue.
void sendTaskReturns(Elevator& elevator, int udpSock) {
    std::vector<ElevatorTask> pending = elevator.getLocalTaskQueue();
    for (const auto& et : pending) {
        std::ostringstream oss;
        oss << "TASK_RETURN," << et.task.floorNumber << ","
            << (et.task.direction == UP ? "UP" : "DOWN");
        std::string returnMsg = oss.str();
        sendElevatorUpdate(udpSock, returnMsg);
        std::cout << "[Elevator " << elevator.getID() << "] Returning task: " << returnMsg << std::endl;
    }
    // Clear the elevator's internal task queue.
    elevator.clearTaskQueue();
}

// Global flag for fault injection is now per-elevator (member variable in Elevator)

// Main processing loop: continuously process tasks from the internal queue.
void processTaskLoop(Elevator& elevator, int udpSock) {
    while (true) {
        
        if (!elevator.getLocalTaskQueue().empty()) {
            simulateMovement(elevator, udpSock);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
    }
}

// UDP receiver thread: listens for TASK_ASSIGN messages and adds tasks to the elevator's queue.
void udpReceiverThread(Elevator& elevator, int udpSock) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in senderAddr;
    socklen_t senderLen = sizeof(senderAddr);
    while (true) {
        int len = recvfrom(udpSock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&senderAddr, &senderLen);
        if (len > 0) {
            buffer[len] = '\0';
            std::string msg(buffer);
            std::cout << "[Elevator " << elevator.getID() << "] Received message: " << msg << std::endl;
            if (msg.find("TASK_ASSIGN") == 0) {
                // Expected format: TASK_ASSIGN,<targetFloor>,<direction>[,<faultFlag>]
                size_t pos1 = msg.find(",");
                size_t pos2 = msg.find(",", pos1 + 1);
                std::string directionStr, faultFlag = "none";
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    int targetFloor = std::stoi(msg.substr(pos1 + 1, pos2 - pos1 - 1));
                    size_t pos3 = msg.find(",", pos2 + 1);
                    if (pos3 == std::string::npos) {
                        directionStr = msg.substr(pos2 + 1);
                    } else {
                        directionStr = msg.substr(pos2 + 1, pos3 - pos2 - 1);
                        faultFlag = trim(msg.substr(pos3 + 1));
                    }

                    std::cout << "[Elevator " << elevator.getID() << "] TASK_ASSIGN: Floor "
                        << targetFloor << ", Direction " << directionStr
                        << ", FaultFlag: " << faultFlag << std::endl;

                    // If elevator is busy moving, add the task to the internal queue.
                    if (elevator.checkStatus() == IN_TRANSIT ) {
                        ElevatorTask newTask;
                        newTask.task = Task(FLOOR_CALL, targetFloor, (directionStr == "UP" ? UP : DOWN), elevator.getID());
                        newTask.passengerCount = generatePassengerCount();
                        elevator.addTaskToQueue(newTask);
                        std::cout << "[Elevator " << elevator.getID() << "] Busy. Queued task. Internal Queue Size: "
                            << elevator.getLocalTaskQueue().size() << std::endl;
                        continue;
                    }

                    // Check capacity.
                    if (currentLoad >= MAX_CAPACITY ) {
                        std::cout << "[Elevator " << elevator.getID() << "] Full. Sending CAPACITY_FULL." << std::endl;
                        std::ostringstream capOss;
                        capOss << "CAPACITY_FULL," << targetFloor << "," << directionStr;
                        sendElevatorUpdate(udpSock, capOss.str());
                        continue;
                    }

                    int groupSize = generatePassengerCount();
                    if (currentLoad + groupSize > MAX_CAPACITY) {
                        std::cout << "[Elevator " << elevator.getID() << "] Cannot board group of size "
                            << groupSize << ". Sending CAPACITY_FULL." << std::endl;
                        std::ostringstream capOss;
                        capOss << "CAPACITY_FULL," << targetFloor << "," << directionStr;
                        sendElevatorUpdate(udpSock, capOss.str());
                        continue;
                    }

                    currentLoad += groupSize;
                    std::cout << "[Elevator " << elevator.getID() << "] " << groupSize
                        << " passengers boarded. Load = " << currentLoad << std::endl;

                    ElevatorTask newTask;
                    newTask.task = Task(FLOOR_CALL, targetFloor, (directionStr == "UP" ? UP : DOWN), elevator.getID());
                    newTask.passengerCount = groupSize;
                    elevator.addTaskToQueue(newTask);
                    std::cout << "[Elevator " << elevator.getID() << "] Internal Queue Size: "
                        << elevator.getLocalTaskQueue().size() << std::endl;

                    // Process fault injection.
                    if (faultFlag == "timeout")
                        elevator.faultInjectedTimeout.store(true);
                    if (faultFlag == "door") 
                        elevator.faultDoorTimeout.store(true);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// simulateMovement: processes tasks, moves floor-by-floor, performs door operations,
// and stops if a timeout fault is injected.
void simulateMovement(Elevator& elevator, int udpSock) {
    while (!elevator.getLocalTaskQueue().empty()) {
        ElevatorTask currentTask = elevator.getLocalTaskQueue().front();
        std::cout << "[Elevator " << elevator.getID() << "] Processing task: Floor "
            << currentTask.task.floorNumber << " with group size "
            << currentTask.passengerCount << ". Queue Size: "
            << elevator.getLocalTaskQueue().size() << std::endl;
        // Remove the processed task.
        std::vector<ElevatorTask> temp = elevator.getLocalTaskQueue();
        temp.erase(temp.begin());
        elevator.clearTaskQueue();
        for (const auto& t : temp)
            elevator.addTaskToQueue(t);

        elevator.setDestination(currentTask.task.floorNumber);
        auto transitStart = std::chrono::steady_clock::now();

        if (elevator.getCurrentLevel() == elevator.retrieveElevatorStatus().destination) {
            elevator.updateStatus(STANDBY);
            std::cout << "[Elevator " << elevator.getID() << "] Warning: Loopback detected." << std::endl;
            continue;
        }

        // Move floor-by-floor.
        while (elevator.getCurrentLevel() != elevator.retrieveElevatorStatus().destination) {
            std::cout << "Elevator has been moving for "
                << std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - transitStart).count())
                << " seconds" << std::endl;

            if (std::chrono::steady_clock::now() - transitStart > std::chrono::seconds(FAULT_TIMEOUT_SECONDS)) {
                elevator.updateStatus(FAULT);
                ElevatorStatusData faultData = elevator.retrieveElevatorStatus();
                std::string faultMsg = "ELEVATOR_UPDATE," + constructUpdateMsg(faultData, currentLoad, MAX_CAPACITY);
                sendElevatorUpdate(udpSock, faultMsg);
                // Before exiting, send back pending tasks.
                sendTaskReturns(elevator, udpSock);
                std::cerr << "[Elevator " << elevator.getID() << "] Fault: Movement timeout. Shutting down." << std::endl;
                exit(1);
            }
            if (elevator.faultInjectedTimeout.load()) {
                elevator.updateStatus(PFAULT);
                std::cout << "[Elevator " << elevator.getID() << "] Fault injection: timeout activated. Stopping movement." << std::endl;
                // Before exiting, send back pending tasks.
                // sendTaskReturns(elevator, udpSock);
                // return;
                // sendElevatorUpdate(udpSock, faultMsg);
                // exit(1);
            }
            elevator.updateStatus(IN_TRANSIT);
            elevator.informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_TRAVEL_TIME));
            if (elevator.faultInjectedTimeout.load()) {
                elevator.updateStatus(PFAULT);
                std::this_thread::sleep_for(std::chrono::seconds(FAULT_TIMEOUT_SECONDS));
            }
            if (elevator.getCurrentLevel() < elevator.retrieveElevatorStatus().destination)
                elevator.incrementFloor();
            else
                elevator.decrementFloor();
            std::cout << "[Elevator " << elevator.getID() << "] Moving: now at floor " << elevator.getCurrentLevel() << std::endl;
        }
        std::cout << "[Elevator " << elevator.getID() << "] Reached floor " << elevator.getCurrentLevel() << std::endl;
        elevator.informScheduler();

        // Door operations.
        elevator.updateStatus(REACHED);
        std::cout << "### [Elevator " << elevator.getID() << "] Reached target floor " << elevator.getCurrentLevel()
            << ". Starting door operations. ###" << std::endl;
        elevator.setDoorStatus(DOOR_OPENING);
        std::cout << ">>> [Elevator " << elevator.getID() << "] Doors opening..." << std::endl;
        elevator.informScheduler();
        std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_OPEN_TIME));

        elevator.setDoorStatus(DOOR_OPEN);
        std::cout << ">>> [Elevator " << elevator.getID() << "] Doors open." << std::endl;
        elevator.informScheduler();
        std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_LOADING_TIME));

        // Unload passengers.
        std::cout << "[Elevator " << elevator.getID() << "] Unloading " << currentTask.passengerCount
            << " passengers at floor " << elevator.getCurrentLevel() << "." << std::endl;
        currentLoad -= currentTask.passengerCount;
        if (currentLoad < 0)
            currentLoad = 0;


        elevator.setDoorStatus(DOOR_CLOSING);
        std::cout << ">>> [Elevator " << elevator.getID() << "] Doors closing..." << std::endl;
        elevator.informScheduler();
        

        if (elevator.faultDoorTimeout.load()) {
            elevator.setDoorStatus(DOOR_STUCK);
            std::cout << ">>> [Elevator " << elevator.getID() << "] Door stuck!" << std::endl;
            elevator.informScheduler();
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_STUCK_TIME));
            //DOOR STUCK!
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(ELEVATOR_DOOR_CLOSE_TIME));
        }
        elevator.setDoorStatus(DOOR_CLOSED);
        elevator.faultDoorTimeout.store(false);
        std::cout << ">>> [Elevator " << elevator.getID() << "] Doors closed." << std::endl;
        elevator.updateStatus(STANDBY);
        elevator.informScheduler();


    }
}

//
// Main function for elevator_main.cpp
//
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: elevator_main <elevatorID>" << std::endl;
        return 1;
    }
    int elevatorID = std::atoi(argv[1]);
    std::cout << "[Elevator " << elevatorID << "] Process started." << std::endl;
    srand(time(nullptr) + elevatorID);

    // Create Elevator object (UDP mode; scheduler and taskQueue are nullptr).
    Elevator elevator(elevatorID, 22, nullptr, nullptr);
    elevator.setMaxCapacity(MAX_CAPACITY);
    elevator.setCurrentLoad(currentLoad);

    // Create UDP socket and bind to unique port: 5001 + elevatorID.
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) {
        std::cerr << "[Elevator " << elevatorID << "] Failed to create UDP socket." << std::endl;
        return 1;
    }
    int elevatorPort = 5001 + elevatorID;
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(elevatorPort);
    if (bind(udpSock, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        std::cerr << "[Elevator " << elevatorID << "] Failed to bind UDP socket on port " << elevatorPort << std::endl;
        return 1;
    }

    // Send initial update to Scheduler.
    ElevatorStatusData initData = elevator.retrieveElevatorStatus();
    std::string initMsg = "ELEVATOR_UPDATE," + constructUpdateMsg(initData, currentLoad, MAX_CAPACITY);
    std::cout << "[Elevator " << elevatorID << "] Sending initial update: " << initMsg << std::endl;
    sendElevatorUpdate(udpSock, initMsg);

    // Start periodic update thread (every 100 ms).
    std::thread updateThread([&]() {
        while (true) {
            ElevatorStatusData currentData = elevator.retrieveElevatorStatus();
            std::string updateMsg = "ELEVATOR_UPDATE," + constructUpdateMsg(currentData, currentLoad, MAX_CAPACITY);
            sendElevatorUpdate(udpSock, updateMsg);
            std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_INTERVAL_MS));
        }
        });
    updateThread.detach();

    // Start UDP receiver thread.
    std::thread receiverThread(udpReceiverThread, std::ref(elevator), udpSock);
    receiverThread.detach();

    // Main processing loop: continuously process tasks from the internal queue.
    processTaskLoop(elevator, udpSock);

    close(udpSock);
    return 0;
}
