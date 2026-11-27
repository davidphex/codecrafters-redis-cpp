cmake_minimum_required(VERSION 3.13)

project(redis-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)

add_executable(redis ${SOURCE_FILES}
        src/main.cpp
        src/resp/Parser.h
        src/resp/Serializer.h
        src/storage/StorageEngine.h
        src/storage/RedisObject.h
        src/CommandHandler.cpp
        src/CommandHandler.h
        src/storage/StorageEngine.cpp
        src/network/Server.cpp
        src/network/Server.h
        src/network/Socket.h
        src/network/Connection.h
        src/network/Poller.cpp
        src/network/Poller.h
        src/CommandRouter.cpp
        src/CommandRouter.h
        src/network/Socket.cpp
        src/network/Connection.cpp)

target_link_libraries(redis PRIVATE asio asio::asio)
target_link_libraries(redis PRIVATE Threads::Threads)