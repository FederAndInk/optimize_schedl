cmake_minimum_required(VERSION 3.15)
project(
  act_tp4
  VERSION 0.1.0
  DESCRIPTION ""
  HOMEPAGE_URL ""
  LANGUAGES CXX
)

add_subdirectory(third_party/fmt)

add_executable(sched)
target_sources(sched PRIVATE sched.cpp)
target_compile_features(sched PRIVATE cxx_std_17)
target_link_libraries(sched PRIVATE fmt::fmt)
