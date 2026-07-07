#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <memory>
#include "Scheduler.h"
#include "Button.h"
#include <thread>
#include <mutex>
#include <condition_variable>

namespace src {

/**
 * Represents an elevator car object in a multi-floor building.
 */
class Elevator {
private:
    std::shared_ptr<Scheduler> sched; // Reference to the scheduler
    int currentFloor = 1;             // All elevators start on the first floor
    bool doorOpen = false;            // Tracks if the door is open
    bool motorOn = false;             // Tracks if the motor is on
    int carId = 0;                    // Elevator car ID
    std::shared_ptr<ButtonPress> recentPress; // Most recent button press

public:
    /**
     * Constructor for the Elevator class.
     */
    Elevator(int carId, const std::shared_ptr<Scheduler>& scheduler);

    /**
     * Returns the current floor of the elevator.
     */
    int getCurrentFloor() const;

    /**
     * Returns the elevator car's ID.
     */
    int getCarId() const;

    /**
     * Runs the elevator thread logic.
     */
    void run();
};

} // namespace src

#endif // ELEVATOR_H
