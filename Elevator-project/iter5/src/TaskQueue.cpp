#include "TaskQueue.h"
#include "Globals.h"
#include <sstream>

using namespace std;

// Adds a new task to the task queue in a thread-safe manner.
void TaskQueue::addTask(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex); // Acquire mutex lock for thread safety
    taskQueue.push(task);                    // Push the task onto the priority queue
    cv.notify_all();                         // Notify waiting threads that a new task is available
}

// Returns the current number of tasks in the queue.
int TaskQueue::getSize() {
    std::lock_guard<std::mutex> lock(mutex); // Acquire mutex lock for thread safety
    return taskQueue.size();                 // Return size of the queue
}

// Removes a specific task identified by floor number and elevator ID from the queue.
// Returns true if a matching task was found and removed; otherwise, false.
bool TaskQueue::removeTask(int floor, int elevatorID) {
    std::lock_guard<std::mutex> lock(mutex); // Acquire mutex lock for thread safety
    bool removed = false;                    // Flag to indicate if removal occurred

    std::vector<Task> tasks;                 // Temporary storage for tasks

    // Iterate through all tasks to find and remove the matching one
    while (!taskQueue.empty()) {
        Task t = taskQueue.top();
        taskQueue.pop();
        if (!removed && t.floorNumber == floor && t.elevatorID == elevatorID) {
            removed = true;                  // Mark as removed and skip adding back to temporary storage
            continue;
        }
        tasks.push_back(t);                  // Store non-matching tasks temporarily
    }

    // Re-insert remaining tasks back into the priority queue
    for (auto &t: tasks) {
        taskQueue.push(t);
    }

    return removed;                          // Return whether removal was successful
}

// Checks if there are any pending tasks in the queue.
// Returns true if at least one task is present; otherwise, false.
bool TaskQueue::hasPendingTasks() {
    std::lock_guard<std::mutex> lock(mutex); // Acquire mutex lock for thread safety
    return !taskQueue.empty();               // Check if queue is non-empty
}

// Returns the next task without removing it from the queue.
// Throws an exception if no tasks are available.
Task TaskQueue::peekTask() {
    std::lock_guard<std::mutex> lock(mutex); // Acquire mutex lock for thread safety

    if (taskQueue.empty()) {                 // Check if queue is empty
        throw std::runtime_error("No tasks available");  // Throw exception if empty
    }

    return taskQueue.top();                  // Return next task without removing it
}

// Waits until a task is available, then retrieves and removes it from the queue.
Task TaskQueue::fetchNextTask() {
    std::unique_lock<std::mutex> lock(mutex);  // Unique lock allows waiting on condition variable

    cv.wait(lock, [this]() {                 // Wait until condition variable signals availability of tasks
        return !taskQueue.empty();
    });

    Task task = taskQueue.top();             // Retrieve next available task
    taskQueue.pop();                         // Remove retrieved task from queue

    return task;                             // Return retrieved task
}

// Waits until a specific elevator has an assigned task, retrieves and removes that specific task from the queue.
Task TaskQueue::fetchTaskForElevator(int elevatorID) {
    std::unique_lock<std::mutex> lock(mutex);  // Unique lock allows waiting on condition variable

    cv.wait(lock, [this, elevatorID]() {      // Wait until a matching elevator-specific task is available
        std::priority_queue<Task, std::vector<Task>, TaskComparator> copy = taskQueue;

        while (!copy.empty()) {               // Check through a copy of queue to avoid modifying original during check
            if (copy.top().elevatorID == elevatorID)
                return true;                  // Matching elevator-specific task found
            copy.pop();
        }
        return false;                         // No matching elevator-specific tasks yet, continue waiting
    });

    std::vector<Task> temp;                   // Temporary storage for non-matching tasks
    Task target;
    bool found = false;

    while (!taskQueue.empty()) {              // Extract all tasks until we find our target elevator-specific one
         Task t = taskQueue.top();
         taskQueue.pop();

         if (!found && t.elevatorID == elevatorID) {
              target = t;                     // Found target elevator-specific task to retrieve and remove
              found = true;
         } else {
              temp.push_back(t);              // Store other tasks temporarily to reinsert later
         }
    }

    for (auto &t : temp) {                    // Reinsert non-matching tasks back into priority queue
         taskQueue.push(t);
    }

    return target;                            // Return retrieved elevator-specific task
}

// Provides direct access to the underlying priority queue.
std::priority_queue<Task, std::vector<Task>, TaskComparator>& TaskQueue::getQueue() {
    return taskQueue;
}

// Counts how many pending tasks are assigned specifically to a given elevator ID.
int TaskQueue::getQueueSizeForElevator(int elevatorID) {
    std::lock_guard<std::mutex> lock(mutex);  // Acquire mutex lock for thread safety

    int count = 0;
    
    std::priority_queue<Task, std::vector<Task>, TaskComparator> copy = taskQueue;

    while (!copy.empty()) {                   // Iterate through copy of queue to count matching elevator-specific tasks
        Task t = copy.top();
        copy.pop();

        if(t.elevatorID == elevatorID)
            count++;                          // Increment count when matching elevator ID found
    }

    return count;                             // Return total number of matching elevator-specific tasks found 
}
