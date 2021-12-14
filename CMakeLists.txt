# root level cmakelists.txt for deepsea project 

cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(deepgarden)

# To make a debug build
set(CMAKE_CXX_FLAGS "-g")

find_package(OpenMP REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
#find_package(Freetype REQUIRED)

# Define the framework files.
set(SRCS
#	src/deepsea_types.h
#	src/deepsea_utilities.cpp
#	src/deepsea_utilities.h
#	src/deepsea.cpp
#	src/deepsea.h
#	src/deepsea_animals.cpp
#	src/deepsea_animals.h
#	src/deepsea_graphics.cpp
#	src/deepsea_graphics.h
#	src/deepsea_usertools.h
#	src/deepsea_usertools.cpp
#	src/deepsea_maps.h
#	src/deepsea_maps.cpp
	src/deepgarden.h
	src/main.cpp
	src/main.h
	src/deepgarden.cpp
	src/deepgarden_graphics.cpp
	src/deepgarden_graphics.h
#	src/main.h
)

link_directories(
#	${CMAKE_SOURCE_DIR}/box2d/build/bin/
#	${CMAKE_SOURCE_DIR}/fann/src/
	${Boost_LIBRARY_DIRS}
)

include_directories (
#	${CMAKE_SOURCE_DIR}/box2d/include/box2d/
#	${CMAKE_SOURCE_DIR}/fann/src/include/
	${SDL2_INCLUDE_DIRS}
#	${FREETYPE_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

add_executable(deepgarden ${SRCS} )

target_link_libraries (
	deepgarden
	OpenMP::OpenMP_CXX
#	${CMAKE_SOURCE_DIR}/box2d/build/bin/libbox2d.a
#	${CMAKE_SOURCE_DIR}/fann/src/libfann.a
	${SDL2_LIBRARIES}
	${OPENGL_LIBRARIES}
#	${FREETYPE_LIBRARIES}
	${Boost_LIBRARIES}
)


