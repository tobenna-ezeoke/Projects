# 🚀 Distributed Elevator Control System

A distributed elevator control system developed in **C++** for **SYSC 3303 – Real-Time Concurrent Systems** at Carleton University.

The project simulates the operation of multiple elevators within a multi-storey building using independent software subsystems that communicate over **UDP sockets**. The system demonstrates concurrent programming, inter-process communication, scheduling algorithms, and fault tolerance.

---

## Project Overview

The simulator consists of three independent subsystems:

- **Scheduler**
  - Coordinates communication between all components.
  - Assigns elevator requests using a C-SCAN-inspired scheduling algorithm.
  - Monitors elevator status.
  - Reassigns requests if failures occur.

- **Elevator Subsystem**
  - Simulates one or more elevators.
  - Processes assigned requests.
  - Reports elevator status continuously.
  - Maintains an internal task queue.

- **Floor Subsystem**
  - Generates passenger requests from an input file.
  - Sends requests to the Scheduler.
  - Displays elevator updates received from the Scheduler.

Each subsystem runs as its own executable and communicates over UDP, closely mimicking a distributed real-world elevator system.

---

## Features

- Distributed architecture using UDP sockets
- Multi-elevator simulation
- Concurrent execution using threads
- C-SCAN inspired scheduling algorithm
- Dynamic task assignment
- Elevator status monitoring
- Fault detection and task reassignment
- ncurses-based real-time Scheduler interface
- Unit testing
- UML diagrams and system documentation

---

## Technologies

- C++
- CMake
- UDP Sockets
- POSIX Threads (pthread)
- ncurses
- Object-Oriented Design
- Concurrent Programming

---

## Project Structure

```
iter5/
├── src/
│   ├── Scheduler
│   ├── Elevator
│   ├── Floor
│   ├── TaskQueue
│   ├── GUI
│   ├── Tests
│   └── CMakeLists.txt
│
└── doc/
    ├── UML Diagrams
    ├── Sequence Diagrams
    ├── Timing Diagrams
    └── Documentation
```

---

## Building the Project

### Requirements

- CMake 3.10+
- C++17 Compiler
- pthread
- ncurses

Install ncurses (Ubuntu):

```bash
sudo apt install libncurses-dev
```

### Build

```bash
mkdir build
cd build
cmake ..
make
```

This generates:

```
scheduler_main
elevator_main
floor_main
```

---

## Running the Simulator

Start the Scheduler first:

```bash
./scheduler_main
```

Start each elevator:

```bash
./elevator_main 1
./elevator_main 2
./elevator_main 3
./elevator_main 4
```

Finally, launch the Floor subsystem:

```bash
./floor_main
```

The Scheduler provides a real-time terminal interface displaying:

- Elevator locations
- Direction of travel
- Active requests
- Queue sizes
- Assignment decisions
- Debug messages

---

## Scheduling Algorithm

The Scheduler implements a **C-SCAN-inspired algorithm** to efficiently allocate requests.

It considers:

- Elevator position
- Current direction
- Destination floor
- Estimated travel distance

If an elevator becomes unavailable or a task assignment fails, the Scheduler automatically reassigns the request.

---

## Documentation

The project includes:

- UML Class Diagram
- Sequence Diagram
- State Diagram
- Timing Diagrams
- Design documentation

These can be found in the `doc/` directory.

---

## Skills Demonstrated

- Object-Oriented Programming
- Distributed Systems
- Network Programming
- Concurrent Programming
- Thread Synchronization
- Software Architecture
- Real-Time Systems
- System Simulation
- Unit Testing

---

## Course Information

**Course:** SYSC 3303 – Real-Time Concurrent Systems

Carleton University

---

## Contributors

- Dearell Tobenna Ezeoke
- Deryck Ho
- Matthew Jong
- Brix Velasco
- Syed Shaheer

---

## License

This project is licensed under the MIT License.
