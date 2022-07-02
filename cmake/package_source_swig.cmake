find_package(SWIG REQUIRED)

function(package_source_swig_python SWIG_PYTHON_EXTRA_ARGS SWIG_PYTHON_OUTDIR outfiles)
    include(${CMAKE_SOURCE_DIR}/RobotRaconteurPython/PythonSwigVars.cmake)
    add_custom_command(
        TARGET package_source_swig
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${SWIG_PYTHON_OUTDIR}
        COMMAND
            "${CMAKE_COMMAND}" -E env "SWIG_LIB=${SWIG_DIR}" "${SWIG_EXECUTABLE}" -python -relativeimport
            -outdir ${SWIG_PYTHON_OUTDIR} -c++ -I${CMAKE_SOURCE_DIR}/RobotRaconteurCore/include -I${CMAKE_SOURCE_DIR}
            -I${CMAKE_SOURCE_DIR}/RobotRaconteurPython -I${CMAKE_SOURCE_DIR}/SWIG -o
            ${SWIG_PYTHON_OUTDIR}/RobotRaconteurPythonPYTHON_wrap.cxx ${SWIG_PYTHON_EXTRA_ARGS}
            ${CMAKE_SOURCE_DIR}/RobotRaconteurPython/RobotRaconteurPython.i)
    set(${outfiles} ${SWIG_PYTHON_OUTDIR}/RobotRaconteurPythonPYTHON.cxx PARENT_SCOPE)
endfunction()

set(PACKAGE_SWIG_SOURCE_ALL OFF CACHE BOOL "Always build swig source")
if(PACKAGE_SWIG_SOURCE_ALL)
    add_custom_target(package_source_swig ALL)
else()
    add_custom_target(package_source_swig)
    set_target_properties(package_source_swig PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
endif()

package_source_swig_python("" ${CMAKE_BINARY_DIR}/generated_src/Python/swigwordsize32 python_sources_32)
package_source_swig_python("-DSWIGWORDSIZE64" ${CMAKE_BINARY_DIR}/generated_src/Python/swigwordsize64 python_sources_64)
