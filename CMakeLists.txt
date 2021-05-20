cmake_minimum_required(VERSION 3.12)

get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
  project(mocandlibs)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_subdirectory(moc)
add_subdirectory(uf)

