cmake_minimum_required (VERSION 3.21)

project(MiDemux
  VERSION 1.2.1
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
set(ldsdb_DIR ${klvp_DIR}/../ldsdb)
find_package(ldsdb 1 CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS property_tree)

add_library(MiDemux STATIC)

target_sources(MiDemux 
  PRIVATE
    src/AccessUnit.h
    src/AccessUnit.cpp
    src/Clock.h
    src/Clock.cpp
    src/KlvDecodeVisitor.h
    src/KlvDecodeVisitor.cpp
    src/KlvParserImpl.h
    src/KlvParserImpl.cpp
    src/KlvSecuritySetParserImpl.h
    src/KlvSecuritySetParserImpl.cpp
    src/KlvSecuritySetVisitor.h
    src/KlvSecuritySetVisitor.cpp
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
    cxx_std_20
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
    lcss::ldsdb
    lcss::mp2tp
    Boost::property_tree
)
