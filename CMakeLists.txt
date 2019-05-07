cmake_minimum_required(VERSION 3.10.2)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message(">>> Building project in DEBUG mode")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

IF(CMAKE_BUILD_TYPE MATCHES Release)
    message(">>> Building project in RELEASE mode")
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenCL")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -l OpenCL")
endif()

# Base direct fourier transform project
project(dft)
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIR})
link_directories(${OpenCL_LIBRARY})
add_executable(dft direct_fourier_transform.c main.c)
target_link_libraries(dft ${OpenCL_LIBRARY} m)

# Unit testing for dft
project(tests)
find_package(GTest REQUIRED)
find_package(OpenCL REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${OPENCL_INCLUDE_DIR})
link_directories(${GTEST_LIBRARIES})
link_directories(${OpenCL_LIBRARY})
add_executable(tests direct_fourier_transform.c unit_testing.cpp)
target_link_libraries(tests ${OpenCL_LIBRARY} ${GTEST_LIBRARIES} pthread)
