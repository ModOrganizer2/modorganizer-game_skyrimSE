CMAKE_MINIMUM_REQUIRED (VERSION 2.8.11)

CMAKE_POLICY(SET CMP0020 NEW)

FILE(GLOB ${PROJ_NAME}_SRCS *.cpp)
FILE(GLOB ${PROJ_NAME}_HDRS *.h)
FILE(GLOB ${PROJ_NAME}_FORMS *.ui)


SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
FIND_PACKAGE(Qt5Widgets REQUIRED)
QT5_WRAP_UI(${PROJ_NAME}_UIHDRS ${${PROJ_NAME}_FORMS})

SET(default_project_path "${CMAKE_SOURCE_DIR}/..")
GET_FILENAME_COMPONENT(${default_project_path} ${default_project_path} REALPATH)

SET(project_path "${default_project_path}" CACHE PATH "path to the other mo projects")

FIND_PACKAGE(Qt5LinguistTools)
SET(translation_sources  ${CMAKE_SOURCE_DIR}/src ${project_path}/game_gamebryo/src)
QT5_CREATE_TRANSLATION(${PROJ_NAME}_translations_qm ${translation_sources} ${CMAKE_SOURCE_DIR}/src/${PROJ_NAME}_en.ts)

SET(Boost_USE_STATIC_LIBS        ON)
SET(Boost_USE_MULTITHREADED      ON)
SET(Boost_USE_STATIC_RUNTIME    OFF)
FIND_PACKAGE(Boost)

IF (Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF ()

SET(lib_path "${project_path}/../../install/libs")

INCLUDE_DIRECTORIES(${project_path}/uibase/src
                    ${project_path}/game_features/src
                    ${project_path}/game_gamebryo/src)
LINK_DIRECTORIES(${project_path}/uibase/build/src
                 ${lib_path}
                 ${LZ4_ROOT}/dll)

ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

ADD_LIBRARY(${PROJ_NAME} SHARED ${${PROJ_NAME}_HDRS} ${${PROJ_NAME}_SRCS} ${${PROJ_NAME}_UIHDRS} ${${PROJ_NAME}_translations_qm})
TARGET_LINK_LIBRARIES(${PROJ_NAME}
                      Qt5::Widgets
                      ${Boost_LIBRARIES}
                      DbgHelp
                      uibase
                      game_gamebryo
                      liblz4
                      version)

IF (MSVC)
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES COMPILE_FLAGS "/std:c++latest")
ENDIF()
IF (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bits
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES LINK_FLAGS "/LARGEADDRESSAWARE")
ENDIF()

IF (NOT "${OPTIMIZE_COMPILE_FLAGS}" STREQUAL "")
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES COMPILE_FLAGS_RELWITHDEBINFO ${OPTIMIZE_COMPILE_FLAGS})
ENDIF()
IF (NOT "${OPTIMIZE_LINK_FLAGS}" STREQUAL "")
  SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO ${OPTIMIZE_LINK_FLAGS})
ENDIF()

###############
## Installation

INSTALL(TARGETS ${PROJ_NAME}
        RUNTIME DESTINATION bin/plugins)
INSTALL(FILES $<TARGET_PDB_FILE:${PROJ_NAME}>
        DESTINATION pdb)
