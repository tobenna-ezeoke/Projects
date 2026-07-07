#include <gtest/gtest.h>
#include "Scheduler.h"
#include "TaskQueue.h"
#include "Globals.h"

// Test case to verify that Scheduler correctly adds elevator requests to the TaskQueue.
TEST(SchedulerTest, AddElevatorRequest) {
    TaskQueue taskQueue;
    Scheduler scheduler(&taskQueue);

    // Create a new elevator request task (floor call at floor 3, direction UP)
    Task request(FLOOR_CALL, 3, UP, 0);
    scheduler.addElevatorRequest(request);

    // Verify that the task queue size is updated correctly after adding the request
    EXPECT_EQ(scheduler.getQueueSize(), 1);
}

// Test case to verify that Scheduler correctly fetches the next available task from the TaskQueue.
TEST(SchedulerTest, FetchNextTask) {
    TaskQueue taskQueue;
    Scheduler scheduler(&taskQueue);

    // Add a new elevator request task (floor call at floor 5, direction DOWN)
    Task request(FLOOR_CALL, 5, DOWN, 0);
    scheduler.addElevatorRequest(request);

    // Fetch the next available task from the scheduler
    Task fetchedTask = scheduler.fetchNextTask();

    // Verify fetched task details match the originally added request
    EXPECT_EQ(fetchedTask.floorNumber, 5);
    EXPECT_EQ(fetchedTask.direction, DOWN);
}

// Test case to verify Scheduler's ability to record and retrieve elevator status updates.
TEST(SchedulerTest, ElevatorUpdate) {
    TaskQueue taskQueue;
    Scheduler scheduler(&taskQueue);

    // Create a new ElevatorStatusData object representing elevator status update
    ElevatorStatusData status(1, 3, UP, 3, STANDBY, DOOR_CLOSED, 0, 10, 0);
    
    // Record this elevator status update in Scheduler
    scheduler.recordElevatorUpdate(status);

    // Retrieve the latest elevator status update from Scheduler
    ElevatorStatusData retrieved = scheduler.retrieveElevatorUpdate();

    // Verify retrieved data matches the recorded elevator status update
    EXPECT_EQ(retrieved.elevatorID, 1);
    EXPECT_EQ(retrieved.currentFloor, 3);
    EXPECT_EQ(retrieved.status, STANDBY);
}

// Test case to verify Scheduler's ability to return queue size for a specific elevator.
TEST(SchedulerTest, GetQueueSizeForElevator) {
    TaskQueue taskQueue;
    Scheduler scheduler(&taskQueue);

    // Add a request specifically assigned to elevator ID=1 (floor call at floor 2)
    Task request(FLOOR_CALL, 2, UP, 1);
    scheduler.addElevatorRequest(request);

    // Verify that Scheduler returns correct queue size for elevator ID=1
    EXPECT_EQ(scheduler.getQueueSizeForElevator(1), 1);
}

// Main function to initialize Google Test framework and execute all tests defined above.
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); // Initialize Google Test with command-line arguments
    return RUN_ALL_TESTS();                 // Execute all defined test cases and return results
}
