#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <memory>
#include <print>

#include "assimp/material.h"
#include "assimp/types.h"
#include "basic_types.hpp"
#include "camera.hpp"
#include "pipeline.hpp"
#include "scene_graph.hpp"
#include "static_mesh.hpp"
#include "texture.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure

static constexpr auto WINDOW_W = 640;
static constexpr auto WINDOW_H = 480;
static auto aspect_ratio =
    static_cast<f32>(WINDOW_W) / static_cast<f32>(WINDOW_H);

static auto program_vertex_object = ShaderProgram{};
static auto program_fragment_object = ShaderProgram{};
static auto pipeline_object = ProgramPipelineObject{};

static auto texture_color = Texture{};
static auto texture_normal = Texture{};

static auto init_context() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  auto window =
      glfwCreateWindow(WINDOW_W, WINDOW_H, "Proto engine", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSetFramebufferSizeCallback(
      window, []([[maybe_unused]] GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        aspect_ratio = static_cast<f32>(width) / static_cast<f32>(height);
      });
  glViewport(0, 0, WINDOW_W, WINDOW_H);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;           // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  return window;
}

static void create_pipeline_object() {
  auto shaders_dir = std::filesystem::current_path() / "shaders";

  auto vertex_shader_obj = ShaderObject{};
  vertex_shader_obj.create(ShaderStage::Vertex);
  vertex_shader_obj.load_source_code(shaders_dir / "basic_shader.vert.glsl");
  vertex_shader_obj.compile();
  auto status = vertex_shader_obj.check_compile_status();
  if (!status)
    std::println("Shader compilation error: {}",
                 vertex_shader_obj.get_compile_log());

  auto fragment_shader_obj = ShaderObject{};
  fragment_shader_obj.create(ShaderStage::Fragment);
  fragment_shader_obj.load_source_code(shaders_dir / "basic_shader.frag.glsl");
  fragment_shader_obj.compile();
  status = fragment_shader_obj.check_compile_status();
  if (!status)
    std::println("Shader compilation error: {}",
                 fragment_shader_obj.get_compile_log());

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
  pipeline_object.bind_program_stage(PipelineStage::VertexShader,
                                     program_vertex_object);
  pipeline_object.bind_program_stage(PipelineStage::FragmentShader,
                                     program_fragment_object);
  status = pipeline_object.validate_pipeline();
  if (!status)
    std::println("pipeline object status: {}",
                 pipeline_object.get_validation_status());
}

