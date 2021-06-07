cmake_minimum_required(VERSION 3.12)

get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
  project(uf)
endif()

set(UF_TARGET_NAME uf)

set(HEADER_FILES
        generic.h
        ufAll.h
        ufDate.h
        ufDef.h
        ufFileNm.h
        ufIHshT.h
        ufList.h
        ufPtr.h
        ufSHshT.h
        ufString.h
        )

set(SOURCE_FILES
        ufAssert.cpp
        ufDate.cpp
        ufDef.cpp
        ufFileNm.cpp
        ufIHshT.cpp
        ufList.cpp
        ufPtr.cpp
        ufSHshT.cpp
        ufString.cpp
        ufUtil.cpp
        )

add_library(${UF_TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_compile_definitions(${UF_TARGET_NAME} PUBLIC USE_STL_STREAMS)

install(TARGETS ${UF_TARGET_NAME}
  EXPORT ${UF_TARGET_NAME}-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/${UF_TARGET_NAME}
  ARCHIVE DESTINATION lib/${UF_TARGET_NAME}
  )
install(EXPORT ${UF_TARGET_NAME}-targets
  DESTINATION lib/${UF_TARGET_NAME})
install(FILES
      ${HEADER_FILES}
  DESTINATION include)

add_executable(ufDefTest ufDef.cpp ufString.cpp)
target_compile_definitions(ufDefTest PUBLIC ufDefTEST DEBUG TEST USE_STL_STREAMS)

add_executable(ufPtrTest ufPtr.cpp)
target_compile_definitions(ufPtrTest PUBLIC ufPtrTEST DEBUG TEST USE_STL_STREAMS)

add_executable(ufStringTest ufString.cpp ufDef.cpp)
target_compile_definitions(ufStringTest PUBLIC ufStringTEST DEBUG TEST USE_STL_STREAMS)

add_executable(ufDateTest ufDate.cpp ufDef.cpp ufString.cpp)
target_compile_definitions(ufDateTest PUBLIC ufDateTEST DEBUG TEST USE_STL_STREAMS)

add_executable(ufListTest ufList.cpp  ufPtr.cpp)
target_compile_definitions(ufListTest PUBLIC ufListTEST DEBUG TEST USE_STL_STREAMS)

add_executable(ufSHshTTest ufSHshT.cpp ufPtr.cpp ufString.cpp ufDef.cpp)
target_compile_definitions(ufSHshTTest PUBLIC ufSHshTTEST DEBUG TEST USE_STL_STREAMS)

add_executable(ufIHshTTest ufIHshT.cpp ufPtr.cpp)
target_compile_definitions(ufIHshTTest PUBLIC ufIHshTTEST DEBUG TEST USE_STL_STREAMS)

