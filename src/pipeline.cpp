#include "pipeline.hpp"

#include <glad/gl.h>

#include <print>
#include <format>
#include <filesystem>
#include <fstream>

// ====================================
//            Shader
// ====================================

bool ShaderObject::is_valid() const
{
  return m_id != 0 && glIsShader(m_id) == GL_TRUE;
}

void ShaderObject::create(ShaderStage stage)
{
  m_id = glCreateShader(static_cast<i32>(stage));
  if(!is_valid())
    std::println("Error on creating shader object (id={})", m_id);
}

void ShaderObject::destroy()
{
  glDeleteShader(m_id);
  m_id = 0;
}

void ShaderObject::load_source_code(const std::filesystem::path& glsl_file) const
{
  if(!std::filesystem::exists(glsl_file))
  {
    std::println("File does not exist: {}", glsl_file.string());
    return;
  }
  auto file = std::ifstream(glsl_file.string());
  if(!file.is_open())
  {
    std::println("Failed to open file: {}", glsl_file.string());
    return;
  }
  
  auto source_code = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  auto data = source_code.c_str();
  glShaderSource(m_id, 1, &data, nullptr);
}

void ShaderObject::compile() const
{
  glCompileShader(m_id);
}

bool ShaderObject::check_compile_status() const
{
  auto status = 0;
  glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);
  return status == GL_TRUE;
}

std::string ShaderObject::get_compile_log() const
{
  auto log_length = 0;
  glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &log_length);
  if(log_length == 0)
    return "";
  
  auto log = std::string(log_length, '\0');
  glGetShaderInfoLog(m_id, log_length, nullptr, log.data());
  return log;
}

// ====================================
//            ShaderProgram
// ====================================

bool ShaderProgram::is_valid() const
{
	return m_id != 0 && glIsProgram(m_id) == GL_TRUE;
}

void ShaderProgram::create()
{
  m_id = glCreateProgram();
  if(!is_valid())
    std::println("Error on creating shader program ({})", m_id);
}

void ShaderProgram::destroy()
{
  glDeleteProgram(m_id);
  m_id = 0;
}

void ShaderProgram::attach_shader(ShaderObject shader) const
{
  glAttachShader(m_id, shader.id());
}

void ShaderProgram::detach_shader(ShaderObject shader) const
{
  glDetachShader(m_id, shader.id());
}

void ShaderProgram::link() const
{
  glLinkProgram(m_id);
}

bool ShaderProgram::check_link_status() const
{
  auto link_status = 0;
  glGetProgramiv(m_id, GL_LINK_STATUS, (int *)&link_status);
  return link_status == GL_TRUE;
}

std::string ShaderProgram::get_link_log() const
{
 	auto length = 0;
  glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &length);

  auto log = std::string(length, '\0');
	glGetProgramInfoLog(m_id, length, &length, log.data());
	return log;
}

void ShaderProgram::use() const
{
  glUseProgram(m_id);
}

void ShaderProgram::set_separable(bool flag) const
{
  glProgramParameteri(m_id, GL_PROGRAM_SEPARABLE, flag ? GL_TRUE : GL_FALSE);
}

void ShaderProgram::get_uniform_value_f32(i32 location, i64 size, f32* value) const
{
  glGetnUniformfv(m_id, location, size, value);
}

void ShaderProgram::get_uniform_value_i32(i32 location, i64 size, i32* value) const
{
  glGetnUniformiv(m_id, location, size, value);
}

i32 ShaderProgram::get_uniform_location(std::string_view name) const
{
  return glGetUniformLocation(m_id, name.data());
}

void ShaderProgram::set_uniform_f32(i32 location, f32 value) const
{
  glUniform1f(location, value); 
}

void ShaderProgram::set_uniform_i32(i32 location, i32 value) const
{
  glUniform1i(location, value); 
}

void ShaderProgram::set_uniform_vector3f(i32 location, const glm::vec3& value) const
{
  glUniform3fv(location, 1, &value[0]);
}

void ShaderProgram::set_uniform_mat4f(i32 location, const glm::mat4& value) const
{
  glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}


// ====================================
//            ProgramPipeline
// ====================================

bool ProgramPipelineObject::is_valid() const
{
	return m_id != 0 && glIsProgramPipeline(m_id) == GL_TRUE;
}

void ProgramPipelineObject::create()
{
	glCreateProgramPipelines(1, &m_id);
	if(!is_valid())
		std::println("Error on creating pipeline object ({})", m_id);
}

void ProgramPipelineObject::destroy()
{
	glDeleteProgramPipelines(1, &m_id);
	m_id = 0;
}

void ProgramPipelineObject::bind_program_stage(PipelineStage stage, ShaderProgram program) const
{
	glUseProgramStages(m_id, static_cast<u32>(stage), program.id());
}

void ProgramPipelineObject::bind() const
{
	glBindProgramPipeline(m_id);
}

void ProgramPipelineObject::unbind() const
{
	glBindProgramPipeline(0);
}

bool ProgramPipelineObject::validate_pipeline() const
{
	this->bind();
	
	glValidateProgramPipeline(m_id);
	auto status = 0;
	glGetProgramPipelineiv(m_id, GL_VALIDATE_STATUS, &status);
	
	this->unbind();
	
	return status == GL_TRUE;
}

std::string ProgramPipelineObject::get_validation_status() const
{
	auto log_len = 0;
  glGetProgramPipelineiv(m_id, GL_INFO_LOG_LENGTH, &log_len);
  if(log_len > 0)
  {
  	std::string log(log_len, '\0');
   	glGetProgramPipelineInfoLog(m_id, log_len, NULL, log.data());
    return log;
  }
  
  return "";
}

void ProgramPipelineObject::set_active_program(ShaderProgram program) const
{
	glActiveShaderProgram(m_id, program.id());
}



ShaderObject create_shader_object(const std::filesystem::path& filepath, ShaderStage stage)
{
  auto shader_object = ShaderObject{};
  shader_object.create(stage);
  shader_object.load_source_code(filepath.string());
  shader_object.compile();
  if (!shader_object.check_compile_status())
    throw std::runtime_error(std::format("Shader compilation error: {}", shader_object.get_compile_log()));
  std::println("Shader compiled successfully");
  return shader_object;
}

ShaderProgram create_shader_program(ShaderObject shader_obj)
{
  auto program = ShaderProgram{};
  program.create();
  program.attach_shader(shader_obj);
  program.set_separable(true);
  program.link();
  if (!program.check_link_status())
    throw std::runtime_error(std::format("Error during the linking: {}", program.get_link_log()));
  
  program.detach_shader(shader_obj);
  std::println("Shader program created successfully");
  return program;
}

ProgramPipelineObject create_pipeline_object(ShaderProgram vertex_program, ShaderProgram fragment_program)
{
  auto pipeline = ProgramPipelineObject{};
  pipeline.create();
  pipeline.bind_program_stage(PipelineStage::VertexShader, vertex_program);
  pipeline.bind_program_stage(PipelineStage::FragmentShader, fragment_program);
  if (!pipeline.validate_pipeline())
    throw std::runtime_error(std::format("Error during the validation: {}", pipeline.get_validation_status()));
  
  return pipeline;
}
