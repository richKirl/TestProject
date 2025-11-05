cmake_minimum_required(VERSION 3.28)

project(untitled2 LANGUAGES CXX)

#
message(STATUS "Start checking versions")
#

# Проверка версии assimp
find_package(assimp REQUIRED)

if (assimp_FOUND)
    message(STATUS "ASSIMP version: ${assimp_VERSION}")
    # Or, if ASSIMP_VERSION is not directly available, you might find other version-related variables
    # message(STATUS "Assimp Library: ${assimp_LIBRARIES}")
    # message(STATUS "Assimp Include Dirs: ${assimp_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "ASSIMP not found!")
endif()

# Проверка версии GLM
find_package(glm REQUIRED)
if("${glm_VERSION}" VERSION_LESS "1000008.0.2")
  message(STATUS "GLM version: ${glm_VERSION}")
  add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
else()
  message(WARNING "GLM version: ${GLM_VERSION}")
endif()

# Проверка версии freetype2
find_package(Freetype REQUIRED)
if("${FREETYPE_VERSION_STRING}" VERSION_LESS "1000008.0.2")
  message(STATUS "FREETYPE version: ${FREETYPE_VERSION_STRING}")
else()
  message(WARNING "FREETYPE version: ${FREETYPE_VERSION_STRING}")
endif()

# Проверка версии glew
find_package(GLEW REQUIRED)
if("${GLEW_VERSION}" VERSION_LESS "1000008.0.2")
  message(STATUS "GLEW version: ${GLEW_VERSION}")
else()
  message(WARNING "GLEW version: ${GLEW_VERSION}")
endif()

# Проверка версии glfw3
find_package(glfw3 REQUIRED)
if("${glfw3_VERSION}" VERSION_LESS "1000008.0.2")
  message(STATUS "GLFW3 version: ${glfw3_VERSION}")
else()
  message(WARNING "GLFW3 version: ${glfw3_VERSION}")
endif()

#
message(STATUS "End checking versions")
#

set(CMAKE_C_COMPILER "/usr/lib/llvm/20/bin/clang")#set compiler
set(CMAKE_CXX_COMPILER "/usr/lib/llvm/20/bin/clang++")#set compiler
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Указание include-пути
include_directories(/usr/include/freetype2)
# Добавление определения макроса
add_definitions(-DSHM -DSTB_IMAGE_IMPLEMENTATION)
add_executable(untitled2
    main.cpp
    Logger.h Logger.cpp
    Animations.h Animations.cpp)
target_compile_options(untitled2 PRIVATE
    -O3
    -msse4.2
    -mavx2
    -ffast-math
#    -Wall
#    -Wextra
)
# Линковка библиотек
target_link_libraries(untitled2
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
target_link_libraries(untitled2 PRIVATE OpenGL::GL)
include(GNUInstallDirs)
install(TARGETS untitled2
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
