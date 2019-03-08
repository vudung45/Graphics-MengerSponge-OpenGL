#include "camera.h"

namespace {
	float pan_speed = 0.1f;
	float roll_speed = 0.1f;
	float rotation_speed = 0.02f;
	float zoom_speed = 0.1f;
};

// FIXME: Calculate the view matrix
const glm::mat4& Camera::get_view_matrix() 
{
	if(is_dirty)
	{
		glm::vec3 tangent = glm::normalize(glm::cross(look_, up_));
		glm::mat3 R =  glm::transpose(glm::mat3(tangent,up_,-look_));
		vm = glm::mat4(R);
		vm[3] = glm::vec4(-R*eye_, 1.0f);
		is_dirty = false;
	}
	return vm;
}

void Camera::zoom_up() {
		eye_ = eye_ + pan_speed * look_;
		is_dirty = true;

}

void Camera::zoom_down() {
		eye_ = eye_ - pan_speed * look_;
		is_dirty = true;

}

void Camera::move_up() {
		eye_ = eye_ + pan_speed * up_;
		is_dirty = true;

}

void Camera::move_down() {
		eye_ = eye_ - pan_speed * up_;
		is_dirty = true;

}

void Camera::move_left() {
		glm::vec3 tangent = glm::normalize(glm::cross(look_, up_));
		eye_ = eye_ -  pan_speed * tangent;
		is_dirty = true;

}

void Camera::move_right() {
	glm::vec3 tangent = glm::normalize(glm::cross(look_, up_));

	eye_ = eye_ +  pan_speed * tangent;
	is_dirty = true;

}

void Camera::turn_mouse(float x, float y)
{
	glm::vec3 tangent = glm::normalize(glm::cross(look_, up_));
	float old_y = tangent[1];
	glm::vec3 mouse_vec(glm::normalize(x* tangent - y*up_));
	glm::vec3 rotate_vec(glm::normalize(glm::cross(mouse_vec, look_)));
	glm::mat4 R = glm::rotate(rotation_speed,rotate_vec);
	tangent = glm::normalize(glm::vec3(R * glm::vec4(tangent,0.0f)));
	tangent[1] = old_y ; //make sure the rolling doesnt change
	up_ = glm::normalize(glm::vec3(R * glm::vec4(up_,0.0f)));
	up_ = up_ - glm::dot(up_,tangent)*tangent; //make up orthogonal to tangent again
	look_ = glm::normalize(glm::cross(up_,tangent));
	is_dirty = true;


}

void Camera::turn_left()
{

	glm::mat4 R = glm::rotate(rotation_speed,-look_);
	up_ = glm::normalize(glm::vec3(R * glm::vec4(up_,0.0f)));
	is_dirty = true;


}

void Camera::turn_right()
{
	glm::mat4 R = glm::rotate(-rotation_speed,-look_);
	up_ = glm::normalize(glm::vec3(R * glm::vec4(up_,0.0f)));
	is_dirty = true;

}

void Camera::turn_up()
{
	glm::vec3 tangent = glm::normalize(glm::cross(look_, up_));
	glm::mat4 R = glm::rotate(rotation_speed,tangent);
	look_ = glm::normalize(glm::vec3(R * glm::vec4(look_,0.0f)));
	up_ = glm::normalize(glm::cross(-look_,tangent));
	is_dirty = true;
}

void Camera::turn_down()
{
	glm::vec3 tangent = glm::normalize(glm::cross(look_, up_));
	glm::mat4 R = glm::rotate(-rotation_speed,tangent);
	look_ = glm::normalize(glm::vec3(R * glm::vec4(look_,0.0f)));
	up_ = glm::normalize(glm::cross(-look_,tangent));
	is_dirty = true;
}

