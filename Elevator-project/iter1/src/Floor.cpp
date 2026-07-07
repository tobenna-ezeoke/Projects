#include "Floor.h"
#include <iostream>

namespace src {

/**
 * Constructor for the Floor class.
 */
Floor::Floor(int FloorNumber, std::shared_ptr<Scheduler> sched)
    : floorNumber(FloorNumber), lampOn(true), sched(std::move(sched)) {}

/**
 * Represents the thread behavior of the Floor.
 */
void Floor::run() {
    while (true) {
        std::shared_ptr<ButtonPress> task;

        // Lock to retrieve a task from the floorQueue
        {
            std::unique_lock<std::mutex> lock(sched->getMutex());

            // Wait if the Floor Queue is empty (no task)
            while(sched->getFloorQueue().empty())
            {
                sched->getConditionVariable().wait(lock);
            }

            // Process task if Floor Queue is not empty
            task = sched->getFloorQueue().front();
            sched->getFloorQueue().erase(sched->getFloorQueue().begin());
            std::cout << "Floor processed task: Floor " << task->getFloorNumber()
                    << ", Direction " << (task->getButtonDirection() ? "Up" : "Down")
                    << std::endl;
            
        } // Lock released here
    }
}

/**
 * Returns the number of the floor.
 */
int Floor::getFloorNumber() const {
    return floorNumber;
}

/**
 * Indicates the button that was most recently pressed.
 */
std::shared_ptr<ButtonPress> Floor::getRecentPress() const {
    return recentPress;
}

} // namespace src
