#add include to directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

#set the proper macros
set(LIBRARY_NAME gpio)
set(SRC_FILES gpio.c)
set(INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../include/gpio.h)

#add the library
add_library(${LIBRARY_NAME} SHARED ${SRC_FILES} ${INCLUDE_FILES})

#installing the library
install (TARGETS ${LIBRARY_NAME} DESTINATION lib)
install (FILES ${INCLUDE_FILES} DESTINATION include)

#copy the library to the raspberry pi /lib directory
install(CODE "execute_process(COMMAND scp -r ${PROJECT_SOURCE_DIR}/build/usr/lib ${RASP_IP}:/)")
