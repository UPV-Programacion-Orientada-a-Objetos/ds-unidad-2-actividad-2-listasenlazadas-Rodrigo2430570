cmake_minimum_required(VERSION 3.16)
project(PRT7Decoder CXX)

# Force static runtime optional for MinGW; keep simple
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Sources
set(SOURCES
  src/main.cpp
  src/serial_win32.cpp
  src/listas.cpp
  src/tramas.cpp
)

set(HEADERS
  include/serial.h
  include/listas.h
  include/tramas.h
)

add_executable(prt7 ${SOURCES} ${HEADERS})
target_include_directories(prt7 PRIVATE include)

if(WIN32)
  target_link_libraries(prt7 PRIVATE setupapi)
endif()

# Doxygen docs target (optional)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/Doxyfile)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()
