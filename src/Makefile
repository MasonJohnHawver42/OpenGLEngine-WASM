CXX = emcc
OUTPUT = index.html
IMGUI_DIR:= ./include/imgui
GLM_DIR:= ./include
STB_DIR:= ./include/stb
TOL_DIR := ./include/tinyobjloader

SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui_tables.cpp


LIBS = -lGL
WEBGL_VER = -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s -s USE_SDL=2 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1
#WEBGL_VER = USE_GLFW=2
USE_WASM = -s WASM=1

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CXX)  $(SOURCES) -std=c++11 -o $(OUTPUT) $(LIBS) $(WEBGL_VER) -O3 $(USE_WASM) --preload-file data -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(GLM_DIR) -I$(STB_DIR) -I$(TOL_DIR) -s LLD_REPORT_UNDEFINED -s ASSERTIONS=1 --shell-file shell_minimal.html

clean:
	rm -f $(OUTPUT)
