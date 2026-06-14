#pragma once

#include "texture.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

class StaticMesh;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec3 tangent;
};

struct Material
{
  glm::vec3 surface_color{ 1.0f, 1.0f, 1.0f };
  Texture tex_diffuse;
  Texture tex_normal;
};

struct MeshInstance 
{
  const StaticMesh* mesh = nullptr;
  Material material;
};

struct LightInstance 
{
  glm::vec3 color{ 1.0f, 1.0f, 1.0f };
  f32 power{ 1.0f };
};

namespace ShaderLocation 
{
  namespace Vertex 
  {
    constexpr auto MatTransform      = 0;
    constexpr auto MatCam            = 1;
    constexpr auto MatPer            = 2;
  }

  namespace Fragment 
  {
    constexpr auto CameraEye         = 0;
    
    // Material
    constexpr auto SurfaceColor      = 5;
    constexpr auto HasTextureColor   = 6;
    constexpr auto HasTextureNormal  = 7;

    // Light
    constexpr auto LightPosition     = 10;
    constexpr auto LightColor        = 11;
    constexpr auto LightPowerWatt    = 12;
  }

  namespace Texture 
  {
    constexpr auto UnitColor         = 0;
    constexpr auto UnitNormal        = 1;
  }
}