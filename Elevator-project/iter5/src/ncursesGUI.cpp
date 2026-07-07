#include "ncursesGUI.h"
#include "Globals.h"
#include <ncurses.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <sstream>

// These extern declarations assume that debugLogs and logMutex are defined in a shared source.
extern std::vector<DebugLogEntry> debugLogs;
extern std::mutex logMutex;

int getDoorColor(DoorStatus ds) {
    switch(ds) {
        case DOOR_OPENING: return 2;  // Yellow
        case DOOR_OPEN:    return 3;  // Green
        case DOOR_CLOSING: return 2;  // Yellow
        case DOOR_CLOSED:  return 4;  // Red
        case DOOR_STUCK:   return 4;  // Red
        default:           return 0;
    }
}

void ncursesGUI(Scheduler* scheduler) {
    initscr();              // Start curses mode.
    cbreak();               // Disable line buffering.
    noecho();               // Don't echo input.
    start_color();          // Enable colors.
    // Initialize ncurses color pairs.
    init_pair(1, COLOR_CYAN, COLOR_BLACK);    // For REACHED status.
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);   // For IN_TRANSIT and door closing.
    init_pair(3, COLOR_GREEN, COLOR_BLACK);    // For STANDBY, UP, and door OPEN.
    init_pair(4, COLOR_RED, COLOR_BLACK);      // For DOWN and also FAULT.
    init_pair(5, COLOR_BLUE, COLOR_BLACK);     // (for headers)
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);  // (unused here; can be used if needed)
    // Define a new color pair (7) for "lime" for DOOR_OPENING if desired.
    if (can_change_color()) {
        init_color(COLOR_GREEN, 500, 1000, 500);
    }
    init_pair(7, COLOR_GREEN, COLOR_BLACK);
    
    int totalRows, totalCols;
    getmaxyx(stdscr, totalRows, totalCols);
    // Reserve bottom 10 lines for logs and info.
    int statusWinHeight = totalRows - 10;
    WINDOW* statusWin = newwin(statusWinHeight, totalCols, 0, 0);
    WINDOW* infoWin = newwin(10, totalCols, statusWinHeight, 0);
    
    while (true) {
        werase(statusWin);
        box(statusWin, 0, 0);
        mvwprintw(statusWin, 1, 2, "Elevator Statuses (Scheduler)");
        // Print header: Removed the Queue column.
        mvwprintw(statusWin, 2, 2, "ID");
        mvwprintw(statusWin, 2, 8, "Current");
        mvwprintw(statusWin, 2, 18, "Target");
        mvwprintw(statusWin, 2, 26, "Direction");
        mvwprintw(statusWin, 2, 38, "Status");
        mvwprintw(statusWin, 2, 50, "Door");
        mvwprintw(statusWin, 2, 62, "Load/Cap");
        
        int row = 3;
        auto statuses = scheduler->getElevatorStatuses();
        for (const auto& pair : statuses) {
            const auto& s = pair.second;
            mvwprintw(statusWin, row, 2, "%-4d", s.elevatorID);
            mvwprintw(statusWin, row, 8, "%-8d", s.currentFloor);
            mvwprintw(statusWin, row, 18, "%-8d", s.destination);
            // Display direction.
            if (s.travelDirection == UP) {
                wattron(statusWin, COLOR_PAIR(3)); // Green.
                mvwprintw(statusWin, row, 26, "%-10s", "UP");
                wattroff(statusWin, COLOR_PAIR(3));
            } else {
                wattron(statusWin, COLOR_PAIR(4)); // Red.
                mvwprintw(statusWin, row, 26, "%-10s", "DOWN");
                wattroff(statusWin, COLOR_PAIR(4));
            }
            // Display status, including FAULT in red.
            if (s.status == STANDBY) {
                wattron(statusWin, COLOR_PAIR(3)); // Green.
                mvwprintw(statusWin, row, 38, "%-10s", "STANDBY");
                wattroff(statusWin, COLOR_PAIR(3));
            } else if (s.status == IN_TRANSIT) {
                wattron(statusWin, COLOR_PAIR(2)); // Yellow.
                mvwprintw(statusWin, row, 38, "%-10s", "IN_TRANSIT");
                wattroff(statusWin, COLOR_PAIR(2));
            } else if (s.status == REACHED) {
                wattron(statusWin, COLOR_PAIR(1)); // Cyan.
                mvwprintw(statusWin, row, 38, "%-10s", "REACHED");
                wattroff(statusWin, COLOR_PAIR(1));
            } else if (s.status == FAULT) {
                wattron(statusWin, COLOR_PAIR(4)); // Red.
                mvwprintw(statusWin, row, 38, "%-10s", "FAULT");
                wattroff(statusWin, COLOR_PAIR(4));
            } else if (s.status == PFAULT) {
                wattron(statusWin, COLOR_PAIR(4)); // Red.
                mvwprintw(statusWin, row, 38, "%-10s", "PENDING FAULT");
                wattroff(statusWin, COLOR_PAIR(4));
            }
            // Display door status.
            wattron(statusWin, COLOR_PAIR(getDoorColor(s.doorStatus)));
            mvwprintw(statusWin, row, 50, "%-10s",
                (s.doorStatus == DOOR_CLOSED ? "CLOSED" :
                 (s.doorStatus == DOOR_OPENING ? "OPENING" :
                 (s.doorStatus == DOOR_OPEN ? "OPEN" :
                 (s.doorStatus == DOOR_CLOSING ? "CLOSING" : "STUCK")))));
      
            wattroff(statusWin, COLOR_PAIR(getDoorColor(s.doorStatus)));
            // Display load/capacity.
            std::ostringstream loadOss;
            loadOss << s.currentLoad << "/" << s.maxCapacity;
            mvwprintw(statusWin, row, 62, "%-10s", loadOss.str().c_str());
            row++;
        }
        wrefresh(statusWin);
        
        // Update info window: display global task queue size and recent debug logs.
        werase(infoWin);
        box(infoWin, 0, 0);
        std::ostringstream infoOss;
        infoOss << "Global Task Queue Size: " << scheduler->getQueueSize();
        mvwprintw(infoWin, 1, 2, infoOss.str().c_str());
        int logRow = 3;
        {
            std::lock_guard<std::mutex> lock(logMutex);
            int start = (debugLogs.size() > 5) ? debugLogs.size() - 5 : 0;
            for (size_t i = start; i < debugLogs.size(); i++) {
                wattron(infoWin, COLOR_PAIR(debugLogs[i].colorPair));
                mvwprintw(infoWin, logRow++, 2, "%s", debugLogs[i].text.c_str());
                wattroff(infoWin, COLOR_PAIR(debugLogs[i].colorPair));
            }
        }
        wrefresh(infoWin);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    delwin(statusWin);
    delwin(infoWin);
    endwin();
}
