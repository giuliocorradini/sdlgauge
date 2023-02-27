cmake_minimum_required(VERSION 3.13)

# set the project name
project(KartDashboard VERSION 1.0)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

install(
    TARGETS dashboard
    CONFIGURATIONS Release
    RUNTIME DESTINATION bin
)