#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include <vector>
#include "FloorSubsystem.h"
#include "Scheduler.h"
#include "Elevator.h"
#include "Floor.h"

namespace src {

void runSimulation(int numElevators) {
    // Step 1: Initialize FloorSubsystem and Scheduler
    auto floorsub = std::make_shared<FloorSubsystem>();
    auto sched = std::make_shared<Scheduler>(floorsub);

    // Step 2: Parse input data
    const std::string inputFile = "test.txt";
    floorsub->parseData(inputFile);
    std::cout << "FloorSubsystem parsed input data from " << inputFile << std::endl;

    // Step 3: Create Floor and multiple Elevator objects
    auto floor = std::make_shared<Floor>(5, sched);
    
    std::vector<std::shared_ptr<Elevator>> elevators;
    std::vector<std::thread> elevatorThreads;
    
    for (int i = 0; i < numElevators; ++i) {
        auto elevator = std::make_shared<Elevator>(i + 1, sched);
        elevators.push_back(elevator);
        elevatorThreads.emplace_back(&Elevator::run, elevator);
    }

    // Step 4: Start other threads
    std::thread schedulerThread(&Scheduler::run, sched);
    std::thread floorThread(&Floor::run, floor);

    std::cout << "Simulation started with " << numElevators << " elevators. Press CTRL+C to terminate." << std::endl;

    // Step 5: Join all threads
    schedulerThread.join();
    floorThread.join();
    
    for (auto& thread : elevatorThreads) {
        thread.join();
    }
}

} // namespace src

int main() {
    try {
        int numElevators = 3;  // You can adjust this number as needed
        src::runSimulation(numElevators);
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
