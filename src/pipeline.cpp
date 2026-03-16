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
    std::println("Error on creating shader object ({})", m_id);
  else
  	std::println("Created new shader object ({})", m_id);
}

void ShaderObject::destroy()
{
  if(!is_valid())
  {
    std::println("Cannot delete an invalid shader object ({})", m_id);
    return;
  }
  
  glDeleteShader(m_id);
  m_id = 0;
}

void ShaderObject::load_source_code(const std::filesystem::path& glsl_file) const
{
  if(!is_valid())
  {
    std::println("Cannot load source code on an invalid shader object({})", m_id);
    return;
  }
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
  if(!is_valid())
  {
    std::println("Cannot compile an invalid shader object ({})", m_id);
    return;
  }
  glCompileShader(m_id);
}

bool ShaderObject::check_compile_status() const
{
  if(!is_valid())
    return false;
  
  auto status = 0;
  glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);
  return status == GL_TRUE;
}

std::string ShaderObject::get_compile_log() const
{
  if(!is_valid())
    return std::format("Invalid shader object ({})", m_id);
  
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
  if(!is_valid())
  {
    std::println("Cannot delete an invalid program object ({})", m_id);
    return;
  }
  
  glDeleteProgram(m_id);
  m_id = 0;
}

void ShaderProgram::attach_shader(ShaderObject shader) const
{
  if(!is_valid())
  {
    std::println("Cannot attach shader on invalid shader program object ({})", m_id);
    return;
  }
  glAttachShader(m_id, shader.id());
}

void ShaderProgram::detach_shader(ShaderObject shader) const
{
  if(!is_valid())
  {
    std::println("Cannot detach shader on invalid shader program object ({})", m_id);
    return;
  }
    
  glDetachShader(m_id, shader.id());
}

void ShaderProgram::link() const
{
  if(!is_valid())
  {
    std::println("Cannot link an invalid shader program object ({})", m_id);
    return;
  }
  
  glLinkProgram(m_id);
}

bool ShaderProgram::check_link_status() const
{
  if(!is_valid())
    return false;
  
  auto link_status = 0;
  glGetProgramiv(m_id, GL_LINK_STATUS, (int *)&link_status);
  return link_status == GL_TRUE;
}

std::string ShaderProgram::get_link_log() const
{
  if(!is_valid())
    return std::format("Invalid shader program object ({})", m_id);
  
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
 if(!is_valid())
  {
    std::println("Cannot specify any parameter on an invalid shader program object ({})", m_id);
    return;
  }
 
 glProgramParameteri(m_id, GL_PROGRAM_SEPARABLE, flag ? GL_TRUE : GL_FALSE);
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
	else
		std::println("Created new pipeline object ({})", m_id);
}

void ProgramPipelineObject::destroy()
{
	if(!is_valid())
	{
		std::println("Cannot delete an invalid pipeline object ({})", m_id);
		return;
	}

	glDeleteProgramPipelines(1, &m_id);
	m_id = 0;
}

void ProgramPipelineObject::bind_program_stage(PipelineStage stage, ShaderProgram program) const
{
	if(!is_valid())
	{
		std::println("Cannot bind a stage on an invalid pipeline object ({})", m_id);
		return;
	}
	
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
	if(!is_valid()) 
		return false;
	
	this->bind();
	
	glValidateProgramPipeline(m_id);
	auto status = 0;
	glGetProgramPipelineiv(m_id, GL_VALIDATE_STATUS, &status);
	
	this->unbind();
	
	return status == GL_TRUE;
}

std::string ProgramPipelineObject::get_validation_status() const
{
	if(!is_valid()) 
		return std::format("Invalid pipeline object ({})", m_id);
	
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
	if(!is_valid())
	{
		std::println("Cannot set on an invalid pipeline object ({})", m_id);
		return;
	}
	
	glActiveShaderProgram(m_id, program.id());
}

