cmake_minimum_required(VERSION 2.8)

project(ulapi)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(src)

## The base library, 'libulapi.so'
add_library(ulapi
  src/inifile.c
  src/serial.c
  src/ulapi_getopt.c
  src/unix_rtapi.c
  src/unix_ulapi.c
  )

## The dynamically-loadable library, 'libdlulapi.so'
add_library(dlulapi
  src/inifile.c
  src/serial.c
  src/ulapi_getopt.c
  src/unix_rtapi.c
  src/unix_ulapi.c
  )

add_library(dlfuncs
  src/dlfuncs.c
  )

## and libdlulapi.so's compile flags
set_target_properties(dlulapi PROPERTIES COMPILE_FLAGS "-shared -fPIC")
set_target_properties(dlfuncs PROPERTIES COMPILE_FLAGS "-shared -fPIC")

## Declare a cpp executable
add_executable(ultest src/ultest.c)
add_executable(dltest src/dltest.c)
add_executable(semtest src/semtest.c)
add_executable(serialtest src/serialtest.c)
add_executable(sockettest src/sockettest.c)

target_link_libraries(dltest dlulapi dl pthread)
target_link_libraries(ultest ulapi dl pthread)
target_link_libraries(semtest ulapi dl pthread)
target_link_libraries(serialtest ulapi dl pthread)
target_link_libraries(sockettest ulapi dl pthread)
