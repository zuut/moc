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

add_executable(ufPtrTest ufPtr.cpp)
target_compile_definitions(ufPtrTest PUBLIC ufPtrTEST DEBUG TEST)

add_executable(ufListTest ufList.cpp  ufPtr.cpp)
target_compile_definitions(ufListTest PUBLIC ufListTEST DEBUG TEST)

add_executable(ufSHshTTest ufSHshT.cpp ufPtr.cpp ufString.cpp)
target_compile_definitions(ufSHshTTest PUBLIC ufSHshTTEST DEBUG TEST)

add_executable(ufIHshTTest ufIHshT.cpp ufPtr.cpp)
target_compile_definitions(ufIHshTTest PUBLIC ufIHshTTEST DEBUG TEST)
