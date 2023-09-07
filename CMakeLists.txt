cmake_minimum_required(VERSION 3.27)

project(nes2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

include(FetchContent)

set(FMT_TEST FF CACHE INTERNAL "disabling fmt tests")
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 10.0.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    assert
    GIT_REPOSITORY https://github.com/jeremy-rifkin/libassert.git
    GIT_TAG v1.2
)
FetchContent_MakeAvailable(assert)

set(SPDLOG_FMT_EXTERNAL 1)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)

    # disable warnings for spdlog
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)

    # fast paced development, TODO: Remove
    add_compile_options(-Wno-unused-parameter)
endif()

add_executable(nes2
    main.cpp
    iNES.cpp
    Cartridge.cpp
    MMC1.cpp
    MMU.cpp
    CPU.cpp
    SimpleMapper.cpp
    System.cpp
)

target_precompile_headers(nes2
  PRIVATE
    pch.h
)

target_link_libraries(nes2
  PRIVATE
    assert
    fmt::fmt-header-only
    spdlog::spdlog
)

add_executable(dump
  dump.cpp
  iNES.cpp  
)

target_link_libraries(dump
  PRIVATE
    assert
    spdlog::spdlog
)
