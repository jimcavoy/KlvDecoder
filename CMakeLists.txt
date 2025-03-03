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
find_package(Boost REQUIRED COMPONENTS algorithm json program_options url)

add_executable(KlvDecoder)

target_sources(KlvDecoder
  PRIVATE
    src/main.cpp
    src/CmdLineParser.cpp
    src/CmdLineParser.h
    src/UdpSender.cpp
    src/UdpSender.h
)

target_include_directories(KlvDecoder
  PRIVATE
    ${Boost_INCLUDE_DIRS})

if (WIN32)
target_link_libraries(KlvDecoder 
  PRIVATE
    MiDemux
    Boost::algorithm
    Boost::json
    Boost::program_options
    Boost::url
    wsock32
    ws2_32
)
else()
target_link_libraries(KlvDecoder 
  PRIVATE
    MiDemux
    Boost::algorithm
    Boost::json
    Boost::program_options
    Boost::url
)
endif()

install(TARGETS KlvDecoder)

# Test cases
enable_testing()

add_test(NAME OutputJson
  COMMAND KlvDecoder ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts
)
set_tests_properties(OutputJson
  PROPERTIES PASS_REGULAR_EXPRESSION "{\"metadata_set\":{\"UNIX_Time_Stamp\":{\"key\":\"2\",\"value\":\"Friday, 16-Jun-17 14:26:20.616000 UTC\"},"
)

add_test(NAME OutputXml
  COMMAND KlvDecoder ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts -F xml
)
set_tests_properties(OutputXml
  PROPERTIES PASS_REGULAR_EXPRESSION "<metadata_set><UNIX_Time_Stamp><key>2</key><value>Friday, 16-Jun-17 14:26:20.616000 UTC</value></UNIX_Time_Stamp>"
)

add_test(NAME OutputInfo
  COMMAND KlvDecoder ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts -F info
)
set_tests_properties(OutputInfo
  PROPERTIES PASS_REGULAR_EXPRESSION "metadata_set
{
    UNIX_Time_Stamp
    {
        key 2
        value \"Friday, 16-Jun-17 14:26:20.616000 UTC\"
    }"
)

add_test(NAME StreamOutput
  COMMAND KlvDecoder ${PROJECT_SOURCE_DIR}/sample/foreman_cif_klv.ts --outputUrl=udp://239.3.1.11:50000?ttl=16
)