#ifndef FLOOR_H
#define FLOOR_H

#include "Scheduler.h"
#include "Button.h"
#include <memory> // For shared pointers

namespace src {

/**
 * Represents a floor of a building and handles interactions with the Scheduler.
 */
class Floor {
private:
    bool buttonDirection; // True for up, False for down
    int floorNumber;
    bool lampOn; // Checks if floor is ready to receive an elevator
    std::shared_ptr<Scheduler> sched; // Shared pointer to a Scheduler instance
    std::shared_ptr<ButtonPress> recentPress;
    std::shared_ptr<ButtonPress> receivedInfo;

public:
    /**
     * Constructor for the Floor class.
     *
     * @param FloorNumber - The number of the floor
     * @param sched - A shared pointer to the Scheduler instance
     */
    Floor(int FloorNumber, std::shared_ptr<Scheduler> sched);

    /**
     * This function represents the thread behavior of the Floor.
     */
    void run();

    /**
     * Returns the number of the floor.
     *
     * @return The floor number
     */
    int getFloorNumber() const;

    /**
     * Indicates the button that was most recently pressed.
     *
     * @return A shared pointer to the most recent ButtonPress
     */
    std::shared_ptr<ButtonPress> getRecentPress() const;
};

} // namespace src

#endif // FLOOR_H
