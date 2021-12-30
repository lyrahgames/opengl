#pragma once
#include <iostream>
#include <mutex>
#include <stdexcept>
//
// GLFW without OpenGL Headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
#include <lyrahgames/opengl/utility.hpp>

namespace lyrahgames::opengl {

struct glfw_context {
 public:
  glfw_context() { init(); }
  ~glfw_context() { free(); }

  // Copying is not allowed.
  glfw_context(const glfw_context&) = delete;
  glfw_context& operator=(const glfw_context&) = delete;

  // For now, moving is not allowed.
  glfw_context(glfw_context&&) = delete;
  glfw_context& operator=(glfw_context&&) = delete;

 public:
  static size_t object_count;
  static mutex context_mutex;

  void init();
  void free();
};

}  // namespace lyrahgames::opengl
