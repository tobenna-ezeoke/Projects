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

// Constructor initializes Floor with a Scheduler instance and a default floorTask
Floor::Floor(Scheduler* sched) : scheduler(sched), floorTask(FLOOR_CALL, 0, UP, 0) {}

// Parses a timestamp string into a chrono time_point object
std::chrono::system_clock::time_point Floor::parseTime(const std::string& timestamp) {
    std::tm tm = {};
    std::istringstream ss(timestamp);
    // Attempt to parse the timestamp using the format "YYYY-MM-DD HH:MM:SS"
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        // If parsing fails, log an error message and return current time as fallback
        {
            ostringstream oss;
            oss << "!!! [Floor] Failed to parse time: \"" << timestamp 
                << "\". Using current time.";
            DEBUG_LOG_COLOR(oss.str(), 4);
        }
        return std::chrono::system_clock::now();
    }
    // Convert parsed time to system_clock time_point
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

// Sends an elevator request task to the scheduler
void Floor::sendElevatorRequest(const Task& task) {
    scheduler->addElevatorRequest(task);
}

// Simulates pressing the elevator call button by reading requests from input file
void Floor::pressCallButton() {
    // Open input file containing elevator requests
    std::ifstream inputFile("input.txt");
    std::string line;

    // Read each line (request) from the file sequentially
    while (std::getline(inputFile, line)) {
        // Simulate delay between button presses (2 seconds)
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Parse the current line into components: timestamp, floor number, direction, fault flag
        std::stringstream ss(line);
        std::string timestamp, floorStr, directionStr, faultFlag = "none";

        // Extract comma-separated values from the line
        std::getline(ss, timestamp, ',');
        std::getline(ss, floorStr, ',');
        std::getline(ss, directionStr, ',');

        // Check if there's an optional fault flag after direction
        if (std::getline(ss, faultFlag, ',')) {
            // Remove leading spaces from faultFlag if present
            faultFlag.erase(faultFlag.begin(), std::find_if(faultFlag.begin(), faultFlag.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        }

        // Lambda function to trim whitespace from strings (both ends)
        auto trim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        };

        // Trim whitespace from parsed strings
        trim(timestamp);
        trim(floorStr);
        trim(directionStr);
        trim(faultFlag);

        // Determine direction enum based on parsed string ("up" or "down")
        Direction dir = (directionStr == "up") ? UP : DOWN;

        int elevatorID = 0;  // Default elevator ID (can be modified as needed)

        // Create a Task object representing this elevator call request
        floorTask = Task(FLOOR_CALL, std::stoi(floorStr), dir, elevatorID);

        // Set the timestamp of the task using parsed time
        floorTask.timestamp = parseTime(timestamp);

        // Log the elevator request details clearly for debugging purposes
        {
            ostringstream oss;
            oss << "=== [Floor] Request: Call to Floor " 
                << floorTask.floorNumber << " (" 
                << (floorTask.direction == UP ? "UP" : "DOWN") 
                << ") at " << timestamp << " [Fault: " << faultFlag << "] ===";
            DEBUG_LOG_COLOR(oss.str(), 1);
        }

        // Send this elevator request task to the scheduler for processing
        sendElevatorRequest(floorTask);
    }

    // Close input file after processing all requests
    inputFile.close();
}

// Updates the floor display based on elevator status updates received from scheduler
void Floor::updateDisplay() {
    // Retrieve latest elevator status data from scheduler
    ElevatorStatusData elevator = scheduler->retrieveElevatorUpdate();

    // Check if elevator has reached its requested floor
    if (elevator.status == REACHED) {
        // Log display update event for debugging purposes
        {
            std::ostringstream oss;
            oss << "+++ [Floor Display] Updating... +++";
            DEBUG_LOG_COLOR(oss.str(), 3);
        }
    }
}
