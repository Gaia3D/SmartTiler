CMAKE_MINIMUM_REQUIRED(VERSION 3.9 FATAL_ERROR)

#--------------------------------------------------------------------
# Project Information
#--------------------------------------------------------------------
PROJECT(SmartTiler)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

IF(WIN32)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE /D_CRT_SECURE_NO_WARNINGS")
ELSE()
	LINK_LIBRARIES(stdc++fs)
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Config Message
include(cmake/ConfigMessage.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

#--------------------------------------------------------------------
# Build Jsoncpp as an external project.
#--------------------------------------------------------------------
SET(JSONCPP_INSTALL_DIR ${CMAKE_BINARY_DIR}/external/jsoncpp)
SET(JSONCPP_INCLUDE_DIR ${JSONCPP_INSTALL_DIR}/include)
SET(JSONCPP_LIBRARY_DIR ${JSONCPP_INSTALL_DIR}/external)
include(ExternalProject)
ExternalProject_Add(jsoncpp_external_project
                    SOURCE_DIR  ${CMAKE_SOURCE_DIR}/external/jsoncpp
                    PREFIX      ${JSONCPP_INSTALL_DIR}
                    INSTALL_DIR ${JSONCPP_INSTALL_DIR}
                    CMAKE_ARGS  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                                -DCMAKE_CXX_FLAGS=${EXTERNAL_PROJECT_CMAKE_CXX_FLAGS}
								-DCMAKE_INSTALL_PREFIX:PATH=${JSONCPP_INSTALL_DIR}
								-DCMAKE_INSTALL_LIBDIR=${JSONCPP_LIBRARY_DIR}
                   )

SET(JSONCPP_INCLUDE_DIRS "${JSONCPP_INCLUDE_DIR}")
SET(JSONCPP_LIBRARIES jsoncpp${STATIC_SUFFIX})

MESSAGE(STATUS "${JSONCPP_INCLUDE_DIRS}")
MESSAGE(STATUS "${JSONCPP_LIBRARIES}")

LINK_DIRECTORIES(${JSONCPP_LIBRARY_DIR})
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${JSONCPP_INCLUDE_DIRS})
LINK_LIBRARIES(${JSONCPP_LIBRARIES})

# A target to combine all of the external projects.
ADD_CUSTOM_TARGET(build_external_projects
                  DEPENDS jsoncpp_external_project)

#--------------------------------------------------------------------
# 
#--------------------------------------------------------------------

add_subdirectory(external)

INCLUDE_DIRECTORIES(include)

FILE(GLOB			PROJECT_HEADERS include/*.h)
FILE(GLOB_RECURSE	PROJECT_SOURCES src/*.cpp src/*.c)
FILE(GLOB			PROJECT_CONFIGS	CMakeLists.txt)
									
FOREACH(item ${PROJECT_SOURCES})
	MESSAGE ("Now Globbing..... ${item}")
ENDFOREACH()

ADD_EXECUTABLE(${PROJECT_NAME} ${PROJECT_SOURCES})
ADD_DEPENDENCIES(${PROJECT_NAME} build_external_projects)

#TARGET_LINK_LIBRARIES(${PROJECT_NAME} )

SET_TARGET_PROPERTIES(${PROJECT_NAME}	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}) 
	
INSTALL(
		TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION bin
		ARCHIVE DESTINATION lib
)