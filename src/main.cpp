#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <memory>
#include <print>

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

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

static constexpr auto WINDOW_W = 640;
static constexpr auto WINDOW_H = 480;
static auto aspect_ratio = static_cast<f32>(WINDOW_W) / static_cast<f32>(WINDOW_H);

static auto program_vertex_object = ShaderProgram{};
static auto program_fragment_object = ShaderProgram{};
static auto pipeline_object = ProgramPipelineObject{};

static auto texture_color = Texture{};
static auto texture_normal = Texture{};

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

static void create_pipeline_object() 
{
  auto shaders_dir = std::filesystem::current_path() / "shaders";

  auto vertex_shader_obj = ShaderObject{};
  vertex_shader_obj.create(ShaderStage::Vertex);
  vertex_shader_obj.load_source_code(shaders_dir / "basic_shader.vert.glsl");
  vertex_shader_obj.compile();
  auto status = vertex_shader_obj.check_compile_status();
  if (!status)
    std::println("Shader compilation error: {}", vertex_shader_obj.get_compile_log());

  auto fragment_shader_obj = ShaderObject{};
  fragment_shader_obj.create(ShaderStage::Fragment);
  fragment_shader_obj.load_source_code(shaders_dir / "basic_shader.frag.glsl");
  fragment_shader_obj.compile();
  status = fragment_shader_obj.check_compile_status();
  if (!status)
    std::println("Shader compilation error: {}", fragment_shader_obj.get_compile_log());

  program_vertex_object = ShaderProgram{};
  program_vertex_object.create();
  program_vertex_object.attach_shader(vertex_shader_obj);
  program_vertex_object.set_separable(true);
  program_vertex_object.link();
  status = program_vertex_object.check_link_status();
  if (!status)
    std::println("Link status: {}", program_vertex_object.get_link_log());

  program_vertex_object.detach_shader(vertex_shader_obj);

  program_fragment_object = ShaderProgram{};
  program_fragment_object.create();
  program_fragment_object.attach_shader(fragment_shader_obj);
  program_fragment_object.set_separable(true);
  program_fragment_object.link();
  status = program_fragment_object.check_link_status();
  if (!status)
    std::println("Link status: {}", program_fragment_object.get_link_log());

  program_fragment_object.detach_shader(fragment_shader_obj);

  pipeline_object = ProgramPipelineObject{};
  pipeline_object.create();
  pipeline_object.bind_program_stage(PipelineStage::VertexShader, program_vertex_object);
  pipeline_object.bind_program_stage(PipelineStage::FragmentShader, program_fragment_object);
  status = pipeline_object.validate_pipeline();
  if (!status)
    std::println("pipeline object status: {}", pipeline_object.get_validation_status());
}

static void handle_camera_input(auto window, auto &camera) 
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) camera.rotate_pitch(+glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) camera.rotate_pitch(-glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) camera.rotate_yaw(+glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.rotate_yaw(-glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.eye += camera.gaze() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.eye -= camera.gaze() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.eye -= camera.right() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.eye += camera.right() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.eye += camera.up() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) camera.eye -= camera.up() * 0.1f;
}

static auto import_model(const std::filesystem::path& filepath)
{
  std::println("=========================");
  std::println("Importing model: {}", filepath.string());
	if(!std::filesystem::exists(filepath) || !std::filesystem::is_regular_file(filepath))
	{
		std::println("File does not exist or is not a regular file: {}", filepath.string());
		exit(1);
	}

  // Create an instance of the Importer class
  auto importer = Assimp::Importer{};
  const auto scene = importer.ReadFile(filepath.string().c_str(), 
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate | 
    aiProcess_FlipUVs |
    aiProcess_JoinIdenticalVertices);

  // If the import failed, report it
  if (scene == nullptr) 
  {
    std::println("Error on loading scene {}: {}", filepath.string(), importer.GetErrorString());
    exit(1);
  }

  // Now we can access the file's contents.
  std::println("num meshes: {}", scene->mNumMeshes);
  auto aimesh = scene->mMeshes[0];
  
  std::println("num vertices: {}", aimesh->mNumVertices);
  
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

  if(!scene->HasMaterials())
    std::println("No materials in this scene!");
  
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
      
      if(!texture_color.is_valid())
      {
        texture_color = Texture::create_from_memory(
          embedded_tex->pcData,
          embedded_tex->mWidth,
          TextureImageFormat::RGB8,
          PixelDataFormat::RGB,
          PixelDataType::UnsignedByte,
          STBI_rgb);
      }
    }
    else 
    {
      std::println("External texture base color/diffuse: {}", path.C_Str());
      if(!texture_color.is_valid())
      {
        texture_color = Texture::create_from_file(
          std::filesystem::current_path() / "res/models/kenny_mini_dungeon/colormap.png", 
          TextureImageFormat::RGB8, 
          PixelDataFormat::RGB,
          PixelDataType::UnsignedByte,
          STBI_rgb);
      }
    }
  }
  
  if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
  {
    std::println("Material normal map: {}", path.C_Str());
  }

  std::println("=========================");
  return std::unique_ptr<StaticMesh>(new StaticMesh(
    vertices.data(), 
    vertices.size(), 
    indices.data(), 
    indices.size()));
}

