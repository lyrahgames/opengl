#include <fstream>
#include <future>
#include <iostream>
#include <vector>
//
#include <lyrahgames/opengl/opengl.hpp>

using namespace std;
using namespace lyrahgames::opengl;

class application : public viewer<application> {
  using base = viewer<application>;

  static constexpr czstring vertex_shader_text =
      "#version 330 core\n"

      "uniform mat4 model;"
      "uniform mat4 view;"
      "uniform mat4 projection;"

      "attribute vec3 vp;"
      "attribute vec3 vn;"

      "out vec4 position;"
      "out vec4 normal;"

      "void main(){"
      "  mat4 mv = view * model;"
      "  position = mv * vec4(vp, 1.0);"
      "  normal = mv * vec4(vn, 0.0);"
      "  gl_Position = projection * position;"
      "}";

  static constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "in vec4 position;"
      "in vec4 normal;"

      "void main(){"
      "  float light = dot(normal, -normalize(position));"
      "  gl_FragColor = vec4(vec3(light), 1.0);"
      "}";

  GLuint vertex_array;
  GLuint vertex_buffer;
  shader_program shader;

  struct vertex {
    vec3 position;
    vec3 normal;
  };
  vector<vertex> vertices;

 public:
  application(czstring file_path)
      : base{500, 500, "Multi-Threaded STL Viewer Test"} {
    fstream file{file_path, ios::in | ios::binary};
    if (!file.is_open()) throw runtime_error("Failed to open given STL file.");

    // Ignore header.
    file.ignore(80);
    uint32_t stl_size;
    file.read(reinterpret_cast<char*>(&stl_size), sizeof(uint32_t));

    vertices.resize(3 * stl_size);
    for (size_t i = 0; i < stl_size; ++i) {
      // file.ignore(12);
      vec3 normal;
      file.read(reinterpret_cast<char*>(&normal), sizeof(vec3));
      for (size_t j = 0; j < 3; ++j) {
        vec3 position;
        file.read(reinterpret_cast<char*>(&position), sizeof(vec3));
        vertices[3 * i + j].position = position;
        vertices[3 * i + j].normal = normal;
      }
      file.ignore(2);
    }

    vec3 aabb_min = vertices[0].position;
    vec3 aabb_max = vertices[0].position;
    for (size_t i = 1; i < size(vertices); ++i) {
      aabb_min = min(aabb_min, vertices[i].position);
      aabb_max = max(aabb_max, vertices[i].position);
    }
    origin = 0.5f * (aabb_max + aabb_min);
    radius = 0.5f * length(aabb_max - aabb_min) *
             (1.0f / tan(0.5f * cam.vfov() * lyrahgames::opengl::pi / 180.0f));
  }

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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]),
                 vertices.data(), GL_STATIC_DRAW);

    // Set the data layout of the position and colors
    // with vertex attribute pointers.
    const auto vpos_location = glGetAttribLocation(shader, "vp");
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*)0);

    const auto vcol_location = glGetAttribLocation(shader, "vn");
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*)(sizeof(vec3)));

    glEnable(GL_DEPTH_TEST);
  }

  void update() {
    base::update();

    // Continuously rotate the triangle.
    auto model = mat4{1.0f};
    // const auto axis = glm::normalize(glm::vec3(0, 1, 0));
    // model = rotate(model, float(glfwGetTime()), axis);

    // Transfer the MVP to the GPU.
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE,
                       glm::value_ptr(cam.view_matrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(cam.projection_matrix()));
  }

  void render() {
    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind vertex array of triangle
    // and use the created shader
    // to render the triangle.
    glUseProgram(shader);
    glBindVertexArray(vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, size(vertices));
  }
};

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "usage: stl-viewer <stl model>\n";
    return 0;
  }

  // Run two applications with different background color in parallel.
  auto app_task = async(launch::async, [&] {
    application app{argv[1]};
    glClearColor(0, 0.5, 0.8, 1.0);
    app.run();
  });

  auto app2_task = async(launch::async, [&] {
    application app{argv[1]};
    glClearColor(0.8, 0.5, 0.0, 1.0);
    app.run();
  });

  app_task.wait();
  app2_task.wait();
}
