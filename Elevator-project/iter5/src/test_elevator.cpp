#include <gtest/gtest.h>
#include "Elevator.h"
#include "Globals.h"

// Test case to verify correct initialization of Elevator object
TEST(ElevatorTest, Initialization) {
    // Create an Elevator instance with ID=1, max floor=10, and nullptr for scheduler and task queue
    Elevator elevator(1, 10, nullptr, nullptr);

    // Expect initial elevator level to be ground floor (0)
    EXPECT_EQ(elevator.getCurrentLevel(), 0);

    // Expect initial elevator status to be STANDBY
    EXPECT_EQ(elevator.checkStatus(), STANDBY);
}

// Test case to verify setting the destination floor correctly updates elevator status
TEST(ElevatorTest, SetDestination) {
    Elevator elevator(1, 10, nullptr, nullptr);

    // Set destination floor to 5
    elevator.setDestination(5);

    // Verify that the elevator's destination is correctly updated to floor 5
    EXPECT_EQ(elevator.retrieveElevatorStatus().destination, 5);
}

// Test case to verify elevator movement logic (moving to destination floor)
TEST(ElevatorTest, MoveElevator) {
    Elevator elevator(1, 10, nullptr, nullptr);

    // Set destination floor to 5 and initiate movement
    elevator.setDestination(5);
    elevator.moveElevator();

    // Verify that elevator has reached the intended destination (floor 5)
    EXPECT_EQ(elevator.getCurrentLevel(), 5);

    // Verify that after reaching destination, elevator status returns to STANDBY
    EXPECT_EQ(elevator.checkStatus(), REACHED);
}

// Test case to verify door operations (opening and open states)
TEST(ElevatorTest, DoorOperations) {
    Elevator elevator(1, 10, nullptr, nullptr);

    // Set door status to DOOR_OPENING and verify update
    elevator.setDoorStatus(DOOR_OPENING);
    EXPECT_EQ(elevator.retrieveElevatorStatus().doorStatus, DOOR_OPENING);

    // Set door status to DOOR_OPEN and verify update
    elevator.setDoorStatus(DOOR_OPEN);
    EXPECT_EQ(elevator.retrieveElevatorStatus().doorStatus, DOOR_OPEN);
}

// Test case to verify updating and retrieving elevator's operational status
TEST(ElevatorTest, StatusUpdate) {
    Elevator elevator(1, 10, nullptr, nullptr);

    // Update elevator status to IN_TRANSIT and verify update
    elevator.updateStatus(IN_TRANSIT);
    EXPECT_EQ(elevator.checkStatus(), IN_TRANSIT);

    // Update elevator status back to STANDBY and verify update
    elevator.updateStatus(STANDBY);
    EXPECT_EQ(elevator.checkStatus(), STANDBY);
}

// Main function that initializes Google Test framework and runs all defined tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv); // Initialize Google Test with command-line arguments
    return RUN_ALL_TESTS();                 // Execute all defined test cases and return results
}
