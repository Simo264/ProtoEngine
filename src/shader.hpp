#pragma once

#include "basic_types.hpp"
#include <filesystem>
#include <string>

using ShaderID = u32;
using ProgramID = u32;

enum class ShaderType : i32
{
  Vertex          = 0x8B31,  // GL_VERTEX_SHADER
  TessControl     = 0x8E88,  // GL_TESS_CONTROL_SHADER
  TessEvaluation  = 0x8E87,  // GL_TESS_EVALUATION_SHADER
  Geometry        = 0x8DD9,  // GL_GEOMETRY_SHADER
  Fragment        = 0x8B30,  // GL_FRAGMENT_SHADER
};

// Example of usage:
// 
// auto shader = Shader{};
// shader.create(ShaderType::Vertex);
// shader.load_source_code(SHADERS_DIR / "basic_shader.vert.glsl");
// shader.compile();
// if(!shader.check_compile_status())
// {
//   auto log = shader.get_compile_log();
//   std::println("Shader compilation error: {}", log);
// }
class Shader 
{
public:
  Shader() : m_id{ 0 } {} // the default value is 0, which is not a valid shader ID
  
  // creates an empty shader object and returns a non-zero value
  void create(ShaderType type);
  // frees the memory and invalidates the name associated with the shader object 
  void destroy();
  // method to load shader source code from a GLSL file
  void load_source_code(const std::filesystem::path& glsl_file) const;
  // method to compile the shader
  void compile() const;
  // method to check the compile status of the shader
  bool check_compile_status() const;
  // method to get the compile log of the shader
  std::string get_compile_log() const;
  // utility method to check if the shader is valid
  bool is_valid() const;
  
  auto id() const { return m_id; };
  
private:
  ShaderID m_id;
};

// Example usage:
// 
// auto program = ShaderProgram{};
// program.create();
// program.attach_shader(vertex_shader_obj);
// program.attach_shader(fragment_shader_obj);
// program.link();
// status = program.check_link_status();
// if(!status)
// {
//   auto log = program.get_link_log();
//   std::println("Link status: {}", log);
// }
// 
// // Always detach shaders after a successful link.
// program.detach_shader(vertex_shader_obj);
// program.detach_shader(fragment_shader_obj);
class ShaderProgram
{
public:
  ShaderProgram() : m_id{ 0 } {} // the default value is 0, which is not a valid program ID
  
  // creates an empty program object and returns a non-zero value
  void create();
  // frees the memory and invalidates the name associated with the program object 
  void destroy();
  // after creating a program, the shader objects you wish to link to it must be attached to the program
  void attach_shader(Shader shader) const;
  // after linking (whether successfully or not), it is a good idea to detach all shader objects from the program
  void detach_shader(Shader shader) const;
  // links the program object 
  void link() const;
  // linking can fail for many reasons. Program link failure can be detected and responded to, in a similar way to shader compilation failure.
  bool check_link_status() const;  
  // returns the link log of the program
  std::string get_link_log() const;
  
  // installs a program object as part of current rendering state
  void use() const;
  
  bool is_valid() const { return m_id != 0; }
  
  auto id() const { return m_id; };
  
private:
  ProgramID m_id;
};


