#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <future>  // For std::future and std::promise
#include <ctime>  // For time_t
#include "Scheduler.h"
#include "Elevator.h"
#include "FloorSubsystem.h"
#include "Button.h"

using namespace src;

/**
 * Test that the Scheduler correctly adds tasks to the queue.
 */
TEST(SchedulerTest, AddToQueue) {
    auto floorsub = std::make_shared<FloorSubsystem>();
    auto scheduler = std::make_shared<Scheduler>(floorsub);

    // Create a valid ButtonPress with 4 arguments
    std::time_t now = std::time(0);  // Get the current time
    auto btnPress = std::make_shared<ButtonPress>(true, 1, 2, now);  // Direction Up, Car Button 1, Floor 2, Current time
    scheduler->addToQueue(btnPress);

    ASSERT_EQ(scheduler->getQueue().size(), 1);
    ASSERT_EQ(scheduler->getQueue()[0]->getFloorNumber(), 2);
}

/**
 * Test that the Scheduler correctly adds tasks to the floor queue.
 */
TEST(SchedulerTest, AddToFloorQueue) {
    auto floorsub = std::make_shared<FloorSubsystem>();
    auto scheduler = std::make_shared<Scheduler>(floorsub);

    std::time_t now = std::time(0);  // Get the current time
    auto btnPress = std::make_shared<ButtonPress>(false, 1, 5, now);  // Direction Down, Car Button 1, Floor 5, Current time
    scheduler->addFloorQueue(btnPress);

    ASSERT_EQ(scheduler->getFloorQueue().size(), 1);
    ASSERT_EQ(scheduler->getFloorQueue()[0]->getFloorNumber(), 5);
}

/**
 * Test that an Elevator correctly receives and processes a task.
 */
TEST(ElevatorTest, ReceiveAndProcessTask) {
    auto floorsub = std::make_shared<FloorSubsystem>();
    auto scheduler = std::make_shared<Scheduler>(floorsub);
    auto elevator = std::make_shared<Elevator>(1, scheduler);

    std::time_t now = std::time(0);  // Get the current time
    auto btnPress = std::make_shared<ButtonPress>(true, 1, 3, now);  // Direction Up, Car Button 1, Floor 3, Current time
    scheduler->addToQueue(btnPress);

    // Use a promise/future to synchronize the elevator task
    std::promise<void> elevatorDone;
    std::future<void> elevatorFuture = elevatorDone.get_future();

    // Start elevator thread with synchronization
    std::thread elevatorThread([&]() {
        elevator->run();
        elevatorDone.set_value();  // Signal completion
    });

    elevatorFuture.get();  // Wait for the elevator task to finish

    // Verify task is removed from the queue after processing
    ASSERT_TRUE(scheduler->getQueue().empty());

    elevatorThread.join();  // Join the thread to ensure cleanup
}

/**
 * Test that multiple elevators can handle different tasks concurrently.
 */
TEST(ElevatorTest, MultipleElevatorsProcessing) {
    auto floorsub = std::make_shared<FloorSubsystem>();
    auto scheduler = std::make_shared<Scheduler>(floorsub);

    auto elevator1 = std::make_shared<Elevator>(1, scheduler);
    auto elevator2 = std::make_shared<Elevator>(2, scheduler);

    std::time_t now = std::time(0);  // Get the current time
    auto btnPress1 = std::make_shared<ButtonPress>(true, 1, 4, now);  // Direction Up, Car Button 1, Floor 4, Current time
    auto btnPress2 = std::make_shared<ButtonPress>(false, 1, 7, now); // Direction Down, Car Button 1, Floor 7, Current time

    scheduler->addToQueue(btnPress1);
    scheduler->addToQueue(btnPress2);

    // Use promise/future to synchronize both elevator tasks
    std::promise<void> elevator1Done, elevator2Done;
    std::future<void> elevator1Future = elevator1Done.get_future();
    std::future<void> elevator2Future = elevator2Done.get_future();

    // Start elevator threads with synchronization
    std::thread elevatorThread1([&]() {
        elevator1->run();
        elevator1Done.set_value();
    });

    std::thread elevatorThread2([&]() {
        elevator2->run();
        elevator2Done.set_value();
    });

    // Wait for both elevators to complete their tasks
    elevator1Future.get();
    elevator2Future.get();

    // Verify both tasks are processed
    ASSERT_TRUE(scheduler->getQueue().empty());

    // Join threads to ensure proper cleanup
    elevatorThread1.join();
    elevatorThread2.join();
}
