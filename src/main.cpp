#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#include <GLES3/gl2ext.h>
#endif

#include <SDL2/SDL.h>

#include "stb_image.h"

#include "tiny_obj_loader.h"
#define TINYOBJLOADER_IMPLEMENTATION

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "shader.hpp"
#include "model.hpp"

#if __EMSCRIPTEN__
EM_JS(int, canvas_get_width, (), {
  return Module.canvas.width;
});

EM_JS(int, canvas_get_height, (), {
  return Module.canvas.height;
});

EM_JS(void, resizeCanvas, (), {
  js_resizeCanvas();
});
#endif

void loop();
static void ModelsGUI();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

SDL_Window * window;

struct Transform {
  float pos[3];
  float rot[3];
  float scale[3];

  Transform() { rot[0] = 0.0f; rot[1] = 0.0f; rot[2] = 0.0f; scale[0] = 1.0f; scale[1] = 1.0f; scale[2] = 1.0f; }
};

struct Object {
  Model model;
  Transform transform;

  char path[128];

  Object() : model(), transform(), path("None") {}
  Object(const char * p, const char * f, const char * name) : model(p, f), transform(), path("None") { strcpy(path, name); }
};


struct Data {
  std::vector<Object> objects;
  int curr;

  //Mouse Stuff
  int x, y, dx, dy;
  float dz;
  bool down;

  Data() { objects = std::vector<Object>(0); curr = 0;  }
};

Shader shader;
Data data;


glm::vec4 clear_color;
glm::vec4 model_color;

int main()
{

  //SDL INIT
  SDL_Init(SDL_INIT_EVERYTHING);

  #if __EMSCRIPTEN__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  #else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  #endif

  //WINDOW CREATION

  window = SDL_CreateWindow("UW", 0, 0, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  SDL_SetWindowResizable(window, SDL_TRUE);

  //HTML CANVAS setup

  emscripten_set_canvas_element_size("canvas", SCR_WIDTH, SCR_HEIGHT);

  EmscriptenWebGLContextAttributes atrs;
  emscripten_webgl_init_context_attributes(&atrs);
  atrs.alpha = true;
  atrs.depth = true;
  atrs.stencil = false;
  atrs.majorVersion = 2;
  atrs.minorVersion = 0;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emctx = emscripten_webgl_create_context("canvas", &atrs);
  emscripten_webgl_make_context_current(emctx);
  std::cout << "GL_VERSION=" << glGetString(GL_VERSION) << std::endl;

  glEnable(GL_DEPTH_TEST);

  // Init Data

  // Load Shader

  shader = Shader("data/shaders/shader.vs", "data/shaders/shader.fs");

  // Load Models

  data.objects.push_back(Object("data/models/UW3.obj", "data/models", "UW"));
  data.objects.push_back(Object("data/models/teapot.obj", "data/models", "teapot"));
  data.objects.push_back(Object("data/models/monkey.obj", "data/models", "suzanne"));
  data.objects.push_back(Object("data/models/bunny.obj", "data/models", "bunny"));
  data.objects.push_back(Object("data/models/dragon.obj", "data/models", "dragon"));

  // Set up Camera

  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 1000.0f);
  shader.setMat4("projection", projection);

  clear_color = glm::vec4(0.0f / 256.0f, 0.0f / 256.0f, 0.0f / 256.0f, 1.0f);
  model_color = glm::vec4(153.0f / 256.0f, 0.0f / 256.0f, 51.0f / 256.0f, 1.0f);

  //SET UP IMGUI

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL2_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init();

  //SET UP LOOP

  #if __EMSCRIPTEN__
  emscripten_set_main_loop(loop, -1, 1);
  #else
  SDL_Event windowEvent;
  while (true)
  {
    if (SDL_PollEvent(&windowEvent))
    {
      if (windowEvent.type == SDL_QUIT) break;
    }
    loop();
  }
  #endif

  // CLean Up

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

void loop()
{

  // Input Updates (cheifly for imgui)
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
      // Forward to Imgui
      ImGui_ImplSDL2_ProcessEvent(&event);

      //Quit
      if (event.type == SDL_QUIT
          || (event.type == SDL_WINDOWEVENT
          && event.window.event == SDL_WINDOWEVENT_CLOSE
          && event.window.windowID == SDL_GetWindowID(window))) {
          //Todo: find quit function
      }

      //Resize
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        //on_size_changed();
        glViewport(0, 0, w, h);
        ImGui::SetCurrentContext(ImGui::GetCurrentContext());
      }

      if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) { data.down = true; }
      if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) { data.down = false; }

      if(event.type == SDL_MOUSEWHEEL)
      {
        data.dz = (float)event.wheel.y * 0.1f;
        //data.objects[data.curr].transform.scale[0] = fmin(fmax(data.objects[data.curr].transform.scale[0], -10.0f), 10.0f);
      }
  }

  int x, y;
  SDL_GetMouseState( &x, &y );

  data.dx = x - data.x;
  data.dy = y - data.y;

  data.x = x;
  data.y = y;

  // if (!ImGui::IsWindowFocused()) {
  //   data.objects[data.curr].transform.rot[1] += (float)dx * 0.1f;
  // }

  //data.objects[data.curr].transform.rot[1] += (float)dx * 0.1f;

  // Draw Call
  glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader.use();

  glm::mat4 projection = glm::mat4(1.0f);
  glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
  glm::mat4 view = glm::mat4(1.0f);

  projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 100000.0f);
  model = glm::rotate(model, data.objects[data.curr].transform.rot[0], glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, data.objects[data.curr].transform.rot[1], glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, data.objects[data.curr].transform.rot[2], glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(data.objects[data.curr].transform.scale[0]));
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));
  // view = glm::rotate(view, rotx, glm::vec3(0.0f, 1.0f, 0.0f));
  // view = glm::rotate(view, roty, glm::vec3(1.0f, 0.0f, 0.0f));

  shader.setMat4("projection", projection);
  shader.setMat4("model", model);
  shader.setMat4("view", view);

  shader.setVec3("ambientLight", clear_color);
  shader.setVec3("objectColor", model_color);

  //data.model.Draw(data.shader);

  data.objects[data.curr].model.Draw(shader);

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();
  //ImGui::ShowDemoWindow();

  ModelsGUI();

  if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) {
      if (data.down) {
        data.objects[data.curr].transform.rot[1] += (float)data.dx / (float)SCR_WIDTH * M_PI * 4.0f;
      }
      data.objects[data.curr].transform.scale[0] += data.dz;
      data.objects[data.curr].transform.scale[0] = fmax(data.objects[data.curr].transform.scale[0], 0.0f);
    }
  }

  data.dz = 0.0f;

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
}

