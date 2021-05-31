# 
# This builds the test application
#
cmake_minimum_required(VERSION 3.12)

#----------------------------------------------------------------------------
# Setup test project
set(CMAKE_BUILD_TARGET_NAME MocTestApp)

#set(USE_CXX20 True)

get_directory_property(hasParent PARENT_DIRECTORY)

if(NOT hasParent)
  project(${CMAKE_BUILD_TARGET_NAME})
endif()


list(APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake-modules )

find_package(Moc REQUIRED)

#----------------------------------------------------------------------------
# Main Target name
set(TARGET_GEN_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/genSrc)

bld_add_custom_moc_target(${CMAKE_SOURCE_DIR}/test_app.moc
    SET_VAR MOC_GEN_SRC_FILES
    INCLUDE ${CMAKE_SOURCE_DIR}
    TEMPLATE ${CMAKE_SOURCE_DIR}/test_app.tpl
    DEPENDS_ON ${CMAKE_SOURCE_DIR}/test_app.moh
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/genSrc
  )

add_executable(${CMAKE_BUILD_TARGET_NAME}
  main.cpp
  ${MOC_GEN_SRC_FILES}
  )
target_include_directories(${CMAKE_BUILD_TARGET_NAME}
  PUBLIC ../include
  PRIVATE ../src ${TARGET_GEN_SRC_DIR})

#----------------------------------------------------------------------------
# Install targets

install(TARGETS ${CMAKE_BUILD_TARGET_NAME}
  DESTINATION bin )


