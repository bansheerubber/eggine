#!/bin/bash

# cut down version of normal setup script

mkdir -p tmp-setup

# configure glfw
pushd .
cd tmp-setup
[[ "$PWD" != *tmp-setup* ]] && echo "Not in tmp-setup" && exit 1

git clone https://github.com/glfw/glfw
cd glfw
git checkout tags/3.3.7 -b 3.3-stable
[[ "$PWD" != *glfw* ]] && echo "Not in glfw" && exit 1

cmake -S . -B build
cd build
make
popd
rm lib/libglfw3.a
mkdir -p lib
cp -v tmp-setup/glfw/build/src/libglfw3.a lib
rm -Rf include/pc/GLFW
mv -v tmp-setup/glfw/include/GLFW include/pc/GLFW # make GLFW lowercase
echo -e "Finished GLFW"

# configure eggscript
pushd .
cd tmp-setup
[[ "$PWD" != *tmp-setup* ]] && echo "Not in tmp-setup" && exit 1

git clone --recurse-submodules https://github.com/bansheerubber/eggscript-interpreter

cd eggscript-interpreter
[[ "$PWD" != *eggscript-interpreter* ]] && echo "Not in eggscript-interpreter" && exit 1

git checkout development
echo -e "Compiling eggscript"
make CC=g++-9 -j 8 library
popd
cp -v tmp-setup/eggscript-interpreter/dist/libeggscript.a lib

if [ -f /opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc ]; then
	pushd .
	cd tmp-setup/eggscript-interpreter
	make clean
	make -f Makefile.nx -j 8 library
	popd
	cp -v tmp-setup/eggscript-interpreter/dist/libeggscript.a libnx
fi

rm -Rf include/common/eggscript
mkdir -p include/common/eggscript
cp -v tmp-setup/eggscript-interpreter/dist/include.cpp/egg.h include/common/eggscript/egg.h
echo -e "Finished eggscript"

# configure eggine-carton
cd tmp-setup
[[ "$PWD" != *tmp-setup* ]] && echo "Not in tmp-setup" && exit 1

git clone https://github.com/bansheerubber/eggine-carton
cd eggine-carton
[[ "$PWD" != *eggine-carton* ]] && echo "Not in eggine-carton" && exit 1

cd ../../
rm -Rf src/carton
mkdir -p src/carton
mv -v tmp-setup/eggine-carton/src/carton/* src/carton/
echo -e "Finished eggine-carton"

# configure litehtml
pushd .
cd tmp-setup
git clone https://github.com/bansheerubber/litehtml
cd litehtml
[[ "$PWD" != *litehtml* ]] && echo "Not in litehtml" && exit 1

mkdir build
cd build
CFLAGS="-O2" cmake -DLITEHTML_UTF8=ON ..
cmake --build . --target litehtml
popd
cp -v tmp-setup/litehtml/build/liblitehtml.a lib

if [ -f /opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc ]; then
	pushd .
	cd tmp-setup/litehtml
	rm -Rf build
	mkdir build
	cd build
	CC=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc CXX=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++ CFLAGS="-O2 -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC" CXXFLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC" LDFLAGS="-fPIE" cmake ..
	cmake --build . --target litehtml
	popd
	cp -v tmp-setup/litehtml/build/liblitehtml.a libnx
fi

rm -Rf include/common/litehtml
mv -v tmp-setup/litehtml/include/* include/common/
echo -e "Finished litehtml"

# configure gumbo parser
pushd .
cd tmp-setup
git clone https://github.com/google/gumbo-parser
cd gumbo-parser
[[ "$PWD" != *gumbo-parser* ]] && echo "Not in gumbo-parser" && exit 1

./autogen.sh
./configure
make
cp -v .libs/libgumbo.a ../../lib

if [ -f /opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc ]; then
	./configure CC=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc LD=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ld AR=/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc-ar CFLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -Wall -fPIC" LDFLAGS="-fPIE"
	make clean
	make
	cp -v .libs/libgumbo.a ../../libnx
fi

popd
echo -e "Finished gumbo"

# delete tmp-setup
rm -Rf tmp-setup