#include "TaskQueue.h"
#include "Globals.h"
#include <sstream>

using namespace std;

void TaskQueue::addTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex);
    taskQueue.push(task);
    // {
    //     ostringstream oss;
    //     oss << CYAN_COLOR << "[TaskQueue] Added task: Floor " << task.floorNumber 
    //         << " | Elevator " << task.elevatorID << RESET_COLOR;
    //     addLog(oss.str());
    // }
    cv.notify_all();
}

int TaskQueue::getSize() {
    return taskQueue.size();
}
bool TaskQueue::removeTask(int floor, int elevatorID) {
    std::lock_guard<std::mutex> lock(mutex);
    bool removed = false;
    std::vector<Task> tasks;
    while (!taskQueue.empty()) {
        Task t = taskQueue.top();
        taskQueue.pop();
        if (!removed && t.floorNumber == floor && t.elevatorID == elevatorID) {
            removed = true;
            continue;
        }
        tasks.push_back(t);
    }
    for (auto &t: tasks) {
        taskQueue.push(t);
    }
    if (removed) {
        // ostringstream oss;
        // oss << GREEN_COLOR << "[TaskQueue] Removed task for Elevator " 
        //     << elevatorID << " on Floor " << floor << RESET_COLOR;
        // addLog(oss.str());
    }
    return removed;
}

bool TaskQueue::hasPendingTasks() {
    std::lock_guard<std::mutex> lock(mutex);
    return !taskQueue.empty();
}

Task TaskQueue::peekTask() {
    std::lock_guard<std::mutex> lock(mutex);
    if (taskQueue.empty()) {
        throw std::runtime_error("No tasks available");
    }
    return taskQueue.top();
}

Task TaskQueue::fetchNextTask() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]() { return !taskQueue.empty(); });
    Task task = taskQueue.top();
    taskQueue.pop();
    return task;
}

Task TaskQueue::fetchTaskForElevator(int elevatorID) {
    std::unique_lock<std::mutex> lock(mutex);
    // Wait until there is at least one task with the matching elevatorID.
    cv.wait(lock, [this, elevatorID]() {
        std::priority_queue<Task, std::vector<Task>, TaskComparator> copy = taskQueue;
        while (!copy.empty()) {
            if (copy.top().elevatorID == elevatorID)
                return true;
            copy.pop();
        }
        return false;
    });
    
    // Remove tasks from the queue until we find one matching elevatorID.
    std::vector<Task> temp;
    Task target;
    bool found = false;
    while (!taskQueue.empty()) {
         Task t = taskQueue.top();
         taskQueue.pop();
         if (!found && t.elevatorID == elevatorID) {
              target = t;
              found = true;
         } else {
              temp.push_back(t);
         }
    }
    // Rebuild the queue with remaining tasks.
    for (auto &t : temp) {
         taskQueue.push(t);
    }
    return target;
}

std::priority_queue<Task, std::vector<Task>, TaskComparator>& TaskQueue::getQueue() {
    return taskQueue;
}
