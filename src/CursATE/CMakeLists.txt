cmake_minimum_required(VERSION 3.0)
project(CursATE)

add_library(CursATEimpl
  Curses/ColorGuard.cpp
  Curses/CursorVisibility.cpp
  Curses/Init.cpp
  Curses/Field/Button.cpp
  Curses/Field/Input.cpp
  Curses/Field/Radio.cpp
  Curses/Field/detail/resizePadded.cpp
  Curses/ScrolableWindow.cpp
  Curses/getChar.cpp
  Curses/detail/ScrolableWindowBackend.cpp
  Screen/FilterTree.cpp
  Screen/LogEntry.cpp
  Screen/LogList.cpp
  Screen/ProgressBar.cpp
  Screen/Search.cpp
  Screen/displayError.cpp
  Screen/help.cpp
  Screen/multiLinePreview.cpp
  Screen/detail/ConstStringDataSource.cpp
  Screen/detail/FilterWindows.cpp
  Screen/detail/FilterTreeDataSource.cpp
  Screen/detail/LogDataSource.cpp
  Screen/detail/LogEntryDataSource.cpp
  Screen/detail/Marks.cpp
  Screen/detail/formatAsPercentage.cpp
  Screen/detail/id2key.cpp
  Screen/detail/splitIntoLines.cpp
  Main.cpp
  extractConfig.cpp
)
target_include_directories(CursATEimpl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CursATEimpl
  boost_program_options
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

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # clang has a problem with C++20-allowed struct - disabling it for the affected file
  set_source_files_properties(extractConfig.cpp PROPERTIES COMPILE_FLAGS "-Wno-c99-designator -Wno-missing-braces")
endif()


install(TARGETS CursATE
        RUNTIME DESTINATION bin)

if(LOGATE_ENABLE_TESTS)
  add_executable(CursATE_uts
    Curses/Field/detail/resizePadded.ut.cpp
    Curses/detail/ScrolableWindowBackend.ut.cpp
    Screen/detail/LogDataSource.ut.cpp
    Screen/detail/Marks.ut.cpp
    Screen/detail/formatAsPercentage.ut.cpp
    Screen/detail/id2key.ut.cpp
    Screen/detail/splitIntoLines.ut.cpp
    main.ut.cpp
  )
  add_executable(CursATE_mts
    main.mt.cpp
  )
  add_executable(CursATE_its
    main.it.cpp
  )

  foreach(app CursATE_uts CursATE_mts CursATE_its)
    message(STATUS "CursATE: adding automated test target ${app}")
    target_link_libraries(${app} CursATEimpl)
    add_test(NAME run_${app}
             COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${app}
                     --order-by=rand
                     --success=true
                     --force-colors=true
                     --duration=true
                     #--reporters=xml
                     #--out=${CMAKE_CURRENT_BINARY_DIR}/${app}_report.xml
            )
    list(APPEND TEST_TARGETS ${app})
  endforeach()

  set(SOURCES_MANUAL
    Curses/form_window.manual.cpp
    Curses/getChar.manual.cpp
    Curses/hello_world.manual.cpp
    Curses/new_scrolable_window.manual.cpp
    Curses/new_window.manual.cpp
    Curses/screen_size_monitor.manual.cpp
    Screen/LogEntry.manual.cpp
    Screen/help.manual.cpp
    Screen/displayError.manual.cpp
    Screen/multiLinePreview.manual.cpp
  )

  foreach(src ${SOURCES_MANUAL})
    get_filename_component(dir  ${src} DIRECTORY)
    get_filename_component(name ${src} NAME_WE)
    set(app CursATE_${name}.manual)
    message(STATUS "CursATE: adding manual test target ${app}")
    add_executable(${app} ${src})
    target_link_libraries(${app} CursATEimpl)
    list(APPEND TEST_TARGETS_MANUAL ${app})
    list(APPEND TEST_TARGETS        ${app})
  endforeach()
endif()
