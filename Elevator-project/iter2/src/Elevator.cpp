// Elevator.cpp
#include "Elevator.h"
#include "Globals.h"   // Include the global mutex
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

        {
            std::unique_lock<std::mutex> lock(sched->getMutex());
            while (sched->getQueue().empty()) {
                sched->getConditionVariable().wait(lock);
            }

            task = sched->getQueue().front();
            sched->getQueue().erase(sched->getQueue().begin());
            {
                std::lock_guard<std::mutex> printLock(g_printMutex);
                std::cout << "[Elevator " << carId << " | Thread ID: " 
                          << std::this_thread::get_id() 
                          << "] Received a task: Floor "
                          << task->getFloorNumber()
                          << ", Direction " << (task->getButtonDirection() ? "Up" : "Down")
                          << std::endl;
            }
        } // Release lock on sched->mtx

        if (task) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::lock_guard<std::mutex> printLock(g_printMutex);
                std::cout << "[Elevator " << carId << " | Thread ID: " 
                          << std::this_thread::get_id()
                          << "] Completed task for Floor " 
                          << task->getFloorNumber() << std::endl;
            }
        }
    }
}

} // namespace src
