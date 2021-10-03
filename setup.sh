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
[[ "$PWD" != *tmp-setup* ]] && echo "\033[0;31mNot in tmp-setup" && exit 1

git clone https://github.com/glfw/glfw
cd glfw
[[ "$PWD" != *glfw* ]] && echo "\033[0;31mNot in glfw" && exit 1

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
[[ "$PWD" != *tmp-setup* ]] && echo "\033[0;31mNot in tmp-setup" && exit 1

git clone https://github.com/freetype/freetype
cd ..
rm -Rf include/freetype
rm include/ft2build.h
mv -v tmp-setup/freetype/include/* include/
echo -e "\033[0;32mFinished freetype"
tput sgr0

# configure eggscript
pushd .
cd tmp-setup
[[ "$PWD" != *tmp-setup* ]] && echo "\033[0;31mNot in tmp-setup" && exit 1

git clone --recurse-submodules https://github.com/bansheerubber/eggscript-interpreter

cd eggscript-interpreter
[[ "$PWD" != *eggscript-interpreter* ]] && echo "\033[0;31mNot in eggscript-interpreter" && exit 1

git checkout arrays-as-objects
echo -e "\033[0;33mCompiling eggscript"
tput sgr0
make -j 8 library
popd
cp -v tmp-setup/eggscript-interpreter/dist/libeggscript.a lib
rm -Rf include/eggscript
mkdir -p include/eggscript
cp -v tmp-setup/eggscript-interpreter/dist/include.cpp/egg.h include/eggscript/egg.h
echo -e "\033[0;32mFinished eggscript"
tput sgr0

# configure eggine-carton
cd tmp-setup
[[ "$PWD" != *tmp-setup* ]] && echo "\033[0;31mNot in tmp-setup" && exit 1

git clone https://github.com/bansheerubber/eggine-carton
cd ..
rm -Rf src/carton
mkdir -p src/carton
mv -v tmp-setup/eggine-carton/src/carton/* src/carton/
echo -e "\033[0;32mFinished eggine-carton"
tput sgr0

# delete tmp-setup
rm -Rf tmp-setup
