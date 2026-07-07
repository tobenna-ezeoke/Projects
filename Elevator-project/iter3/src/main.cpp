#include "Elevator.h"
#include "Floor.h"
#include "Scheduler.h"
#include "TaskQueue.h"
#include "Globals.h"
#include "ncursesGUI.h"  // For GUI functionality.
#include <thread>
#include <chrono>
#include <ncurses.h>
#include <vector>
#include <string>
#include <mutex>

// Do not define debugLogs, logMutex, or addLog here.
// They are defined in GlobalVars.cpp.

// Worker thread functions remain unchanged...

int main() {
    TaskQueue taskQueue;
    Scheduler scheduler(&taskQueue);
    Elevator elevator1(1, 22, &scheduler, &taskQueue);
    Elevator elevator2(2, 22, &scheduler, &taskQueue);
    Elevator elevator3(3, 22, &scheduler, &taskQueue);
    Elevator elevator4(4, 22, &scheduler, &taskQueue);
    Floor floor(&scheduler);
    
    // Launch the ncurses GUI in its own thread.
    std::thread guiThread(ncursesGUI, &scheduler);
    // Launch other worker threads.
    std::thread schedulerThread([](Scheduler* s) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }, &scheduler);
    std::thread floorDisplayThread([&floor](){
        while (true) {
            floor.updateDisplay();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    std::thread floorThread([&floor](){
        floor.pressCallButton();
    });
    std::thread elevatorThread1([](Elevator* e) {
        while (true) {
            e->handleRequests();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }, &elevator1);
    std::thread elevatorThread2([](Elevator* e) {
        while (true) {
            e->handleRequests();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }, &elevator2);
    std::thread elevatorThread3([](Elevator* e) {
        while (true) {
            e->handleRequests();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }, &elevator3);
    std::thread elevatorThread4([](Elevator* e) {
        while (true) {
            e->handleRequests();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }, &elevator4);

    // Join threads.
    floorThread.join();
    elevatorThread1.join();
    elevatorThread2.join();
    elevatorThread3.join();
    elevatorThread4.join();
    schedulerThread.join();
    floorDisplayThread.join();
    guiThread.join();
    
    return 0;
}
