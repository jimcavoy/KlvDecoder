# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.21)
project (KlvDecoder
	VERSION 1.0.0
	DESCRIPTION "Converts KLV binary encoded metadata in a STANAG 4609 Motion Imagery stream to a human readable text format."
	LANGUAGES CXX)

include(GNUInstallDirs)

# Include sub-projects.
add_subdirectory (MiDemux)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS algorithm json program_options)

add_executable(KlvDecoder)

target_sources(KlvDecoder
  PRIVATE
    src/main.cpp
	src/CmdLineParser.cpp
	src/CmdLineParser.h
)

target_include_directories(KlvDecoder
  PRIVATE
    ${Boost_INCLUDE_DIRS})

target_link_libraries(KlvDecoder 
  PRIVATE
    MiDemux
    Boost::algorithm
    Boost::json
    Boost::program_options
)

install(TARGETS KlvDecoder)

# Test cases
# enable_testing()

# add_test(NAME Parse
  # COMMAND avc2json ${PROJECT_SOURCE_DIR}/sample/svt_testset_420_720p50_klved_4774.h264
# )
