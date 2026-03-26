#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <print>
#include <array>

#include "basic_types.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "pipeline.hpp"
#include "buffer.hpp"
#include "vertex_array.hpp"
#include "camera.hpp"
#include "texture.hpp"

#include <glm/mat2x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/trigonometric.hpp>

#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static constexpr auto WINDOW_W = 640;
static constexpr auto WINDOW_H = 480;
static auto aspect_ratio = static_cast<f32>(WINDOW_W) / static_cast<f32>(WINDOW_H);

static auto program_vertex_object = ShaderProgram{};
static auto program_fragment_object = ShaderProgram{};
static auto pipeline_object = ProgramPipelineObject{};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec3 tangent;
};

GLFWwindow* init_context()
{
	glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DEPTH_BITS, 24); 
  auto window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Proto engine", nullptr, nullptr);
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
  auto& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");
  
  return window;
}

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
  fragment_shader_obj.create(ShaderStage::Fragment);
  fragment_shader_obj.load_source_code(shaders_dir / "basic_shader.frag.glsl");
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
  program_fragment_object.attach_shader(fragment_shader_obj);
  program_fragment_object.set_separable(true);
  program_fragment_object.link();
  status = program_fragment_object.check_link_status();
  if(!status)
    std::println("Link status: {}", program_fragment_object.get_link_log());
   
  program_fragment_object.detach_shader(fragment_shader_obj);
  
  pipeline_object = ProgramPipelineObject{};
  pipeline_object.create();
  pipeline_object.bind_program_stage(PipelineStage::VertexShader, program_vertex_object);
  pipeline_object.bind_program_stage(PipelineStage::FragmentShader, program_fragment_object);
  status = pipeline_object.validate_pipeline();
  if(!status)
   	std::println("pipeline object status: {}", pipeline_object.get_validation_status());
}

void compute_tangents(std::array<Vertex, 36>& vertices) 
{
  for (auto i = 0u; i < vertices.size(); i += 3) 
  {
  	auto& v0 = vertices.at(i);
  	auto& v1 = vertices.at(i+1);
  	auto& v2 = vertices.at(i+2);
   	auto e1 = v1.position - v0.position;
    auto e2 = v2.position - v0.position; 
    auto delta_U1 = v1.texcoord.x - v0.texcoord.x;
    auto delta_V1 = v1.texcoord.y - v0.texcoord.y;
    auto delta_U2 = v2.texcoord.x - v0.texcoord.x;
    auto delta_V2 = v2.texcoord.y - v0.texcoord.y;
    
    // The UV matrix is a square matrix 2x2
   	auto mat_uv = glm::mat2(
      glm::vec2(delta_U1, delta_U2),
      glm::vec2(delta_V1, delta_V2)
    );
    // The E matrix is a rectangular matrix 2x3
    auto mat_edge = glm::mat3x2(
      glm::vec2(e1.x, e2.x),
      glm::vec2(e1.y, e2.y),
      glm::vec2(e1.z, e2.z)
    );
    
    auto mat_uv_inv = glm::inverse(mat_uv);
    
    // The TB matrix is a rectangular matrix 2x3
    auto mat_tb =  mat_uv_inv * mat_edge;

    auto tangent = glm::vec3{};
    tangent.x = mat_tb[0][0];
    tangent.y = mat_tb[1][0];
    tangent.z = mat_tb[2][0];
    
    v0.tangent = tangent; 
    v1.tangent = tangent; 
    v2.tangent = tangent;
  }
}

VerteArray create_cube_object(u32& n_vertices, u32& n_indices)
{
	// let's define our vertices in local space
	constexpr auto n_vertex_components = 11u;
	auto vertices = std::array {
    Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    Vertex{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} }
	};
	
	compute_tangents(vertices);
	
  auto vbo = Buffer{};
  vbo.create();
  vbo.allocate_storage(sizeof(vertices), vertices.data(), BufferUsageFlags::DynamicStorage);
  
  // auto ibo = Buffer{};
  // ibo.create();
  // ibo.allocate_storage(sizeof(indices), indices.data(), BufferUsageFlags::DynamicStorage);

  auto vao = VerteArray{};
  vao.create();
  // Attribute 0: position(xyz) — offset 0
  vao.set_attrib_format_float(0, 3, VertexAttribType::Float, false, 0);
  // Attribute 1: normal(x,y,z) — offset 3 * sizeof(f32)
  vao.set_attrib_format_float(1, 3, VertexAttribType::Float, false, 3 * sizeof(f32));
  // Attribute 2: texcoord(uv) — offset 6 * sizeof(f32)
  vao.set_attrib_format_float(2, 2, VertexAttribType::Float, false, 6 * sizeof(f32));
  // Attribute 3: tangent(x,y,z) — offset 8 * sizeof(f32)
  vao.set_attrib_format_float(3, 3, VertexAttribType::Float, false, 8 * sizeof(f32));

  vao.attach_vertex_buffer(0, vbo.id(), 0, n_vertex_components * sizeof(f32));
  vao.link_attrib(0, 0);
  vao.link_attrib(1, 0); 
  vao.link_attrib(2, 0); 
  vao.link_attrib(3, 0); 
  vao.enable_attrib(0);
  vao.enable_attrib(1);
  vao.enable_attrib(2);
  vao.enable_attrib(3);
  //vao.attach_index_buffer(ibo.id());
  
  //n_indices = indices.size(); 
  n_indices = 0;
  n_vertices = vertices.size();
  return vao;
}

