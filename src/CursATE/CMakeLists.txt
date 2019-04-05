cmake_minimum_required(VERSION 3.0)
project(CursATE)

add_library(CursATEimpl
  Curses/ScrolableWindow.cpp
)
target_include_directories(CursATEimpl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CursATEimpl
  ncurses
  LogATE
)

add_executable(CursATE
  main.cpp
)
target_include_directories(CursATE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CursATE
  CursATEimpl
)


install(TARGETS CursATE
        RUNTIME DESTINATION bin)

if(LOGATE_ENABLE_TESTS)

  set(SOURCES_MANUAL
    Curses/hello_world.manual.cpp
    Curses/new_scrolable_window.manual.cpp
    Curses/new_window.manual.cpp
    Curses/screen_size_monitor.manual.cpp
  )

  foreach(src ${SOURCES_MANUAL})
    get_filename_component(dir  ${src} DIRECTORY)
    get_filename_component(name ${src} NAME_WE)
    set(app CursATE_${name}.manual)
    message(STATUS "LogATE: adding manual test target ${app}")
    add_executable(${app} ${src})
    target_link_libraries(${app} CursATEimpl)
    list(APPEND TEST_TARGETS_MANUAL ${app})
    list(APPEND TEST_TARGETS        ${app})
  endforeach()
endif()
