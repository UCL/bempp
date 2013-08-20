include_directories(${CMAKE_BINARY_DIR}/include)


function(find_subdirectories location outvar)
  file(GLOB ALL_FILES RELATIVE ${location} ${location}/*)
  set(RESULT_VAR) 
  foreach(candidate ${ALL_FILES})
    if(IS_DIRECTORY ${location}/${candidate})
      set(RESULT_VAR ${RESULT_VAR} ${candidate})
    endif()
  endforeach()
  set(${outvar} ${RESULT_VAR} PARENT_SCOPE)
endfunction(find_subdirectories)

add_definitions(-DBOOST_TEST_DYN_LINK)
#find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_INSTALL_PREFIX}/bempp/include)

# Figure out environment variable with wich to run test
get_filename_component(ENVVARS ${BOOST_UNIT_TEST_LIB} PATH)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(ENVVARS "DYLD_LIBRARY_PATH=${ENVVARS}:$ENV{DYLD_LIBRARY_PATH}")
else()
  set(ENVVARS "LD_LIBRARY_PATH=${ENVVARS}:$ENV{LD_LIBRARY_PATH}")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Files to be included in all tests.
file(GLOB TEST_MAIN_SOURCES *.cpp)
find_subdirectories(${CMAKE_CURRENT_SOURCE_DIR} SUBDIRECTORIES)
# meshes has no test
list(REMOVE_ITEM SUBDIRECTORIES "meshes")

foreach(directory ${SUBDIRECTORIES}) 

  # Files for this particular test directory
  file(GLOB TEST_SOURCES ${directory}/test_*.cpp)
  if("${directory}" STREQUAL "assembly")
    if(NOT WITH_AHMED)
      list(REMOVE_ITEM TEST_SOURCES   
           ${CMAKE_CURRENT_SOURCE_DIR}/assembly/test_aca_mode_assembly.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/assembly/test_discrete_aca_boundary_operator.cpp
           ${CMAKE_CURRENT_SOURCE_DIR}/assembly/test_synthetic_boundary_operator.cpp)
    endif()
  endif()

  foreach(testfile ${TEST_SOURCES})
    get_filename_component(filename ${testfile} NAME_WE)
    set(testname ${directory}_${filename})

    # Checks if needs extra files
    set(ADDITIONAL_SOURCE)
    file(READ ${testfile} SOURCE_TEXT)
    if(SOURCE_TEXT MATCHES "createRegularTriangularGrid")
      set(ADDITIONAL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/assembly/create_regular_grid.cpp)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${directory}/${filename}.hpp)
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${directory}/${filename}.hpp HEADER_TEXT)
    endif()
    if(NOT "${filename}" STREQUAL "test_grid" )
      if(SOURCE_TEXT MATCHES "SimpleTriangularGridManager"
         OR HEADER_TEXT MATCHES "SimpleTriangularGridManager"
         OR HEADER_TEXT MATCHES "test_grid_view.hpp")
        set(ADDITIONAL_SOURCE ${ADDITIONAL_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/grid/test_grid.cpp)
      endif()
    endif()
    if(SOURCE_TEXT MATCHES "Laplace3dDirichletFixture")
        set(ADDITIONAL_SOURCE ${ADDITIONAL_SOURCE}
            ${CMAKE_CURRENT_SOURCE_DIR}/linalg/laplace_3d_dirichlet_fixture.cpp)
    endif()
    add_executable(${testname} ${TEST_MAIN_SOURCES} ${testfile} ${ADDITIONAL_SOURCE})
      
    target_link_libraries(${testname} bempp ${BOOST_UNIT_TEST_LIB})
  
    add_test(NAME ${testname}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
             COMMAND ${testname} )
    set_tests_properties(${testname} PROPERTIES ENVIRONMENT ${ENVVARS})
  endforeach()
endforeach()

# Meshes
file(GLOB_RECURSE TEST_MESHES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.msh)
if (NOT (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR}))
    foreach (f ${TEST_MESHES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${f}
                       ${CMAKE_CURRENT_BINARY_DIR}/${f}
                       COPYONLY)
    endforeach ()
endif ()
