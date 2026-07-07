cmake_minimum_required(VERSION 3.10)
project(ElevatorSimulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ncurses.
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# Common source and header files (without GUI code).
set(COMMON_SOURCES
    Elevator.cpp
    Elevator.h
    Floor.cpp
    Floor.h
    Scheduler.cpp
    Scheduler.h
    TaskQueue.cpp
    TaskQueue.h
    Globals.h
    Globals.cpp    # Contains debugLogs, logMutex, and addLog.
)

# GUI sources (only needed for targets that display the GUI).
set(GUI_SOURCES
    ncursesGUI.cpp
    ncursesGUI.h
)

# Option to build the original monolithic main executable.
option(BUILD_MAIN "Build the combined main executable" ON)

# Build the monolithic main executable (uses GUI).
if(BUILD_MAIN)
    add_executable(main_exe main.cpp ${COMMON_SOURCES} ${GUI_SOURCES})
    target_link_libraries(main_exe pthread ${CURSES_LIBRARIES})
endif()

# Build the Scheduler executable (uses GUI).
add_executable(scheduler_main scheduler_main.cpp ${COMMON_SOURCES} ${GUI_SOURCES})
target_link_libraries(scheduler_main pthread ${CURSES_LIBRARIES})

# Build the Elevator executable (does NOT use GUI).
add_executable(elevator_main elevator_main.cpp ${COMMON_SOURCES})
target_link_libraries(elevator_main pthread)

# Build the Floor executable (assuming it does not use the GUI).
add_executable(floor_main floor_main.cpp ${COMMON_SOURCES})
target_link_libraries(floor_main pthread)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Unit test executable
add_executable(test_elevator 
    test_elevator.cpp 
    Elevator.cpp 
    Scheduler.cpp 
    TaskQueue.cpp 
    Globals.cpp
)
target_link_libraries(test_elevator ${GTEST_LIBRARIES} pthread)


# add_executable(test_floor 
#     test_floor.cpp 
#     Floor.cpp 
#     Elevator.cpp 
#     Scheduler.cpp 
#     TaskQueue.cpp 
#     Globals.cpp
# )
# target_link_libraries(test_floor ${GTEST_LIBRARIES} pthread)

add_executable(test_scheduler 
    test_scheduler.cpp 
    Scheduler.cpp 
    TaskQueue.cpp 
    Globals.cpp
)
target_link_libraries(test_scheduler ${GTEST_LIBRARIES} pthread)

add_executable(test_task_queue
    test_task_queue.cpp
    Scheduler.cpp    
    TaskQueue.cpp 
    Globals.cpp
)
target_link_libraries(test_task_queue ${GTEST_LIBRARIES} pthread)


