#!/bin/sh

mkdir -p tmp-setup

# download glad
wget -O tmp-setup/glad.zip https://bansheerubber.com/i/f/_8dqL.zip
unzip tmp-setup/glad.zip -d tmp-setup/glad

# configure glad
rm -Rf glad
mkdir glad
cp -v tmp-setup/glad/src/gl.c glad

rm -Rf include/glad
mkdir include/glad
cp -v tmp-setup/glad/include/glad/gl.h include/glad

rm -Rf include/KHR
mkdir include/KHR
cp -v tmp-setup/glad/include/KHR/khrplatform.h include/KHR

echo -e "\033[0;32mFinished GLAD"
tput sgr0

# configure glfw
pushd .
cd tmp-setup
git clone https://github.com/glfw/glfw
cd glfw
cmake -S . -B build
cd build
make
popd
rm lib/libglfw3.a
mkdir -p lib
cp -v tmp-setup/glfw/build/src/libglfw3.a lib
rm -Rf include/glfw
mv -v tmp-setup/glfw/include/GLFW include/glfw # make GLFW lowercase
echo -e "\033[0;32mFinished GLFW"
tput sgr0

# configure freetype
cd tmp-setup
git clone https://github.com/freetype/freetype
cd ..
rm -Rf include/freetype
rm include/ft2build.h
mv -v tmp-setup/freetype/include/* include/
echo -e "\033[0;32mFinished freetype"
tput sgr0

# configure torquescript
pushd .
cd tmp-setup
git clone --recurse-submodules https://github.com/bansheerubber/torquescript-interpreter
cd torquescript-interpreter
git checkout tssl-and-friends
echo -e "\033[0;33mCompiling torquescrcipt"
tput sgr0
make -j 8 library
popd
cp -v tmp-setup/torquescript-interpreter/dist/libtorquescript.so lib
echo -e "\033[0;32mFinished torquescript"
tput sgr0

# configure eggine-carton
cd tmp-setup
git clone https://github.com/bansheerubber/eggine-carton
cd ..
rm -Rf src/carton
mkdir -p src/carton
mv -v tmp-setup/eggine-carton/src/carton/* src/carton/
echo -e "\033[0;32mFinished eggine-carton"
tput sgr0

# delete tmp-setup
rm -Rf tmp-setup
