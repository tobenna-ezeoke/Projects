#ifndef BUTTON_H
#define BUTTON_H

#include <ctime>

namespace src {

class ButtonPress {
private:
    bool buttonDirection; // True for up, False for down
    int carButton;
    int floorNumber;
    std::time_t currTime;

public:
    /**
     * Constructor for a ButtonPress.
     */
    ButtonPress(bool buttonDirection, int carButton, int floorNumber, std::time_t buttonTime);

    /**
     * Getter method for buttonDirection.
     */
    bool getButtonDirection() const;

    /**
     * Getter method for floorNumber.
     */
    int getFloorNumber() const;

    /**
     * Getter method for carButton.
     */
    int getCarButton() const;

    /**
     * Setter method for carButton.
     */
    void setCarButton(int newButton);
};

} // namespace src

#endif // BUTTONPRESS_H
