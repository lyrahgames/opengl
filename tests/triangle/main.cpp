#include <future>
//
#include <lyrahgames/opengl/opengl.hpp>

using namespace std;
using namespace lyrahgames::opengl;

class application : public viewer<application> {
  using base = viewer<application>;

  static constexpr struct {
    float x, y;     // 2D Position
    float r, g, b;  // Color
  } vertices[3] = {
      {-0.6f, -0.4f, /**/ 1.f, 0.0f, 0.0f},
      {0.6f, -0.4f, /**/ 0.0f, 1.f, 0.0f},
      {0.0f, 0.6f, /**/ 0.0f, 0.0f, 1.f},
  };

  static constexpr czstring vertex_shader_text =
      "#version 330 core\n"
      "uniform mat4 MVP;"
      "attribute vec3 vCol;"
      "attribute vec2 vPos;"
      "varying vec3 color;"
      "void main(){"
      "  gl_Position = MVP * vec4(vPos, 0.0, 1.0);"
      "  color = vCol;"
      "}";

  static constexpr czstring fragment_shader_text =
      "#version 330 core\n"
      "varying vec3 color;"
      "void main(){"
      "  gl_FragColor = vec4(color, 1.0);"
      "}";

  GLuint vertex_array;
  GLuint vertex_buffer;
  shader_program shader;

 public:
  application() : base{500, 500, "Multi-Threaded Triangle Test"} {}

  void setup() {
    base::setup();

    // Create shader for triangle.
    shader = shader_program({vertex_shader_text}, {fragment_shader_text});

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
    const auto vpos_location = glGetAttribLocation(shader, "vPos");
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*)0);
    const auto vcol_location = glGetAttribLocation(shader, "vCol");
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*)(sizeof(float) * 2));
  }

  void update() {
    base::update();

    // Continuously rotate the triangle.
    auto model = glm::mat4{1.0f};
    const auto axis = glm::normalize(glm::vec3(1, 1, 1));
    model = rotate(model, float(glfwGetTime()), axis);

    // Compute the model-view-projection matrix (MVP).
    const auto mvp = cam.projection_matrix() * cam.view_matrix() * model;
    // Transfer the MVP to the GPU.
    glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE,
                       glm::value_ptr(mvp));
  }

  void render() {
    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind vertex array of triangle
    // and use the created shader
    // to render the triangle.
    glUseProgram(shader);
    glBindVertexArray(vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }
};

int main() {
  // Run two applications with different background color in parallel.
  auto app_task = async(launch::async, [&] {
    application app{};
    glClearColor(0, 0.5, 0.8, 1.0);
    app.run();
  });

  auto app2_task = async(launch::async, [&] {
    application app{};
    glClearColor(0.8, 0.5, 0.0, 1.0);
    app.run();
  });

  app_task.wait();
  app2_task.wait();
}
