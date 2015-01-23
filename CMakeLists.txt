cmake_minimum_required(VERSION 2.8)

project(ulapi)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(src)

## The base library, 'libulapi.a'
add_library(ulapi
  src/inifile.c
  src/serial.c
  src/ulapi_getopt.c
  src/unix_rtapi.c
  src/unix_ulapi.c
  )

## The dynamically-loadable library, 'libdlulapi.so'
add_library(dlulapi SHARED
  src/inifile.c
  src/serial.c
  src/ulapi_getopt.c
  src/unix_rtapi.c
  src/unix_ulapi.c
  )
set_target_properties(dlulapi PROPERTIES COMPILE_FLAGS "-shared -fPIC")

## The shared object function test file, 'libdlfuncs.so'
add_library(dlfuncs SHARED
  src/dlfuncs.c
  )
set_target_properties(dlfuncs PROPERTIES COMPILE_FLAGS "-shared -fPIC")

## Declare a cpp executable
add_executable(ultest src/ultest.c)
add_executable(dltest src/dltest.c)
add_executable(semtest src/semtest.c)
add_executable(serialtest src/serialtest.c)
add_executable(sockettest src/sockettest.c)

target_link_libraries(ultest ulapi dl pthread)
target_link_libraries(dltest dlulapi dl pthread)
target_link_libraries(semtest ulapi dl pthread)
target_link_libraries(serialtest ulapi dl pthread)
target_link_libraries(sockettest ulapi dl pthread)

install(TARGETS ulapi dlulapi DESTINATION lib)

install(FILES
  src/inifile.h
  src/rtapi.h
  src/rtapi_app.h
  src/serial.h
  src/ulapi.h
  src/ulapi_getopt.h
  DESTINATION include)
