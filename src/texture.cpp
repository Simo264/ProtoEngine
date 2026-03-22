#include "texture.hpp"

#include <glad/gl.h>
#include <print>

bool Texture::is_valid() const
{
  return m_id != 0 && glIsTexture(m_id) == GL_TRUE;
}

void Texture::create(TextureType type)
{  
  glCreateTextures(static_cast<u32>(type), 1, &m_id);
  if (!is_valid())
    std::println("Failed to create texture (id={})", m_id);
}

void Texture::destroy()
{
  if (is_valid())
  {
    glDeleteTextures(1, &m_id);
    m_id = 0;
  }
}

void Texture::set_storage_tex2D(i32 levels, TextureImageFormat image_format, i32 width, i32 height)
{
  glTextureStorage2D(m_id, levels, static_cast<u32>(image_format), width, height);
  m_image_format = image_format;
}

void Texture::update_content_tex2D(i32 level, 
                                   i32 xoffset, 
                                   i32 yoffset, 
                                   i32 width, 
                                   i32 height, 
                                   PixelDataFormat format, 
                                   PixelDataType type, 
                                   const void* data) const
{
  glTextureSubImage2D(m_id, level, xoffset, yoffset, width, height, static_cast<u32>(format), static_cast<u32>(type), data);
}

void Texture::set_wrap_mode(TextureWrapMode wrap_s, TextureWrapMode  wrap_t) const
{
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, static_cast<u32>(wrap_s));
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, static_cast<u32>(wrap_t));
}

void Texture::set_border_color(f32 r, f32 g, f32 b, f32 a) const
{
  f32 color[] = { r, g, b, a };
  glTextureParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, color);
}

void Texture::set_magnification_filter(TextureFilteringMode filter) const
{
  // If the filter is not GL_NEAREST or GL_LINEAR, the texture will not be filtered.  
  // The default value is GL_LINEAR.
   
  if(filter == TextureFilteringMode::Nearest || filter == TextureFilteringMode::Linear)
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, static_cast<u32>(filter));
  else
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, static_cast<u32>(TextureFilteringMode::Linear));
}

void Texture::set_minification_filter(TextureFilteringMode filter) const
{
  glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, static_cast<u32>(filter));
}

void Texture::generate_mipmaps() const
{
  glGenerateTextureMipmap(m_id);
}

void Texture::bind_texture_unit(u32 unit) const
{
  glBindTextureUnit(unit, m_id);
}