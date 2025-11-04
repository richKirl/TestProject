cmake_minimum_required(VERSION 3.16)

project(AnimationRefactorV2 LANGUAGES CXX)
set(CMAKE_C_COMPILER "/usr/lib/llvm/20/bin/clang")#set compiler
set(CMAKE_CXX_COMPILER "/usr/lib/llvm/20/bin/clang++")#set compiler
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AnimationRefactorV2 AnimationRefactorV2.cpp)
target_compile_options(AnimationRefactorV2 PRIVATE
    -msse4.2
    -mavx2
    -ffast-math
#    -Wall
#    -Wextra
)
# Линковка библиотек
target_link_libraries(AnimationRefactorV2
    PRIVATE
    GL
    GLU
    GLEW
    glfw
    m
    freetype #for this test
    assimp
)

# Обеспечить поиск библиотек
find_package(OpenGL REQUIRED)
target_link_libraries(AnimationRefactorV2 PRIVATE OpenGL::GL)
include(GNUInstallDirs)
install(TARGETS untitled2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
