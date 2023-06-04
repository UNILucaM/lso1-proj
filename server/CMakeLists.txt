cmake_minimum_required(VERSION 3.16)
project(LSODrinkServer VERSION 0.1 LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)
set(HEADERDIR ${PROJECT_SOURCE_DIR}/include)
set(SOURCEDIR ${PROJECT_SOURCE_DIR}/src)     
set(FILES bstnode 
	config
	errorhandling 
	httphelper
	linkedlist
	mlog 
	serverinfo 
	strutil 
	threadroutines)
set(SRCFILES ${FILES})
list(INSERT SRCFILES 0 main)
list(TRANSFORM SRCFILES PREPEND ${SOURCEDIR}/)
list(TRANSFORM SRCFILES APPEND .c)
set(HEADERFILES ${FILES})
list(INSERT HEADERFILES 0 config)
list(TRANSFORM HEADERFILES PREPEND ${HEADERDIR}/)
list(TRANSFORM HEADERFILES APPEND .h)
add_executable(server ${SRCFILES} ${HEADERFILES})
target_include_directories(server PRIVATE ${HEADERDIR}) 
find_package(PostgreSQL REQUIRED)
target_include_directories(server PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${PostgreSQL_LIBRARIES})
find_package(Jansson REQUIRED)
target_include_directories(server PRIVATE ${JANSSON_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${JANSSON_LIBRARIES})