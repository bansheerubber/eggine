## Compilation

### Linux
For first-time setup, please consult `setup.sh`.

Libraries needed:
```
freetype
glfw
openal
libogg
libvorbis
libpng
zlib
```

To compile,
```
$ make
```

### Windows
Compilation is done through MSYS2 MinGW x64. Look up a tutorial on how to install MSYS2 with MinGW. MinGW is required to produce a build without redistributable DLLs. Packages required for compiling dependencies:
```
unzip
wget
autoconf
automake
libtool
cmake
python
```

Libraries required for eggine:
```
freetype
glew
glfw
openal
libogg
libvorbis
libpng
zlib
```

For first time setup, please consult `setup.sh`.

To compile,
```
$ make -f Makefile.win
```