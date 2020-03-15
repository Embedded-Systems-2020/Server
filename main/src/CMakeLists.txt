#include directories to building path
include_directories(${PROJECT_SOURCE_DIR}/libgpio/include)
include_directories(${PROJECT_SOURCE_DIR}/librsa/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)



# add the executable
add_executable (Server server.c)

#link against required libraries
target_link_libraries(Server gpio rsa ulfius jansson crypto)

#install the binary
install (TARGETS Server DESTINATION bin)

#copy the executable to the raspberry pi /bin directory
install(CODE "execute_process(COMMAND scp -r ${PROJECT_SOURCE_DIR}/build/usr/bin ${RASP_IP}:/)")

#package source generation
set(MAJOR "0")
set(MINOR "1")
set(PATCH "1")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${MAJOR}.${MINOR}.${PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
