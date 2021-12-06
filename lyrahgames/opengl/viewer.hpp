#pragma once
#include <lyrahgames/opengl/camera.hpp>
#include <lyrahgames/opengl/glfw_application.hpp>
#include <lyrahgames/opengl/utility.hpp>

namespace lyrahgames::opengl {

template <typename T>
class viewer : public glfw_application<T> {
  using base = glfw_application<T>;

 public:
  viewer(int width = 800, int height = 450, czstring title = "GLFW Viewer")
      : base{width, height, title} {}

  void scroll_callback(double x, double y);
  void process_events();
  void resize(int width, int height);
  void setup();
  void update();

  void update_view();
  void turn(const vec2& mouse_move);
  void shift(const vec2& mouse_move);
  void zoom(const vec2& mouse_scroll);

 protected:
  using base::context;
  using base::window;

  // World Origin
  vec3 origin;
  // Basis Vectors of Right-Handed Coordinate System
  vec3 up{0, 1, 0};
  vec3 right{1, 0, 0};
  vec3 front{0, 0, 1};
  // Spherical/Horizontal Coordinates of Camera
  float radius = 10;
  float altitude = 0;
  float azimuth = 0;

  // Mouse Interaction
  vec2 old_mouse_pos;
  vec2 mouse_pos;
  bool view_should_update = true;

  camera cam;
};

template <typename T>
void viewer<T>::scroll_callback(double x, double y) {
  zoom({x, y});
}

template <typename T>
void viewer<T>::process_events() {
  // Close application when escape has been pressed.
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Compute the mouse move vector.
  old_mouse_pos = mouse_pos;
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mouse_pos = vec2{xpos, ypos};
  const auto mouse_move = mouse_pos - old_mouse_pos;

  // Left mouse button should rotate the camera by using spherical coordinates.
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    turn(mouse_move);

  // Right mouse button should translate the camera.
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    shift(mouse_move);
}

template <typename T>
void viewer<T>::resize(int width, int height) {
  // Set the viewport to the whole window.
  glViewport(0, 0, width, height);
  // Update the camera and projection matrix;
  cam.set_screen_resolution(width, height);
}

template <typename T>
void viewer<T>::update_view() {
  // Computer camera position by using spherical coordinates.
  // This transformation is a variation of the standard
  // called horizontal coordinates often used in astronomy.
  auto p = cos(altitude) * cos(azimuth) * right +  //
           cos(altitude) * sin(azimuth) * front +  //
           sin(altitude) * up;
  p *= radius;
  p += origin;
  cam.move(p).look_at(origin, up);
}

template <typename T>
void viewer<T>::turn(const vec2& mouse_move) {
  altitude += mouse_move.y * 0.01;
  azimuth += mouse_move.x * 0.01;
  constexpr float bound = pi / 2 - 1e-5f;
  altitude = std::clamp(altitude, -bound, bound);
  view_should_update = true;
}

template <typename T>
void viewer<T>::shift(const vec2& mouse_move) {
  const auto shift = mouse_move.x * cam.right() + mouse_move.y * cam.up();
  const auto scale = 1.3f * cam.pixel_size() * radius;
  origin += scale * shift;
  view_should_update = true;
}

template <typename T>
void viewer<T>::zoom(const vec2& mouse_scroll) {
  radius *= exp(-0.1f * float(mouse_scroll.y));
  view_should_update = true;
}

template <typename T>
void viewer<T>::setup() {
  // Before starting the game loop,
  // make sure to initialize the projection matrix once
  // by using the standard 'resize' method.
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  resize(width, height);
}

template <typename T>
void viewer<T>::update() {
  // We only want to update the view
  // if the camera has actually changed.
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }
}

}  // namespace lyrahgames::opengl
