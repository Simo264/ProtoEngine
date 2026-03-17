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

constexpr auto WINDOW_W = 1080;
constexpr auto WINDOW_H = 720;
auto aspect_ratio = (f32)WINDOW_W / (f32)WINDOW_H;

ProgramPipelineObject create_pipeline_object()
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
  
  auto program_vertex_object = ShaderProgram{};
  program_vertex_object.create();
  program_vertex_object.attach_shader(vertex_shader_obj);
  program_vertex_object.set_separable(true);
  program_vertex_object.link();
  status = program_vertex_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_vertex_object.get_link_log());
  
  program_vertex_object.detach_shader(vertex_shader_obj);
  
  auto program_fragment_object = ShaderProgram{};
  program_fragment_object.create();
  program_fragment_object.attach_shader(vertex_shader_obj);
  program_fragment_object.set_separable(true);
  program_fragment_object.link();
  status = program_fragment_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_fragment_object.get_link_log());
   
  program_fragment_object.detach_shader(vertex_shader_obj);
  
  auto pipeline_object = ProgramPipelineObject{};
  pipeline_object.create();
  pipeline_object.bind_program_stage(PipelineStage::VertexShader, program_vertex_object);
  pipeline_object.bind_program_stage(PipelineStage::FragmentShader, program_fragment_object);
  status = pipeline_object.validate_pipeline();
  if(!status)
   	std::println("pipeline object status: {}", pipeline_object.get_validation_status());
  
  return pipeline_object;
}

void apply_model_transformation(f32* vertices, i32 n_vertices, const glm::mat4& M)
{
 	for (int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*4 + 0];
    auto& y = vertices[i*4 + 1];
    auto& z = vertices[i*4 + 2];
    glm::vec4 p = M * glm::vec4(x, y, z, 1.0f);
    x = p.x;
    y = p.y;
    z = p.z;
  }
}

int main() 
{
  glfwInit();
 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Hello World", NULL, NULL);
  glfwMakeContextCurrent(window);

  gladLoadGL(glfwGetProcAddress);
  
  glfwSetFramebufferSizeCallback(window, []([[ maybe_unused ]]GLFWwindow* window, int width, int height){ 
    glViewport(0, 0, width, height); 
    aspect_ratio = (f32)width / (f32)height;
    init_camera(aspect_ratio);
  });
  glViewport(0, 0, WINDOW_W, WINDOW_H);
  
  init_camera(aspect_ratio);
  
  // Let's define the pipeline
  auto pipeline_object = create_pipeline_object();

  // let's define our vertices in local space
  constexpr auto n_vertices = 8;
  constexpr auto vertices = std::array<f32, 4 * n_vertices>{
    -0.5f, -0.5f,  0.5f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f
  };
  constexpr auto indices = std::array<u32, 36>{
    0, 1, 2,   2, 3, 0,
    1, 5, 6,   6, 2, 1,
    5, 4, 7,   7, 6, 5,
    4, 0, 3,   3, 7, 4,
    3, 2, 6,   6, 7, 3,
    4, 5, 1,   1, 0, 4
  };
  
  auto vertices_buffer = vertices;
  
//  auto print_vertices = [&](const char* label) {
//      printf("\n--- %s ---\n", label);
//      for (int i = 0; i < n_vertices; i++) {
//          printf("v%d: (%.3f, %.3f, %.3f, %.3f)\n", i,
//              vertices_buffer[i*4+0],
//              vertices_buffer[i*4+1],
//              vertices_buffer[i*4+2],
//              vertices_buffer[i*4+3]);
//      }
//  };
  
  frame_to_canonical(vertices_buffer.data(), n_vertices);
  //print_vertices("world space");
  
  canonical_to_camera(vertices_buffer.data(), n_vertices);
  //print_vertices("camera space");
  
  //camera_to_ortho(vertices_buffer.data(), n_vertices);
  camera_to_perspective(vertices_buffer.data(), n_vertices);
  // print_vertices("clip space");

  auto vbo = Buffer{};
  vbo.create();
  vbo.allocate_storage(sizeof(vertices_buffer), vertices_buffer.data(), BufferUsageFlags::DynamicStorage);
  
  auto ibo = Buffer{};
  ibo.create();
  ibo.allocate_storage(sizeof(indices), indices.data(), BufferUsageFlags::DynamicStorage);

  auto vao = VerteArray{};
  vao.create();
  vao.enable_attrib(0);
  vao.set_attrib_format_float(0, 4, VertexAttribType::Float, false, 0);  
  vao.attach_vertex_buffer(0, vbo.id(), 0, 4 * sizeof(f32));
  vao.link_attrib(0, 0);
  vao.attach_index_buffer(ibo.id());
  
  //auto cube_position = glm::vec3(0.f);
  //auto cube_rotation = glm::vec3(0.f); // degrees
  //auto cube_scale = glm::vec3(1.f);
  
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
   
    // vertices_buffer = vertices;
    
    // cube_scale.x = glm::sin(glfwGetTime());
  
    // auto S = glm::mat3 {
    //  	cube_scale.x, 0.0f, 0.0f, 
    //  	0.0f, cube_scale.y, 0.0f, 
    //  	0.0f, 0.0f, cube_scale.z 
    // };
    // 
    // f32 cx = glm::cos(cube_rotation.x);
    // f32 sx = glm::sin(cube_rotation.x);
    // auto R_x = glm::mat3 {
	  //   1.0f, 0.0f, 0.0f,
	  //   0.0f, cx,    sx,
	  //   0.0f, -sx,   cx
    // };
    // 
    // f32 cy = glm::cos(cube_rotation.y);
    // f32 sy = glm::sin(cube_rotation.y);
    // auto R_y = glm::mat3 {
	  //   cy,  0.0f, -sy,
	  //   0.0f, 1.0f, 0.0f,
	  //   sy,  0.0f,  cy
    // };
    // 
    // f32 cz = glm::cos(cube_rotation.z);
    // f32 sz = glm::sin(cube_rotation.z);
    // auto R_z = glm::mat3 {
	  //   cz,  	sz,  	0.0f,
	  //  -sz,  	cz,  	0.0f,
	  //  	0.0f, 0.0f, 1.0f 
    // };
    // 
    // auto R = R_z * R_y * R_x;
    // auto RS = R * S;
    // 
    // auto M = glm::mat4 {
    //   glm::vec4(RS[0], 0.0f),
    //   glm::vec4(RS[1], 0.0f),
    //   glm::vec4(RS[2], 0.0f),
    //   glm::vec4(cube_position, 1.0f)    
    // }; 
    
    // apply_model_transformation(vertices_buffer.data(), n_vertices, M);
    // frame_to_canonical(vertices_buffer.data(), n_vertices);
    // canonical_to_camera(vertices_buffer.data(), n_vertices);
    // apply_persp_projection(vertices_buffer.data(), n_vertices);
    // apply_ortho_projection(vertices_buffer.data(), n_vertices);
    // vbo.update_data(0, vertices_buffer.size(), vertices_buffer.data());
    
    pipeline_object.bind();
    vao.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}