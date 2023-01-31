cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb -DDEBUG -Wall -Wextra -Wconversion -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wconversion -Wno-pragmas")

project(micro-webservice)

set(BOOST_ROOT $ENV{BOOST_ROOT})
find_package(Boost 1.71.0 COMPONENTS date_time REQUIRED)
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libs: ${Boost_LIBRARIES}")

add_executable(micro-webservice 
  server.cpp
  httpworker.cpp
)
set_target_properties(micro-webservice PROPERTIES LINK_FLAGS_RELEASE "-dead_strip")

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")
  set(PLATFORM_DEPENDENT_LIBRARIES, "-lpthread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
else()
  set(PLATFORM_DEPENDENT_LIBRARIES, "")
endif()

add_custom_command(TARGET micro-webservice 
  POST_BUILD
  COMMAND strip micro-webservice)

target_include_directories(micro-webservice
	PRIVATE ${PROJECT_INCLUDE_DIRS}
	PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(micro-webservice
	${Boost_LIBRARIES}
)

add_custom_command(TARGET micro-webservice
  POST_BUILD
  COMMAND strip micro-webservice)

install(TARGETS micro-webservice RUNTIME DESTINATION bin)
