#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include "FloorSubsystem.h"
#include "Scheduler.h"
#include "Elevator.h"
#include "Floor.h"

namespace src {
// hi 2
void runSimulation() {
    // Step 1: Initialize FloorSubsystem and Scheduler
    auto floorsub = std::make_shared<FloorSubsystem>();
    auto sched = std::make_shared<Scheduler>(floorsub);

    // Step 2: Parse input data
    const std::string inputFile = "test.txt";
    floorsub->parseData(inputFile);
    std::cout << "FloorSubsystem parsed input data from " << inputFile << std::endl;

    // Step 3: Create Floor and Elevator objects
    auto elevator = std::make_shared<Elevator>(1, sched);
    auto floor = std::make_shared<Floor>(5, sched);

    // Step 4: Start threads
    std::thread schedulerThread(&Scheduler::run, sched);
    std::thread elevatorThread(&Elevator::run, elevator);
    std::thread floorThread(&Floor::run, floor);

    // std::cout << "Simulation started. Press CTRL+C to terminate." << std::endl;

    // Step 5: Join threads (simulation runs indefinitely)
    schedulerThread.join();
    elevatorThread.join();
    floorThread.join();
}

} // namespace src

int main() {
    try {
        src::runSimulation();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
