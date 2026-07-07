#include <gtest/gtest.h>
#include "TaskQueue.h"
#include "Globals.h"

class TaskQueueTest : public ::testing::Test {
protected:
    TaskQueue taskQueue;

    // Setup code (if necessary)
    void SetUp() override {
        // You can add any setup code for your tests here
    }

    // Cleanup code (if necessary)
    void TearDown() override {
        // You can add any cleanup code for your tests here
    }
};

// Test addTask and getSize
TEST_F(TaskQueueTest, AddTaskAndGetSize) {
    Task task1(FLOOR_CALL, 1, UP, 0);
    Task task2(FLOOR_CALL, 2, DOWN, 1);

    taskQueue.addTask(task1);
    taskQueue.addTask(task2);

    EXPECT_EQ(taskQueue.getSize(), 2);
}

// Test removeTask
TEST_F(TaskQueueTest, RemoveTask) {
    Task task1(FLOOR_CALL, 1, UP, 0);
    Task task2(FLOOR_CALL, 2, DOWN, 1);

    taskQueue.addTask(task1);
    taskQueue.addTask(task2);

    // Removing a specific task
    EXPECT_TRUE(taskQueue.removeTask(1, 0)); // Remove task at floor 1, elevator 0
    EXPECT_EQ(taskQueue.getSize(), 1);       // Only one task left
}

// Test hasPendingTasks
TEST_F(TaskQueueTest, HasPendingTasks) {
    Task task1(FLOOR_CALL, 1, UP, 0);

    taskQueue.addTask(task1);

    EXPECT_TRUE(taskQueue.hasPendingTasks());

    taskQueue.removeTask(1, 0);
    EXPECT_FALSE(taskQueue.hasPendingTasks());
}

// Test peekTask (it throws exception when queue is empty)
TEST_F(TaskQueueTest, PeekTask) {
    Task task1(FLOOR_CALL, 1, UP, 0);
    taskQueue.addTask(task1);

    Task peekedTask = taskQueue.peekTask();
    EXPECT_EQ(peekedTask.floorNumber, 1);
    EXPECT_EQ(peekedTask.elevatorID, 0);

    taskQueue.removeTask(1, 0);

    // Try peeking when empty
    EXPECT_THROW(taskQueue.peekTask(), std::runtime_error);
}

// Test fetchNextTask
TEST_F(TaskQueueTest, FetchNextTask) {
    Task task1(FLOOR_CALL, 1, UP, 0);
    Task task2(FLOOR_CALL, 2, DOWN, 1);

    taskQueue.addTask(task1);
    taskQueue.addTask(task2);

    Task fetchedTask = taskQueue.fetchNextTask();
    EXPECT_EQ(fetchedTask.floorNumber, 1);  // First task added, should be fetched first
    EXPECT_EQ(fetchedTask.elevatorID, 0);
}

// Test fetchTaskForElevator
TEST_F(TaskQueueTest, FetchTaskForElevator) {
    Task task1(FLOOR_CALL, 1, UP, 0);
    Task task2(FLOOR_CALL, 2, DOWN, 1);
    Task task3(FLOOR_CALL, 3, UP, 0);

    taskQueue.addTask(task1);
    taskQueue.addTask(task2);
    taskQueue.addTask(task3);

    Task fetchedTask = taskQueue.fetchTaskForElevator(0);
    EXPECT_EQ(fetchedTask.floorNumber, 1);  // First task for elevator 0
    EXPECT_EQ(fetchedTask.elevatorID, 0);

    fetchedTask = taskQueue.fetchTaskForElevator(1);
    EXPECT_EQ(fetchedTask.floorNumber, 2);  // First task for elevator 1
    EXPECT_EQ(fetchedTask.elevatorID, 1);
}

// Test getQueueSizeForElevator
TEST_F(TaskQueueTest, GetQueueSizeForElevator) {
    Task task1(FLOOR_CALL, 1, UP, 0);
    Task task2(FLOOR_CALL, 2, DOWN, 1);
    Task task3(FLOOR_CALL, 3, UP, 0);

    taskQueue.addTask(task1);
    taskQueue.addTask(task2);
    taskQueue.addTask(task3);

    EXPECT_EQ(taskQueue.getQueueSizeForElevator(0), 2);  // Elevator 0 has 2 tasks
    EXPECT_EQ(taskQueue.getQueueSizeForElevator(1), 1);  // Elevator 1 has 1 task
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