static void ModelsGUI()
{
    ImGui::SetNextWindowSize(ImVec2(375, 80));
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("Render");
    ImGui::ColorEdit4("Background Color", (float*)&clear_color);
    ImGui::ColorEdit4("Model Color", (float*)&model_color);
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(500, 175));
    ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 500 - 10, SCR_HEIGHT - 175 - 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Models", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        // Left
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150, 0), true);
            for (int i = 0; i < data.objects.size(); i++) {
              if (ImGui::Selectable(data.objects[i].path, data.curr == i))
              {
                data.curr = i;
              }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("Object : %s", data.objects[data.curr].path);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Properties"))
                {
                    ImGui::Text("Rotate");
                    ImGui::SliderFloat("X", &data.objects[data.curr].transform.rot[0], -2 * M_PI, 2 * M_PI);
                    ImGui::SliderFloat("Y", &data.objects[data.curr].transform.rot[1], -2 * M_PI, 2 * M_PI);
                    ImGui::SliderFloat("Z", &data.objects[data.curr].transform.rot[2], -2 * M_PI, 2 * M_PI);
                    ImGui::Text("Scale");
                    ImGui::SliderFloat("Scl", &data.objects[data.curr].transform.scale[0], -10.0f, 10.0f);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Details"))
                {
                    for (int i = 0; i < data.objects[data.curr].model.meshes.size(); i++) {
                      ImGui::Text("Mesh %d: %lu Vertexes, %lu Triangles", i, data.objects[data.curr].model.meshes[i]->vertices->size(), data.objects[data.curr].model.meshes[i]->indices->size() / 3);
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if (ImGui::Button("Reset")) { data.objects[data.curr].transform.rot[0] = 0; data.objects[data.curr].transform.rot[1] = 0; data.objects[data.curr].transform.rot[2] = 0; data.objects[data.curr].transform.scale[0] = 1.0; }
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}
