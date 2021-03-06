cmake_minimum_required(VERSION 2.8)
project(Escape-from-TuxKov)
set(CMAKE_CXX_STANDARD 17)
include(CheckCXXCompilerFlag)

foreach(COMP_ARG "-Weverything -Wall -Wno-zero-as-null-pointer-constant -Wno-sign-conversion" "-Wno-covered-switch-default" "-Wno-c++98-compat" "-Wno-padded" "-Wno-c++98-compat-pedantic" "-Wno-weak-vtables" "-Wno-documentation-unknown-command" "-Wno-old-style-cast" "-Wno-float-equal" "-Wno-unused-command-line-argument -Wno-reserved-id-macro -Wno-cast-align -Wno-exit-time-destructors -Wno-global-constructors")
  CHECK_CXX_COMPILER_FLAG("${COMP_ARG}" ARG_SUPPORTED)
  if(ARG_SUPPORTED)
    set(WARNING_ARGS "${WARNING_ARGS} ${COMP_ARG}")
  endif()
endforeach(COMP_ARG)

message("Supported warning args: ${WARNING_ARGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_ARGS} -pedantic -ldl -fpic -isystem${CMAKE_SOURCE_DIR}/modules -DMVERBOSE=0 -DNUM_THREADS=4 -DMEMUTILS_CUSTOM_RW -DPATTERN_SCAN_PAGE_SCAN")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s -fvisibility=hidden -fvisibility-inlines-hidden")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE RELEASE)
endif(NOT CMAKE_BUILD_TYPE)

if(USE_EAC_LAUNCHER)
  message("Using EAC Launcher")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_EAC_LAUNCHER=1")
endif()

if(MTR_ENABLED)
  message("Enabling minitrace")
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DMTR_ENABLED=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMTR_ENABLED=1")
endif()

if(BUILD_WITH_SANITIZERS)
	message("Enabling ASan")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif(BUILD_WITH_SANITIZERS)

foreach(SFILE "threading.cpp" "semaphores.cpp" "atomic_lock.cpp" "mutex.cpp" "pattern_scan.cpp")
  list(APPEND SOURCE_FILES_CXX "${CMAKE_SOURCE_DIR}/modules/m0dular/utils/${SFILE}")
endforeach(SFILE)

foreach(SFILE "src/*.cpp" "src/utils/*.cpp" "src/sdk/*.cpp" "src/features/*.cpp" "modules/vmread/hlapi/*.cpp" "modules/peeper/client/peeper.cpp" "src/hacks/*.cpp")
  file(GLOB GLOB_FILES "${CMAKE_SOURCE_DIR}/${SFILE}")
  list(APPEND SOURCE_FILES_CXX ${GLOB_FILES})
endforeach(SFILE)

file(GLOB_RECURSE SOURCE_FILES_C "${CMAKE_SOURCE_DIR}/modules/*.c")

list(REMOVE_ITEM SOURCE_FILES_C "${CMAKE_SOURCE_DIR}/modules/peeper/ext/imgui/examples/libs/gl3w/GL/gl3w.c")
list(REMOVE_ITEM SOURCE_FILES_C "${CMAKE_SOURCE_DIR}/modules/vmread/example.c")
list(REMOVE_ITEM SOURCE_FILES_C "${CMAKE_SOURCE_DIR}/modules/vmread/vmmem.c")
list(REMOVE_ITEM SOURCE_FILES_C "${CMAKE_SOURCE_DIR}/modules/vmread/intmem.c")
list(REMOVE_ITEM SOURCE_FILES_C "${CMAKE_SOURCE_DIR}/modules/vmread/kmem.c")
list(REMOVE_ITEM SOURCE_FILES_CXX "${CMAKE_SOURCE_DIR}/modules/vmread/example.cpp")

message("Source files: ${SOURCE_FILES_CXX}")
message("Source files(C): ${SOURCE_FILES_C}")

find_package(Threads)

include_directories(${INCLUDE_DIRS})

add_library(TuxKov SHARED ${SOURCE_FILES_CXX} ${SOURCE_FILES_C} "${CMAKE_SOURCE_DIR}/modules/vmread/intmem.c" src/hacks/aimbot.cpp src/hacks/aimbot.h)
target_link_libraries(TuxKov dl rt ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(TuxKov PUBLIC LMODE=MODE_QEMU_INJECT)

add_executable(TuxKovExternal ${SOURCE_FILES_CXX} ${SOURCE_FILES_C} "${CMAKE_SOURCE_DIR}/modules/vmread/vmmem.c")
target_link_libraries(TuxKovExternal dl rt ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(TuxKovExternal PUBLIC LMODE=MODE_EXTERNAL)
