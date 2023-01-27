CC = g++
SC = glslc

INCLUDE = ./inc
SOURCE = ./src
BUILD = ./build
BINARY = .

CPP = main.o
GLSL = shader.vert.spv shader.frag.spv

all: program shaders

program: $(CPP)
	$(CC) $(addprefix $(BUILD)/, $^) -o $(BINARY)/run -lvulkan -lglfw

shaders: $(GLSL)

$(CPP):
	$(CC) -g -c $(SOURCE)/$(basename $@).cpp -o $(BUILD)/$@ -I $(INCLUDE) -I $(SOURCE)

$(GLSL):
	$(SC) -c $(SOURCE)/$(basename $@) -o $(BUILD)/$@ -I $(INCLUDE) -I $(SOURCE)

