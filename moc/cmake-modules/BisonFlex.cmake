
find_program (FLEX_TOOL NAMES flex REQUIRED)
find_program (BISON_TOOL NAMES bison REQUIRED)
find_program (SED_TOOL NAMES sed REQUIRED)

function(bld_verbose_message message)
  if (Verbose)
    message(${message})
  endif()
endfunction()

#bld_add_custom_bison_target(INPUT.y
#  SET_VAR MY_BISON_FILES -required
#  YY_ERROR myError - opt to give a completely different name to yyerror
#  YY_LEX myGetToken- opt to give a completely different name yylex
#  YY_PREFIX my - required
#  DEPENDS ${GENERATED_SOURCE}/exists myHeader.h )
function(bld_add_custom_bison_target InputFile)
  cmake_parse_arguments(P "" "PREFIX;SET_VAR;YY_ERROR;YY_LEX;YY_PREFIX" "DEPENDS" ${ARGN})
  if ((NOT P_SET_VAR) OR (NOT P_YY_PREFIX))
    message(FATAL_ERROR "You must supply SET_VAR and YY_PREFIX")
  endif()
  if (NOT P_YY_ERROR)
    set(P_YY_ERROR "${P_YY_PREFIX}error")
  endif()
  if (NOT P_YY_LEX)
    set(P_YY_LEX "${P_YY_PREFIX}lex")
  endif()
  get_filename_component(Basename ${InputFile} NAME_WE)
  set(TempName "${GENERATED_SOURCE}/${Basename}.temp")
  set(TempHeaderName "${GENERATED_SOURCE}/${Basename}.y.tab.h.temp")
  set(GenSrcName "${GENERATED_SOURCE}/${Basename}.l.cpp")
  set(GenHeaderName "${GENERATED_SOURCE}/${Basename}Tokens.h")
  set(${P_SET_VAR} "${GenSrcName};${GenHeaderName}" PARENT_SCOPE)
  bld_verbose_message("COMMAND: ${BISON_TOOL} --defines=${TempHeaderName} -o ${TempName} ${CMAKE_CURRENT_SOURCE_DIR}/${InputFile}")
  bld_verbose_message("COMMAND: ${SED_TOOL} -e 's/yyerror/${P_YY_ERROR}/g' -e 's/yylex/${P_YY_LEX}/g' -e 's/yy/${P_YY_PREFIX}/g' < '${TempHeaderName}' > ${GenHeaderName}")
  bld_verbose_message("COMMAND: ${SED_TOOL} -e 's/yyerror/${P_YY_ERROR}/g' -e 's/yylex/${P_YY_LEX}/g' -e 's/yy/${P_YY_PREFIX}/g' < '${TempName}' > ${GenSrcName}")
  add_custom_command(
    COMMENT ""
    OUTPUT "${TempName}" "${GenSrcName}" "${TempHeaderName}" "${GenHeaderName}"
    DEPENDS "${InputFile}" ${P_DEPENDS}
    COMMAND ${BISON_TOOL} --defines=${TempHeaderName} -o ${TempName} ${CMAKE_CURRENT_SOURCE_DIR}/${InputFile}
    COMMAND ${SED_TOOL} -e 's/yyerror/${P_YY_ERROR}/g' -e 's/yylex/${P_YY_LEX}/g' -e 's/yy/${P_YY_PREFIX}/g' < "${TempHeaderName}" > ${GenHeaderName} 
    COMMAND ${SED_TOOL} -e 's/yyerror/${P_YY_ERROR}/g' -e 's/yylex/${P_YY_LEX}/g' -e 's/yy/${P_YY_PREFIX}/g' < "${TempName}" > ${GenSrcName} )
endfunction()

#bld_add_custom_flex_target(parser.l
#  SET_VAR MY_FLEX_FILES
#  YY_ERROR myError - opt to give a completely different name to yyerror
#  YY_LEX myGetToken- opt to give a completely different name yylex
#  YY_PREFIX my - required
#  DEPENDS ${GENERATED_SOURCE}/exists MyHeader.h )
function(bld_add_custom_flex_target InputFile)
  cmake_parse_arguments(P "" "PREFIX;SET_VAR;YY_ERROR;YY_LEX;YY_PREFIX" "DEPENDS" ${ARGN})
  if ((NOT P_SET_VAR) OR (NOT P_YY_PREFIX))
    message(FATAL_ERROR "You must supply SET_VAR and YY_PREFIX")
  endif()
  if (NOT P_YY_ERROR)
    set(P_YY_ERROR "${P_YY_PREFIX}error")
  endif()
  if (NOT P_YY_LEX)
    set(P_YY_LEX "${P_YY_PREFIX}lex")
  endif()
  get_filename_component(Basename ${InputFile} NAME_WE)
  set(TempName "${GENERATED_SOURCE}/${Basename}.temp")
  set(GenSrcName "${GENERATED_SOURCE}/${Basename}.l.cpp")
  set(${P_SET_VAR} "${GenSrcName}" PARENT_SCOPE)
  bld_verbose_message("COMMAND: ${FLEX_TOOL} -L -o ${TempName} ${CMAKE_CURRENT_SOURCE_DIR}/${InputFile}")
  bld_verbose_message("COMMAND: ${SED_TOOL} -e 's/yyerror/${P_YY_ERROR}/g' -e 's/yylex/${P_YY_LEX}/g' -e 's/yy/${P_YY_PREFIX}/g' < '${TempName}' > ${GenSrcName}")
  add_custom_command(
    COMMENT ""
    OUTPUT "${TempName}" "${GenSrcName}"
    DEPENDS "${InputFile}"  ${P_DEPENDS}
    COMMAND ${FLEX_TOOL} -L -o ${TempName} ${CMAKE_CURRENT_SOURCE_DIR}/${InputFile}
    COMMAND ${SED_TOOL} -e 's/yyerror/${P_YY_ERROR}/g' -e 's/yylex/${P_YY_LEX}/g' -e 's/yy/${P_YY_PREFIX}/g' < "${TempName}" > ${GenSrcName} )

endfunction()
