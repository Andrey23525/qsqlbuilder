cmake_minimum_required(VERSION 3.22)

project(SQLBuilder
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

qt_add_library(SQLBuilder STATIC
    ${SQLBUILDER_SOURCES}
    ${SQLBUILDER_HEADERS}
)

add_library(SQLBuilder::SQLBuilder ALIAS SQLBuilder)

target_include_directories(SQLBuilder
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(SQLBuilder PUBLIC cxx_std_23)
target_precompile_headers(SQLBuilder PUBLIC include/pch.h)

target_link_libraries(SQLBuilder
    PUBLIC
        Qt6::Core
        Qt6::Sql
)
