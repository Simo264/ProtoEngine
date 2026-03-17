#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <print>

#include "basic_types.hpp"
#include "glm/fwd.hpp"
#include "pipeline.hpp"
#include "buffer.hpp"
#include "vertex_array.hpp"

#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

constexpr auto WINDOW_W = 1080;
constexpr auto WINDOW_H = 720;

constexpr auto camera_eye   = glm::vec3(0.0f, 0.0f, 6.f);
constexpr auto camera_gaze  = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr auto camera_up    = glm::vec3(0.0f, 1.0f, 0.0f);

constexpr auto l = -0.1f;
constexpr auto r =  0.1f;
constexpr auto b = -0.1f;
constexpr auto t =  0.1f;
constexpr auto n =  1.0f;
constexpr auto f =  100.0f;

constexpr glm::mat4 m_ortho = glm::mat4(
  glm::vec4(2.0f / (r - l), 0.0f, 0.0f, 0.0f),
  glm::vec4(0.0f, 2.0f / (t - b), 0.0f, 0.0f),
  glm::vec4(0.0f, 0.0f, 2.0f / (n - f), 0.0f),
  glm::vec4(-(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), 1.0f)
);
constexpr glm::mat4 P = {
	glm::vec4(n, 0, 0, 0),
  glm::vec4(0, n, 0, 0),
  glm::vec4(0, 0, n + f, -1), 
  glm::vec4(0, 0, f * n, 0)
};
constexpr auto m_per = m_ortho * P;

auto CURRENT_PATH = std::filesystem::current_path();
auto SHADERS_DIR = CURRENT_PATH / "shaders";

ProgramPipelineObject create_pipeline_object()
{
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

void frame_to_canonical(f32* vertices, i32 n_vertices)
{  
  // let's transform the vertices from local space to global space.
  // The frame-to-canonical matrix takes a point expressed in the local system (e,u,v,w) and converts it into the global system 
  // (o,x,y,z):
  // 
  //         | x_u   x_v   x_w   x_e | | u_p | 
  //         | y_u   y_v   y_w   y_e | | v_p | 
  // P_xyz = | z_u   z_v   z_w   z_e | | w_p | 
  //         | 0     0     0     1   | | 1   | 
   
  constexpr glm::vec3 e = glm::vec3(0.0f, 0.0f, 0.0f); 
  constexpr glm::vec3 u = glm::vec3(1.0f, 0.0f, 0.0f); 
  constexpr glm::vec3 v = glm::vec3(0.0f, 1.0f, 0.0f);
  constexpr glm::vec3 w = glm::vec3(0.0f, 0.0f, 1.0f);
  constexpr glm::mat4 ftc = {
    glm::vec4(u, 0.0f),
    glm::vec4(v, 0.0f),
    glm::vec4(w, 0.0f),
    glm::vec4(e, 1.0f)
  };
  
  for(i32 i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*3+ 0];
    auto& y = vertices[i*3+ 1];
    auto& z = vertices[i*3+ 2];
    
    glm::vec3 p_local = glm::vec3(x, y, z);
    glm::vec4 p_world = ftc * glm::vec4(p_local, 1);
    
    x = p_world.x;
    y = p_world.y;
    z = p_world.z;
  }
}

void canonical_to_camera(f32* vertices, i32 n_vertices)
{
  // The camera transformation, that converts points from the canonical coordinate system to camera 
  // coordinates system.
  // Let's denote the eye position e, the gaze direction g and the view-up vector t. 
  // These vectors allow us to construct a coordinate system with origin e and the basis:
  // - w = - (g/||g||)
  // - u = (t x w) / (|| t x w ||)
  // - v = w x u
  glm::vec3 w = -glm::normalize(camera_gaze);
  glm::vec3 u = glm::normalize(glm::cross(camera_up, w));
  glm::vec3 v = glm::cross(w, u);
  
  // We just need to convert these coordinates into into the camera frame coordinate system. 
  // We can do this using the following matrix transformation:
  // 
  //                          | x_u   y_u   z_u   -x_e |
  //         | u v w e |^1    | x_v   y_v   z_v   -y_e |
  // M_cam = | 0 0 0 1 |    = | x_w   y_w   z_w   -z_e |
  //                          | 0     0     0      1   |
  
  glm::mat4 m_cam = {
    glm::vec4(u.x, v.x, w.x, 0.0f),
    glm::vec4(u.y, v.y, w.y, 0.0f),
    glm::vec4(u.z, v.z, w.z, 0.0f),
    glm::vec4(-glm::dot(u, camera_eye), -glm::dot(v, camera_eye), -glm::dot(w, camera_eye), 1.0f)
  };
  
  for(int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*3 + 0];
    auto& y = vertices[i*3 + 1];
    auto& z = vertices[i*3 + 2];
    
    auto p_world = glm::vec4(x, y, z, 1);
    auto p_cam = m_cam * p_world;
    
    x = p_cam.x;
    y = p_cam.y;
    z = p_cam.z; 
  }
}

