cmake_minimum_required(VERSION 3.17)
project(mathlogic)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS}  -std=c++11 -Wall     -fstrict-aliasing   -fopt-info-vec-optimized-optimized -ffast-math  -march=native  -O3  -fopt-info-vec-optimized-all -funsafe-math-optimizations -fopt-info-vec-optimized-optimized " )#    ")
add_executable(mathlogic dpll_solver.cpp formula_presentation.h tseitin_transformation.h interpretation.h memento_state_pattern.h solver.h constansts_and_typedefs.h parser.h)