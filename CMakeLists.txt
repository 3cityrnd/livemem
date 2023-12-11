cmake_minimum_required(VERSION 3.14)
project(livemem_proj) 
set(LIVEMEM_VERSION 1.0)
enable_testing()
set(CTEST_OUTPUT_ON_FAILURE TRUE)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=inconsistent-missing-override")


#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests_bin)


add_subdirectory(src)
add_subdirectory(test)


#add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS test_livemem)
#add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND} )
#add_dependencies(run_tests livemem)