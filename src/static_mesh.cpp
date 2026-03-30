#include "static_mesh.hpp"

#include <cstddef>
#include <stdexcept>

StaticMesh::StaticMesh(Vertex* vertices, u32 nr_vertices, u32* indices, u32 nr_indices) : 
	m_nr_vertices{ nr_vertices },
	m_nr_indices{ nr_indices }
{
	if(nr_vertices == 0 || nr_vertices == static_cast<u32>(-1))
		throw std::runtime_error("Invalid value of nr_vertices");
	
	m_vbo.create();
	m_vbo.allocate_storage(nr_vertices * sizeof(Vertex), vertices, BufferUsageFlags::DynamicStorage);
	
	if(nr_indices != 0 && indices != nullptr)
	{
		m_ibo.create();
		m_ibo.allocate_storage(nr_indices * sizeof(u32), indices, BufferUsageFlags::DynamicStorage);	
	}	
	
	m_vao.create();
  // Attribute 0: position(xyz)
  m_vao.set_attrib_format_float(0, 3, VertexAttribType::Float, false, offsetof(Vertex, position));
  // Attribute 1: normal(x,y,z)
  m_vao.set_attrib_format_float(1, 3, VertexAttribType::Float, false, offsetof(Vertex, normal));
  // Attribute 2: texcoord(uv)
  m_vao.set_attrib_format_float(2, 2, VertexAttribType::Float, false, offsetof(Vertex, texcoord));
  // Attribute 3: tangent(x,y,z)
  m_vao.set_attrib_format_float(3, 3, VertexAttribType::Float, false, offsetof(Vertex, tangent));
  m_vao.attach_vertex_buffer(0, m_vbo, 0, sizeof(Vertex));
  
  m_vao.link_attrib(0, 0);
  m_vao.link_attrib(1, 0); 
  m_vao.link_attrib(2, 0); 
  m_vao.link_attrib(3, 0); 
  m_vao.enable_attrib(0);
  m_vao.enable_attrib(1);
  m_vao.enable_attrib(2);
  m_vao.enable_attrib(3);
  
  if(m_ibo.is_valid())
  	m_vao.attach_index_buffer(m_ibo);
}

StaticMesh::~StaticMesh()
{
	m_vbo.destroy();
	m_ibo.destroy();
	m_vao.destroy();
}