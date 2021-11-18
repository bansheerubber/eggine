#!/bin/sh

mkdir -p tmp-setup

# download glad
wget -O tmp-setup/glad.zip https://bansheerubber.com/i/f/_8dqL.zip
unzip tmp-setup/glad.zip -d tmp-setup/glad

# configure glad
rm -Rf glad
mkdir glad
cp -v tmp-setup/glad/src/gl.c glad

rm -Rf include/pc/glad
mkdir include/pc/glad
cp -v tmp-setup/glad/include/glad/gl.h include/pc/glad

rm -Rf include/pc/KHR
mkdir include/pc/KHR
cp -v tmp-setup/glad/include/KHR/khrplatform.h include/pc/KHR

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
rm -Rf include/pc/GLFW
mv -v tmp-setup/glfw/include/GLFW include/pc/GLFW # make GLFW lowercase
echo -e "\033[0;32mFinished GLFW"
tput sgr0

# configure freetype
cd tmp-setup
[[ "$PWD" != *tmp-setup* ]] && echo "\033[0;31mNot in tmp-setup" && exit 1

git clone https://github.com/freetype/freetype
cd ..
rm -Rf include/pc/freetype
rm include/pc/ft2build.h
mv -v tmp-setup/freetype/include/* include/common/
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
rm -Rf include/common/eggscript
mkdir -p include/common/eggscript
cp -v tmp-setup/eggscript-interpreter/dist/include.cpp/egg.h include/common/eggscript/egg.h
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

# configure imgui
mkdir -p imgui
pushd .
cd tmp-setup
git clone https://github.com/ocornut/imgui
cd imgui
cp -v imgui.cpp ../../imgui/imgui.cc
cp -v imgui.h ../../imgui/imgui.h
cp -v imgui_draw.cpp ../../imgui/imgui_draw.cc
cp -v imgui_tables.cpp ../../imgui/imgui_tables.cc
cp -v imgui_widgets.cpp ../../imgui/imgui_widgets.cc
cp -v imgui_internal.h ../../imgui/imgui_internal.h
cp -v imstb_rectpack.h ../../imgui/imstb_rectpack.h
cp -v imstb_textedit.h ../../imgui/imstb_textedit.h
cp -v imstb_truetype.h ../../imgui/imstb_truetype.h
cp -v imconfig.h ../../imgui/imconfig.h
cp -v backends/imgui_impl_glfw.cpp ../../imgui/imgui_impl_glfw.cc
cp -v backends/imgui_impl_glfw.h ../../imgui/imgui_impl_glfw.h
cp -v backends/imgui_impl_opengl3.cpp ../../imgui/imgui_impl_opengl3.cc
cp -v backends/imgui_impl_opengl3.h ../../imgui/imgui_impl_opengl3.h
cp -v backends/imgui_impl_opengl3_loader.h ../../imgui/imgui_impl_opengl3_loader.h
popd
echo -e "\033[0;32mFinished imgui"
tput sgr0

# configure litehtml
pushd .
cd tmp-setup
git clone https://github.com/bansheerubber/litehtml
cd litehtml
mkdir build
cd build
CFLAGS="-O2" cmake ..
cmake --build . --target litehtml
popd
cp -v tmp-setup/litehtml/build/liblitehtml.a lib
mv -v tmp-setup/litehtml/include/* include/common/
pushd .
cd tmp-setup/litehtml
rm -Rf build
mkdir build
cd build
CC=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc CXX=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++ CFLAGS="-O2 -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC" CXXFLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC" LDFLAGS="-fPIE" cmake ..
cmake --build . --target litehtml
popd
cp -v tmp-setup/litehtml/build/liblitehtml.a libnx
echo -e "\033[0;32mFinished litehtml"
tput sgr0

# configure gumbo parser
pushd .
cd tmp-setup
git clone https://github.com/google/gumbo-parser
cd gumbo-parser
./autogen.sh
./configure
make
cp -v .libs/libgumbo.a ../../lib
./configure CC=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc LD=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ld AR=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc-ar CFLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -Wall -fPIC" LDFLAGS="-fPIE"
make clean
make
cp -v .libs/libgumbo.a ../../libnx
popd
echo -e "\033[0;32mFinished gumbo"
tput sgr0

# delete tmp-setup
rm -Rf tmp-setup
