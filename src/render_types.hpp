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
    constexpr auto MatTransform      = 0; // transform matrix
    constexpr auto MatCam            = 1; // camera matrix
    constexpr auto MatPer            = 2; // perspective matrix
  }

  namespace Fragment 
  {
    constexpr auto CameraEye         = 0;
    constexpr auto HasTextureAlbedo  = 1;
    constexpr auto HasTextureNormal  = 2;
    
    // Material
    constexpr auto Albedo      = 5; // the albedo color
    constexpr auto Metallic    = 6; // the metallic factor
    constexpr auto Roughness   = 7; // the roughness factor

    // Light properties
    constexpr auto LightPosition     = 10;
    constexpr auto LightColor        = 11;
    constexpr auto LightPowerWatt    = 12;
  }

  namespace Texture
  {
    constexpr auto AlbedoUnit        = 0; // the albedo texture binding unit
    constexpr auto NormalUnit        = 1; // the normal texture binding unit
    constexpr auto RoughnessUnit     = 2; // the roughness texture binding unit
  }
}