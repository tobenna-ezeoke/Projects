# SYSC 3303 Elevator - Iteration 1

## Group Members / Responsibilities 
Deryck Ho - backend refactoring\
Matthew Jong - most of backend code\
Brix Velasco - uml diagrams\
Syed Shaheer - uml diagrams


Files:
src/ contains all source code. \
Button.cpp represents a container to store a button press i.e. info\
Elevator.cpp represents one elevator.\
ElevatorSubsystem represents the entire system of elevators, but there's only one elevator as of now.\
Floor and FloorSubsystem are the floor objects that recieve queued tasks\
Scheduler assigns tasks.\
Main is just the test program,really

## Compilation instructions
Navigate into the src/ subdirectory, then compile with g++, i.e. `g++ *.cpp -o TestProgram`. This will then read from the file named test.txt within the src directory. If this doesnt work include `-pthread` in the flags

## Notes
UML diagrams are also in the src directory.
