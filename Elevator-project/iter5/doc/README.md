
# Elevator Simulator

  

A distributed elevator simulator in C++ that models realistic elevator behavior in a multi-floor building using multiple executables. The system is composed of separate Scheduler, Elevator, and Floor processes that communicate via UDP.

  

## Overview of Subsystems

  

-  **Scheduler:**

- Listens on UDP port 5000.

- Receives floor requests and elevator status updates.

- Maintains an internal task queue and an elevator status map.

- Uses a C‑scan–style algorithm to select the best elevator for each floor request.

- Forwards `TASK_ASSIGN` messages to elevators and informs the Floor process of status updates.

  

-  **Elevator:**

- Each elevator process is started with a unique elevator ID.

- Binds to a unique UDP port (5001 + elevatorID).

- Maintains its own internal task queue.

- Processes incoming `TASK_ASSIGN` messages sequentially.

- Sends continuous status updates (including current floor, destination, direction, and door state) to the Scheduler.

- Notifies the Scheduler if a task is discarded.

  

-  **Floor:**

- Sends floor requests (e.g., read from an input file) to the Scheduler.

- Listens for elevator status updates forwarded by the Scheduler.

  

## Features

  

-  **Distributed Architecture:**

Separate executables for Scheduler, Elevator, and Floor processes communicate over UDP.

  

-  **Realistic Scheduling:**

Implements a C‑scan–style algorithm to choose the optimal elevator from a fleet (e.g., 4 elevators serving 22 floors).

  

-  **Task Queueing:**

Each elevator maintains a local task queue to process multiple tasks sequentially.

  

-  **Robust Error Handling:**

If a task assignment fails, the Scheduler re‑queues the task to avoid losing it.

  

-  **Extensive Logging:**

The system logs key events such as floor requests, elevator status updates, task assignments, and any discarded tasks for easier debugging.

  
## Prerequisites

  

-  **CMake:** Version 3.10 or higher.

-  **Compiler:** A C++17 compliant compiler (GCC, Clang, etc.).

-  **Libraries:**

- pthread

- ncurses (if not installed, `sudo apt-get install libncurses-dev` - this is required for the UI to function)

- gtest (for unit tests)

## Build Instructions

  

This project uses [CMake](https://cmake.org/) as its build system.
1.  **Clone the Repository:**\
or just download this if you're the TA marking this

```bash
git clone <repository_url>
cd elevator_simulator
```
 
2. **Create a Build Directory & Run  CMake:**

```bash
mkdir build && cd build
cmake ..
```
  

## Compile:
  ```bash
make
```
(Optionally, for faster compilation, `make -jn`, where n is the amount of cores you want to use to compile.) 
This will generate three executables:
```
scheduler_main
elevator_main
floor_main
```

## Run Instructions
Follow this order to run the system:\
**Start the Scheduler:**
```bash 
./scheduler_main
```
Binds to UDP port 5000.
Displays a real-time ncurses GUI with debug logs and elevator statuses.
**IMPORTANT** Ensure the terminal is big enough before starting!! Otherwise, the GUI will not show.

**Start the Elevators:**
Launch each elevator process with a unique ID (for example, 1 through 4):
```bash
./elevator_main 1
./elevator_main 2
./elevator_main 3
./elevator_main 4
```

Each elevator binds to port 5001 + elevatorID (i.e., ports 5002, 5003, etc.).
Continuously sends status updates and processes tasks from its internal queue.

**Start the Floor Process:**
```bash
./floor_main
```
Binds to UDP port 5001.
Sends floor requests to the Scheduler.
Listens for elevator status updates.


## Scheduling Algorithm

The Scheduler employs a C‑scan–style algorithm:

**Wrap-Around Distance:**
For each idle elevator, the Scheduler computes the wrap-around distance from the elevator’s current floor to the requested floor.\
**Direction Consideration:**
If an elevator is idle, the distance is calculated simply. For moving elevators, a penalty is added if the elevator is moving opposite the desired direction.\
**Optimal Selection:**
The elevator with the smallest computed distance is chosen for the floor request.\
**Task Reassignment:**
If sending a `TASK_ASSIGN` message fails, the task is re‑queued for retry.

## Debugging

Scheduler:
Uses a detailed debug log (via `ncurses`) to show:
Incoming floor requests.
Elevator statuses and task queue size.
Assignment attempts and any re-queued tasks.

Elevator:
Outputs its current status and local task queue size to the console.

Floor:
Logs when it sends floor requests and receives status updates.

These logs are essential to diagnose issues such as tasks not being assigned or updates not reaching the intended component.

## Contributing
Contributions and suggestions are welcome! Please fork the repository and submit a pull request or open an issue for any bugs or feature requests.

## License

  This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgements

  Thanks to the open-source community for their contributions to networking and concurrency libraries in C++.

Special thanks to contributors who provided feedback and suggestions to improve the elevator scheduling algorithm and overall system design.
