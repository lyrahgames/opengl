#pragma once
#include <cassert>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <string>

#include <glbinding/gl/gl.h>

#include <glm/glm.hpp>
//
#include <glm/ext.hpp>

#include <lyrahgames/xstd/builtin_types.hpp>

namespace lyrahgames::opengl {

using namespace std;
using namespace gl;
using namespace glm;
using namespace lyrahgames::xstd;

constexpr auto pi = numbers::pi_v<float>;

}  // namespace lyrahgames::opengl
