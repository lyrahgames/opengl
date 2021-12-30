#include <bit>
#include <cstddef>
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
      "attribute float vl;"
      "attribute vec3 vdl;"
      "attribute float vlv;"
      "attribute float vlvg;"

      "out vec4 position;"
      "out vec4 normal;"
      "out float light;"
      "out vec3 light_gradient;"
      "out float light_variation;"
      "out float light_variation_gradient;"

      "void main(){"
      "  mat4 mv = view * model;"
      "  position = model * vec4(vp, 1.0);"
      "  normal = model * vec4(vn, 0.0);"
      // "  light = 100000 * length(vdl);"
      "  light = vl;"
      "  light_gradient = vdl;"
      "  light_variation = vlv;"
      "  light_variation_gradient = vlvg;"
      "  gl_Position = projection * view * position;"
      "}";

  static constexpr czstring geometry_shader_text =
      "#version 330 core\n"

      "layout (triangles) in;"
      "layout (line_strip, max_vertices = 6) out;"

      "in float light_variation_gradient[];"

      "void main(){"
      "  vec4 x = gl_in[0].gl_Position;"
      "  vec4 y = gl_in[1].gl_Position;"
      "  vec4 z = gl_in[2].gl_Position;"

      "  float lx = light_variation_gradient[0];"
      "  float ly = light_variation_gradient[1];"
      "  float lz = light_variation_gradient[2];"

      "  bool a = lx * ly < 0;"
      "  bool b = ly * lz < 0;"
      "  bool c = lz * lx < 0;"

      "  vec4 pa = (abs(ly) * x + abs(lx) * y) / (abs(lx) + abs(ly));"
      "  vec4 pb = (abs(lz) * y + abs(ly) * z) / (abs(ly) + abs(lz));"
      "  vec4 pc = (abs(lx) * z + abs(lz) * x) / (abs(lz) + abs(lx));"

      "  if (a && b) {"
      "    gl_Position = pa;"
      "    EmitVertex();"
      "    gl_Position = pb;"
      "    EmitVertex();"
      "  }"
      "  if (b && c) {"
      "    gl_Position = pb;"
      "    EmitVertex();"
      "    gl_Position = pc;"
      "    EmitVertex();"
      "  }"
      "  if (c && a) {"
      "    gl_Position = pc;"
      "    EmitVertex();"
      "    gl_Position = pa;"
      "    EmitVertex();"
      "  }"
      "  EndPrimitive();"
      "}";

  static constexpr czstring fragment_shader_text =
      "#version 330 core\n"

      "uniform vec3 view_dir;"

      "in vec4 position;"
      "in vec4 normal;"
      "in float light;"
      "in vec3 light_gradient;"
      "in float light_variation;"
      "in float light_variation_gradient;"

      "float near = 0.1;"
      "float far  = 100.0;"
      "float linearize_depth(float depth) {"
      "  float z = depth * 2.0 - 1.0;"
      "  return (2.0 * near * far) / (far + near - z * (far - near));"
      "}"

      "void main(){"
      // "  float light = abs(dot(normal, position)) / "
      // "length(vec3(position));"
      // "  float light = max(0.0, -dot(vec3(normal), view_dir));"
      "  gl_FragColor = vec4(vec3(1.0 - light), 1.0);"
      // "  if (light < 4e-1)"
      // "    gl_FragColor = vec4(0.0);"
      // "  else"
      // "    gl_FragColor = vec4(1.0);"
      // "  gl_FragColor = vec4(1.0);"
      // "  gl_FragColor = vec4(light_gradient, 1.0);"
      // "  gl_FragColor = vec4(vec3(length(light_gradient)), 1.0);"
      // "  gl_FragColor = vec4(vec3(light_variation), 1.0);"
      // "  gl_FragColor = vec4(vec3((abs(light_variation_gradient))), 1.0);"
      "}";

  GLuint vertex_array;
  GLuint pel_vertex_array;
  GLuint vertex_buffer;
  GLuint element_buffer;
  shader_program shader;
  shader_program pel_shader;

  static constexpr czstring line_vertex_shader_text =
      "#version 330 core\n"

      "uniform mat4 model;"
      "uniform mat4 view;"
      "uniform mat4 projection;"

      "attribute vec3 vp;"

      "void main(){"
      "  gl_Position = projection * view * model * vec4(vp, 1.0);"
      "}";

  static constexpr czstring line_fragment_shader_text =
      "#version 330 core\n"
      "void main(){"
      "  gl_FragColor = vec4(vec3(0.0), 1.0);"
      "}";

  GLuint line_vertex_array;
  GLuint line_vertex_buffer;
  shader_program line_shader;

  struct vertex {
    vec3 position;
    vec3 normal;
    float light;
    vec3 light_gradient;
    float light_variation;
    float light_variation_gradient;
  };
  vector<vertex> vertices;
  using face = array<uint32_t, 3>;
  vector<face> faces;

  struct line_vertex {
    vec3 position;
  };
  vector<line_vertex> line_vertices;
  using edge = array<uint32_t, 2>;
  vector<edge> edges;

 public:
  using base::window;
  application(czstring file_path)
      : base{500, 500, "Multi-Threaded STL Viewer Test"} {
    fstream file{file_path, ios::in | ios::binary};
    if (!file.is_open()) throw runtime_error("Failed to open given STL file.");

    // Ignore header.
    file.ignore(80);
    uint32_t stl_size;
    file.read(reinterpret_cast<char*>(&stl_size), sizeof(uint32_t));

    // vertices.resize(3 * stl_size);
    // for (size_t i = 0; i < stl_size; ++i) {
    //   // file.ignore(12);
    //   vec3 normal;
    //   file.read(reinterpret_cast<char*>(&normal), sizeof(vec3));
    //   for (size_t j = 0; j < 3; ++j) {
    //     vec3 position;
    //     file.read(reinterpret_cast<char*>(&position), sizeof(vec3));
    //     vertices[3 * i + j].position = position;
    //     vertices[3 * i + j].normal = normal;
    //   }
    //   file.ignore(2);
    // }

    unordered_map<vec3, size_t, decltype([](const auto& v) -> size_t {
                    return (bit_cast<uint32_t>(v.x) << 7) ^
                           (bit_cast<uint32_t>(v.y) << 3) ^
                           bit_cast<uint32_t>(v.z);
                  })>
        position_index{};

    for (size_t i = 0; i < stl_size; ++i) {
      vec3 normal;
      file.read(reinterpret_cast<char*>(&normal), sizeof(normal));

      array<vec3, 3> v;
      file.read(reinterpret_cast<char*>(&v), sizeof(v));

      face f{};
      for (size_t j = 0; j < 3; ++j) {
        const auto k = (j + 1) % 3;
        const auto l = (j + 2) % 3;
        const auto p = v[k] - v[j];
        const auto q = v[l] - v[j];
        const auto weight = length(cross(p, q)) / dot(p, p) / dot(q, q);
        // const auto n = cross(p, q) / dot(p, p) / dot(q, q);

        const auto it = position_index.find(v[j]);
        if (it == end(position_index)) {
          const int index = vertices.size();
          f[j] = index;
          position_index.emplace(v[j], index);
          // vertices.push_back({v[j], normal});
          vertices.push_back({v[j], weight * normal});
          // vertices.push_back({v[j], n});
          continue;
        }

        const auto index = it->second;
        f[j] = index;
        // vertices[index].normal += normal;
        vertices[index].normal += weight * normal;
        // vertices[index].normal += n;
      }

      faces.push_back(f);

      file.ignore(2);
    }

    for (auto& v : vertices) {
      v.normal = normalize(v.normal);
    }

    // AABB computation
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
    pel_shader = shader_program({vertex_shader_text}, {geometry_shader_text},
                                {fragment_shader_text});

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

    glGenBuffers(1, &element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(faces[0]),
                 faces.data(), GL_STATIC_DRAW);

    // Set the data layout of the position and colors
    // with vertex attribute pointers.
    const auto vpos_location = glGetAttribLocation(shader, "vp");
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]),
                          (void*)offsetof(vertex, position));

    const auto vcol_location = glGetAttribLocation(shader, "vn");
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*)offsetof(vertex, normal));

    const auto vl_location = glGetAttribLocation(shader, "vl");
    glEnableVertexAttribArray(vl_location);
    glVertexAttribPointer(vl_location, 1, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*)offsetof(vertex, light));

    const auto vdl_location = glGetAttribLocation(shader, "vdl");
    glEnableVertexAttribArray(vdl_location);
    glVertexAttribPointer(vdl_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]),
                          (void*)offsetof(vertex, light_gradient));

    const auto vlv_location = glGetAttribLocation(shader, "vlv");
    glEnableVertexAttribArray(vlv_location);
    glVertexAttribPointer(vlv_location, 1, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]),
                          (void*)offsetof(vertex, light_variation));

    const auto vlvg_location = glGetAttribLocation(shader, "vlvg");
    glEnableVertexAttribArray(vlvg_location);
    glVertexAttribPointer(vlvg_location, 1, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]),
                          (void*)offsetof(vertex, light_variation_gradient));

    glGenVertexArrays(1, &pel_vertex_array);
    glBindVertexArray(pel_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
    // Set the data layout of the position and colors
    // with vertex attribute pointers.
    {
      const auto vpos_location = glGetAttribLocation(pel_shader, "vp");
      glEnableVertexAttribArray(vpos_location);
      glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, position));
    }

    {
      const auto vcol_location = glGetAttribLocation(pel_shader, "vn");
      glEnableVertexAttribArray(vcol_location);
      glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, normal));
    }
    {
      const auto vl_location = glGetAttribLocation(pel_shader, "vl");
      glEnableVertexAttribArray(vl_location);
      glVertexAttribPointer(vl_location, 1, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, light));
    }
    {
      const auto vdl_location = glGetAttribLocation(pel_shader, "vdl");
      glEnableVertexAttribArray(vdl_location);
      glVertexAttribPointer(vdl_location, 3, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, light_gradient));
    }

    {
      const auto vlv_location = glGetAttribLocation(pel_shader, "vlv");
      glEnableVertexAttribArray(vlv_location);
      glVertexAttribPointer(vlv_location, 1, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, light_variation));
    }

    {
      const auto vlvg_location = glGetAttribLocation(pel_shader, "vlvg");
      glEnableVertexAttribArray(vlvg_location);
      glVertexAttribPointer(vlvg_location, 1, GL_FLOAT, GL_FALSE,
                            sizeof(vertices[0]),
                            (void*)offsetof(vertex, light_variation_gradient));
    }

    // line_shader = {{line_vertex_shader_text}, {line_fragment_shader_text}};
    // // Use a vertex array to be able to reference the vertex buffer and
    // // the vertex attribute arrays of the triangle with one single variable.
    // glGenVertexArrays(1, &line_vertex_array);
    // glBindVertexArray(line_vertex_array);

    // // Generate and bind the buffer which shall contain the triangle data.
    // glGenBuffers(1, &line_vertex_buffer);
    // glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);

    // const auto tmp = glGetAttribLocation(line_shader, "vp");
    // glEnableVertexAttribArray(tmp);
    // glVertexAttribPointer(tmp, 3, GL_FLOAT, GL_FALSE,
    // sizeof(line_vertices[0]),
    //                       (void*)offsetof(line_vertex, position));

    glEnable(GL_DEPTH_TEST);
    glLineWidth(4.0f);
  }

  void update_light() {
    const auto d = cam.direction();
    for (auto& v : vertices) {
      // v.light = std::max(
      //     0.0f, -dot(v.normal, normalize(v.position - cam.position())));
      v.light = std::max(0.0f, -dot(v.normal, d));
      v.light_gradient = {};
      v.light_variation = 0;
      v.light_variation_gradient = 0;
    }

    vector<size_t> counts(vertices.size(), 0);

    for (const auto& f : faces) {
      const auto x = vertices[f[0]].position;
      const auto y = vertices[f[1]].position;
      const auto z = vertices[f[2]].position;

      const auto lx = vertices[f[0]].light;
      const auto ly = vertices[f[1]].light;
      const auto lz = vertices[f[2]].light;

      const auto u = y - x;
      const auto v = z - x;

      const auto dlu = ly - lx;
      const auto dlv = lz - lx;

      const auto u2 = dot(u, u);
      const auto v2 = dot(v, v);
      const auto uv = dot(u, v);

      const auto inv_det = 1 / (u2 * v2 - uv * uv);

      const auto p = (v2 * dlu - uv * dlv) * inv_det;
      const auto q = (u2 * dlv - uv * dlu) * inv_det;

      const auto grad = p * u + q * v;

      for (int i = 0; i < 3; ++i) {
        vertices[f[i]].light_gradient += grad;
        ++counts[f[i]];
      }
    }

    float max_variation = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
      vertices[i].light_gradient /= counts[i];

      const auto t = length(vertices[i].light_gradient);
      max_variation = std::max(max_variation, t);
      vertices[i].light_variation = t;
    }

    for (auto& v : vertices) {
      v.light_variation /= max_variation;
    }

    for (const auto& f : faces) {
      const auto x = vertices[f[0]].position;
      const auto y = vertices[f[1]].position;
      const auto z = vertices[f[2]].position;

      const auto lx = length(vertices[f[0]].light_gradient);
      const auto ly = length(vertices[f[1]].light_gradient);
      const auto lz = length(vertices[f[2]].light_gradient);

      const auto u = y - x;
      const auto v = z - x;

      const auto dlu = ly - lx;
      const auto dlv = lz - lx;

      const auto u2 = dot(u, u);
      const auto v2 = dot(v, v);
      const auto uv = dot(u, v);

      const auto inv_det = 1 / (u2 * v2 - uv * uv);

      const auto p = (v2 * dlu - uv * dlv) * inv_det;
      const auto q = (u2 * dlv - uv * dlu) * inv_det;

      const auto grad = p * u + q * v;

      for (int i = 0; i < 3; ++i) {
        vertices[f[i]].light_variation_gradient +=
            dot(grad, normalize(vertices[f[i]].light_gradient));
      }
    }

    float max_variation_gradient = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
      vertices[i].light_variation_gradient /= counts[i];
      const auto t = vertices[i].light_variation_gradient;
      max_variation_gradient = std::max(max_variation_gradient, t);
    }

    // for (auto& v : vertices) {
    //   v.light_variation_gradient /= max_variation_gradient;
    // }

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    // The data is not changing rapidly. Therefore we use GL_STATIC_DRAW.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]),
                 vertices.data(), GL_DYNAMIC_DRAW);

    line_vertices.clear();
    for (const auto& f : faces) {
      const auto x = vertices[f[0]].position;
      const auto y = vertices[f[1]].position;
      const auto z = vertices[f[2]].position;

      const auto lx = vertices[f[0]].light_variation_gradient;
      const auto ly = vertices[f[1]].light_variation_gradient;
      const auto lz = vertices[f[2]].light_variation_gradient;

      const auto a = lx * ly < 0;
      const auto b = ly * lz < 0;
      const auto c = lz * lx < 0;

      const auto pa = (abs(ly) * x + abs(lx) * y) / (abs(lx) + abs(ly));
      const auto pb = (abs(lz) * y + abs(ly) * z) / (abs(ly) + abs(lz));
      const auto pc = (abs(lx) * z + abs(lz) * x) / (abs(lz) + abs(lx));

      constexpr float bound = 1e-1f;

      if (a && b) {
        line_vertices.push_back({pa});
        line_vertices.push_back({pb});
      }
      if (b && c) {
        line_vertices.push_back({pb});
        line_vertices.push_back({pc});
      }
      if (c && a) {
        line_vertices.push_back({pc});
        line_vertices.push_back({pa});
      }
    }

    // glBindVertexArray(line_vertex_array);
    // glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);
    // // The data is not changing rapidly. Therefore we use GL_STATIC_DRAW.
    // glBufferData(GL_ARRAY_BUFFER,
    //              line_vertices.size() * sizeof(line_vertices[0]),
    //              line_vertices.data(), GL_DYNAMIC_DRAW);
  }

  void update() {
    base::update();

    // Continuously rotate the triangle.
    auto model = mat4{1.0f};
    // const auto axis = glm::normalize(glm::vec3(0, 1, 0));
    // model = rotate(model, float(glfwGetTime()), axis);

    // Transfer the MVP to the GPU.
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE,
                       glm::value_ptr(cam.view_matrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(cam.projection_matrix()));
    glUniform3fv(glGetUniformLocation(shader, "view_dir"), 1,
                 value_ptr(cam.direction()));

    glUseProgram(line_shader);
    glUniformMatrix4fv(glGetUniformLocation(line_shader, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(line_shader, "view"), 1, GL_FALSE,
                       glm::value_ptr(cam.view_matrix()));
    glUniformMatrix4fv(glGetUniformLocation(line_shader, "projection"), 1,
                       GL_FALSE, glm::value_ptr(cam.projection_matrix()));

    // model = scale(model, {1.1f, 1.1f, 1.1f});
    // model = translate(model, 1e-3f * cam.position());

    glUseProgram(pel_shader);
    glUniformMatrix4fv(glGetUniformLocation(pel_shader, "model"), 1, GL_FALSE,
                       glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(pel_shader, "view"), 1, GL_FALSE,
                       glm::value_ptr(cam.view_matrix()));
    glUniformMatrix4fv(glGetUniformLocation(pel_shader, "projection"), 1,
                       GL_FALSE, glm::value_ptr(cam.projection_matrix()));

    update_light();
  }

  void render() {
    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.5, 0.5, 0.5, 1.0);

    // Bind vertex array of triangle
    // and use the created shader
    // to render the triangle.
    glBindVertexArray(vertex_array);
    // glDrawArrays(GL_TRIANGLES, 0, size(vertices));
    // glDepthMask(GL_TRUE);
    // glDepthFunc(GL_LESS);
    glUseProgram(shader);
    glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(pel_vertex_array);
    // glDepthMask(GL_FALSE);
    // glDepthFunc(GL_LEQUAL);
    glUseProgram(pel_shader);
    glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_INT, 0);

    // glUseProgram(line_shader);
    // glBindVertexArray(line_vertex_array);
    // glDrawArrays(GL_LINES, 0, line_vertices.size());
  }
};

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "usage: stl-viewer <stl model>\n";
    return 0;
  }

  // Run two applications with different background color in parallel.
  // auto app_task = async(launch::async, [&] {
  application app{argv[1]};
  glClearColor(0, 0.5, 0.8, 1.0);
  app.run();
  // });

  // auto app2_task = async(launch::async, [&] {
  //   application app{argv[1]};
  //   glClearColor(0.8, 0.5, 0.0, 1.0);
  //   app.run();
  // });

  // app_task.wait();
  // app2_task.wait();
}
