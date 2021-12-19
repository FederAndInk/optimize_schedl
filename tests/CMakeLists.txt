cmake_minimum_required(VERSION 3.15)

enable_testing()

add_executable(neighborhood_test)
target_sources(neighborhood_test PRIVATE neighborhood_test.cpp)
target_compile_features(neighborhood_test PRIVATE cxx_std_17)
target_link_libraries(neighborhood_test PRIVATE Boost::boost fmt::fmt)
target_compile_options(
  neighborhood_test
  PRIVATE -fsanitize=address
          -fno-lto
          -UNDEBUG
          -Og
          -g3
          -fno-optimize-sibling-calls
          -fno-omit-frame-pointer
)
target_link_options(
  neighborhood_test
  PRIVATE
  -fsanitize=address
)

add_test(neighboorhood_test
         COMMAND "${CMAKE_CURRENT_BINARY_DIR}/neighboorhood_test"
)
