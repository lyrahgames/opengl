#pragma once
//
// GLFW without OpenGL Headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <lyrahgames/opengl/glfw_context.hpp>
#include <lyrahgames/opengl/utility.hpp>

namespace lyrahgames::opengl {

class glfw_window {
 public:
  explicit glfw_window(glfw_context&,
                       int width = 800,
                       int height = 450,
                       czstring title = "GLFW Window");
  ~glfw_window();

  operator GLFWwindow*() { return window; }

  // Copying is not allowed.
  glfw_window(const glfw_window&) = delete;
  glfw_window& operator=(const glfw_window&) = delete;

  // Moving
  glfw_window(glfw_window&& x) : window{x.window} { x.window = nullptr; }
  glfw_window& operator=(glfw_window&& x) {
    swap(window, x.window);
    return *this;
  }

 private:
  GLFWwindow* window = nullptr;
};

}  // namespace lyrahgames::opengl
