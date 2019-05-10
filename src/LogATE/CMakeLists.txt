cmake_minimum_required(VERSION 3.0)
project(LogATE)

add_library(LogATE
  Log.cpp
  Net/TcpClient.cpp
  Net/TcpRawClient.cpp
  Net/TcpServer.cpp
  Printers/OrderedPrettyPrint.cpp
  Tree/Node.cpp
  Tree/Logs.cpp
  Tree/Path.cpp
  Tree/SimpleNode.cpp
  Tree/Filter/AcceptAll.cpp
  Tree/Filter/Explode.cpp
  Tree/Filter/Grep.cpp
  Tree/FilterFactory.cpp
  Tree/findParent.cpp
  Utils/WorkerThreads.cpp
  Utils/matchesLog.cpp
  Utils/value2str.cpp
  SequenceNumber.cpp
)
target_include_directories(LogATE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LogATE
  PocoNet
  PocoFoundation
  /usr/local/lib/libbut.a
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
    Printers/OrderedPrettyPrint.ut.cpp
    Printers/jsonLine.ut.cpp
    Tree/Logs.ut.cpp
    Tree/Node.ut.cpp
    Tree/Path.ut.cpp
    Tree/SimpleNode.ut.cpp
    Tree/Filter/Explode.ut.cpp
    Tree/Filter/Grep.ut.cpp
    Tree/FilterFactory.ut.cpp
    Utils/matchesLog.ut.cpp
    Log.ut.cpp
    main.ut.cpp
  )
  add_executable(LogATE_mts
    main.mt.cpp
  )
  add_executable(LogATE_its
    Net/TcpServer.it.cpp
    main.it.cpp
  )

  foreach(app LogATE_uts LogATE_mts LogATE_its)
    message(STATUS "LogATE: adding automated test target ${app}")
    target_link_libraries(${app} LogATE)
    add_test(NAME run_${app}
             COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${app}
                     --success=true
                     --force-colors=true
                     --order-by=rand
                     --reporters=console
                     #--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${app}_report.xml
            )
    list(APPEND TEST_TARGETS ${app})
  endforeach()

  set(SOURCES_MANUAL
    Net/TcpClient.manual.cpp
    Net/TcpServer.manual.cpp
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
endif()
