#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <print>

#include "basic_types.hpp"
#include "pipeline.hpp"

auto CURRENT_PATH = std::filesystem::current_path();
auto SHADERS_DIR = CURRENT_PATH / "shaders";

int main() 
{
  glfwInit();
 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  GLFWwindow* window = glfwCreateWindow(1080, 720, "Hello World", NULL, NULL);
  glfwMakeContextCurrent(window);

  gladLoadGL(glfwGetProcAddress);
  
  // Let's define the pipeline
  // ========================
  auto vertex_shader_obj = ShaderObject{};
  vertex_shader_obj.create(ShaderStage::Vertex);
  vertex_shader_obj.load_source_code(SHADERS_DIR / "basic_shader.vert.glsl");
  vertex_shader_obj.compile();
  auto status = vertex_shader_obj.check_compile_status();
  if(!status) 
  	std::println("Shader compilation error: {}", vertex_shader_obj.get_compile_log());
  
  auto fragment_shader_obj = ShaderObject{};
  fragment_shader_obj.create(ShaderStage::Vertex);
  fragment_shader_obj.load_source_code(SHADERS_DIR / "basic_shader.vert.glsl");
  fragment_shader_obj.compile();
  status = fragment_shader_obj.check_compile_status();
  if(!status)
    std::println("Shader compilation error: {}", fragment_shader_obj.get_compile_log());
  
  auto program_vertex_object = ShaderProgram{};
  program_vertex_object.create();
  program_vertex_object.attach_shader(vertex_shader_obj);
  program_vertex_object.link();
  status = program_vertex_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_vertex_object.get_link_log());
  
  program_vertex_object.detach_shader(vertex_shader_obj);
  
  auto program_fragment_object = ShaderProgram{};
  program_fragment_object.create();
  program_fragment_object.attach_shader(vertex_shader_obj);
  program_fragment_object.link();
  status = program_fragment_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_fragment_object.get_link_log());
   
  program_fragment_object.detach_shader(vertex_shader_obj);
  
  auto pipeline_object = ProgramPipelineObject{};
  pipeline_object.create();
  pipeline_object.bind_program_stage(PipelineStage::VertexShader, program_vertex_object);
  pipeline_object.bind_program_stage(PipelineStage::VertexShader, program_fragment_object);
  status = pipeline_object.validate_pipeline();
  if(!status)
   	std::println("pipeline object status: {}", pipeline_object.get_validation_status());
  
  // ========================
 
 
  // let's define our vertices in screen space
  constexpr f32 vertices[] = {
    -0.5f, -0.5f, 0.0f, // bottom left
     0.5f, -0.5f, 0.0f,  // bottom right
     0.0f,  0.5f, 0.0f,  // top center
  };
  
  u32 vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT); 
  
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}