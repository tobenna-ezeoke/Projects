#include "FloorSubsystem.h"
#include <fstream> // For file handling
#include <sstream> // For string stream processing
#include <iostream> // For debugging/logging
#include <algorithm>

namespace src {

FloorSubsystem::FloorSubsystem() : fileName("input.txt") {}

/**
 * Parse the default data from input.txt.
 */
void FloorSubsystem::parseData() {
    parseData(fileName);
}

/**
 * Parse the data received in the input file.
 */
void FloorSubsystem::parseData(const std::string& fileName) {
    this->fileName = fileName;
    std::ifstream file(fileName);

    // Make sure the file can be opened
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << fileName << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string timeStr, directionStr;
        int floorNumber, carButton;
        bool direction = false;

        // Parse the line
        if (!(iss >> timeStr >> floorNumber >> directionStr >> carButton)) {
            std::cerr << "Error: Malformed line in input file: " << line << std::endl;
            continue;
        }

        // Determine direction
        if (directionStr == "Up") {
            direction = true;
        } else if (directionStr == "Down") {
            direction = false;
        } else {
            std::cerr << "Error: Invalid direction in line: " << line << std::endl;
            continue;
        }

        // Convert time string to time_t (placeholder, time parsing can be expanded)
        std::time_t buttonTime = std::time(nullptr); // Placeholder for real time parsing

        // Create and add the ButtonPress object
        auto buttonPress = std::make_shared<ButtonPress>(direction, carButton, floorNumber, buttonTime);
        info.push_back(buttonPress);
    }

    file.close();
}

/**
 * Adds a ButtonPress to the info vector.
 */
void FloorSubsystem::addIn(const std::shared_ptr<ButtonPress>& buttonPress) {
    info.push_back(buttonPress);
}

/**
 * Removes a ButtonPress from the info vector.
 */
void FloorSubsystem::removeOut(const std::shared_ptr<ButtonPress>& removee) {
    auto it = std::find(info.begin(), info.end(), removee);
    if (it != info.end()) {
        info.erase(it);
    }
}

/**
 * Removes a ButtonPress from the info vector using an index.
 */
void FloorSubsystem::removeOut(size_t index) {
    if (index < info.size()) {
        info.erase(info.begin() + index);
    }
}

/**
 * Gets the info vector.
 */
std::vector<std::shared_ptr<ButtonPress>>& FloorSubsystem::getInfo() {
    return info;
}

/**
 * Gets the info vector as a constant.
 */
const std::vector<std::shared_ptr<ButtonPress>>& FloorSubsystem::getInfo() const {
    return info;
}


}