cmake_minimum_required(VERSION 3.22.1)
project(apocalypse_engine LANGUAGES CXX)

add_library(apocalypse_engine SHARED game_engine.cpp model_pipeline.cpp survival_system.cpp verity_3d.cpp)
target_compile_features(apocalypse_engine PRIVATE cxx_std_17)
target_compile_options(apocalypse_engine PRIVATE -Wall -Wextra)
find_library(android-log log)
find_library(gles-lib GLESv3)
target_link_libraries(apocalypse_engine PRIVATE ${gles-lib} ${android-log})