# SYSC 3303 Elevator - Iteration 2

## Group Members / Responsibilities 
Deryck Ho - backend refactoring\
Matthew Jong - UML Class Diagram\
Brix Velasco - UML Sequence Diagram\
Syed Shaheer - State Machine diagrams\
Dearell Tobenna Ezeoke - README.md file


## Files:
src/ contains all source code. \
Button.cpp represents a container to store a button press i.e. info\
Elevator.cpp represents one elevator.\
ElevatorSubsystem.cpp represents the entire system of elevators, there should be multiple elevators in this iteration.\
Floor.cpp and FloorSubsystem.cpp are the floor objects that receive queued tasks\
Scheduler.cpp assigns tasks.\
Main.cpp is run to the test program. 

## Compilation instructions
Navigate into the src/ subdirectory, then compile with g++, i.e. `g++ -std=c++11 -pthread main.cpp Scheduler.cpp Elevator.cpp Floor.cpp FloorSubsystem.cpp Button.cpp Globals.cpp -o test`. This will then be read from the file named test.txt within the src directory. If this doesn't work include `-pthread` in the flags

Compile unitTest.cpp
g++ -std=c++11 -pthread unitTest.cpp Scheduler.cpp Elevator.cpp FloorSubsystem.cpp Button.cpp Globals.cpp -o unitTest -lgtest -lgtest_main

## Notes
UML and State machine diagrams are also in the src directory.