static void handle_camera_input(auto window, auto &camera) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    camera.rotate_pitch(+glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    camera.rotate_pitch(-glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    camera.rotate_yaw(+glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    camera.rotate_yaw(-glm::radians(1.0f));
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.eye += camera.gaze() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.eye -= camera.gaze() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.eye -= camera.right() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.eye += camera.right() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.eye += camera.up() * 0.1f;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera.eye -= camera.up() * 0.1f;
}

#if 0
static Texture create_texture_color_from_file(const std::filesystem::path& filepath)
{
 	if(!std::filesystem::exists(filepath))
    exit(1);

  auto width{ 0 }, height{ 0 }, nr_channels{ 0 };
  auto data = stbi_load(filepath.string().c_str(), &width, &height, &nr_channels, STBI_rgb);
  auto levels = static_cast<u32>(std::floor(std::log2(std::max(width, height))) + 1);  // levels = floor(log_2(max(width, height))) + 1

	auto texture = Texture{};
 	texture.create(TextureType::Texture2D);
  texture.set_storage_tex2D(levels, TextureImageFormat::RGB8, width, height);
  texture.update_content_tex2D(0, 0, 0, width, height, PixelDataFormat::RGB, PixelDataType::UnsignedByte, data);
  stbi_image_free(data);

  texture.set_wrap_mode(TextureWrapMode::Repeat, TextureWrapMode::Repeat);
  texture.set_magnification_filter(TextureFilteringMode::Linear);
  texture.set_minification_filter(TextureFilteringMode::LinearMipmapLinear);
  texture.generate_mipmaps();
  return texture;
}

static Texture create_texture_normal_from_file(const std::filesystem::path& filepath)
{
  if(!std::filesystem::exists(filepath))
    exit(1);

  auto width{ 0 }, height{ 0 }, nr_channels{ 0 };
  auto data = stbi_load(filepath.string().c_str(), &width, &height, &nr_channels, STBI_rgb);
  auto levels = static_cast<u32>(std::floor(std::log2(std::max(width, height))) + 1);  // levels = floor(log_2(max(width, height))) + 1

  auto texture = Texture{};
  texture.create(TextureType::Texture2D);
  texture.set_storage_tex2D(levels, TextureImageFormat::RGB8, width, height);
  texture.update_content_tex2D(0, 0, 0, width, height, PixelDataFormat::RGB, PixelDataType::UnsignedByte, data);
  stbi_image_free(data);

  texture.set_wrap_mode(TextureWrapMode::Repeat, TextureWrapMode::Repeat);
  texture.set_magnification_filter(TextureFilteringMode::Linear);
  texture.set_minification_filter(TextureFilteringMode::LinearMipmapLinear);
  texture.generate_mipmaps();
  return texture;
}
#endif

static auto import_model(const std::filesystem::path &filepath) {
  if (!std::filesystem::exists(filepath))
    exit(1);

  // Create an instance of the Importer class
  auto importer = Assimp::Importer{};
  const auto scene = importer.ReadFile(
      filepath.string().c_str(), aiProcess_CalcTangentSpace |
                                     aiProcess_Triangulate | aiProcess_FlipUVs |
                                     aiProcess_JoinIdenticalVertices);

  // If the import failed, report it
  if (scene == nullptr) {
    std::println("Error on loading scene {}: {}", filepath.string(),
                 importer.GetErrorString());
    exit(1);
  }

  // Now we can access the file's contents.
  std::println("num meshes: {}", scene->mNumMeshes);
  auto aimesh = scene->mMeshes[0];

  // load vertices
  auto vertices = std::vector<Vertex>{};
  vertices.reserve(aimesh->mNumVertices);
  for (auto i = 0u; i < aimesh->mNumVertices; i++) {
    auto &vertex = vertices.emplace_back();
    vertex.position = glm::vec3{aimesh->mVertices[i].x, aimesh->mVertices[i].y,
                                aimesh->mVertices[i].z};
    if (aimesh->HasNormals())
      vertex.normal = glm::vec3{aimesh->mNormals[i].x, aimesh->mNormals[i].y,
                                aimesh->mNormals[i].z};
    if (aimesh->HasTextureCoords(0))
      vertex.texcoord = glm::vec2{aimesh->mTextureCoords[0][i].x,
                                  aimesh->mTextureCoords[0][i].y};
    if (aimesh->HasTangentsAndBitangents())
      vertex.tangent = glm::vec3{
          aimesh->mTangents[i].x,
          aimesh->mTangents[i].y,
          aimesh->mTangents[i].z,
      };
  }

  // load indices
  auto indices = std::vector<u32>{};
  indices.reserve(aimesh->mNumFaces * 3);
  for (auto i = 0u; i < aimesh->mNumFaces; i++) {
    auto face = aimesh->mFaces[i];
    indices.emplace_back(face.mIndices[0]);
    indices.emplace_back(face.mIndices[1]);
    indices.emplace_back(face.mIndices[2]);
  }

  auto material = scene->mMaterials[aimesh->mMaterialIndex];
  std::println("num unkknow texturess: {}",
               material->GetTextureCount(aiTextureType_UNKNOWN));
  std::println("num diffuse textures: {}",
               material->GetTextureCount(aiTextureType_DIFFUSE));
  std::println("num normal textures: {}",
               material->GetTextureCount(aiTextureType_NORMALS));

  auto path = aiString{};
  if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
    auto ai_texture = scene->GetEmbeddedTexture(path.C_Str());
    std::println("Texture color found: width={}, height={}", ai_texture->mWidth,
                 ai_texture->mHeight);

    auto width{0}, height{0}, nr_channels{0};
    auto data = stbi_load_from_memory(
        reinterpret_cast<unsigned char *>(ai_texture->pcData),
        ai_texture->mWidth, &width, &height, &nr_channels, STBI_rgb);

    if (!data) {
      std::println("Errore caricamento texture embedded");
      exit(1);
    }

    auto levels =
        static_cast<u32>(std::floor(std::log2(std::max(width, height))) + 1);

    texture_color.create(TextureType::Texture2D);
    texture_color.set_storage_tex2D(levels, TextureImageFormat::RGB8, width,
                                    height);
    texture_color.update_content_tex2D(0, 0, 0, width, height,
                                       PixelDataFormat::RGB,
                                       PixelDataType::UnsignedByte, data);

    texture_color.set_wrap_mode(TextureWrapMode::Repeat,
                                TextureWrapMode::Repeat);
    texture_color.set_magnification_filter(TextureFilteringMode::Linear);
    texture_color.set_minification_filter(
        TextureFilteringMode::LinearMipmapLinear);
    texture_color.generate_mipmaps();
    stbi_image_free(data);
  }
  if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS) {
    std::println("Texture normal found");
    // auto ai_texture = scene->GetEmbeddedTexture(path.C_Str());
  }
  if (material->GetTexture(aiTextureType_UNKNOWN, 0, &path) == AI_SUCCESS) {
    std::println("Unkown texture found");
  }

  return std::shared_ptr<StaticMesh>(new StaticMesh(
      vertices.data(), vertices.size(), indices.data(), indices.size()));
}

