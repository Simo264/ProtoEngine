#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <memory>
#include <print>
#include <stdexcept>

#include "basic_types.hpp"
#include "pipeline.hpp"
#include "static_mesh.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "transformation.hpp"
#include "vertex.hpp"
#include "scene_graph.hpp"
#include "static_mesh.hpp"
#include "texture.hpp"

#include "gui/gui.hpp"

#include <glm/mat2x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static constexpr auto WINDOW_W = 640;
static constexpr auto WINDOW_H = 480;
static auto aspect_ratio = static_cast<f32>(WINDOW_W) / static_cast<f32>(WINDOW_H);

static const auto models_dir = std::filesystem::current_path() / "models";
static const auto shaders_dir = std::filesystem::current_path() / "shaders";

// static auto program_vertex_object = ShaderProgram{};
// static auto program_fragment_object = ShaderProgram{};
// static auto pipeline_object = ProgramPipelineObject{};

struct ModelInfo
{
  Texture tex_diffuse;
  Texture tex_normal;
  std::vector<Vertex> vertices;
  std::vector<u32> indices;
};

static auto init_context() 
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  auto window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Proto engine", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSetFramebufferSizeCallback(window, []([[maybe_unused]] GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    aspect_ratio = static_cast<f32>(width) / static_cast<f32>(height);
  });
  glViewport(0, 0, WINDOW_W, WINDOW_H);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;           // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
  return window;
}

static auto import_model(const std::filesystem::path& filepath)
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

int main() 
{ 
	auto window = init_context();
	
  auto vertex_shader = create_shader_object(shaders_dir / "shader.vert.glsl", ShaderStage::Vertex);
  auto vertex_program = create_shader_program(vertex_shader);
  auto base_color_shader = create_shader_object(shaders_dir / "base_color.frag.glsl", ShaderStage::Fragment);
  auto base_color_program = create_shader_program(base_color_shader);
  auto normal_color_shader = create_shader_object(shaders_dir / "normal_color.frag.glsl", ShaderStage::Fragment);
  auto normal_color_program = create_shader_program(normal_color_shader);
  auto lighting_shader = create_shader_object(shaders_dir / "lighting.frag.glsl", ShaderStage::Fragment);
  auto lighting_program = create_shader_program(lighting_shader);

  auto pipeline_object = create_pipeline_object(vertex_program, normal_color_program);

 	auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);
  camera.eye.z = 1.0f;

  // let's define the scene graph hierarchy
  
  auto meshes = std::vector<std::unique_ptr<StaticMesh>>{};
  auto scene = Scene{};
  auto world_node = scene.create_node("World");
  auto lion_head_node = scene.create_node("Lion head");
  world_node->add_child(lion_head_node);
  scene.set_root(world_node);
  
  auto model = import_model(models_dir / "lion_head/lion_head_1k.gltf");
  model.tex_diffuse.bind_texture_unit(0);
  model.tex_normal.bind_texture_unit(1);
  
  auto mesh_object = std::make_unique<StaticMesh>(
    model.vertices.data(),
    model.vertices.size(),
    model.indices.data(),
    model.indices.size()
  );
  lion_head_node->set_mesh(mesh_object.get());

  glEnable(GL_DEPTH_TEST);  	// enable depth testing
  glDepthFunc(GL_LESS);    	// specify the value used for depth buffer comparisons
  glDepthMask(GL_TRUE);    	// enable/disable writing into the depth buffer
  glClearDepthf(1.0f);       	// specify the clear value for the depth buffer
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // specify the clear value for the color buffer

  while (!glfwWindowShouldClose(window)) 
  {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffers to preset values

    scene.update();
    
    camera.handle_input(window);
    camera.aspect = aspect_ratio;
    auto mat_camera = camera.canonical_to_camera();
    auto mat_persp = camera.get_perspective();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    gui_camera_window(camera);
    gui_hierarchy_window(scene.root());
    gui_draw_inspector();
    
    pipeline_object.bind();
    pipeline_object.set_active_program(normal_color_program);
    //normal_color_program.set_uniform_vector3f(0, &camera.eye[0]); // u_camera_eye => location 0
    
    pipeline_object.set_active_program(vertex_program);
    vertex_program.set_uniform_mat4f(1, &mat_camera[0][0]);    // mat_cam => location 1
    vertex_program.set_uniform_mat4f(2, &mat_persp[0][0]);     // mat_per => location 2
    scene.render(vertex_program);
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
    {
      auto backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
