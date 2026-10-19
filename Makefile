CC = clang
CXX = clang++

INCLUDES = \
	-I$(VULKAN_SDK)/include \
	-I/usr/local/include \
	-I/opt/homebrew/include \
	-isystem external

COMMON = -Wall -g -fsanitize=address \
	-Wno-nullability-completeness \
	-MMD -MP

CFLAGS = -std=c23 -fenable-matrix $(COMMON) $(INCLUDES)

# vma.cpp is the only C++: VMA's implementation is written in C++
CXXFLAGS = -std=c++17 -fno-exceptions -fno-rtti $(COMMON) $(INCLUDES)

LDFLAGS = \
	-L$(VULKAN_SDK)/lib -lvulkan \
	-Wl,-rpath,$(VULKAN_SDK)/lib \
	-L/opt/homebrew/lib -lglfw \
	-framework Cocoa \
	-framework IOKit \
	-framework CoreVideo

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o) build/vma.o

main: build/main

-include $(OBJ:.o=.d)

# linked with clang++ so the C++ runtime VMA needs comes along
build/main: $(OBJ)
	$(CXX) $(COMMON) $(OBJ) $(LDFLAGS) -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/vma.o: src/vma.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: build/main
	MallocNanoZone=0 ./build/main

clean:
	rm -f build/*.o build/*.d build/main



