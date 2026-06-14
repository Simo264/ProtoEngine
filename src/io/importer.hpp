#pragma once

#include "../basic_types.hpp"
#include "../render_types.hpp"

#include <vector>

struct ModelInfo
{
  std::vector<Vertex> vertices;
  std::vector<u32> indices;

  Material material;
};

ModelInfo import_model(const std::filesystem::path& filepath);