static void gui_camera_window(auto &camera) {
  ImGui::Begin("Camera");
  auto fov = glm::degrees(camera.fovy);
  if (ImGui::DragFloat("Vertical FOV", &fov, 0.5f, 30.0f, 120.0f))
    camera.fovy = glm::radians(fov);

  auto euler_angles = glm::degrees(camera.get_euler_angles());
  ImGui::Text("Camera Position: X:%.2f, Y:%.2f, Z:%.2f", camera.eye.x,
              camera.eye.y, camera.eye.z);
  ImGui::BulletText("P: %.2f°  Y: %.2f°  R: %.2f°", euler_angles.x,
                    euler_angles.y, euler_angles.z);
  if (ImGui::Button("Reset Camera")) {
    camera.eye = glm::vec3(0.0f, 0.0f, 5.0f);
    camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    camera.fovy = glm::radians(45.0f);
  }
  ImGui::End();

  // ImGui::Spacing();
  // ImGui::Separator();
  // ImGui::Spacing();
  // if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
  // {
  //   ImGui::DragFloat3("Position", &model_transformation.position[0], 0.1f);
  //   ImGui::DragFloat3("Rotation", &model_transformation.rotation[0], 1.0f);
  //   ImGui::DragFloat3("Scale", &model_transformation.scale[0], 0.05f,
  //   0.1f, 10.0f); if (ImGui::Button("Reset"))
  //   {
  //     model_transformation.position = glm::vec3(0.0f);
  //     model_transformation.rotation = glm::vec3(0.0f);
  //     model_transformation.scale = glm::vec3(1.0f);
  //   }
  // }
}

static SceneNode* selected_node = nullptr;
static void draw_scene_node(SceneNode* node) 
{
  auto flags = node->children().empty() ? ImGuiTreeNodeFlags_Leaf : 0;

  if (node == selected_node)
    flags |= ImGuiTreeNodeFlags_Selected;

  auto open = ImGui::TreeNodeEx((void *)node, flags, "%s", node->name.data());
  if (ImGui::IsItemClicked())
    selected_node = node;

  if (open) 
  {
    for (auto child : node->children())
      draw_scene_node(child);

    ImGui::TreePop();
  }
}