glm::mat4 calculate_transformation_matrix(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
{
  auto S = glm::mat3 {
   	scale.x, 0.0f, 0.0f, 
   	0.0f, scale.y, 0.0f, 
   	0.0f, 0.0f, scale.z 
  };
  
  auto cx = glm::cos(rotation.x);
  auto sx = glm::sin(rotation.x);
  auto R_x = glm::mat3 {
      1.0f,  0.0f, 0.0f,
      0.0f,  cx,   sx,
      0.0f, -sx,   cx
  };
  auto cy = glm::cos(rotation.y);
  auto sy = glm::sin(rotation.y);
  auto R_y = glm::mat3 {
       cy,  0.0f, sy,
      0.0f, 1.0f, 0.0f,
      -sy,  0.0f, cy
  };
  
  auto cz = glm::cos(rotation.z);
  auto sz = glm::sin(rotation.z);
  auto R_z = glm::mat3 {
       cz,  sz,   0.0f,
      -sz,  cz,   0.0f,
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
	
	constexpr auto frame_e = glm::vec3(0.0f, 0.0f, 0.0f); 
	constexpr auto frame_u = glm::vec3(1.0f, 0.0f, 0.0f); 
	constexpr auto frame_v = glm::vec3(0.0f, 1.0f, 0.0f);
	constexpr auto frame_w = glm::vec3(0.0f, 0.0f, 1.0f);
	constexpr auto m_ftc = glm::mat4 {
	  glm::vec4(frame_u, 0.0f),
	  glm::vec4(frame_v, 0.0f),
	  glm::vec4(frame_w, 0.0f),
	  glm::vec4(frame_e, 1.0f)
	};
	return m_ftc;
}

Texture create_texture_color(const std::filesystem::path& filepath)
{
 	if(!std::filesystem::exists(filepath))
    exit(1);
  
  auto width{ 0 }, height{ 0 }, nr_channels{ 0 };
  auto data = stbi_load(filepath.string().c_str(), &width, &height, &nr_channels, STBI_rgb);
  auto levels = static_cast<u32>(std::floor(std::log2(std::max(width, height))) + 1);  // levels = floor(log_2(max(width, height))) + 1
  
	auto texture = Texture{};
 	texture.create(TextureType::Texture2D);
  texture.set_storage_tex2D(levels, TextureImageFormat::RGB8, width, height);
  texture.update_content_tex2D(0, 0, 0, width, height, PixelDataFormat::RGB, PixelDataType::UnsignedByte, data);
  stbi_image_free(data);
  
  texture.set_wrap_mode(TextureWrapMode::Repeat, TextureWrapMode::Repeat);
  texture.set_magnification_filter(TextureFilteringMode::Linear);
  texture.set_minification_filter(TextureFilteringMode::LinearMipmapLinear);
  texture.generate_mipmaps();
  return texture;
}

Texture create_texture_normal(const std::filesystem::path& filepath)
{
  if(!std::filesystem::exists(filepath))
    exit(1);
  
  auto width{ 0 }, height{ 0 }, nr_channels{ 0 };
  auto data = stbi_load(filepath.string().c_str(), &width, &height, &nr_channels, STBI_rgb);
  auto levels = static_cast<u32>(std::floor(std::log2(std::max(width, height))) + 1);  // levels = floor(log_2(max(width, height))) + 1
  
  auto texture = Texture{};
  texture.create(TextureType::Texture2D);
  texture.set_storage_tex2D(levels, TextureImageFormat::RGB8, width, height);
  texture.update_content_tex2D(0, 0, 0, width, height, PixelDataFormat::RGB, PixelDataType::UnsignedByte, data);
  stbi_image_free(data);
  
  texture.set_wrap_mode(TextureWrapMode::Repeat, TextureWrapMode::Repeat);
  texture.set_magnification_filter(TextureFilteringMode::Linear);
  texture.set_minification_filter(TextureFilteringMode::LinearMipmapLinear);
  texture.generate_mipmaps();
  return texture;
}

int main() 
{ 
	auto window = init_context();
	
  // define the program pipeline
  create_pipeline_object();
  
  // create the texture color and normal map
  auto texture_color = create_texture_color(std::filesystem::current_path() / "res/materials/stonebricks/StoneBricksSplitface_COL_2K.jpg");
  auto texture_normal = create_texture_normal(std::filesystem::current_path() / "res/materials/stonebricks/StoneBricksSplitface_NRM_2K.png");
  
  // define the camera
  auto camera = Camera(0.1f, 100.0f, 45.f, aspect_ratio);
  
  // define the cube object
  auto cube_indices = 0u, cube_vertices = 0u;
  auto cube_vao = create_cube_object(cube_vertices, cube_indices);
  auto cube_position = glm::vec3(0.f);
  auto cube_rotation = glm::vec3(0.f);
  auto cube_scale = glm::vec3(1.f);
  
  glEnable(GL_DEPTH_TEST);              // enable depth testing
  glDepthFunc(GL_LESS);                 // specify the value used for depth buffer comparisons
  glDepthMask(GL_TRUE);                 // enable/disable writing into the depth buffer
  glClearDepthf(1.0f);                  // specify the clear value for the depth buffer
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // specify the clear value for the color buffer
  
  while (!glfwWindowShouldClose(window))
  {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    camera.rotate_pitch(+glm::radians(1.0f));
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  camera.rotate_pitch(-glm::radians(1.0f));
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  camera.rotate_yaw(+glm::radians(1.0f));
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera.rotate_yaw(-glm::radians(1.0f));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)     camera.eye += camera.gaze() * 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)     camera.eye -= camera.gaze() * 0.1f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)     camera.eye -= camera.right() * 0.1f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)     camera.eye += camera.right() * 0.1f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)          camera.eye += camera.up() * 0.1f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)   camera.eye -= camera.up() * 0.1f;
    
    auto time = glfwGetTime();
    cube_rotation.y = glm::radians(time) * 32;
    
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffers to preset values
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::Begin("Scene");
  	{
	    if (ImGui::CollapsingHeader("Proprietà Camera", ImGuiTreeNodeFlags_DefaultOpen)) 
	    {
	      auto fov = glm::degrees(camera.fovy);
	      if (ImGui::DragFloat("Vertical FOV", &fov, 0.5f, 30.0f, 120.0f))
	        camera.fovy = glm::radians(fov);
	  
				auto euler_angles = glm::degrees(camera.get_euler_angles());
				ImGui::Text("Camera Position: X:%.2f, Y:%.2f, Z:%.2f", camera.eye.x, camera.eye.y, camera.eye.z);
    		ImGui::BulletText("P: %.2f°  Y: %.2f°  R: %.2f°", euler_angles.x, euler_angles.y, euler_angles.z);
	      
	      if (ImGui::Button("Reset Camera")) 
	      {
	        camera.eye = glm::vec3(0.0f, 0.0f, 5.0f);
	        camera.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	        camera.fovy = glm::radians(45.0f);
	      }
	    }    
	    ImGui::Spacing();
	    ImGui::Separator();
	    ImGui::Spacing();
	    if (ImGui::CollapsingHeader("Cube", ImGuiTreeNodeFlags_DefaultOpen)) 
	    {
	      ImGui::DragFloat3("Position", &cube_position[0], 0.1f);
	      ImGui::DragFloat3("Rotation", &cube_rotation[0], 1.0f); // In gradi
	      ImGui::DragFloat3("Scale", &cube_scale[0], 0.05f, 0.1f, 10.0f);
	      if (ImGui::Button("Reset"))
	      {
	        cube_position = glm::vec3(0.0f);
	        cube_rotation = glm::vec3(0.0f);
	        cube_scale = glm::vec3(1.0f);
	      }
	    }
   	}
    ImGui::End();
    
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
    pipeline_object.set_active_program(program_fragment_object);
    program_fragment_object.set_uniform_vector3f(program_fragment_object.get_uniform_location("u_camera_eye"), &camera.eye[0]); 
    texture_color.bind_texture_unit(0);
    texture_normal.bind_texture_unit(1);
    
    cube_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices);
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    auto& io = ImGui::GetIO();
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