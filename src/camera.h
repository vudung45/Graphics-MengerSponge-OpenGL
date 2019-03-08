#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <glm/gtx/io.hpp>
#include <math.h>
class Camera {
public:
	const glm::mat4& get_view_matrix();
	void move_up() ;
	void move_down() ;
	void zoom_up() ;
	void zoom_down() ;
	void move_left();
	void move_right();
	void turn_left();
	void turn_right();
	void turn_mouse(float x, float y);
	const glm::vec3& get_eye() const
	{
		return eye_;
	}
	void turn_up();
	void set_proj(const glm::mat4& m)
	{
		project_mat = m;
	}
	void turn_down();
	void turn_yaw(double delta_x);
	// FIXME: add functions to manipulate camera objects.
private:
	glm::mat4 project_mat;
	float camera_distance_ = 10.0;
	glm::vec3 look_ = glm::vec3(0.0f, -1.0/sqrt(2), -1.0/sqrt(2));
	glm::vec3 up_ = glm::vec3(0.0f, 1.0/sqrt(2), -1.0/sqrt(2));
	glm::vec3 eye_ = glm::vec3(0.0f, camera_distance_, camera_distance_);
	glm::mat4 vm;
	// Note: you may need additional member variables
	bool is_dirty = true;
};

#endif
 