#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <print>
#include <array>

#include "basic_types.hpp"
#include "glm/fwd.hpp"
#include "pipeline.hpp"
#include "buffer.hpp"
#include "vertex_array.hpp"
#include "camera.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/trigonometric.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static constexpr auto WINDOW_W = 640;
static constexpr auto WINDOW_H = 480;
static auto aspect_ratio = (f32)WINDOW_W / (f32)WINDOW_H;

static auto program_vertex_object = ShaderProgram{};
static auto program_fragment_object = ShaderProgram{};
static auto pipeline_object = ProgramPipelineObject{};

void create_pipeline_object()
{
  auto shaders_dir = std::filesystem::current_path() / "shaders";
  
  auto vertex_shader_obj = ShaderObject{};
  vertex_shader_obj.create(ShaderStage::Vertex);
  vertex_shader_obj.load_source_code(shaders_dir / "basic_shader.vert.glsl");
  vertex_shader_obj.compile();
  auto status = vertex_shader_obj.check_compile_status();
  if(!status) 
  	std::println("Shader compilation error: {}", vertex_shader_obj.get_compile_log());
  
  auto fragment_shader_obj = ShaderObject{};
  fragment_shader_obj.create(ShaderStage::Vertex);
  fragment_shader_obj.load_source_code(shaders_dir / "basic_shader.vert.glsl");
  fragment_shader_obj.compile();
  status = fragment_shader_obj.check_compile_status();
  if(!status)
    std::println("Shader compilation error: {}", fragment_shader_obj.get_compile_log());
  
  program_vertex_object = ShaderProgram{};
  program_vertex_object.create();
  program_vertex_object.attach_shader(vertex_shader_obj);
  program_vertex_object.set_separable(true);
  program_vertex_object.link();
  status = program_vertex_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_vertex_object.get_link_log());
  
  program_vertex_object.detach_shader(vertex_shader_obj);
  
  program_fragment_object = ShaderProgram{};
  program_fragment_object.create();
  program_fragment_object.attach_shader(vertex_shader_obj);
  program_fragment_object.set_separable(true);
  program_fragment_object.link();
  status = program_fragment_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_fragment_object.get_link_log());
   
  program_fragment_object.detach_shader(vertex_shader_obj);
  
  pipeline_object = ProgramPipelineObject{};
  pipeline_object.create();
  pipeline_object.bind_program_stage(PipelineStage::VertexShader, program_vertex_object);
  pipeline_object.bind_program_stage(PipelineStage::FragmentShader, program_fragment_object);
  status = pipeline_object.validate_pipeline();
  if(!status)
   	std::println("pipeline object status: {}", pipeline_object.get_validation_status());
}

