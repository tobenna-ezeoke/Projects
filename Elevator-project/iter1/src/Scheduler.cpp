// Scheduler.cpp
#include "Scheduler.h"
#include <iostream>
#include <thread>

namespace src {

// Constructors
Scheduler::Scheduler(const std::shared_ptr<FloorSubsystem>& floorsub)
    : floorsub(floorsub) {}

Scheduler::Scheduler(const std::shared_ptr<FloorSubsystem>& floorsub, const std::vector<std::shared_ptr<ButtonPress>>& initialQueue)
    : floorsub(floorsub), queue(initialQueue) {}

// Accessors
std::vector<std::shared_ptr<ButtonPress>>& Scheduler::getQueue() {
    return queue;
}

std::vector<std::shared_ptr<ButtonPress>>& Scheduler::getFloorQueue() {
    return floorQueue;
}

std::mutex& Scheduler::getMutex() {
    return mtx;
}

std::condition_variable& Scheduler::getConditionVariable() {
    return cv;
}

// Queue management
void Scheduler::addToQueue(const std::shared_ptr<ButtonPress>& btn) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_back(btn);
    cv.notify_all();
}

void Scheduler::addFloorQueue(const std::shared_ptr<ButtonPress>& buttonpress) {
    // Lock the shared mutex for floorQueue
    std::unique_lock<std::mutex> lock(mtx);
    floorQueue.push_back(buttonpress);
    std::cout << "Scheduler added task to FloorQueue: Floor "
              << buttonpress->getFloorNumber() << std::endl;

    // Notify waiting threads
    cv.notify_all();
}


void Scheduler::run() {
    while (true) {
        std::shared_ptr<ButtonPress> task;

        // Acquire lock to check and retrieve data from floorsub
        {
            std::unique_lock<std::mutex> lock(mtx);
            // std::cout << "Scheduler::run: Acquired lock." << std::endl;

            // Wait for data to be available in the floorsub
            while(floorsub->getInfo().empty())
            {
                cv.wait(lock);
            }

            
            task = floorsub->getInfo().front();
            floorsub->getInfo().erase(floorsub->getInfo().begin());
            queue.push_back(task);
            std::cout << "Scheduler::run: Task added to queue: Floor "
                        << task->getFloorNumber() << std::endl;
            
        } // Lock released here

        // Simulate processing the task outside the critical section
        if (task) {
            std::string message_processing = "Scheduler is processing task for floor " + std::to_string(task->getFloorNumber() ) + "\n";
            std::cout << message_processing;

            // std::cout << "Scheduler is processing task for Floor "
                    //   << task->getFloorNumber() << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::string message_done = "Scheduler finished processing task for floor " + std::to_string(task->getFloorNumber() ) + "\n";
            std::cout << message_done;


            // Add the task to the floorQueue
            addFloorQueue(task);
        }
    }
}




} // namespace src
