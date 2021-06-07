
get_filename_component(MOC_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../include/moc" ABSOLUTE)
get_filename_component(MOC_CMAKE_MODULES "${CMAKE_CURRENT_LIST_DIR}/../../lib/cmake" ABSOLUTE)
set(MOC_LIBRARY "${CMAKE_CURRENT_LIST_DIR}")

# allow the cmake folder to be found
list(APPEND CMAKE_MODULE_PATH ${MOC_CMAKE_MODULES})

# include the MOC targets
include(${CMAKE_CURRENT_LIST_DIR}/moc-targets.cmake)
# We could use the cmake search path below but 
# instead we'll specify the full path to Moc.cmake to
# ensure we get the correct one.
#include(Moc) 
include(${MOC_CMAKE_MODULES}/Moc.cmake)