int main() 
{ 
	auto window = init_context();
	
	create_pipeline_object();
	
 	auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);

  // let's define the scene graph hierarchy
  auto meshes = std::vector<std::unique_ptr<StaticMesh>>{};
  auto scene = Scene{};
  auto node_world = scene.create_node("World");
  scene.set_root(node_world);
  for(const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path() / "res/models/kenny_mini_dungeon"))
  {
    auto filename = entry.path().filename();
    auto extension = filename.extension();
    if(extension == ".glb")
    {
      auto& mesh = meshes.emplace_back(import_model(entry.path()));
      auto node = scene.create_node(filename.stem().string());
      node_world->add_child(node);
      node->set_mesh(mesh.get());
      
      // arrange on a centered grid so meshes don't overlap at the origin
      constexpr auto spacing = 2.5f; // distance between objects
      constexpr auto cols = 8;         // number of columns in the grid
      constexpr auto base_y = 0.0f;  // shared Y coordinate for all meshes

      auto idx = meshes.size() - 1; // index of the mesh we just pushed
      auto col = static_cast<int>(idx % cols);
      auto row = static_cast<int>(idx / cols);

      // center columns around X = 0
      auto x_offset = (static_cast<float>(col) - (static_cast<float>(cols - 1) / 2.0f)) * spacing;
      auto z_offset = static_cast<float>(row) * spacing;

      auto t = Transformation{};
      t.position = glm::vec3{ x_offset, base_y, z_offset };
      // leave rotation and scale as defaults for now
      node->set_transform(t);
    }
  }
  
  // auto banner_node = scene.create_node("banner");
  // scene.set_root(node_world);
  // node_world->add_child(banner_node);
  // banner_node->set_mesh(mesh_banner.get());
  
  glEnable(GL_DEPTH_TEST);  	// enable depth testing
  glDepthFunc(GL_LESS);    	// specify the value used for depth buffer comparisons
  glDepthMask(GL_TRUE);    	// enable/disable writing into the depth buffer
  glClearDepthf(1.0f);       	// specify the clear value for the depth buffer
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // specify the clear value for the color buffer
  texture_color.bind_texture_unit(0);

  while (!glfwWindowShouldClose(window)) 
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffers to preset values

    scene.update();
    
    handle_camera_input(window, camera);
    camera.aspect = aspect_ratio;
    auto mat_camera = camera.canonical_to_camera();
    auto mat_persp = camera.get_perspective();

    //auto time = glfwGetTime();
    //auto t = car_node_1->local_transform();
    //t.rotation.y = glm::radians(time) * 32;
    //car_node_1->set_transform(t);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    gui_camera_window(camera);
    gui_hierarchy_window(scene.root());
    gui_draw_inspector();
    
    pipeline_object.bind();
    pipeline_object.set_active_program(program_fragment_object);
    program_fragment_object.set_uniform_vector3f(program_fragment_object.get_uniform_location("u_camera_eye"), &camera.eye[0]); 
    
    pipeline_object.set_active_program(program_vertex_object);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_cam"), &mat_camera[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_per"), &mat_persp[0][0]);
    scene.render(program_vertex_object);
    
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
