cmake_minimum_required(VERSION 3.22.1)


project("fortest")


add_library(${CMAKE_PROJECT_NAME} SHARED

    fortest.cpp )


#------------------------------------------
add_subdirectory(${PROJECT_SOURCE_DIR}/SDK/)
add_subdirectory(${PROJECT_SOURCE_DIR}/IO/)

#
include_directories(${PROJECT_SOURCE_DIR}/SDK/)
include_directories(${PROJECT_SOURCE_DIR}/IO/)

#add_library(io_kpl SHARED IMPORTED)
#set_target_properties(io_kpl PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/build/${ANDROID_ABI}/libKPL_IO.so)
#add_library(sdk_kpl SHARED IMPORTED)
#set_target_properties(sdk_kpl PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/build/${ANDROID_ABI}/libKPL_SDK.so)
#-------------------------------------------

target_link_libraries(${CMAKE_PROJECT_NAME}  log)
target_link_libraries(${CMAKE_PROJECT_NAME}  KPL_SDK)
target_link_libraries(${CMAKE_PROJECT_NAME}  KPL_IO)







