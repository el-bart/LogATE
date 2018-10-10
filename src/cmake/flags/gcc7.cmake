set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if(${LOGATE_ENABLE_PIC})
  message(STATUS "LogATE: enable PIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -g3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -O3 -g1 -DNDEBUG -pg")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_PROFILE} -pg")

# fix for gmock/gtest issue...
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-limit=150") # fix for GCC's performance bug, on inlining heuristics
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s") # strip binaries for GCC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type") # remove nonhelpful warning

# NOTE: LTO is causes issues with GCC-7.1 and sanitizers, thus disabled

if(LOGATE_SANITIZE)
  message(STATUS "LogATE: enabling ${LOGATE_SANITIZE} sanitizer")
  add_compile_options(-fsanitize=${LOGATE_SANITIZE} -fno-sanitize-recover=all)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${LOGATE_SANITIZE} -fno-sanitize-recover=all")
endif()

if("${LOGATE_ENABLE_COVERAGE}" STREQUAL "yes")
  message(STATUS "LogATE: enabing coverage counting")
  add_compile_options(--coverage)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
