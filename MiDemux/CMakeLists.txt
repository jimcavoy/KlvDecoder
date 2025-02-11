cmake_minimum_required (VERSION 3.21)

project(MiDemux
  VERSION 1.0.0
  DESCRIPTION "STANAG 4609 Motion Imagery De-multiplexer"
  LANGUAGES CXX
)

# set the postfix "d" for the resulting .so or .dll files when building the
# library in debug mode
set(CMAKE_DEBUG_POSTFIX d )

# include the module `FetchContent`
include(FetchContent)

# Add an external project from a downloaded source archive
FetchContent_Declare(loki-lib
  URL https://github.com/snaewe/loki-lib/archive/refs/tags/Release_0_1_5.tar.gz
  URL_HASH MD5=74e60c683f745dc15c6e772927349483
)
FetchContent_MakeAvailable(loki-lib)
FetchContent_GetProperties(loki-lib)
set(LOKI_DIR "${loki-lib_SOURCE_DIR}")

find_package(mp2tp 1 CONFIG REQUIRED)
find_package(klvp 1 CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

add_library(MiDemux STATIC)

target_sources(MiDemux 
  PRIVATE
    src/AccessUnit.h
    src/AccessUnit.cpp
    src/Clock.h
    src/Clock.cpp
    src/MiDemux.cpp
    src/Mpeg2TsDecoder.h
    src/Mpeg2TsDecoder.cpp
    src/PmtProxy.h
    src/PmtProxy.cpp
    include/MiDemux/MiDemux.h
)

set_property(TARGET MiDemux PROPERTY POSITION_INDEPENDENT_CODE ON)

# specify the C++ standard
target_compile_features(MiDemux
  PUBLIC 
    cxx_std_17
)

target_include_directories(MiDemux
  PRIVATE
    include/MiDemux
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${LOKI_DIR}/include>
)

target_link_libraries(MiDemux 
  PRIVATE 
    lcss::klvp
    lcss::mp2tp
    Microsoft.GSL::GSL
)
