// floor_main.cpp
#define USE_NCURSES_LOGGING

#include "Floor.h"
#include "Globals.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SCHEDULER_PORT 5000
#define FLOOR_PORT 5001
#define BUFFER_SIZE 1024

// Helper: sends a UDP floor request to the Scheduler.
void sendFloorRequest(int sock, const std::string& msg) {
    struct sockaddr_in schedAddr;
    memset(&schedAddr, 0, sizeof(schedAddr));
    schedAddr.sin_family = AF_INET;
    schedAddr.sin_port = htons(SCHEDULER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &schedAddr.sin_addr);
    sendto(sock, msg.c_str(), msg.size(), 0,
           (struct sockaddr*)&schedAddr, sizeof(schedAddr));
}

int main() {
    std::cout << "Floor process started." << std::endl;
    Floor floor(nullptr);
    
    int udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpSock < 0) {
        std::cerr << "Floor: Failed to create UDP socket." << std::endl;
        return 1;
    }
    
    // Bind to FLOOR_PORT for receiving updates.
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(FLOOR_PORT);
    if(bind(udpSock, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        std::cerr << "Floor: Failed to bind UDP socket on port " << FLOOR_PORT << std::endl;
        return 1;
    }
    
    // Thread to read from input.txt and send floor requests.
    std::thread requestThread([&](){
        std::ifstream inputFile("input.txt");
        if(!inputFile.is_open()) {
            std::cerr << "Floor: Unable to open input.txt" << std::endl;
            return;
        }
        std::string line;
        while (std::getline(inputFile, line)) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::stringstream ss(line);
            std::string timestamp, floorStr, directionStr, faultFlag = "none";
            std::getline(ss, timestamp, ',');
            std::getline(ss, floorStr, ',');
            std::getline(ss, directionStr, ',');
            if(std::getline(ss, faultFlag, ',')) {
                faultFlag.erase(faultFlag.begin(), std::find_if(faultFlag.begin(), faultFlag.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            }
            auto trim = [](std::string &s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(), s.end());
            };
            trim(timestamp);
            trim(floorStr);
            trim(directionStr);
            trim(faultFlag);
            Direction dir = (directionStr == "up" || directionStr == "UP") ? UP : DOWN;
            std::string reqMsg = "FLOOR_REQUEST," + timestamp + "," + floorStr + "," + (dir == UP ? "UP" : "DOWN") + "," + faultFlag;
            std::cout << "Floor: Sending floor request: " << reqMsg << std::endl;
            DEBUG_LOG_COLOR("Floor: Sending floor request", 2);
            sendFloorRequest(udpSock, reqMsg);
        }
        inputFile.close();
    });
    
    // Thread to listen for elevator status updates.
    std::thread udpListener([&](){
        char buffer[BUFFER_SIZE];
        struct sockaddr_in senderAddr;
        socklen_t senderLen = sizeof(senderAddr);
        while(true) {
            int recvLen = recvfrom(udpSock, buffer, BUFFER_SIZE-1, 0,
                                    (struct sockaddr*)&senderAddr, &senderLen);
            if(recvLen > 0) {
                buffer[recvLen] = '\0';
                std::string msg(buffer);
                if(msg.find("ELEVATOR_STATUS") == 0) {
                    std::cout << "Floor received update: " << msg << std::endl;
                    DEBUG_LOG_COLOR("Floor: Received elevator status update", 1);
                }
            }
        }
    });
    
    requestThread.join();
    udpListener.join();
    close(udpSock);
    return 0;
}
