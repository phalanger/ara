cmake_minimum_required(VERSION 2.8)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

PROJECT(ara)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # require at least gcc 4.8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC version must be at least 4.8!")
    endif()
    set(CMAKE_CXX_FLAGS "-std=c++11 -O2 ${CMAKE_CXX_FLAGS}")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    # require at least clang 3.2
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
        message(FATAL_ERROR "Clang version must be at least 3.2!")
    endif()
    set(CMAKE_CXX_FLAGS "-std=c++11 -O2 ${CMAKE_CXX_FLAGS}")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    # require at least VS2015
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
        message(FATAL_ERROR "VC version must be at least 2015!")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with VC, Clang and GCC.")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if($ENV{TRAVIS})
    if($ENV{TRAVIS} STREQUAL "true")
        message(STATUS "Building on Travis-CI.")
        set(IS_TRAVIS_BUILD TRUE)
    endif()
endif()

option(GB_GENERATE_COVERAGE_INFO "If set, line coverage info will be generated from debug test runs." OFF)
if(GB_GENERATE_COVERAGE_INFO)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # this is an undocumented option for changing the output extension from *.cpp.ext
        # to *.ext; As there is no reasonable way to obtain the list of object files
        # this seems to be the best option to generate the naming gcov expects.
        set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
        set(GCOV_COMMAND gcov CACHE STRING "Command to invoke to run gcov.")
    endif()
endif()

set(LIB_MAJOR_VERSION "0")
set(LIB_MINOR_VERSION "0")
set(LIB_PATCH_VERSION "1")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

include_directories("./")

file(GLOB ARA_HEADERS ara/*.h)
file(GLOB ARA_INTERNAL_HEADERS "ara/internal/*.h")
SOURCE_GROUP("ara" FILES ${ARA_HEADERS})
SOURCE_GROUP("ara\\internal" FILES ${ARA_INTERNAL_HEADERS})
install(DIRECTORY ara/ DESTINATION include/ara FILES_MATCHING PATTERN "*.h")

file(GLOB ARA_ASYNC_TEST_SRC test/test_async_*.cpp)
file(GLOB ARA_TEST_SRC test/*.cpp)
list(REMOVE_ITEM ARA_TEST_SRC ${ARA_ASYNC_TEST_SRC})

# Find boost
INCLUDE(FindBoost)
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.56.0 COMPONENTS date_time thread system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(unittest ${ARA_TEST_SRC} ${ARA_HEADERS} ${ARA_INTERNAL_HEADERS} ${ARA_ASYNC_TEST_SRC})
    target_link_libraries(unittest ${Boost_LIBRARIES})
else()
    add_executable(unittest ${ARA_TEST_SRC} ${ARA_HEADERS} ${ARA_INTERNAL_HEADERS})
endif()

#############################################################
# set link argument

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC unittest "-pthread -lrt")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(unittest "${CMAKE_THREAD_LIBS_INIT}" rt)
endif()

#############################################################
# add test

enable_testing()
add_test(NAME AraTest COMMAND unittest)
