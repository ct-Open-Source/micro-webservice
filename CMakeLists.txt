cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)

project(micro-webservice VERSION 3 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -pedantic")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -Wno-deprecated-declarations")
endif (UNIX)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_WIN32_WINDOWS /wd4018 /wd4267 /wd4244 /wd4365 /wd4514 /wd4668 /wd4710 /wd4711 /wd4625 /wd4626 /wd4702 /wd4820 /wd5027 /wd5039 /wd5045 /wd5262")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Od /Wall")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O2")
endif (MSVC)

set(BOOST_ROOT $ENV{BOOST_ROOT})
find_package(Boost 1.81.0 REQUIRED COMPONENTS url)
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libs: ${Boost_LIBRARIES}")

if (${Boost_VERSION} VERSION_LESS "1.81.0")
  add_definitions(-DUSE_STRINGVIEW_TOSTRING)
endif()

# GMP
find_path(GMP_C_INCLUDES
  NAMES gmp.h
  HINTS $ENV{GMP_ROOT}/include
  REQUIRED
  DOC "GMP C header"
)
message(STATUS "GMP include dir: ${GMP_C_INCLUDES}")
find_library(GMP_C_LIBRARIES
  NAMES gmp libgmp-13.lib 
  HINTS $ENV{GMP_ROOT}/lib
  REQUIRED
  DOC "GMP C libraries"
)
cmake_path(GET GMP_C_LIBRARIES PARENT_PATH GMP_LIB_DIR)
message(STATUS "GMP lib dir: ${GMP_LIB_DIR}")

include_directories(${GMP_C_INCLUDES})
link_directories(${GMP_LIB_DIR})

add_executable(micro-webservice 
  src/main.cpp
  src/httpworker.cpp
  src/helper.cpp
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
	gmp
)

install(TARGETS micro-webservice RUNTIME DESTINATION bin)
