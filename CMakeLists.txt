cmake_minimum_required (VERSION 2.6)

project(Server)
add_subdirectory(main)
add_subdirectory(libgpio)

SET(RASP_IP "root@192.168.100.13" CACHE INTERNAL "Raspberry pi ip")
