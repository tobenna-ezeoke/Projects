#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <vector>
#include <mutex>
#include <atomic>

// Structure for a debug log entry.
struct DebugLogEntry {
    std::string text;
    int colorPair;
};

extern std::vector<DebugLogEntry> debugLogs;
extern std::mutex logMutex;

// Global logging function declaration.
extern void addLog(const std::string& logEntry, int colorPair);

// Debug logging macro.
#ifdef USE_NCURSES_LOGGING
  #define DEBUG_LOG_COLOR(msg, color) addLog(msg, color)
#else
  #define DEBUG_LOG_COLOR(msg, color) ((void)0)
#endif

// Elevator system enumerations.
enum Direction { UP, DOWN };

enum ElevatorStatus { STANDBY, IN_TRANSIT, REACHED, PFAULT, FAULT };

enum DoorStatus { DOOR_CLOSED, DOOR_OPENING, DOOR_OPEN, DOOR_CLOSING, DOOR_STUCK };

// Timing and simulation constants.
#define ELEVATOR_TRAVEL_TIME 1500 // ms
#define ELEVATOR_DOOR_LOADING_TIME 10000 // ms
#define ELEVATOR_DOOR_OPEN_TIME 5000 // ms
#define ELEVATOR_DOOR_CLOSE_TIME 5000 // ms
#define ELEVATOR_DOOR_STUCK_TIME 10000 //ms
#define FAULT_TIMEOUT_SECONDS 120 //s
#define MOVE_TIMEOUT_SECONDS 120  //s

// Elevator status data structure.
struct ElevatorStatusData {
    int elevatorID;
    int destination;
    Direction travelDirection;
    int currentFloor;
    ElevatorStatus status;
    DoorStatus doorStatus;
    int currentLoad;
    int maxCapacity;
    int queueSize;  // Local internal task queue size

    ElevatorStatusData()
      : elevatorID(-1), destination(-1), travelDirection(UP), currentFloor(-1),
        status(STANDBY), doorStatus(DOOR_CLOSED), currentLoad(0), maxCapacity(0), queueSize(0) {}

    ElevatorStatusData(int id, int dest, Direction dir, int floor,
                       ElevatorStatus st = STANDBY, DoorStatus ds = DOOR_CLOSED,
                       int load = 0, int cap = 0, int qSize = 0)
        : elevatorID(id), destination(dest), travelDirection(dir), currentFloor(floor),
          status(st), doorStatus(ds), currentLoad(load), maxCapacity(cap), queueSize(qSize) {}
};

// Declare the fault injection flag as extern.
// extern std::atomic<bool> faultInjectedTimeout;

#endif // GLOBALS_H
