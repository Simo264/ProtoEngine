#pragma once

#include "basic_types.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

class Camera
{
public:
	// If the flag is true then the perspective projection matrix is used, otherwise the orthographic one
	Camera(f32 n, f32 f, f32 fovy, f32 aspect) : 
		eye{0.0f, 0.0f, 5.0f},
		gaze{0.0f, 0.0f, -1.0f},
		up{0.0f, 1.0f, 0.0f},
		near{n},
		far{f},
		fovy{glm::radians(fovy)},
		aspect{aspect}
{}

	// Converts points from the canonical coordinate system to camera coordinates system.
	glm::mat4 canonical_to_camera() const;
	// Returns the perspective projection matrix 
	glm::mat4 get_perspective() const;
	// Returns the orthographics projection matrix
	glm::mat4 get_orthographic() const;
	
	glm::vec3 eye; 	// camera position
	glm::vec3 gaze; // camera view direction
	glm::vec3 up;		// the camera up vector

	f32 near;				// near plane distance
	f32 far;				// far plane distance
	f32 fovy; 			// vertical fov
	f32 aspect;			// the aspect ratio of the viewport
};