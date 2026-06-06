#include "importer.hpp"

#include <print>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>


ModelInfo import_model(const std::filesystem::path& filepath)
{
  std::println("=========================");
  std::println("Importing model: {}", filepath.string());
	if(!std::filesystem::exists(filepath))
    throw std::runtime_error("File does not exist");

  auto importer = Assimp::Importer{};
  const auto* scene = importer.ReadFile(filepath.string().c_str(), 
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate | 
    aiProcess_FlipUVs |
    aiProcess_JoinIdenticalVertices);

  if (!scene) 
    throw std::runtime_error(std::format("Error on loading scene: {}", importer.GetErrorString()));

  std::println("num_meshes: {}", scene->mNumMeshes);
  auto aimesh = scene->mMeshes[0];
  
  std::println("num_vertices: {}", aimesh->mNumVertices);
  
  // load vertices
  
  auto vertices = std::vector<Vertex>{};
  vertices.reserve(aimesh->mNumVertices);
  for (auto i = 0u; i < aimesh->mNumVertices; i++) 
  {
    auto &vertex = vertices.emplace_back();
    vertex.position = glm::vec3{aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z};
    if (aimesh->HasNormals())
      vertex.normal = glm::vec3{aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z};
    if (aimesh->HasTextureCoords(0))
      vertex.texcoord = glm::vec2{aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y};
    if (aimesh->HasTangentsAndBitangents())
      vertex.tangent = glm::vec3{ aimesh->mTangents[i].x, aimesh->mTangents[i].y, aimesh->mTangents[i].z, };
  }

  // load indices

  auto indices = std::vector<u32>{};
  indices.reserve(aimesh->mNumFaces * 3);
  for (auto i = 0u; i < aimesh->mNumFaces; i++) 
  {
    auto face = aimesh->mFaces[i];
    indices.emplace_back(face.mIndices[0]);
    indices.emplace_back(face.mIndices[1]);
    indices.emplace_back(face.mIndices[2]);
  }

  auto texture_diffuse = Texture{};
  auto texture_normal = Texture{};

  auto material = scene->mMaterials[aimesh->mMaterialIndex];
  std::println("Material name: {}", material->GetName().C_Str());
  std::println("Base color texture count: {}", material->GetTextureCount(aiTextureType_BASE_COLOR));
  std::println("Diffuse texture count: {}", material->GetTextureCount(aiTextureType_DIFFUSE));
  std::println("Normal texture count: {}", material->GetTextureCount(aiTextureType_NORMALS));
  std::println("Height texture count: {}", material->GetTextureCount(aiTextureType_HEIGHT));
  
  auto base_color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
  if (material->Get(AI_MATKEY_BASE_COLOR, base_color) == AI_SUCCESS || 
      material->Get(AI_MATKEY_COLOR_DIFFUSE, base_color) == AI_SUCCESS)
    std::println("Material base color: r={}, g={}, b={}, a={}", base_color.r, base_color.g, base_color.b, base_color.a);
  
  auto path = aiString{};
  if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == AI_SUCCESS || 
      material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
  {
    auto embedded_tex = scene->GetEmbeddedTexture(path.C_Str());
    if (embedded_tex)
    {
      std::println("Embedded texture base color/diffuse: {}", path.C_Str());
      
      if(!texture_diffuse.is_valid())
      {
        texture_diffuse = Texture::create_from_memory(
          embedded_tex->pcData,
          embedded_tex->mWidth,
          TextureImageFormat::SRGB8,
          PixelDataFormat::RGB,
          PixelDataType::UnsignedByte,
          STBI_rgb
        );
      }
    }
    else 
    {
      std::println("External texture base color/diffuse: {}", path.C_Str());
      if(!texture_diffuse.is_valid())
      {
        texture_diffuse = Texture::create_from_file(
          filepath.parent_path() / path.C_Str(),
          TextureImageFormat::SRGB8, 
          PixelDataFormat::RGB,
          PixelDataType::UnsignedByte,
          STBI_rgb
        );
      }
    }
  }
  
  if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS || 
      material->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS)
  {
    auto embedded_tex = scene->GetEmbeddedTexture(path.C_Str());
    if (embedded_tex) 
    {
      if(!texture_normal.is_valid()) 
      {
        texture_normal = Texture::create_from_memory(
          embedded_tex->pcData, 
          embedded_tex->mWidth, 
          TextureImageFormat::RGB8,
          PixelDataFormat::RGB,
          PixelDataType::UnsignedByte, 
          STBI_rgb
        );
      }
    } 
    else  
    {
      if(!texture_normal.is_valid()) 
      {
        texture_normal = Texture::create_from_file(
          filepath.parent_path() / path.C_Str(), 
          TextureImageFormat::RGB8, 
          PixelDataFormat::RGB, 
          PixelDataType::UnsignedByte, 
          STBI_rgb
        );
      }
    }
  }

  std::println("=========================");
  
  return ModelInfo{
    .tex_diffuse = texture_diffuse,
    .tex_normal = texture_normal,
    .vertices = std::move(vertices),
    .indices = std::move(indices)
  };
}
