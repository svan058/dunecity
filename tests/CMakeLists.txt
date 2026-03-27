# CMakeLists.txt - Test suite for Dune Legacy / DuneCity
cmake_minimum_required(VERSION 3.10)

# Find Catch2 package
find_package(Catch2 3 REQUIRED)

# Test executable
add_executable(dunelegacy_tests)

# Source files - Catch2-based unit tests
# Note: Catch2WithMain provides main(), no testmain.cpp needed
target_sources(dunelegacy_tests
    PRIVATE
        # DuneCity Phase 3-4 tests (header-only, no game runtime needed)
        ZoneStructureTestCase/ZoneStructureTestCase.cpp
        CityStructuresTestCase/CityStructuresTestCase.cpp
        # DuneLegacy DuneCity tests
        CityScoreTestCase/CityScoreTestCase.cpp
        OverlayLegendTestCase/OverlayLegendTestCase.cpp
        # City infrastructure tests
        PathBudgetTestCase/PathBudgetTestCase.cpp
        # Network tests
        NetworkManagerTestCase/NetworkManagerTestCase.cpp
        # Global variable stubs for test binary
        globals_test_stub.cpp
        # ENet library sources (needed for NetworkManager tests)
        ${CMAKE_SOURCE_DIR}/src/enet/callbacks.c
        ${CMAKE_SOURCE_DIR}/src/enet/compress.c
        ${CMAKE_SOURCE_DIR}/src/enet/host.c
        ${CMAKE_SOURCE_DIR}/src/enet/list.c
        ${CMAKE_SOURCE_DIR}/src/enet/packet.c
        ${CMAKE_SOURCE_DIR}/src/enet/peer.c
        ${CMAKE_SOURCE_DIR}/src/enet/protocol.c
        $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_SOURCE_DIR}/src/enet/unix.c>
        $<$<PLATFORM_ID:Windows>:${CMAKE_SOURCE_DIR}/src/enet/win32.c>
        # fmt library (needed for THROW macro in ENet headers)
        ${CMAKE_SOURCE_DIR}/src/misc/format.cpp
)

# Include directories
target_include_directories(dunelegacy_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/enet  # For enet headers
        ${CMAKE_SOURCE_DIR}/src      # For src-level headers
)

# Link libraries
target_link_libraries(dunelegacy_tests
    PRIVATE
        Catch2::Catch2WithMain
        SDL2::SDL2
        $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>
)

if(WIN32)
    target_link_libraries(dunelegacy_tests
        PRIVATE
            ws2_32
            winmm
            imm32
            version
    )
endif()

# Compiler options
target_compile_options(dunelegacy_tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

# Add test
add_test(NAME dunelegacy_tests COMMAND dunelegacy_tests)

# Set test properties
set_tests_properties(dunelegacy_tests PROPERTIES
    ENVIRONMENT "DUNECITY_DATADIR=${CMAKE_SOURCE_DIR}/data;DUNE_CITY_SOURCE_DIR=${CMAKE_SOURCE_DIR}"
)

# Copy test data to build directory
add_custom_command(
    TARGET dunelegacy_tests
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/tests/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "${CMAKE_BINARY_DIR}/tests/data"
    COMMENT "Copying test data to build directory"
)
