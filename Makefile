CXX		:= g++
OUTPUT  := sfmlgame

CXX_FLAGS   := -O3 -std=c++20 -Wno-unused-result
INCLUDES	:= -I./src -I./src/imgui -I./src/imgui-sfml-2.6 -I./src/imgui/misc/cpp
LDFLAGS	 	:= -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL

SRC_FILES   := $(wildcard src/*.cpp src/imgui/*.cpp src/imgui-sfml-2.6/*.cpp src/imgui/misc/cpp/*.cpp)
OBJ_FILES   := $(SRC_FILES:.cpp=.o)

all:$(OUTPUT)

$(OUTPUT):$(OBJ_FILES) Makefile
		$(CXX) $(OBJ_FILES) $(LDFLAGS) -o ./bin/$@

.cpp.o:
		$(CXX) -c $(CXX_FLAGS) $(INCLUDES) $< -o $@
		
clean:
		rm -f $(OBJ_FILES) ./bin/sfmlgame
		
run: $(OUTPUT)
		cd bin && ./sfmlgame && cd ..
