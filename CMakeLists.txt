cmake_minimum_required(VERSION 3.18)

project(data-structures)
set(CMAKE_CXX_STANDARD 20)
include_directories(inc)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
file(GLOB_RECURSE TESTS test/*.cpp)
add_executable(${CMAKE_PROJECT_NAME} ${TESTS})
add_test(NAME ${CMAKE_PROJECT_NAME} COMMAND ${CMAKE_PROJECT_NAME})
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC gtest)
include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME})

if(CMAKE_COMPILER_IS_GNUCXX)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -O0 -fno-inline -fprofile-arcs -ftest-coverage")

        add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND lcov --directory . --zerocounters
            COMMAND ${CMAKE_PROJECT_NAME} --gtest_output=xml:../report/unittest/index.xml
            COMMAND xsltproc ../report/unittest/gtest-result.xsl ../report/unittest/index.xml > ../report/unittest/index.html
            COMMAND bash -c "gcov --branch-probabilities --branch-counts $(find CMakeFiles/data-structures.dir/test/*.o | paste -sd ' ')"
            COMMAND lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file total.info
            COMMAND lcov --rc lcov_branch_coverage=1 --remove total.info "/usr/include/*" "*/gtest/*" "*/test/*" --output-file filtered.info
            COMMAND genhtml --branch-coverage --demangle-cpp filtered.info --output-directory ../report/coverage
            VERBATIM
        )

        add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
            COMMAND doxygen doxygen.conf
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()