static void draw_inspector() 
{
  ImGui::Begin("Inspector");
  if (selected_node) 
  {
    auto t = selected_node->local_transform();
    auto changed = false;
    changed |= ImGui::DragFloat3("Position", &t.position.x, 0.1f);
    changed |= ImGui::DragFloat3("Rotation", &t.rotation.x, 0.5f);
    changed |= ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
    if (changed) 
      selected_node->set_transform(t);
  }

  ImGui::End();
}

static void gui_hierarchy_window(SceneNode* root) 
{
  ImGui::Begin("Scene graph");
  draw_scene_node(root);
  ImGui::End();
}

int main() 
{ 
	auto window = init_context();
	
	create_pipeline_object();
	
 	auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);

  // let's define the scene graph hierarchy
  auto car_mesh = import_model(std::filesystem::current_path() / "res/models/car.glb");

  auto scene = Scene{};
  auto root_node = scene.create_node("World");
  auto car_node_1 = scene.create_node("Car_1");
  auto car_node_2 = scene.create_node("Car_2");
  root_node->add_child(car_node_1);
  car_node_1->add_child(car_node_2);
  car_node_1->set_transform(Transformation{ .position = { 0.0f, 0.0f, 0.0f }, .scale={ 0.05f,0.05f,0.05f } });
  car_node_2->set_transform(Transformation{ .position = { 0.0f, 0.0f, 100.0f } });
  car_node_1->set_mesh(car_mesh.get());
  car_node_2->set_mesh(car_mesh.get());
  
  glEnable(GL_DEPTH_TEST);  	// enable depth testing
  glDepthFunc(GL_LESS);    	// specify the value used for depth buffer comparisons
  glDepthMask(GL_TRUE);    	// enable/disable writing into the depth buffer
  glClearDepthf(1.0f);       	// specify the clear value for the depth buffer
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // specify the clear value for the color buffer

  while (!glfwWindowShouldClose(window)) 
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffers to preset values

    scene.update();
    
    handle_camera_input(window, camera);
    camera.aspect = aspect_ratio;
    auto mat_camera = camera.canonical_to_camera();
    auto mat_persp = camera.get_perspective();

    // auto time = glfwGetTime();
    // model_transformation.rotation.y = glm::radians(time) * 32;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    gui_camera_window(camera);
    gui_hierarchy_window(root_node);
    draw_inspector();

    auto mat_transform = car_node_1->world_matrix();
    pipeline_object.bind();
    pipeline_object.set_active_program(program_vertex_object);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_transform"), &mat_transform[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_cam"), &mat_camera[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_per"), &mat_persp[0][0]);
    pipeline_object.set_active_program(program_fragment_object);
    program_fragment_object.set_uniform_vector3f(program_fragment_object.get_uniform_location("u_camera_eye"), &camera.eye[0]); 
    texture_color.bind_texture_unit(0);

    car_node_1->mesh()->vao().bind();
    glDrawElements(GL_TRIANGLES, car_node_1->mesh()->nr_indices(), GL_UNSIGNED_INT, 0);
    
    mat_transform = car_node_2->world_matrix();
    pipeline_object.bind();
    pipeline_object.set_active_program(program_vertex_object);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_transform"), & mat_transform[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_cam"), &mat_camera[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_per"), &mat_persp[0][0]);
    pipeline_object.set_active_program(program_fragment_object);
    program_fragment_object.set_uniform_vector3f(program_fragment_object.get_uniform_location("u_camera_eye"), &camera.eye[0]); 
    texture_color.bind_texture_unit(0);
    car_node_2->mesh()->vao().bind();
    glDrawElements(GL_TRIANGLES, car_node_2->mesh()->nr_indices(), GL_UNSIGNED_INT, 0);
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    auto &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
    {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
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
