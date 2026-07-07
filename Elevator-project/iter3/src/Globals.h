#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <vector>
#include <mutex>

struct DebugLogEntry {
    std::string text;
    int colorPair;
};

extern std::vector<DebugLogEntry> debugLogs;
extern std::mutex logMutex;

// Declare the global logging function.
extern void addLog(const std::string& logEntry, int colorPair);

#ifdef USE_NCURSES_LOGGING
  #define DEBUG_LOG_COLOR(msg, color) addLog(msg, color)
#else
  #define DEBUG_LOG_COLOR(msg, color) ((void)0)
#endif

enum Direction {
    UP,
    DOWN
};

enum ElevatorStatus {
    STANDBY,
    IN_TRANSIT,
    REACHED
};

enum DoorStatus {
    DOOR_CLOSED,
    DOOR_OPENING,
    DOOR_OPEN,
    DOOR_CLOSING
};

#define ELEVATOR_TRAVEL_TIME 1500 // ms
#define ELEVATOR_DOOR_LOADING_TIME 10000 // ms
#define ELEVATOR_DOOR_OPEN_TIME 5000 // ms
#define ELEVATOR_DOOR_CLOSE_TIME 5000 // ms

struct ElevatorStatusData {
    int elevatorID;
    int destination;
    Direction travelDirection;
    int currentFloor;
    ElevatorStatus status;
    DoorStatus doorStatus;

    ElevatorStatusData()
      : elevatorID(-1), destination(-1), travelDirection(UP), currentFloor(-1),
        status(STANDBY), doorStatus(DOOR_CLOSED) {}

    ElevatorStatusData(int id, int dest, Direction dir, int floor, ElevatorStatus st = STANDBY, DoorStatus ds = DOOR_CLOSED)
        : elevatorID(id), destination(dest), travelDirection(dir), currentFloor(floor),
          status(st), doorStatus(ds) {}
};

#endif // GLOBALS_H
