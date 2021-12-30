#include <lyrahgames/opengl/glfw_context.hpp>

namespace lyrahgames::opengl {

// These static variables need to be defined in a source file.
size_t glfw_context::object_count = 0;
mutex glfw_context::context_mutex{};

void glfw_context::init() {
  scoped_lock lock{context_mutex};
  if (object_count++) return;
  if (!glfwInit()) throw runtime_error("Failed to initialize GLFW.");
  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error("GLFW Error " + to_string(error) + ": " + description);
  });
}

void glfw_context::free() {
  scoped_lock lock{context_mutex};
  if (--object_count) return;
  glfwTerminate();
}

}  // namespace lyrahgames::opengl
