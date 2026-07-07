#include "Button.h"

namespace src {

ButtonPress::ButtonPress(bool buttonDirection, int carButton, int floorNumber, std::time_t buttonTime)
    : buttonDirection(buttonDirection), carButton(carButton), floorNumber(floorNumber), currTime(buttonTime) {}


/**
 * Gets the Button Direction.
 */
bool ButtonPress::getButtonDirection() const {
    return buttonDirection;
}

/**
 * Gets the Button Floor Number.
 */
int ButtonPress::getFloorNumber() const {
    return floorNumber;
}

/**
 * Gets the Car button.
 */
int ButtonPress::getCarButton() const {
    return carButton;
}

/**
 * Sets the Car button.
 */    
void ButtonPress::setCarButton(int newButton) {
    carButton = newButton;
}

} // namespace src