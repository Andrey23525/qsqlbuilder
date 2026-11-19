cmake_minimum_required(VERSION 3.22)

project(QSQLBuilder
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(QSQLBUILDER_ENABLE_LTO "Enable link-time optimization in Release and RelWithDebInfo builds." ${QSQLBuilder_IS_TOP_LEVEL})
set(_qsqlbuilder_lto OFF)
if(QSQLBuilder_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _qsqlbuilder_lto OUTPUT _qsqlbuilder_lto_output LANGUAGES CXX)
    if(_qsqlbuilder_lto_output)
        # Initializes INTERPROCEDURAL_OPTIMIZATION_<CONFIG> of every target created below,
        # QWindowKit and the example included.
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
        set(_qsqlbuilder_lto ON)
    else()
        message(WARNING "Link-time optimization is not supported by the toolchain, building without it:\n"
            "${_qsqlbuilder_lto_output}")
    endif()
endif()


set(_sqlbuilder_qt_fallback_paths)
foreach(_var IN ITEMS QT_DIR QTDIR)
    if(${_var})
        list(APPEND _sqlbuilder_qt_fallback_paths "${${_var}}")
    endif()
    if(DEFINED ENV{${_var}})
        file(TO_CMAKE_PATH "$ENV{${_var}}" _path)
        list(APPEND _sqlbuilder_qt_fallback_paths "${_path}")
    endif()
endforeach()

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Sql
    PATHS ${_sqlbuilder_qt_fallback_paths}
)

qt_standard_project_setup()

file(GLOB_RECURSE SQLBUILDER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Headers are listed as sources so that AUTOMOC processes them: Q_GADGET classes live in include/
file(GLOB_RECURSE SQLBUILDER_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

qt_add_library(${PROJECT_NAME} STATIC
    ${SQLBUILDER_SOURCES}
    ${SQLBUILDER_HEADERS}
)

add_library(QSQLBuilder::QSQLBuilder ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/qsqlbuilder/pch.h)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt6::Core
        Qt6::Sql
)

option(QSQLBUILDER_BUILD_TESTS "Build deterministic SQL generation tests." ${QSQLBuilder_IS_TOP_LEVEL})
if(QSQLBUILDER_BUILD_TESTS)
    enable_testing()
    add_executable(qsqlbuilder_command_tests tests/commands.cpp)
    target_precompile_headers(qsqlbuilder_command_tests PRIVATE include/qsqlbuilder/pch.h)
    target_link_libraries(qsqlbuilder_command_tests PRIVATE QSQLBuilder::QSQLBuilder)
    add_test(NAME qsqlbuilder_command_tests COMMAND qsqlbuilder_command_tests)
endif()
