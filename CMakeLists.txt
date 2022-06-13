cmake_minimum_required(VERSION 3.0)
project("LogATE_dev")
if(LOGATE_ENABLE_TESTS)
  # must be in a source dir root to work...
  enable_testing()
endif()
add_subdirectory(src)
