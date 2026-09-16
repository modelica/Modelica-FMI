cmake_minimum_required (VERSION 4.0)

set (MODEL_IDENTIFIER "{{ model_identifier }}")
set (FMI_MAJOR_VERSION "{{ fmi_major_version }}")
set (TARGET_PATH "{{ target_path }}")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project (${MODEL_IDENTIFIER})

# Detect if the compilation target is 32-bit or 64-bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64_BIT TRUE)
else()
    set(IS_64_BIT FALSE)
endif()

# Normalize host processor string
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" HOST_ARCH)

# Detect architecture based on target bitness
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(IS_64_BIT)
        # 64-bit Windows Target
        if(FMI_MAJOR_VERSION EQUAL "2")
            set(FMI_PLATFORM "win64")
        else()
            set(FMI_PLATFORM "x86_64-windows")
        endif()
    else()
        # 32-bit Windows Target (Triggers on -A Win32)
        if(FMI_MAJOR_VERSION EQUAL "2")
            set(FMI_PLATFORM "win32")
        else()
            set(FMI_PLATFORM "x86-windows")
        endif()
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(HOST_ARCH STREQUAL "aarch64")
        set(FMI_PLATFORM "aarch64-linux")
    elseif(IS_64_BIT)
        if(FMI_MAJOR_VERSION EQUAL "2")
            set(FMI_PLATFORM "linux64")
        else()
            set(FMI_PLATFORM "x86_64-linux")
        endif()
    else()
        if(FMI_MAJOR_VERSION EQUAL "2")
            set(FMI_PLATFORM "linux32")
        else()
            set(FMI_PLATFORM "x86-linux")
        endif()
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if(HOST_ARCH STREQUAL "aarch64")
        set(FMI_PLATFORM "aarch64-darwin")
    else()
        set(FMI_PLATFORM "x86_64-darwin")
    endif()

else()
    message(FATAL_ERROR "Unknown or unsupported platform: ${CMAKE_SYSTEM_NAME}-${HOST_ARCH}")
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

add_library(${MODEL_IDENTIFIER} SHARED 
{%- for source in sources %}
    "{{ source }}"
{%- endfor %}
)

target_compile_definitions(${MODEL_IDENTIFIER} PUBLIC
{%- for definition in definitions %}
    "{{ definition }}"
{%- endfor %}
)

target_include_directories(${MODEL_IDENTIFIER} PUBLIC
{%- for include_dir in include_dirs %}
    "{{ include_dir }}"
{%- endfor %}
)

set_target_properties(${MODEL_IDENTIFIER} PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    LIBRARY_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    ARCHIVE_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/src/binaries/${FMI_PLATFORM}"
)

add_custom_command(
    TARGET ${MODEL_IDENTIFIER} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E tar cfv "${TARGET_PATH}" --format=zip .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMENT "Creating ZIP archive..."
)
