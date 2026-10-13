cmake_minimum_required(VERSION 3.20)

project(kaleidoscope
    VERSION 1.0.0
    DESCRIPTION "Modern C++ Kaleidoscope Compiler with LLVM Backend"
    LANGUAGES CXX C
)

# Enforce C++17 across all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose build type" FORCE)
endif()

# Compiler warnings and options
if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Optional LLVM dependency discovery
find_package(LLVM CONFIG QUIET)

if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})
    set(HAVE_LLVM TRUE)
    add_compile_definitions(KALEIDOSCOPE_HAS_LLVM=1)

    # LLVM components needed for IR code generation, JIT execution, and optimization passes
    llvm_map_components_to_libnames(LLVM_LIBS
        core
        support
        native
        mcjit
        orcjit
        executionengine
        target
        analysis
        transformutils
        scalaropts
    )
else()
    message(STATUS "LLVM not detected; building compiler frontend, AST optimizer, and diagnostic toolchain.")
    message(STATUS "Pass -DLLVM_DIR=<path> to build with full LLVM JIT & CodeGen support.")
    set(HAVE_LLVM FALSE)
    add_compile_definitions(KALEIDOSCOPE_HAS_LLVM=0)
    set(LLVM_LIBS "")
endif()

# Core compiler library sources
set(KALEIDOSCOPE_CORE_SOURCES
    src/Diagnostic.cpp
    src/Token.cpp
    src/Lexer.cpp
    src/AST.cpp
    src/ASTVisitor.cpp
    src/ASTOptimizer.cpp
    src/Parser.cpp
    src/CodeGen.cpp
    src/Optimizer.cpp
    src/Driver.cpp
)

add_library(kaleidoscope_core STATIC ${KALEIDOSCOPE_CORE_SOURCES})

target_include_directories(kaleidoscope_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(HAVE_LLVM)
    target_link_libraries(kaleidoscope_core PUBLIC ${LLVM_LIBS})
endif()

# Main executable
add_executable(kaleidoscope src/main.cpp)
target_link_libraries(kaleidoscope PRIVATE kaleidoscope_core)

# Testing configuration
option(BUILD_TESTS "Build test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
