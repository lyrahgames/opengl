#pragma once
#include <lyrahgames/opengl/utility.hpp>

namespace lyrahgames::opengl {

class camera {
 public:
  constexpr camera(int w = 500,
                   int h = 500,
                   float yfov = 45.0f,
                   float n = 0.1f,
                   float f = 100.0f) noexcept {
    set_perspective(w, h, yfov, n, f);
  }

  constexpr auto position() const noexcept { return pos; }
  constexpr auto direction() const noexcept { return dir; }
  constexpr auto right() const noexcept { return r; }
  constexpr auto up() const noexcept { return u; }
  constexpr auto front() const noexcept { return -dir; }

  constexpr auto hfov() const noexcept { return fov.x; }
  constexpr auto vfov() const noexcept { return fov.y; }

  constexpr auto screen_width() const noexcept { return width; }
  constexpr auto screen_height() const noexcept { return height; }

  constexpr auto near() const noexcept { return dmin; }
  constexpr auto far() const noexcept { return dmax; }

  constexpr auto aspect_ratio() const noexcept { return ratio; }
  constexpr auto pixel_size() const noexcept { return pixel; }

  constexpr const auto& view_matrix() const noexcept { return view; }
  auto projection_matrix() const noexcept {
    return perspective(vfov(), aspect_ratio(), near(), far());
  }

  auto set_perspective(int w, int h, float yfov, float n, float f) noexcept
      -> camera& {
    width = w;
    height = h;

    ratio = float(w) / h;
    fov.y = yfov;
    fov.x = ratio * fov.y;

    pixel = 2.0f * tan(pi / 180.0f * fov.y / 2) / height;

    dmin = n;
    dmax = f;

    return *this;
  }

  auto set_screen_resolution(int w, int h) noexcept -> camera& {
    set_perspective(w, h, fov.y, dmin, dmax);
    return *this;
  }

  auto move(const vec3& t) noexcept -> camera& {
    pos = t;
    view = translate(mat4{1}, t);
    return *this;
  }

  auto look_at(const vec3& focus, const vec3& rel_up) noexcept -> camera& {
    dir = normalize(focus - pos);
    r = normalize(cross(-dir, rel_up));
    u = normalize(cross(r, -dir));
    view = lookAt(pos, focus, rel_up);
    return *this;
  }

 private:
  vec3 pos{};
  vec3 dir{0, 0, -1};
  vec3 u{0, 1, 0};  // up
  vec3 r{1, 0, 0};  // right

  vec2 fov{45.0f, 45.0f};

  int width = 500;
  int height = 500;

  float ratio = 1;
  float pixel;

  float dmin = 0.1f;
  float dmax = 100.0f;

  mat4 view{1};
};

}  // namespace lyrahgames::opengl
