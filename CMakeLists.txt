cmake_minimum_required(VERSION 3.15)
project(
  act_tp4
  VERSION 0.1.0
  DESCRIPTION ""
  HOMEPAGE_URL ""
  LANGUAGES CXX
)

execute_process(
  COMMAND git submodule init
  COMMAND git submodule update
  COMMAND echo submodule fetched
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

find_package(Boost REQUIRED COMPONENTS program_options)

if("${CMAKE_CXX_COMPILER_ID}"
   MATCHES
   "GNU"
   OR "${CMAKE_CXX_COMPILER_ID}"
      MATCHES
      "Clang"
)
  add_compile_options(
    -Wall
    -Wextra
    -Wswitch-enum
    -pedantic
    -pedantic-errors
    -Werror=write-strings
    -Wold-style-cast
    -Wconversion
    -Wsign-conversion
    -Werror=return-type
  )
endif(
  "${CMAKE_CXX_COMPILER_ID}"
  MATCHES
  "GNU"
  OR "${CMAKE_CXX_COMPILER_ID}"
     MATCHES
     "Clang"
)

if("${CMAKE_CXX_COMPILER_ID}"
   MATCHES
   "Clang"
)
  add_compile_options(-Wdirect-ivar-access)
  # add_compile_options(-Wdocumentation)
  add_compile_options(-Wgcc-compat)
  add_compile_options(-Wheader-hygiene)
  add_compile_options(-Warray-bounds-pointer-arithmetic)
endif()

add_subdirectory(third_party/fmt)

add_executable(schedl)
target_sources(
  schedl
  PRIVATE schedl.cpp
          heuristics.cpp
          local_search.cpp
          neighborhood.cpp
          Task.cpp
          iterated_local_search.cpp
)
target_compile_features(schedl PRIVATE cxx_std_17)
target_link_libraries(
  schedl
  PRIVATE Boost::boost
          Boost::program_options
          fmt::fmt
)

add_subdirectory(tests)
