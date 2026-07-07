#include "ElevatorSubsystem.h"

namespace src {

// Constructor implementation
ElevatorSubsystem::ElevatorSubsystem(int carId)
    : initialFloor(1), // Default starting floor
      finalFloor(1),   // Default destination floor
      doorOpen(false), // Default door state
      motorOn(false),  // Default motor state
      carId(carId) {}

// Getter for carId
int ElevatorSubsystem::getCarId() const {
    return carId;
}

} // namespace src