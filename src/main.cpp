#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <array>
#include <vector>
#include <filesystem>
#include <memory>
#include <print>

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

static constexpr auto window_w = 640;
static constexpr auto window_h = 480;
auto aspect_ratio = static_cast<f32>(window_w) / static_cast<f32>(window_h);

static const auto models_dir = std::filesystem::current_path() / "models";
static const auto shaders_dir = std::filesystem::current_path() / "shaders";

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
	auto window = init_glfw_context(window_w, window_h, "ProtoEngine");
	
  auto vertex_shader = create_shader_object(shaders_dir / "shader.vert.glsl", ShaderStage::Vertex);
  auto vertex_program = create_shader_program(vertex_shader);
  // auto base_color_shader = create_shader_object(shaders_dir / "base_color.frag.glsl", ShaderStage::Fragment);
  // auto base_color_program = create_shader_program(base_color_shader);
  // auto normal_color_shader = create_shader_object(shaders_dir / "normal_color.frag.glsl", ShaderStage::Fragment);
  // auto normal_color_program = create_shader_program(normal_color_shader);
  auto pbr_shader = create_shader_object(shaders_dir / "pbr.frag.glsl", ShaderStage::Fragment);
  auto pbr_program = create_shader_program(pbr_shader);
  
  auto current_fragment_program = pbr_program;
  auto pipeline_object = create_pipeline_object(vertex_program, current_fragment_program);

 	auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);
  camera.eye.z = 1.0f;

  // let's define the scene graph hierarchy
  
  auto scene = Scene{};
  auto world_node = scene.create_node("World");
  auto mesh_node = scene.create_node("Mesh");
  // auto floor_node = scene.create_node("Floor");
  auto light_node = scene.create_node("Light source");

  light_node->set_light(LightInstance{ 
    .color=glm::vec3{ 1.0f }, 
    .power=10.0f});
  light_node->set_transform(Transformation{ 
    .position=glm::vec3{ 0.0f, 0.5f, 0.5f }});
  
  scene.set_root(world_node);
  world_node->add_child(mesh_node);
  world_node->add_child(light_node);
  // world_node->add_child(floor_node);
  
  auto model_info = import_model(models_dir / "murble_bust/marble_bust_01_1k.gltf");
  auto model = std::make_unique<StaticMesh>(
    model_info.vertices.data(), model_info.vertices.size(),
    model_info.indices.data(), model_info.indices.size());
  mesh_node->set_mesh(MeshInstance{ model.get(), model_info.material } );
  
  // auto floor_mesh = create_floor_mesh();
  // floor_node->set_mesh(MeshInstance{floor_mesh.get(), Material{
  //   .surface_color=glm::vec3{ 0.5f }, .tex_diffuse={}, .tex_normal={}}});

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback([](
    [[maybe_unused]] GLenum source, 
    [[maybe_unused]] GLenum type,
    [[maybe_unused]] GLuint id,
    [[maybe_unused]] GLenum severity,
    [[maybe_unused]] GLsizei length,
    [[maybe_unused]] const GLchar* message,
    [[maybe_unused]] const void* userParam) 
  {
    std::println("OpenGL Debug: {}", message);
  }, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  
  glEnable(GL_DEPTH_TEST);  	// enable depth testing
  glDepthFunc(GL_LESS);    	// specify the value used for depth buffer comparisons
  glDepthMask(GL_TRUE);    	// enable/disable writing into the depth buffer
  glClearDepthf(1.0f);       	// specify the clear value for the depth buffer
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // specify the clear value for the color buffer

  // auto albedo = glm::vec3(1.0f, 1.0f, 1.0f);
  // auto metallic = 0.0f;
  // auto roughness = 0.5f;
  
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

    start_imgui_frame();

    gui_camera(camera);
    gui_hierarchy(scene.root());
    gui_inspector();

    pipeline_object.bind();
    pipeline_object.set_active_program(vertex_program);
    vertex_program.set_uniform_mat4f(ShaderLocation::Vertex::MatCam, mat_camera);
    vertex_program.set_uniform_mat4f(ShaderLocation::Vertex::MatPer, mat_persp);
  
    pipeline_object.set_active_program(current_fragment_program);
    current_fragment_program.set_uniform_vector3f(ShaderLocation::Fragment::CameraEye, camera.eye);
    scene.render(pipeline_object,
                 vertex_program,
                 current_fragment_program);

    imgui_render();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
