#ifndef FLOORSUBSYSTEM_H
#define FLOORSUBSYSTEM_H

#include <vector>
#include <string>
#include <memory> // For using smart pointers
#include "Button.h" // Assuming ButtonPress is defined here


namespace src {
class FloorSubsystem {
private:
    std::vector<std::shared_ptr<ButtonPress>> info; // ArrayList equivalent
    std::string fileName; // Current location for the file

public:
    /**
     * Default Constructor
     */
    FloorSubsystem();

    /**
     * Parse the data received in the input file.
     *
     * @param fileName - the name of the input file assuming it is .txt
     */
    void parseData(const std::string& fileName);


    /**
     * Parse the default data from input.txt.
     */
    void parseData();

    /**
     * Adds a ButtonPress to the info vector.
     *
     * @param buttonPress - the addee
     */
    void addIn(const std::shared_ptr<ButtonPress>& buttonPress);

    /**
     * Removes a ButtonPress from the info vector.
     *
     * @param removee - the item to remove
     */
    void removeOut(const std::shared_ptr<ButtonPress>& removee);

    /**
     * Removes a ButtonPress from the info vector using an index.
     *
     * @param index - the index of the item to remove
     */
    void removeOut(size_t index);

    /**
     * Gets the info vector.
     *
     * @return A vector of ButtonPress objects.
     */
    // Ensure getInfo returns a non-const reference
    std::vector<std::shared_ptr<ButtonPress>>& getInfo();
    const std::vector<std::shared_ptr<ButtonPress>>& getInfo() const;


};
}

#endif 
// FLOORSUBSYSTEM_H
