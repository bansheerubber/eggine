target = game
cclibs = -lpthread -llitehtml -lgumbo -lGL -lglfw3 -ldl -lpng -lfreetype -leggscript -lfmt -lz -lopenal -lvorbis -lvorbisfile -lssl -lcrypto -lvulkan -lX11 -lXxf86vm -lXrandr -lXi
ccinclude = -Iinclude/pc/glm/ -Iinclude/common/ -Iinclude/common/robin-map/include -Iinclude/common/fmt/include/ -Iinclude/pc/ -Iimgui -Llib
CC = g++
CPPFLAGS = -O2 -Wall -Wno-switch -Wno-class-memaccess -Wno-delete-incomplete -Wno-attributes -Bsymbolic -fPIC -fno-semantic-interposition --static -std=c++17 -DEGGINE_DEVELOPER_MODE -DGLM_EXT_INCLUDED
soflags =
ldflags =

cpp_source = $(shell find imgui -type f -name "*.cc") $(shell find src -type f -name "*.cc" ! -path "src/include*")
cpp_source_tmp = $(subst src, tmp, $(cpp_source))
cpp_source_without = $(subst src\/, , $(cpp_source))

cpp_headers = $(shell find imgui -type f -name "*.h") $(shell find src -type f -name "*.h" ! -path "src/include*") 
cpp_headers_tmp = $(subst src, tmp, $(cpp_headers))

cpp_objects = $(patsubst %.cc, %.o, $(cpp_source))
cpp_objects_tmp = $(patsubst %.cc, %.o, $(cpp_source_tmp))
cpp_objects_without = $(patsubst src\/, , $(cpp_source))

.PHONY: default clean

# force synchronization for preprocessor
default:
	@"$(MAKE)" preprocessor --no-print-directory
	@"$(MAKE)" dist/$(target) --no-print-directory

preprocessor:
	@echo -e "   PY      tools/preprocessor.py"
	@python3.10 tools/preprocessor.py

glad/gl.o: glad/gl.c
	@echo -e "   CC      $@"
	@$(CC) $(CPPFLAGS) $(soflags) $(ccinclude) -c $< -o $@

$(cpp_objects_tmp) : %.o : %.cc
	@mkdir -p $(dir $@)
	@echo -e "   CC      $<"
	@$(CC) $(CPPFLAGS) $(soflags) $(ccinclude) -c $< -o $@

dist/$(target): $(cpp_objects_tmp) glad/gl.o
	@mkdir -p $(dir dist/$(target))
	@echo -e "   CC      $@"
	@$(CC) $(cpp_objects_tmp) glad/gl.o -Wall $(ccinclude) $(cclibs) -o $@

clean:
	@echo -e "   RM      tmp"
	@rm -Rf tmp

	@echo -e "   RM      dist/$(target)"
	@rm -f dist/$(target)