void apply_persp_projection(f32* vertices, i32 n_vertices)
{
	// In perspective, the further away an object is (z larger), the smaller it should appear on the screen. 
	// Mathematically, this means that the coordinates x and y must be divided by z (proportionally 1/z).
	// 
	// To implement perspective in 3D we use the convention of the camera at the origin looking towards -z. 
	// We define two planes: the near plane (n) and the far plane (f) which are the same as seen in orthographic projection. 
	// The resulting matrix is this:
	// 
  //     | n		0 	0		0 	|
  //     | 0  	n  	0 	0 	|
  // P = | 0  	0  	n+f -fn |
  //     | 0  	0   1   0   |
  // 
  // The perspective matrix simply maps the perspective view volume (or frustum) to the orthographic view volume 
  // which is axis-aligned box, and then we can use the orthographic transform to get the canonical view volume. 
  // Concatenating P with M_ortho we get the perspective projection matrix:
  // 
  //   					| (2n)/(r-l)   	0         		(l+r)/(l-r) 	0 					|
  //            | 0         		(2n)/(t-b)   	(b+t)/(b-t) 	0						|
  // M_per =  	| 0         		0         		(f+n)/(n-f)		(2n)/(f-n)	|
  //            | 0         		0         		1         		0           |
  
  for(int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*3 + 0];
    auto& y = vertices[i*3 + 1];
    auto& z = vertices[i*3 + 2];
    
    auto p_cam = glm::vec4(x, y, z, 1);
    auto p_proj = m_per * p_cam; // [-1, +1]
    
    if (p_proj.w != 0.0f) 
    {
      x = p_proj.x / p_proj.w;
      y = p_proj.y / p_proj.w;
      z = p_proj.z / p_proj.w;
    } 
    else 
    {
      x = p_proj.x; 
      y = p_proj.y; 
      z = p_proj.z;
    }
  }
}

void apply_ortho_projection(f32* vertices, i32 n_vertices)
{
  // The projection transformation, that moves points from camera space to the view volume.
  // In the case of orthographic view volume, this volume is an axis-aligned box with where its 
  // side are [l,r] x [b,t] x [f,n].
  // We need to perform another transform from orthographic to canonical view volume and this 
  // is windowing transform, we can simply substitute the bounds of the orthographic and the 
  // canonical view volume to obtain the following matrix: 
  // 
  //            | 2/(r-l)   0         0         -(r+l)/(r-l) |
  //            | 0         2/(t-b)   0         -(t+b)/(t-b) |
  // M_ortho =  | 0         0         2/(n-f)   -(n+f)/(n-f) |
  //            | 0         0         0         1            |
 
  for(int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*3 + 0];
    auto& y = vertices[i*3 + 1];
    auto& z = vertices[i*3 + 2];
    
    auto p_cam = glm::vec4(x, y, z, 1);
    auto p_proj = m_ortho * p_cam; // [-1, +1]
    
    x = p_proj.x;
    y = p_proj.y;
    z = p_proj.z;
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
  
  glfwSetFramebufferSizeCallback(window, []([[ maybe_unused ]]GLFWwindow* window, int width, int height){ glViewport(0, 0, width, height); });
  glViewport(0, 0, WINDOW_W, WINDOW_H);
  
  // Let's define the pipeline
  auto pipeline_object = create_pipeline_object();

  // let's define our vertices in local space
  constexpr auto n_vertices = 8;
  f32 vertices[3 * n_vertices] = {
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f
  };
  constexpr u32 indices[] = {
    0, 1, 2,   2, 3, 0,
    1, 5, 6,   6, 2, 1,
    5, 4, 7,   7, 6, 5,
    4, 0, 3,   3, 7, 4,
    3, 2, 6,   6, 7, 3,
    4, 5, 1,   1, 0, 4
  };
  
  frame_to_canonical(vertices, n_vertices);
  canonical_to_camera(vertices, n_vertices);
  apply_persp_projection(vertices, n_vertices);
  //apply_ortho_projection(vertices, n_vertices);

  auto vbo = Buffer{};
  vbo.create();
  vbo.allocate_storage(sizeof(vertices), vertices, BufferUsageFlags::DynamicStorage);
  
  auto ibo = Buffer{};
  ibo.create();
  ibo.allocate_storage(sizeof(indices), indices, BufferUsageFlags::DynamicStorage);

  auto vao = VerteArray{};
  vao.create();
  vao.enable_attrib(0);
  vao.set_attrib_format_float(0, 3, VertexAttribType::Float, false, 0);  
  vao.attach_vertex_buffer(0, vbo.id(), 0, 3 * sizeof(f32));
  vao.link_attrib(0, 0);
  vao.attach_index_buffer(ibo.id());
  
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    pipeline_object.bind();
    vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}