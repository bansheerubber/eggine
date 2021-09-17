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

rm -Rf tmp-setup
