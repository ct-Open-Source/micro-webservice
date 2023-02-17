cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb -DDEBUG -pedantic -Wall -Wextra -Wconversion -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wno-deprecated")

project(prime-webservice)

set(BOOST_ROOT $ENV{BOOST_ROOT})
find_package(Boost 1.81.0 REQUIRED COMPONENTS regex url)
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libs: ${Boost_LIBRARIES}")

if (${Boost_VERSION} VERSION_LESS "1.81.0")
  add_definitions(-DUSE_STRINGVIEW_TOSTRING)
endif()

# GMP
find_path(GMP_C_INCLUDES
  NAMES gmp.h
  DOC "GMP C header"
)
message(STATUS "GMP include dir: ${GMP_C_INCLUDES}")
find_library(GMP_C_LIBRARIES
  NAMES gmp
  DOC "GMP C libraries"
)
cmake_path(GET GMP_C_LIBRARIES PARENT_PATH GMP_LIB_DIR)
message(STATUS "GMP lib dir: ${GMP_LIB_DIR}")

include_directories(${GMP_C_INCLUDES})
link_directories(${GMP_LIB_DIR})

add_executable(prime-webservice 
  main.cpp
  httpworker.cpp
  helper.cpp
  trip/router.cpp
)
set_target_properties(prime-webservice PROPERTIES LINK_FLAGS_RELEASE "-dead_strip")

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")
  set(PLATFORM_DEPENDENT_LIBRARIES, "-lpthread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
else()
  set(PLATFORM_DEPENDENT_LIBRARIES, "")
endif()

add_custom_command(TARGET prime-webservice 
  POST_BUILD
  COMMAND strip prime-webservice)

target_include_directories(prime-webservice
	PRIVATE ${PROJECT_INCLUDE_DIRS}
	PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(prime-webservice
	${Boost_LIBRARIES}
  gmp
)

add_custom_command(TARGET prime-webservice
  POST_BUILD
  COMMAND strip prime-webservice)

install(TARGETS prime-webservice RUNTIME DESTINATION bin)
