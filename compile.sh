# clang++-20 \
#  -std=c++26 -O3 -msse4.2 -mavx2 \
#  main.cpp Window.cpp CubeV.cpp Object3D.cpp Shader.cpp Font.cpp AABB.cpp BVH.cpp Collision.cpp Camera.cpp Terrain.cpp Route.cpp Ray.cpp Light.cpp Utils.cpp Shadow.cpp SHDR.cpp Descr.cpp PickObject.cpp Logger.cpp Animations.cpp Instances.cpp Orientation.cpp \
#  -o \
#  bin/testcolBB \
#  -DSHM -DGLM_ENABLE_EXPERIMENTAL -I/usr/include/freetype2/ -lGL -lGLU -lGLEW -lglfw -lm -lfreetype -lassimp
DIRECTORY="build"
if [ -d "$DIRECTORY" ]; then
   echo "Directory '$DIRECTORY' exists."
   rm -rf build;
   mkdir build;
else
   echo "Directory '$DIRECTORY' does not exist."
   mkdir build;
fi

cd build;cmake ..;make;cd ..;cd bin;ls
