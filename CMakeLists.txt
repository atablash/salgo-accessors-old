cmake_minimum_required (VERSION 3.0)
project (salgo)

if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)









#===============================================================================
# BUILD OPTIONS
#===============================================================================
option(SALGO_BUILD_TESTS "Build salgo tests" OFF)







#===============================================================================
# Additional cmake modules
#===============================================================================
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#set(CMAKE_CXX_STANDARD 17) # not supported by older cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe -march=native -mtune=native -mfpmath=sse")






#===============================================================================
# set compiler options, output all warnings
#===============================================================================
add_definitions(
	-Wall -Wextra -Werror -Wshadow
)







#===============================================================================
# packages
#===============================================================================
find_package(glog REQUIRED)
if (GLOG_FOUND)
	link_libraries(glog)
endif ()

#find_package(Eigen3 3.2 REQUIRED)
#if (EIGEN3_FOUND)
#	include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
#endif()






#===============================================================================
# testing
#===============================================================================
if(SALGO_BUILD_TESTS)
	enable_testing()
	find_package(GTest REQUIRED)
	include_directories(${GTEST_INCLUDE_DIRS})
	add_subdirectory(test)
endif(SALGO_BUILD_TESTS)









#===============================================================================
# output build configuration
#===============================================================================
message("\n")
message("Build configuration:\n")
message("** Build type: " ${CMAKE_BUILD_TYPE})
message("** Build tests: " ${SALGO_BUILD_TESTS})
#message("** salgo version: " ${SALGO_VERSION})
#message("** Build shared libs: " ${SALGO_BUILD_SHARED})
#message("** Build docs: " ${SALGO_BUILD_DOC})
message("\n")


