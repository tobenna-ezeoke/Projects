#include "ncursesGUI.h"
#include "Globals.h"
#include <ncurses.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <sstream>

// These extern declarations assume that debugLogs and logMutex are defined in one shared source.
extern std::vector<DebugLogEntry> debugLogs;
extern std::mutex logMutex;

int getDoorColor(DoorStatus ds) {
    switch(ds) {
        case DOOR_OPENING: return 2;  // Yellow
        case DOOR_OPEN:    return 3;  // Green
        case DOOR_CLOSING: return 2;  // Yellow
        case DOOR_CLOSED:  return 4;  // Red
        default:           return 0;
    }
}

void ncursesGUI(Scheduler* scheduler) {
    initscr();              // Start curses mode.
    cbreak();               // Disable line buffering.
    noecho();               // Don't echo input.
    start_color();          // Enable colors.
    // Initialize ncurses color pairs.
    init_pair(1, COLOR_CYAN, COLOR_BLACK);    // For REACHED
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);   // For IN_TRANSIT and door closing
    init_pair(3, COLOR_GREEN, COLOR_BLACK);    // For STANDBY, UP, and door OPEN
    init_pair(4, COLOR_RED, COLOR_BLACK);      // For DOWN and door CLOSED
    init_pair(5, COLOR_BLUE, COLOR_BLACK);     // (unused here)
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);  // (unused here)
    // Define a new color pair (7) for "lime" for DOOR_OPENING.
    if (can_change_color()) {
        init_color(COLOR_GREEN, 500, 1000, 500);
    }
    init_pair(7, COLOR_GREEN, COLOR_BLACK);
    
    int totalRows, totalCols;
    getmaxyx(stdscr, totalRows, totalCols);
    int statusWinHeight = totalRows - 25;  // Reserve bottom 25 lines for logs.
    int logWinHeight = 25;
    WINDOW* statusWin = newwin(statusWinHeight, totalCols, 0, 0);
    WINDOW* logWin = newwin(logWinHeight, totalCols, statusWinHeight, 0);
    
    while (true) {
        // Update the status window.
        werase(statusWin);
        box(statusWin, 0, 0);
        mvwprintw(statusWin, 1, 2, "Elevator Statuses");
        // Print header.
        mvwprintw(statusWin, 2, 2, "ID   Current  Target");
        mvwprintw(statusWin, 2, 24, "Direction");
        mvwprintw(statusWin, 2, 35, "Status");
        mvwprintw(statusWin, 2, 55, "Door");
        int row = 3;
        auto statuses = scheduler->getElevatorStatuses();
        for (const auto& pair : statuses) {
            const auto& s = pair.second;
            mvwprintw(statusWin, row, 2, "%-4d %-8d %-7d", 
                      s.elevatorID, s.currentFloor, s.destination);
            if (s.travelDirection == UP) {
                wattron(statusWin, COLOR_PAIR(3)); // green
                mvwprintw(statusWin, row, 24, "%-10s", "UP");
                wattroff(statusWin, COLOR_PAIR(3));
            } else {
                wattron(statusWin, COLOR_PAIR(4)); // red
                mvwprintw(statusWin, row, 24, "%-10s", "DOWN");
                wattroff(statusWin, COLOR_PAIR(4));
            }
            if (s.status == STANDBY) {
                wattron(statusWin, COLOR_PAIR(3)); // green
                mvwprintw(statusWin, row, 35, "%-10s", "STANDBY");
                wattroff(statusWin, COLOR_PAIR(3));
            } else if (s.status == IN_TRANSIT) {
                wattron(statusWin, COLOR_PAIR(6)); // magenta
                mvwprintw(statusWin, row, 35, "%-10s", "IN_TRANSIT");
                wattroff(statusWin, COLOR_PAIR(6));
            } else if (s.status == REACHED) {
                wattron(statusWin, COLOR_PAIR(1)); // cyan
                mvwprintw(statusWin, row, 35, "%-10s", "REACHED");
                wattroff(statusWin, COLOR_PAIR(1));
            }
            wattron(statusWin, COLOR_PAIR(getDoorColor(s.doorStatus)));
            mvwprintw(statusWin, row, 55, "%-10s",
                      (s.doorStatus == DOOR_CLOSED ? "CLOSED" :
                       (s.doorStatus == DOOR_OPENING ? "OPENING" :
                        (s.doorStatus == DOOR_OPEN ? "OPEN" : "CLOSING"))));
            wattroff(statusWin, COLOR_PAIR(getDoorColor(s.doorStatus)));
            row++;
        }
        wrefresh(statusWin);
        
        // Update the log window.
        werase(logWin);
        box(logWin, 0, 0);
        mvwprintw(logWin, 1, 2, "Debug Logs:");
        {
            std::lock_guard<std::mutex> lock(logMutex);
            int logRow = 2;
            int maxLogEntries = logWinHeight - 3;
            int startIndex = (debugLogs.size() > (size_t)maxLogEntries) ? debugLogs.size() - maxLogEntries : 0;
            for (size_t i = startIndex; i < debugLogs.size(); i++) {
                wattron(logWin, COLOR_PAIR(debugLogs[i].colorPair));
                mvwprintw(logWin, logRow++, 2, "%s", debugLogs[i].text.c_str());
                wattroff(logWin, COLOR_PAIR(debugLogs[i].colorPair));
            }
        }
        wrefresh(logWin);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Cleanup (unreachable).
    delwin(statusWin);
    delwin(logWin);
    endwin();
}