glm::mat4 calculate_transformation_matrix(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
{
  auto S = glm::mat3 {
   	scale.x, 0.0f, 0.0f, 
   	0.0f, scale.y, 0.0f, 
   	0.0f, 0.0f, scale.z 
  };
  
  f32 cx = glm::cos(rotation.x);
  f32 sx = glm::sin(rotation.x);
  auto R_x = glm::mat3 {
    1.0f,  0.0f, 0.0f,
    0.0f,  cx,   sx,
    0.0f, -sx,   cx 
  };
  
  f32 cy = glm::cos(rotation.y);
  f32 sy = glm::sin(rotation.y);
  auto R_y = glm::mat3 {
	    cy,  0.0f, -sy,
	    0.0f, 1.0f, 0.0f,
	    sy,  0.0f,  cy
  };
  
  f32 cz = glm::cos(rotation.z);
  f32 sz = glm::sin(rotation.z);
  auto R_z = glm::mat3 {
	    cz,  	sz,  	0.0f,
	   -sz,  	cz,  	0.0f,
	   	0.0f, 0.0f, 1.0f 
  };
  
  auto R = R_z * R_y * R_x;
  auto RS = R * S;
  
  auto M = glm::mat4 {
    glm::vec4(RS[0], 0.0f),
    glm::vec4(RS[1], 0.0f),
    glm::vec4(RS[2], 0.0f),
    glm::vec4(position, 1.0f)    
  }; 
  
  return M;
}

constexpr glm::mat4 calculate_frame_to_canonical()
{
	// let's transform the vertices from local space to global space.
	// The frame-to-canonical matrix takes a point expressed in the local system (e,u,v,w) and converts it into the global system 
	// (o,x,y,z):
	// 
	//         | x_u   x_v   x_w   x_e | | u_p | 
	//         | y_u   y_v   y_w   y_e | | v_p | 
	// P_xyz = | z_u   z_v   z_w   z_e | | w_p | 
	//         | 0     0     0     1   | | 1   | 
	
	constexpr glm::vec3 frame_e = glm::vec3(0.0f, 0.0f, 0.0f); 
	constexpr glm::vec3 frame_u = glm::vec3(1.0f, 0.0f, 0.0f); 
	constexpr glm::vec3 frame_v = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 frame_w = glm::vec3(0.0f, 0.0f, 1.0f);
	constexpr glm::mat4 m_ftc = {
	  glm::vec4(frame_u, 0.0f),
	  glm::vec4(frame_v, 0.0f),
	  glm::vec4(frame_w, 0.0f),
	  glm::vec4(frame_e, 1.0f)
	};
	return m_ftc;
}

int main() 
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Proto engine", NULL, NULL);
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSetFramebufferSizeCallback(window, []([[ maybe_unused ]]GLFWwindow* window, int width, int height){ 
    glViewport(0, 0, width, height); 
    aspect_ratio = static_cast<f32>(width) / static_cast<f32>(height);
  });
  glViewport(0, 0, WINDOW_W, WINDOW_H);
  
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsLight();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
	
  // Let's define the pipeline
  create_pipeline_object();

  // let's define our vertices in local space
  constexpr auto n_vertices = 8;
  constexpr auto vertices = std::array<f32, 3 * n_vertices>{
    -0.5f, -0.5f,  0.5f, 
     0.5f, -0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f, 
    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
     0.5f,  0.5f, -0.5f, 
    -0.5f,  0.5f, -0.5f
  };
  constexpr auto indices = std::array<u32, 36>{
    0, 1, 2,   2, 3, 0,
    1, 5, 6,   6, 2, 1,
    5, 4, 7,   7, 6, 5,
    4, 0, 3,   3, 7, 4,
    3, 2, 6,   6, 7, 3,
    4, 5, 1,   1, 0, 4
  };
  
  auto vbo = Buffer{};
  vbo.create();
  vbo.allocate_storage(sizeof(vertices), vertices.data(), BufferUsageFlags::DynamicStorage);
  
  auto ibo = Buffer{};
  ibo.create();
  ibo.allocate_storage(sizeof(indices), indices.data(), BufferUsageFlags::DynamicStorage);

  auto vao = VerteArray{};
  vao.create();
  vao.enable_attrib(0);
  vao.set_attrib_format_float(0, 3, VertexAttribType::Float, false, 0);  
  vao.attach_vertex_buffer(0, vbo.id(), 0, 3 * sizeof(f32));
  vao.link_attrib(0, 0);
  vao.attach_index_buffer(ibo.id());
  
  auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);
  
  auto cube_position = glm::vec3(0.f);
  auto cube_rotation = glm::vec3(0.f);
  auto cube_scale = glm::vec3(1.f);
  
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::Begin("Camera properties", nullptr);
    auto fov = glm::degrees(camera.fovy); 
    ImGui::DragFloat("Camera vertical FOV", &fov, 1.0f, 30.0f, 120.0f);
    camera.fovy = glm::radians(fov);
    ImGui::End();
    
    //cube_scale.x = glm::abs(glm::sin(glfwGetTime()));
    cube_rotation.z = glm::radians(glfwGetTime()) * 32;
    
    auto mat_transform = calculate_transformation_matrix(cube_position, cube_scale, cube_rotation);
    constexpr auto mat_ftc = calculate_frame_to_canonical();
    camera.aspect = aspect_ratio;
    auto mat_camera = camera.canonical_to_camera();
    auto mat_persp = camera.get_perspective();
 
    pipeline_object.bind();
    pipeline_object.set_active_program(program_vertex_object);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_transform"), &mat_transform[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_ftc"), &mat_ftc[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_cam"), &mat_camera[0][0]);
    program_vertex_object.set_uniform_mat4f(program_vertex_object.get_uniform_location("mat_per"), &mat_persp[0][0]);
    
    vao.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
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