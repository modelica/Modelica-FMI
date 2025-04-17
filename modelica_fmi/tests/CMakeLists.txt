cmake_minimum_required (VERSION 3.17)

project (Modelica-FMI)

set(UNZIP_DIR "" CACHE PATH "Path to the extracted FMU")
set(INSTANTIATION_TOKEN "" CACHE STRING "Instantiation token of the FMU")
set(FMI_VERSION 2 CACHE STRING "FMI Version")
set_property(CACHE FMI_VERSION PROPERTY STRINGS 2 3)

add_executable(import_fmu
    ../FMI/Resources/Include/FMI.h
    ../FMI/Resources/Include/FMI2.h
    ../FMI/Resources/Include/FMI3.h
    ../FMI/Resources/Include/ModelicaFMI.h
    ../FMI/Resources/Include/ModelicaFMI2.h
    ../FMI/Resources/Include/ModelicaFMI3.h
    ModelicaUtilities.c
    simulate_fmi${FMI_VERSION}_cs.c
)

set_target_properties(import_fmu PROPERTIES
    C_STANDARD 90
)

target_include_directories(import_fmu PRIVATE
    .
    ../FMI/Resources/Include
)

target_compile_definitions(import_fmu PRIVATE
    UNZIP_DIR="${UNZIP_DIR}"
    INSTANTIATION_TOKEN="${INSTANTIATION_TOKEN}"
)

target_link_libraries(import_fmu
    ${CMAKE_DL_LIBS}
)
