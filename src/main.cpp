#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <print>

#include "basic_types.hpp"
#include "shader.hpp"

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
  
  // let's define our vertices in screen space
  constexpr f32 vertices[] = {
    -0.5f, -0.5f, 0.0f, // bottom left
     0.5f, -0.5f, 0.0f,  // bottom right
     0.0f,  0.5f, 0.0f,  // top center
  };
  
  u32 vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);
  
  // ========================
  auto vertex_shader_obj = Shader{};
  vertex_shader_obj.create(ShaderType::Vertex);
  vertex_shader_obj.load_source_code(SHADERS_DIR / "basic_shader.vert.glsl");
  vertex_shader_obj.compile();
  auto status = vertex_shader_obj.check_compile_status();
  if(!status)
  {
    auto log = vertex_shader_obj.get_compile_log();
    std::println("Shader compilation error: {}", log);
  }
  
  auto fragment_shader_obj = Shader{};
  fragment_shader_obj.create(ShaderType::Vertex);
  fragment_shader_obj.load_source_code(SHADERS_DIR / "basic_shader.vert.glsl");
  fragment_shader_obj.compile();
  status = fragment_shader_obj.check_compile_status();
  if(!status)
  {
    auto log = fragment_shader_obj.get_compile_log();
    std::println("Shader compilation error: {}", log);
  }
  
  auto program = ShaderProgram{};
  program.create();
  program.attach_shader(vertex_shader_obj);
  program.attach_shader(fragment_shader_obj);
  program.link();
  status = program.check_link_status();
  if(!status)
  {
    auto log = program.get_link_log();
    std::println("Link status: {}", log);
  }
  
  // Always detach shaders after a successful link.
  program.detach_shader(vertex_shader_obj);
  program.detach_shader(fragment_shader_obj); 
  // ========================
  
  
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