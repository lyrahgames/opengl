#include "application.hpp"

application::application() : base{500, 500, "My Application"} {}

void application::key_callback(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void application::scroll_callback(double x, double y) {}

// void application::process_events() {
//   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//     glfwSetWindowShouldClose(window, true);
// }

void application::init_shader() {
  // Compile and create the vertex shader.
  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr);
  glCompileShader(vertex_shader);
  {
    // Check for errors.
    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
      throw runtime_error(
          string("OpenGL Error: Failed to compile vertex shader!: ") +
          info_log);
    }
  }

  // Compile and create the fragment shader.
  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr);
  glCompileShader(fragment_shader);
  {
    // Check for errors.
    GLint success;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
      throw runtime_error(
          string("OpenGL Error: Failed to compile fragment shader!: ") +
          info_log);
    }
  }

  // Link vertex shader and fragment shader to shader program.
  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  {
    // Check for errors.
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      char info_log[512];
      glGetProgramInfoLog(program, 512, nullptr, info_log);
      throw runtime_error(
          string("OpenGL Error: Failed to link shader program!: ") + info_log);
    }
  }

  // Delete unused shaders.
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Get identifier locations in the shader program
  // to change their values from the outside.
  mvp_location = glGetUniformLocation(program, "MVP");
  vpos_location = glGetAttribLocation(program, "vPos");
  vcol_location = glGetAttribLocation(program, "vCol");
}

void application::init_vertex_data() {
  // Use a vertex array to be able to reference the vertex buffer and
  // the vertex attribute arrays of the triangle with one single variable.
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  // Generate and bind the buffer which shall contain the triangle data.
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  // The data is not changing rapidly. Therefore we use GL_STATIC_DRAW.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set the data layout of the position and colors
  // with vertex attribute pointers.
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)0);
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)(sizeof(float) * 2));
}

void application::setup() {
  init_shader();
  init_vertex_data();

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  resize(width, height);
}

void application::resize(int width, int height) {
  // Update size parameters and compute aspect ratio.
  const auto aspect_ratio = float(width) / height;
  // Make sure rendering takes place in the full screen.
  glViewport(0, 0, width, height);
  // Use a perspective projection with correct aspect ratio.
  projection = glm::perspective(45.0f, aspect_ratio, 0.1f, 100.f);
  // Position the camera in space by using a view matrix.
  view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2));
}

void application::update() {
  // Compute and set MVP matrix in shader.
  model = glm::mat4{1.0f};
  const auto axis = glm::normalize(glm::vec3(1, 1, 1));
  model = rotate(model, float(glfwGetTime()), axis);
  const auto mvp = projection * view * model;
  glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
}

void application::render() {
  // Clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Bind vertex array of triangle
  // and use the created shader
  // to render the triangle.
  glUseProgram(program);
  glBindVertexArray(vertex_array);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}