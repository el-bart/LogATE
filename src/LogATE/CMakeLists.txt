cmake_minimum_required(VERSION 3.0)
project(LogATE)

add_library(LogATE
  placeholder.cpp
)
target_include_directories(LogATE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LogATE
    pthread
)


install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/LogATE
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")
install(TARGETS LogATE
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)


if(LOGATE_ENABLE_TESTS)
  add_executable(LogATE_uts
    placeholder.ut.cpp
  )
  add_executable(LogATE_mts
    placeholder.mt.cpp
  )
  add_executable(LogATE_its
    placeholder.it.cpp
  )

  foreach(app LogATE_uts LogATE_mts LogATE_its)
    message(STATUS "LogATE: adding automated test target ${app}")
    target_link_libraries(${app} LogATE)
    target_link_libraries(${app} gmock_main gmock gtest)
    add_test(NAME run_${app}
             COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${app}
                     --gtest_shuffle
                     --gtest_color=yes
                     --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${app}_report.xml)
    list(APPEND TEST_TARGETS ${app})
  endforeach()

  set(SOURCES_MANUAL
    placeholder.manual.cpp
  )

  foreach(src ${SOURCES_MANUAL})
    get_filename_component(dir  ${src} DIRECTORY)
    get_filename_component(name ${src} NAME_WE)
    set(app LogATE_${name}.manual)
    message(STATUS "LogATE: adding manual test target ${app}")
    add_executable(${app} ${src})
    target_link_libraries(${app} LogATE)
    list(APPEND TEST_TARGETS_MANUAL ${app})
    list(APPEND TEST_TARGETS        ${app})
  endforeach()

  add_custom_target(LogATE_tests_manual DEPENDS ${TEST_TARGETS_MANUAL})
endif()
