#include "glfw_context.hpp"
//
#include <iostream>
#include <mutex>
#include <stdexcept>
//
// GLFW without OpenGL Headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace lyrahgames::opengl {

// Use static linkage for thread-sync variables.
namespace {
size_t object_count = 0;
mutex mutex{};
}  // namespace

void glfw_context::init() {
  scoped_lock lock{mutex};
  if (object_count++) return;
  if (!glfwInit()) throw runtime_error("Failed to initialize GLFW.");
  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error("GLFW Error " + to_string(error) + ": " + description);
  });
}

void glfw_context::free() {
  scoped_lock lock{mutex};
  if (--object_count) return;
  glfwTerminate();
}

}  // namespace lyrahgames::opengl
