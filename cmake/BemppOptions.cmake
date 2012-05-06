# Options (can be modified by user)
option(WITH_TESTS "Compile unit tests (can be run with 'make test')" ON)
option(WITH_AHMED "Link to the AHMED library to enable ACA mode assembly)" OFF)
option(WITH_TRILINOS "Use sparse linear solvers from the Trilinos library)" OFF)
option(WITH_OPENCL "Add OpenCL support for Fiber module" OFF)
option(BEMPP_DUNE "Use included DUNE distribution" ON)
option(BEMPP_TRILINOS "Use included Trilinos distribution" ON)
option(BEMPP_ARMADILLO "Use included Armadillo distribution" ON)

option(ENABLE_SINGLE_PRECISION "Enable support for single-precision calculations" ON)
option(ENABLE_DOUBLE_PRECISION "Enable support for double-precision calculations" ON)
option(ENABLE_COMPLEX_KERNELS  "Enable support for complex-valued kernel functions" ON)
option(ENABLE_COMPLEX_BASIS_FUNCTIONS  "Enable support for complex-valued basis functions" ON)

