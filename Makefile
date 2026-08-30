CC = clang++

CFLAGS = -std=c++20 -Wall \
	-I$(VULKAN_SDK)/include \
	-I/usr/local/include \
	-I/opt/homebrew/include \
	-fsanitize=address -g

LDFLAGS = \
	-L$(VULKAN_SDK)/lib -lvulkan \
	-Wl,-rpath,$(VULKAN_SDK)/lib \
	-L/opt/homebrew/lib -lglfw \
	-framework Cocoa \
	-framework IOKit \
	-framework CoreVideo

CPP = $(wildcard src/*.cpp)
OBJ = $(CPP:src/%.cpp=build/%.o)

main: build/main

build/main: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LDFLAGS) -o $@

build/%.o: src/%.cpp
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

run: build/main
	MallocNanoZone=0 ./build/main

clean:
	rm -f build/*.o build/main
