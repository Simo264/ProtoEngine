#pragma once

#include "basic_types.hpp"
#include <glm/ext/vector_float3.hpp>

struct LightProperties 
{
  glm::vec3 color{ 1.0f };
  f32 power{ 1.0f };
};