set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

if(${LOGATE_ENABLE_PIC})
  message(STATUS "LogATE: enable PIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -ggdb")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -g3 -DNDEBUG")

set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -O3 -g1 -DNDEBUG -pg")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_PROFILE} -pg")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-limit=150") # fix for GCC's performance bug, on inlining heuristics
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s") # strip binaries for GCC
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type") # remove nonhelpful warning

message(STATUS "LogATE: enabling LTO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")

#set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -flto")
#set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_PROFILE} -flto")

# LTO plugin flags for AR and RANLIB are needed for GCC only
# unfortunately 'archive' flags do not have per-build-type variants...
if(DEFINED CMAKE_BUILD_TYPE AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    execute_process(COMMAND gcc --print-file-name=liblto_plugin.so
                    OUTPUT_VARIABLE LTO_PLUGIN_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "LogATE: using LTO plugin path: ${LTO_PLUGIN_PATH}")
    set(LTO_PLUGIN "--plugin=\"${LTO_PLUGIN_PATH}\"")
    set(CMAKE_CXX_ARCHIVE_CREATE "${CMAKE_CXX_ARCHIVE_CREATE} ${LTO_PLUGIN}")
    set(CMAKE_CXX_ARCHIVE_FINISH "${CMAKE_CXX_ARCHIVE_FINISH} ${LTO_PLUGIN}")
endif()

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

# there's a bug in GCC 12:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105562
# if both are true:
#  - optimizations are enabled
#  - address sanitizer is on
# there incorrect "may be used uninitialized" warnings comming up from standard library's internals.
if("${CMAKE_BUILD_TYPE}" MATCHES "^Release|Relwithdebinfo|Profile$")
  message(WARNING "BUT: disabling -Wmaybe-uninitialized for buggy GCC version")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
endif()
