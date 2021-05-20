get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
  project(uf)
endif()

set(SOURCE_FILES
        generic.h
        ufAll.h
        ufAssert.C
        ufDate.C
        ufDate.h
        ufDef.C
        ufDef.h
        ufFileNm.C
        ufFileNm.h
        ufIHshT.C
        ufIHshT.h
        ufList.C
        ufList.h
        ufPtr.C
        ufPtr.h
        ufSHshT.C
        ufSHshT.h
        ufString.C
        ufString.h
        ufUtil.C)

add_library(uf ${SOURCE_FILES})

add_executable(ufPtrTest ufPtr.C)
target_compile_definitions(ufPtrTest PUBLIC ufPtrTEST DEBUG TEST)

add_executable(ufListTest ufList.C  ufPtr.C)
target_compile_definitions(ufListTest PUBLIC ufListTEST DEBUG TEST)

add_executable(ufSHshTTest ufSHshT.C ufPtr.C ufString.C)
target_compile_definitions(ufSHshTTest PUBLIC ufSHshTTEST DEBUG TEST)

add_executable(ufIHshTTest ufIHshT.C ufPtr.C)
target_compile_definitions(ufIHshTTest PUBLIC ufIHshTTEST DEBUG TEST)
