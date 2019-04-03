cmake_minimum_required(VERSION 3.0)
project(CursATE)

add_executable(CursATE
  main.cpp
)
target_include_directories(CursATE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CursATE
  LogATE
)


install(TARGETS CursATE
        RUNTIME DESTINATION bin)
