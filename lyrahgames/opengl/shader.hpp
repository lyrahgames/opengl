#pragma once
#include <lyrahgames/opengl/utility.hpp>

namespace lyrahgames::opengl {

template <auto shader_type>
class shader_object {
 public:
  struct compile_error : runtime_error {
    using base = runtime_error;
    compile_error(auto&& x) : base(forward<decltype(x)>(x)) {}
  };

  shader_object() = default;

  shader_object(czstring source) {
    handle = glCreateShader(shader_type);
    glShaderSource(handle, 1, &source, nullptr);
    glCompileShader(handle);

    // Check for errors.
    GLint success;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetShaderInfoLog(handle, 512, nullptr, info_log);
      throw compile_error(string("Failed to compile shader object. ") +
                          info_log);
    }
  }

  ~shader_object() {
    // Zero values are ignored by this function.
    glDeleteShader(handle);
  }

  // Copying is not allowed.
  shader_object(const shader_object&) = delete;
  shader_object& operator=(const shader_object&) = delete;

  // Moving
  shader_object(shader_object&& x) : handle{x.handle} { x.handle = 0; }
  shader_object& operator=(shader_object&& x) {
    swap(handle, x.handle);
    return *this;
  }

  operator GLuint() const { return handle; }

 private:
  GLuint handle{};
};

using vertex_shader = shader_object<GL_VERTEX_SHADER>;
using fragment_shader = shader_object<GL_FRAGMENT_SHADER>;

class shader_program {
 public:
  struct link_error : runtime_error {
    using base = runtime_error;
    link_error(auto&& x) : base(forward<decltype(x)>(x)) {}
  };

  shader_program() = default;

  shader_program(const vertex_shader& vs, const fragment_shader& fs) {
    handle = glCreateProgram();
    glAttachShader(handle, vs);
    glAttachShader(handle, fs);
    glLinkProgram(handle);

    // Check for errors.
    GLint success;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetProgramInfoLog(handle, 512, nullptr, info_log);
      throw link_error(string("Failed to link shader program. ") + info_log);
    }
  }

  ~shader_program() {
    // Zero values are ignored by this function.
    glDeleteProgram(handle);
  }

  // Copying is not allowed.
  shader_program(const shader_program&) = delete;
  shader_program& operator=(const shader_program&) = delete;

  // Moving
  shader_program(shader_program&& x) : handle{x.handle} { x.handle = 0; }
  shader_program& operator=(shader_program&& x) {
    swap(handle, x.handle);
    return *this;
  }

  operator GLuint() const { return handle; }

 private:
  GLuint handle{};
};

}  // namespace lyrahgames::opengl
