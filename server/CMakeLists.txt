cmake_minimum_required(VERSION 3.16)
project(LSODrinkServer VERSION 0.1 LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(HEADERDIR ${PROJECT_SOURCE_DIR}/include)
set(SOURCEDIR ${PROJECT_SOURCE_DIR}/src)     
set(FILES bstnode 
	errorhandling 
	httphelper 
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
