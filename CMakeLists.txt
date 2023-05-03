cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(micro-webservice VERSION 0.1 LANGUAGES CXX)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -pedantic")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
endif (UNIX)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_WIN32_WINDOWS /wd4018 /wd4267 /wd4244 /wd4365 /wd4514 /wd4668 /wd4710 /wd4711 /wd4625 /wd4626 /wd4702 /wd4820 /wd5027 /wd5039 /wd5045 /wd5262")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Od /Wall")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2")
endif (MSVC)

set(BOOST_ROOT $ENV{BOOST_ROOT})
find_package(Boost 1.71.0 REQUIRED)
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libs: ${Boost_LIBRARIES}")

add_executable(micro-webservice 
  server.cpp
  httpworker.cpp
)

if(UNIX)
  set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")
  set(PLATFORM_DEPENDENT_LIBRARIES, "-lpthread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  add_custom_command(TARGET micro-webservice 
    POST_BUILD
    COMMAND strip micro-webservice)
else()
  set(PLATFORM_DEPENDENT_LIBRARIES, "")
endif()

target_include_directories(micro-webservice
	PRIVATE ${PROJECT_INCLUDE_DIRS}
	PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(micro-webservice
	${Boost_LIBRARIES}
)

install(TARGETS micro-webservice RUNTIME DESTINATION bin)
