get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
  project(uf)
endif()

set(SOURCE_FILES
        generic.h
        ufAll.h
        ufAssert.cpp
        ufDate.cpp
        ufDate.h
        ufDef.cpp
        ufDef.h
        ufFileNm.cpp
        ufFileNm.h
        ufIHshT.cpp
        ufIHshT.h
        ufList.cpp
        ufList.h
        ufPtr.cpp
        ufPtr.h
        ufSHshT.cpp
        ufSHshT.h
        ufString.cpp
        ufString.h
        ufUtil.cpp)

add_library(uf ${SOURCE_FILES})
target_compile_definitions(uf PUBLIC USE_STL_STREAMS)

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

