# Variables, functions and macros use BLD_ or bld_ prefix to distinguish
# themselves.

#set(BLD_BuildVerbose TRUE)

function(bld_verbose_message)
  if (BLD_BuildVerbose)
    message("conditionaL: ${ARGN}")
  endif()
endfunction()

find_program (MOC_TOOL NAMES moc moc.exe REQUIRED)
bld_verbose_message("found moc at ${MOC_TOOL}")



# Example:
#  bld_add_custom_moc_target(inputfile.moc
#    SET_VAR MOC_GEN_SRC
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#    INCLUDE ${CMAKE_SOURCE_DIR}
#    TEMPLATE ${CMAKE_SOURCE_DIR}/test_app.tpl
#    DEPENDS_ON Def.moh
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/genSrc
#  )
function(bld_add_custom_moc_target InputFile)
  cmake_parse_arguments(MOCBLD "" "WORKING_DIRECTORY;SET_VAR;TEMPLATE;OUTPUT" "INCLUDE;DEPENDS_ON" ${ARGN})
  if (MOCBLD_WORKING_DIRECTORY)
    set(WORKING_DIR ${MOCBLD_WORKING_DIRECTORY})
  else()
    set(WORKING_DIR ${CMAKE_SOURCE_DIR})
  endif()
  get_filename_component(Basename ${InputFile} NAME_WE)
  set(INCLUDES "")
  foreach(DIR IN LISTS MOCBLD_INCLUDE )
    if ( INCLUDES STREQUAL "" )
      set(INCLUDES "-I${DIR}")
    else()
      set(INCLUDES "${INCLUDES};-I${DIR}")
    endif()
  endforeach()
  set(SequenceLogFile "${MOCBLD_OUTPUT}/${Basename}.sequence")
  set(GeneratedSources "${SequenceLogFile}")
  if (MOCBLD_SET_VAR)
    set(${MOCBLD_SET_VAR} "${GeneratedSources}" PARENT_SCOPE)
  endif()
  bld_verbose_message("
    WORKING_DIR ${WORKING_DIR}
    COMMAND ${MOC_TOOL} ${INCLUDES} -S ${SequenceLogFile} -o ${MOCBLD_OUTPUT} -T ${MOCBLD_TEMPLATE} ${InputFile}
  ")
  add_custom_command(
    COMMENT ""
    WORKING_DIR ${WORKING_DIR}
    DEPENDS "${InputFile}" ${MOCBLD_DEPENDS_ON}
    COMMAND ${MOC_TOOL} ${INCLUDES} -S ${SequenceLogFile} -o ${MOCBLD_OUTPUT} -T ${MOCBLD_TEMPLATE} ${InputFile}
    OUTPUT ${GeneratedSources}
    COMMAND_EXPAND_LISTS
  )
endfunction()

# Example:
#  bld_add_generated_sources(SRC DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genSrc)
function(bld_add_generated_sources SET_VAR)
  cmake_parse_arguments(MOCBLD "" "DIRECTORY" "" ${ARGN})
  set(GeneratedSrcDirectory ${MOCBLD_DIRECTORY})
  bld_verbose_message("looking for generated sources in '" ${GeneratedSrcDirectory} "'")
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0")
    file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${GeneratedSrcDirectory}/*.cpp ${GeneratedSrcDirectory}/*.C ${GeneratedSrcDirectory}/*.cxx ${GeneratedSrcDirectory}/*.cc ${GeneratedSrcDirectory}/*.h ${GeneratedSrcDirectory}/*.hh)
  else()
    file(GLOB_RECURSE SOURCE_FILES ${GeneratedSrcDirectory}/*.cpp ${GeneratedSrcDirectory}/*.C ${GeneratedSrcDirectory}/*.cxx ${GeneratedSrcDirectory}/*.cc ${GeneratedSrcDirectory}/*.h ${GeneratedSrcDirectory}/*.hh)
  endif()
  bld_verbose_message("found sources '" ${SOURCE_FILES} "'")
  set(${SET_VAR} "${SOURCE_FILES}" PARENT_SCOPE)
endfunction()

