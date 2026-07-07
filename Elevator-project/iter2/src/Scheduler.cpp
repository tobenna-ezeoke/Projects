// Scheduler.cpp
#include "Scheduler.h"
#include "Globals.h"   // Include the global mutex
#include <iostream>
#include <thread>
#include <chrono>

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
    std::unique_lock<std::mutex> lock(mtx);
    floorQueue.push_back(buttonpress);
    {
        std::lock_guard<std::mutex> printLock(g_printMutex);
        std::cout << "Scheduler added task to FloorQueue: Floor "
                  << buttonpress->getFloorNumber() << std::endl;
    }
    cv.notify_all();
}

void Scheduler::run() {
    while (true) {
        std::shared_ptr<ButtonPress> task;

        {
            std::unique_lock<std::mutex> lock(mtx);
            while(floorsub->getInfo().empty()) {
                cv.wait(lock);
            }
            
            task = floorsub->getInfo().front();
            floorsub->getInfo().erase(floorsub->getInfo().begin());
            queue.push_back(task);
            {
                std::lock_guard<std::mutex> printLock(g_printMutex);
                std::cout << "Scheduler::run: Task added to queue: Floor "
                          << task->getFloorNumber() << std::endl;
            }
        } // Release lock on mtx

        if (task) {
            {
                std::lock_guard<std::mutex> printLock(g_printMutex);
                std::cout << "Scheduler is processing task for floor " 
                          << task->getFloorNumber() << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::lock_guard<std::mutex> printLock(g_printMutex);
                std::cout << "Scheduler finished processing task for floor " 
                          << task->getFloorNumber() << std::endl;
            }

            // Add the task to the floorQueue
            addFloorQueue(task);
        }
    }
}

} // namespace src
