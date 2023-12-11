
set(TESTS_NAMES simple_link )

include_directories(../src)

foreach(T IN ITEMS ${TESTS_NAMES})
    message("Add test ${T}")
    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PRIVATE livemem)
    add_test(NAME ${T} COMMAND ${T})
endforeach()


set(TESTS_NAMES_NO_LINK hook_posix_memalign)


foreach(T IN ITEMS ${TESTS_NAMES_NO_LINK})
    message("Add test ${T}")
    add_executable(${T} ${T}.cpp)
    #target_link_libraries(${T} PRIVATE livemem)
    add_test(NAME ${T} COMMAND ${T})
    set_tests_properties(${T} PROPERTIES
    ENVIRONMENT "LD_PRELOAD=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/liblivemem_fortestonly.so"
    )


endforeach()
