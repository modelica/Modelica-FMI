cmake_minimum_required (VERSION 3.17)

project (ModelicaFMI)

if (WIN32)
    set(MODELICA_PLATFORM win)
elseif (APPLE)
    set(MODELICA_PLATFORM darwin)
else ()
    set(MODELICA_PLATFORM linux)
endif ()

if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "AMD64|x86_64|aarch64")
    set(MODELICA_PLATFORM "${MODELICA_PLATFORM}64")
else ()
    set(MODELICA_PLATFORM "${MODELICA_PLATFORM}32")
endif ()

# set(UNZIP_DIR "" CACHE PATH "Path to the extracted FMU")
# set(INSTANTIATION_TOKEN "" CACHE STRING "Instantiation token of the FMU")
# set(FMI_VERSION 3 CACHE STRING "FMI Version")
# set_property(CACHE FMI_VERSION PROPERTY STRINGS 2 3)

add_library(ModelicaFMI SHARED
    include/FMI.h
    include/FMI2.h
    include/FMI3.h
    include/ModelicaFMI.h
    include/ModelicaFMI2.h
    include/ModelicaFMI3.h
    src/FMI.c
    src/FMI2.c
    src/FMI3.c
    src/ModelicaFMI.c
    src/ModelicaFMI2.c
    src/ModelicaFMI3.c
    src/ModelicaUtilities.c
)

target_include_directories(ModelicaFMI PRIVATE 
    ../FMI/Resources/Include
    include
)

set_property(TARGET ModelicaFMI PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

install(TARGETS ModelicaFMI DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../FMI/Resources/Library/${MODELICA_PLATFORM})

# add_executable(ModelicaFMI_tests
#     tests/simulate_fmi${FMI_VERSION}_cs.c
# )

# target_include_directories(ModelicaFMI_tests PRIVATE include)

# target_compile_definitions(ModelicaFMI_tests PRIVATE
#     UNZIP_DIR="${UNZIP_DIR}"
#     INSTANTIATION_TOKEN="${INSTANTIATION_TOKEN}"
# )

# add_dependencies(ModelicaFMI_tests ModelicaFMI)

# target_link_libraries(ModelicaFMI_tests PUBLIC ModelicaFMI)
