cmake_minimum_required(VERSION 3.16)
project(kurrencykonverter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Network Widgets)
find_package(KF6Runner REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6KCMUtils REQUIRED)

add_subdirectory(src)
