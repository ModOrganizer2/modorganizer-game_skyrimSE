cmake_minimum_required(VERSION 3.16)

set(additional_translations ${modorganizer_super_path}/game_gamebryo/src)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/src.cmake)
else()
	include(../../cmake_common/src.cmake)
endif()
requires_project(game_gamebryo game_features)
