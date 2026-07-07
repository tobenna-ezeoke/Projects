// Scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "Button.h"
#include "FloorSubsystem.h"

namespace src {

class Scheduler {
private:
    std::vector<std::shared_ptr<ButtonPress>> queue;
    std::vector<std::shared_ptr<ButtonPress>> floorQueue;
    std::shared_ptr<FloorSubsystem> floorsub;
    std::mutex mtx;
    std::condition_variable cv;

public:
    // Constructor
    Scheduler(const std::shared_ptr<FloorSubsystem>& floorsub);
    Scheduler(const std::shared_ptr<FloorSubsystem>& floorsub, const std::vector<std::shared_ptr<ButtonPress>>& initialQueue);

    // Accessors
    std::vector<std::shared_ptr<ButtonPress>>& getQueue();
    std::vector<std::shared_ptr<ButtonPress>>& getFloorQueue();
    std::mutex& getMutex();
    std::condition_variable& getConditionVariable();

    // Queue management
    void addToQueue(const std::shared_ptr<ButtonPress>& btn);
    void addFloorQueue(const std::shared_ptr<ButtonPress>& buttonpress);

    // Thread logic
    void run();
};

} // namespace src

#endif // SCHEDULER_H
