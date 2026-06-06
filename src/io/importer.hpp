#pragma once

#include "../basic_types.hpp"
#include "../texture.hpp"
#include "../vertex.hpp"

#include <vector>

struct ModelInfo
{
  Texture tex_diffuse;
  Texture tex_normal;
  std::vector<Vertex> vertices;
  std::vector<u32> indices;
};

ModelInfo import_model(const std::filesystem::path& filepath);