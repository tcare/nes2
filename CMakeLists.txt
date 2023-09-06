cmake_minimum_required(VERSION 3.27)

project(nes2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

include(FetchContent)

# Fetch and add spdlog
FetchContent_Declare(
    assert
    GIT_REPOSITORY https://github.com/jeremy-rifkin/libassert.git
    GIT_TAG v1.2
)
FetchContent_MakeAvailable(assert)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

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
