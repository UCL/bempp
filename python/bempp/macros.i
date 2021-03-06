%define BEMPP_ITERATE_OVER_TYPES(MACRO)
    MACRO(float)
    MACRO(double)
    MACRO(std::complex<float> )
    MACRO(std::complex<double> )
%enddef // BEMPP_ITERATE_OVER_TYPES

%define BEMPP_ITERATE_OVER_RESULT_TYPES(MACRO)
    MACRO(float)
    MACRO(double)
    MACRO(std::complex<float> )
    MACRO(std::complex<double> )
%enddef // BEMPP_ITERATE_OVER_TYPES

%define BEMPP_ITERATE_OVER_BASIS_TYPES(MACRO)
    MACRO(float, float32)
    MACRO(double, float64)
    MACRO(std::complex<float> , complex64)
    MACRO(std::complex<double> , complex128)
%enddef // BEMPP_ITERATE_OVER_BASIS_TYPES

%define BEMPP_ITERATE_OVER_BASIS_AND_RESULT_TYPES(MACRO)
    MACRO(float, float, float32, float32)
    MACRO(float, std::complex<float> , float32, complex64)
    MACRO(std::complex<float> , std::complex<float> , complex64, complex64)
    MACRO(double, double, float64, float64)
    MACRO(double, std::complex<double> , float64, complex128)
    MACRO(std::complex<double> , std::complex<double> , complex128, complex128)
%enddef // BEMPP_ITERATE_OVER_BASIS_AND_RESULT_TYPES

%define BEMPP_ITERATE_OVER_BASIS_KERNEL_AND_RESULT_TYPES(MACRO)
    MACRO(float, float, float, float32, float32, float32)
    MACRO(float, float, std::complex<float> , float32, float32, complex64)
    MACRO(float, std::complex<float> , std::complex<float> , float32, complex64, complex64)
    MACRO(std::complex<float> , float, std::complex<float> , complex64, float32, complex64)
    MACRO(std::complex<float> , std::complex<float> , std::complex<float> , complex64, complex64, complex64)
    MACRO(double, double, double, float64, float64, float64)
    MACRO(double, double, std::complex<double> , float64, float64, complex128)
    MACRO(double, std::complex<double> , std::complex<double> , float64, complex128, complex128)
    MACRO(std::complex<double> , double, std::complex<double> , complex128, float64, complex128)
    MACRO(std::complex<double> , std::complex<double> , std::complex<double> , complex128, complex128, complex128)
%enddef // BEMPP_ITERATE_OVER_BASIS_KERNEL_AND_RESULT_TYPES
