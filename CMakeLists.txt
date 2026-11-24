cmake_minimum_required(VERSION 3.20)
project(CallCenterPatterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(PROJECT_HEADERS
    include/Operator.h
    include/Call.h
    include/CallStateHistory.h
    include/Database.h
    include/IOperatorRepository.h
    include/SQLiteOperatorRepository.h
    include/TestData.h
    include/CallState.h
    include/CallContext.h
    include/IPriorityStrategy.h
    include/PriorityStrategies.h
    include/CallObserver.h
    include/CallCommand.h
)

set(PROJECT_CORE_SOURCES
    src/SQLiteOperatorRepository.cpp
    src/TestData.cpp
    src/CallState.cpp
    src/CallContext.cpp
    src/PriorityStrategies.cpp
    src/CallObserver.cpp
    src/CallCommand.cpp
)

add_executable(CallCenterApp
    src/main.cpp
    ${PROJECT_CORE_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(CallCenterApp PRIVATE include)
target_link_libraries(CallCenterApp PRIVATE
    sqlite_orm::sqlite_orm
    unofficial::sqlite3::sqlite3
)

add_executable(CallCenterTests
    src/Tests.cpp
    ${PROJECT_CORE_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(CallCenterTests PRIVATE include)
target_link_libraries(CallCenterTests PRIVATE
    sqlite_orm::sqlite_orm
    unofficial::sqlite3::sqlite3
)