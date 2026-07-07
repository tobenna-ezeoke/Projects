// Elevator.cpp
#include "Elevator.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace src {

/**
 * Constructor for the Elevator class.
 */
Elevator::Elevator(int carId, const std::shared_ptr<Scheduler>& scheduler)
    : carId(carId), sched(scheduler) {}

/**
 * Returns the current floor of the elevator.
 */
int Elevator::getCurrentFloor() const {
    return currentFloor;
}

/**
 * Returns the elevator car's ID.
 */
int Elevator::getCarId() const {
    return carId;
}

/**
 * Runs the elevator thread logic.
 */
void Elevator::run() {
    while (true) {
        std::shared_ptr<ButtonPress> task;

        // Lock to retrieve a task from the scheduler queue
        {
            std::unique_lock<std::mutex> lock(sched->getMutex());

            while(sched->getQueue().empty())
            {
                sched->getConditionVariable().wait(lock);
            }

            
            task = sched->getQueue().front();
            sched->getQueue().erase(sched->getQueue().begin());
            std::cout << "Elevator received a task: Floor "
                        << task->getFloorNumber()
                        << ", Direction " << (task->getButtonDirection() ? "Up" : "Down")
                        << std::endl;
            
        } // Lock released here

        // Simulate task processing

        if (task) {
            
            std::string message = "Elevator completed task for Floor " + std::to_string(task->getFloorNumber() ) + "\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << message;
        }
    }
}

} // namespace src
