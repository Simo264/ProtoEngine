#include <array>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <memory>

#include "basic_types.hpp"
#include "render_types.hpp"
#include "pipeline.hpp"
#include "static_mesh.hpp"
#include "camera.hpp"
#include "scene_graph.hpp"
#include "static_mesh.hpp"

#include "io/importer.hpp"

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
#include <vector>

static constexpr auto WINDOW_W = 640;
static constexpr auto WINDOW_H = 480;
static auto aspect_ratio = static_cast<f32>(WINDOW_W) / static_cast<f32>(WINDOW_H);

static const auto models_dir = std::filesystem::current_path() / "models";
static const auto shaders_dir = std::filesystem::current_path() / "shaders";

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

static auto create_floor_mesh() 
{
  constexpr auto size = 10.0f;
  constexpr auto half_size = size / 2.0f;
  constexpr auto uv_scale = 5.0f;

  constexpr auto vertices = std::array<Vertex, 4>{
    Vertex{{-half_size, 0.0f, -half_size}, {0.0f, 1.0f, 0.0f}, {0.0f, uv_scale},     {1.0f, 0.0f, 0.0f}}, // 0: Alto-SX
    Vertex{{ half_size, 0.0f, -half_size}, {0.0f, 1.0f, 0.0f}, {uv_scale, uv_scale}, {1.0f, 0.0f, 0.0f}}, // 1: Alto-DX
    Vertex{{ half_size, 0.0f,  half_size}, {0.0f, 1.0f, 0.0f}, {uv_scale, 0.0f},     {1.0f, 0.0f, 0.0f}}, // 2: Basso-DX
    Vertex{{-half_size, 0.0f,  half_size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f},         {1.0f, 0.0f, 0.0f}}  // 3: Basso-SX
  };

  constexpr auto indices = std::array<u32, 6>{
    0, 3, 1,
    1, 3, 2
  };

  return std::make_unique<StaticMesh>(
    vertices.data(), vertices.size(),
    indices.data(), indices.size()
  );
}

int main() 
{ 
	auto window = init_context();
	
  auto vertex_shader = create_shader_object(shaders_dir / "shader.vert.glsl", ShaderStage::Vertex);
  auto vertex_program = create_shader_program(vertex_shader);
  //auto base_color_shader = create_shader_object(shaders_dir / "base_color.frag.glsl", ShaderStage::Fragment);
  //auto base_color_program = create_shader_program(base_color_shader);
  //auto normal_color_shader = create_shader_object(shaders_dir / "normal_color.frag.glsl", ShaderStage::Fragment);
  //auto normal_color_program = create_shader_program(normal_color_shader);
  auto lighting_shader = create_shader_object(shaders_dir / "lighting.frag.glsl", ShaderStage::Fragment);
  auto lighting_program = create_shader_program(lighting_shader);

  auto current_fragment_program = lighting_program;
  auto pipeline_object = create_pipeline_object(vertex_program, current_fragment_program);

 	auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);
  camera.eye.z = 1.0f;

  // let's define the scene graph hierarchy
  
  auto scene = Scene{};
  auto world_node = scene.create_node("World");

  auto lion_head_node = scene.create_node("Lion head");
  auto floor_node = scene.create_node("Floor");
  auto light_node = scene.create_node("Light source");

  light_node->set_light(LightInstance{ .color=glm::vec3{ 1.0f }, .power=10.0f});
  light_node->set_transform(Transformation{ .position=glm::vec3{ 0.0f, 0.5f, 0.5f }});
  
  scene.set_root(world_node);
  world_node->add_child(lion_head_node);
  world_node->add_child(light_node);
  world_node->add_child(floor_node);
  
  auto lion_model = import_model(models_dir / "lion_head/lion_head_1k.gltf");
  auto lion_mesh = std::make_unique<StaticMesh>(
    lion_model.vertices.data(), lion_model.vertices.size(),
    lion_model.indices.data(), lion_model.indices.size());
  lion_head_node->set_mesh(MeshInstance{lion_mesh.get(), lion_model.material } );

  auto floor_mesh = create_floor_mesh();
  floor_node->set_mesh(MeshInstance{floor_mesh.get(), Material{}});

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

    gui_camera(camera);
    gui_hierarchy(scene.root());
    gui_inspector();
    
    scene.render(pipeline_object, 
                 vertex_program, 
                 current_fragment_program, 
                 camera.eye, 
                 mat_camera, 
                 mat_persp);
    
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
