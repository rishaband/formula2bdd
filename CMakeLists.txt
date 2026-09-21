cmake_minimum_required(VERSION 3.15)
project(formula2bdd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

# Core library.
add_library(formula2bdd_core
  src/Ast.cpp
  src/Lexer.cpp
  src/Parser.cpp
  src/Evaluator.cpp
  src/BddManager.cpp
  src/Exporters.cpp
)
target_include_directories(formula2bdd_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# CLI executable.
add_executable(formula2bdd app/main.cpp)
target_link_libraries(formula2bdd PRIVATE formula2bdd_core)

# Tests.
enable_testing()
add_executable(test_formula2bdd tests/test_formula2bdd.cpp)
target_link_libraries(test_formula2bdd PRIVATE formula2bdd_core)
add_test(NAME unit COMMAND test_formula2bdd)
