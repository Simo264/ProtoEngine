#include "camera.hpp"

#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

extern i32 WINDOW_W;
extern i32 WINDOW_H;
extern f32 aspect;

constexpr auto camera_eye   = glm::vec3(0.0f, 0.0f, 4.0f);
constexpr auto camera_gaze  = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr auto camera_up    = glm::vec3(0.0f, 1.0f, 0.0f);

constexpr auto n =  0.1f, f =  100.0f;
constexpr f32 fov_y = glm::radians(45.0f);
const f32 t = n * glm::tan(fov_y / 2.0f);

static f32 r; 
static f32 b; 
static f32 l; 

void init_camera(f32 aspect_ratio)
{
  r = t * aspect_ratio;
  b = -t;
  l = -r; 
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
    auto& x = vertices[i*4+ 0];
    auto& y = vertices[i*4+ 1];
    auto& z = vertices[i*4+ 2];
    
    glm::vec4 p_local = glm::vec4(x, y, z, 1.0f);
    glm::vec4 p_world = ftc * p_local;
    
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
    glm::vec4(u.x, u.y, u.z, 0.0f),
    glm::vec4(v.x, v.y, v.z, 0.0f),
    glm::vec4(w.x, w.y, w.z, 0.0f),
    glm::vec4(-glm::dot(u, camera_eye), -glm::dot(v, camera_eye), -glm::dot(w, camera_eye), 1.0f)
  };
  
  for(int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*4 + 0];
    auto& y = vertices[i*4 + 1];
    auto& z = vertices[i*4 + 2];
    
    auto p_world = glm::vec4(x, y, z, 1.0f);
    auto p_cam = m_cam * p_world;
    
    x = p_cam.x;
    y = p_cam.y;
    z = p_cam.z; 
  }
}

void camera_to_perspective(f32* vertices, i32 n_vertices)
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
  //   					            | (2n)/(r-l)   	0         		(l+r)/(l-r) 	0 					|
  //                        | 0         		(2n)/(t-b)   	(b+t)/(b-t) 	0						|
  // M_per = M_ortho * P  = | 0         		0         		(f+n)/(n-f)		(2n)/(f-n)	|
  //                        | 0         		0         		1         		0           |
 
  auto m_per = glm::mat4{
    glm::vec4(2.0f*n/(r-l),    0.0f,           0.0f,          0.0f),  // col 0
    glm::vec4(0.0f,     2.0f*n/(t-b),           0.0f,          0.0f),  // col 1
    glm::vec4((l+r)/(l-r), (b+t)/(b-t), (f+n)/(f-n),         -1.0f),  // col 2
    glm::vec4(0.0f,          0.0f,      2.0f*f*n/(f-n),        0.0f)   // col 3
  };
  
  for(int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*4 + 0];
    auto& y = vertices[i*4 + 1];
    auto& z = vertices[i*4 + 2];
    auto& w = vertices[i*4 + 3];
    
    auto p_cam = glm::vec4(x, y, z, 1.0f);
    auto p_clip = m_per * p_cam; // [-1, +1]
  
    x = p_clip.x; 
    y = p_clip.y; 
    z = p_clip.z;
    w = p_clip.w;
  }
}

void camera_to_ortho(f32* vertices, i32 n_vertices)
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

  auto m_ortho = glm::mat4(
    glm::vec4(2.0f/(r-l),       0.0f,          0.0f,        0.0f),
    glm::vec4(0.0f,        2.0f/(t-b),          0.0f,        0.0f),
    glm::vec4(0.0f,             0.0f,     -2.0f/(f-n),       0.0f),
    glm::vec4(-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n),    1.0f)
  );
  
  for(int i = 0; i < n_vertices; i++)
  {
    auto& x = vertices[i*4 + 0];
    auto& y = vertices[i*4 + 1];
    auto& z = vertices[i*4 + 2];
    
    auto p_cam = glm::vec4(x, y, z, 1);
    auto p_proj = m_ortho * p_cam; // [-1, +1]
    
    x = p_proj.x;
    y = p_proj.y;
    z = p_proj.z;
  }
}