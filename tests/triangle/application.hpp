#pragma once
#include <lyrahgames/opengl/opengl.hpp>

using namespace lyrahgames::opengl;

class application : public glfw_application<application> {
  using base = glfw_application<application>;

 public:
  application();

  void key_callback(int key, int scancode, int action, int mods);
  void scroll_callback(double x, double y);
  // void process_events();
  void init_shader();
  void init_vertex_data();
  void setup();
  void resize(int width, int height);
  void update();
  void render();

 private:
  struct {
    float x, y;     // 2D Position
    float r, g, b;  // Color
  } vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                   {0.6f, -0.4f, 0.f, 1.f, 0.f},
                   {0.f, 0.6f, 0.f, 0.f, 1.f}};

  czstring vertex_shader_text =
      "#version 330 core\n"
      "uniform mat4 MVP;"
      "attribute vec3 vCol;"
      "attribute vec2 vPos;"
      "varying vec3 color;"
      "void main(){"
      "  gl_Position = MVP * vec4(vPos, 0.0, 1.0);"
      "  color = vCol;"
      "}";

  czstring fragment_shader_text =
      "#version 330 core\n"
      "varying vec3 color;"
      "void main(){"
      "  gl_FragColor = vec4(color, 1.0);"
      "}";

  // Vertex Data Handles
  GLuint vertex_array;
  GLuint vertex_buffer;

  // Shader Handles
  // GLuint program;
  shader_program program;
  GLint mvp_location, vpos_location, vcol_location;

  // Transformation Matrices
  glm::mat4 model, view, projection;
};
