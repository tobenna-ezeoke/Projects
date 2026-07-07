#include "Floor.h"
#include "Globals.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>

using namespace std;

Floor::Floor(Scheduler* sched) : scheduler(sched), floorTask(FLOOR_CALL, 0, UP, 0) {}

std::chrono::system_clock::time_point Floor::parseTime(const std::string& timestamp) {
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        {
            ostringstream oss;
            oss << "!!! [Floor] Failed to parse time: \"" << timestamp 
                << "\". Using current time.";
            // Red message → pair 4
            DEBUG_LOG_COLOR(oss.str(), 4);
        }
        return std::chrono::system_clock::now();
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

void Floor::sendElevatorRequest(const Task& task) {
    scheduler->addElevatorRequest(task);
}

void Floor::pressCallButton() {
    ifstream inputFile("input.txt");
    string line;
    while (getline(inputFile, line)) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        stringstream ss(line);
        string timestamp, floorStr, directionStr;
        getline(ss, timestamp, ',');
        getline(ss, floorStr, ',');
        getline(ss, directionStr, ',');
        auto trim = [](string& str) {
            str.erase(str.begin(), find_if(str.begin(), str.end(), [](unsigned char ch) { return !isspace(ch); }));
            str.erase(find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !isspace(ch); }).base(), str.end());
        };
        trim(timestamp);
        trim(floorStr);
        trim(directionStr);
        Direction dir = (directionStr == "up") ? UP : DOWN;
        int elevatorID = 0;
        floorTask = Task(FLOOR_CALL, stoi(floorStr), dir, elevatorID);
        floorTask.timestamp = parseTime(timestamp);
        {
            ostringstream oss;
            oss << "=== [Floor] Request: Call to Floor " 
                << floorTask.floorNumber << " (" 
                << (floorTask.direction == UP ? "UP" : "DOWN") 
                << ") at " << timestamp << " ===";
            // Cyan message → pair 1
            DEBUG_LOG_COLOR(oss.str(), 1);
        }
        sendElevatorRequest(floorTask);
    }
    inputFile.close();
}

void Floor::updateDisplay() {
    ElevatorStatusData elevator = scheduler->retrieveElevatorUpdate();
    if (elevator.status == REACHED) {
        {
            ostringstream oss;
            oss << "+++ [Floor Display] Updating... +++";
            // Green message → pair 3
            DEBUG_LOG_COLOR(oss.str(), 3);
        }
    }
}
