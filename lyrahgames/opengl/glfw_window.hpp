#pragma once
#include <iostream>
#include <mutex>
#include <thread>
//
// GLFW without OpenGL Headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
// glbinding as OpenGL loader
#include <glbinding/glbinding.h>
//
#include <lyrahgames/opengl/glfw_context.hpp>
#include <lyrahgames/opengl/utility.hpp>

namespace lyrahgames::opengl {

class glfw_window {
  friend struct scoped_current_context_lock;

 public:
  explicit glfw_window(glfw_context&, int width = 800, int height = 450,
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
  mutex context_mutex{};
};

struct scoped_current_context_lock : scoped_lock<mutex> {
  using base = scoped_lock<mutex>;
  using mutex_type = typename base::mutex_type;

  scoped_current_context_lock(glfw_window& w) : base{w.context_mutex} {
    glfwMakeContextCurrent(w);
  }

  ~scoped_current_context_lock() {  //
    glfwMakeContextCurrent(nullptr);
  }

  scoped_current_context_lock(const scoped_current_context_lock&) = delete;
  scoped_current_context_lock& operator=(const scoped_current_context_lock&) =
      delete;
};

glfw_window::glfw_window(glfw_context& context, int width, int height,
                         czstring title) {
  {
    scoped_lock lock{context.context_mutex};

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  }
  // scoped_lock lock{context_mutex};
  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);
  glfwMakeContextCurrent(nullptr);
}

glfw_window::~glfw_window() {
  if (window) glfwDestroyWindow(window);
}

}  // namespace lyrahgames::opengl
