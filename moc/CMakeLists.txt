cmake_minimum_required(VERSION 3.12)

get_directory_property(hasParent PARENT_DIRECTORY)

set(CMAKE_BUILD_TARGET_NAME moc)

set(MOC_NAME ${CMAKE_BUILD_TARGET_NAME})
set(MOC_MAJOR_VERSION 0)
set(MOC_MINOR_VERSION 9)
set(MOC_BUILD 401)
set(MOC_VERSION ${MOC_MAJOR_VERSION}.${MOC_MINOR_VERSION}.${MOC_BUILD})

if(NOT hasParent)
  project(${CMAKE_BUILD_TARGET_NAME} VERSION ${MOC_VERSION})
endif()

set(GENERATED_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/genSrc)

add_custom_command(OUTPUT "${GENERATED_SOURCE}/exists" ALL
        COMMAND echo "building ${GENERATED_SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_SOURCE}
        COMMAND ${CMAKE_COMMAND} -E touch ${GENERATED_SOURCE}/exists )

# project defined functions and macros
include(cmake-modules/BisonFlex.cmake)

# NOTE: the bison source is configured to
# rename 'yylex' => 'mocGetInputToken' while
# the flex source is configured to
# rename 'yylex' => 'mocInputlex'
# This allows the yylex implementation in the bison source
# to wrap the call to the flex generated mocInputlex()
# function. See MOC.y's yylex() function call to mocInputlex()
# This pattern is used for all the *.y/*.l combos below
bld_add_custom_flex_target(MOCParser.l
  SET_VAR MOC_FLEX_FILES
  YY_ERROR mocError
  YY_LEX mocInputlex
  YY_PREFIX mocInput
  DEPENDS ${GENERATED_SOURCE}/exists MOCInternal.h )
bld_add_custom_bison_target(MOC.y
  SET_VAR MOC_BISON_FILES
  YY_ERROR mocError
  YY_LEX mocGetInputToken
  YY_PREFIX mocInput
  DEPENDS ${GENERATED_SOURCE}/exists MOCInternal.h )



bld_add_custom_flex_target(IFParser.l
  SET_VAR IF_FLEX_FILES
  YY_ERROR mocError
  YY_LEX mocIflex
  YY_PREFIX mocIf
  DEPENDS ${GENERATED_SOURCE}/exists MOCInternal.h )
bld_add_custom_bison_target(IF.y
  SET_VAR IF_BISON_FILES
  YY_ERROR mocError
  YY_LEX mocGetIfToken
  YY_PREFIX mocIf
  DEPENDS ${GENERATED_SOURCE}/exists MOCInternal.h )



bld_add_custom_flex_target(CONTROLParser.l
  SET_VAR CONTROL_FLEX_FILES
  YY_ERROR mocError
  YY_LEX mocControllex
  YY_PREFIX mocControl
  DEPENDS ${GENERATED_SOURCE}/exists MOCInternal.h )
bld_add_custom_bison_target(CONTROL.y
  SET_VAR CONTROL_BISON_FILES
  YY_ERROR mocError
  YY_LEX mocGetControlToken
  YY_PREFIX mocControl
  DEPENDS ${GENERATED_SOURCE}/exists MOCInternal.h )

configure_file(Version.cpp.in Version.cpp @ONLY)

add_executable(${CMAKE_BUILD_TARGET_NAME}
        Input.cpp
        Input.h
        main.cpp
        Output.cpp
        Output.h
        ${CMAKE_CURRENT_BINARY_DIR}/Version.cpp
        ${MOC_FLEX_FILES}
        ${MOC_BISON_FILES}
        ${IF_FLEX_FILES}
        ${IF_BISON_FILES}
        ${CONTROL_FLEX_FILES}
        ${CONTROL_BISON_FILES})
target_include_directories(${CMAKE_BUILD_TARGET_NAME}
  PRIVATE
      ../uf
      ${GENERATED_SOURCE}
      ${CMAKE_CURRENT_BINARY_DIR}
      .
      )
target_link_libraries(${CMAKE_BUILD_TARGET_NAME} uf)

install(TARGETS ${CMAKE_BUILD_TARGET_NAME}
  EXPORT moc-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/${CMAKE_BUILD_TARGET_NAME}
  ARCHIVE DESTINATION lib/${CMAKE_BUILD_TARGET_NAME}
  )
install(EXPORT moc-targets
  DESTINATION lib/${CMAKE_BUILD_TARGET_NAME})
install(FILES moc-config.cmake
  DESTINATION lib/${CMAKE_BUILD_TARGET_NAME})
install(FILES
      cmake-modules/Moc.cmake
      cmake-modules/BisonFlex.cmake
  DESTINATION lib/cmake)
