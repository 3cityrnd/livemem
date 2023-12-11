set(SOURCES
    livemem.cpp
)

add_library(livemem SHARED ${SOURCES})
add_library(livemem_fortestonly SHARED ${SOURCES})
target_compile_definitions(livemem_fortestonly PRIVATE TON=1)