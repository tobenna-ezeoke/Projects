#ifndef ELEVATORSUBSYSTEM_H
#define ELEVATORSUBSYSTEM_H

namespace src {

/**
 * The ElevatorSubsystem class is a placeholder for future milestones.
 * Elevators will make calls to the Scheduler and send data back.
 */
class ElevatorSubsystem {
private:
    int initialFloor;
    int finalFloor;
    bool doorOpen;
    bool motorOn;
    int carId;

public:
    // Constructor
    ElevatorSubsystem(int carId);

    // Getter for carId
    int getCarId() const;

    // Future functionality placeholder
};

} // namespace src

#endif // ELEVATORSUBSYSTEM_H
