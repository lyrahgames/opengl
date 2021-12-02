#include "glfw_window.hpp"
//
// glbinding as OpenGL loader
#include <glbinding/glbinding.h>

namespace lyrahgames::opengl {

glfw_window::glfw_window(glfw_context&, int width, int height, czstring title) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);
}

glfw_window::~glfw_window() {
  if (window) glfwDestroyWindow(window);
}

}  // namespace lyrahgames::opengl
