#include "menger.h"
#include <vector>
#include <iostream>
#include <glm/gtx/io.hpp>
namespace {
	const int kMinLevel = 0;
	const int kMaxLevel = 4;
};

Menger::Menger()
{
	// Add additional initialization if you like
}

Menger::~Menger()
{
}

void
Menger::set_nesting_level(int level)
{
	nesting_level_ = level;
	dirty_ = true;
}

bool
Menger::is_dirty() const
{
	return dirty_;
}

void
Menger::set_clean()
{
	dirty_ = false;
}

// FIXME generate Menger sponge geometry

void generate_faces(const glm::vec3& bmin, const glm::vec3& bmax, 
					std::vector<glm::vec4>& obj_vertices,
					std::vector<glm::uvec3>& obj_faces, int i_x, int i_y, int i_z)
{
	int start = obj_vertices.size();
	// (0,0,0),
	// (0,1,0),
	// (1,1,0),
	// (1,0,0),

	// (0,0,1),
	// (0,1,1),
	// (1,1,1),
	// (1,0,1));
	obj_vertices.push_back(glm::vec4(bmin[0], bmin[1], bmin[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmin[0], bmax[1], bmin[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmax[0], bmax[1], bmin[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmax[0], bmin[1], bmin[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmin[0], bmin[1], bmax[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmin[0], bmax[1], bmax[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmax[0], bmax[1], bmax[2], 1.0f));
	obj_vertices.push_back(glm::vec4(bmax[0], bmin[1], bmax[2], 1.0f));
	// (0,1,2),
	// (0,2,3),

	// (6,5,4),
	// (7,6,4),

	// (3,2,6),
	// (3,6,7),

	// (4,5,1),
	// (4,1,0),

	// (1,5,2),
	// (5,6,2),

	// (4,0,3),
	// (4,3,7)

	//front
	obj_faces.push_back(glm::uvec3(start, start+1, start+2));
	obj_faces.push_back(glm::uvec3(start, start+2, start+3));
	
	//back
	obj_faces.push_back(glm::uvec3(start+6, start+5, start+4));
	obj_faces.push_back(glm::uvec3(start+7, start+6, start+4));

	//right

	obj_faces.push_back(glm::uvec3(start+3, start+2, start+6));
	obj_faces.push_back(glm::uvec3(start+3, start+6, start+7));
	
	//left
	obj_faces.push_back(glm::uvec3(start+4, start+5, start+1));
	obj_faces.push_back(glm::uvec3(start+4, start+1, start+0));
	
	//top
	obj_faces.push_back(glm::uvec3(start+1, start+5, start+2));
	obj_faces.push_back(glm::uvec3(start+5, start+6, start+2));

	//bottom
	obj_faces.push_back(glm::uvec3(start+4, start+0, start+3));
	obj_faces.push_back(glm::uvec3(start+4, start+3, start+7));
}
void expand(int L, std::vector<glm::vec4>& obj_vertices,
                          std::vector<glm::uvec3>& obj_faces)
{
	while(L > 0)
	{
		std::vector<glm::vec4> n_obj_vertices;
		std::vector<glm::uvec3> n_obj_faces;
		//for each cube
		for(int i = 0; i < obj_vertices.size(); i+=8)
		{
			glm::vec4 bmin = obj_vertices[i];
			glm::vec4 bmax = obj_vertices[i+6];
			double z_part = (bmax[2] - bmin[2]) /3;
			double y_part = (bmax[1] - bmin[1]) /3;
			double x_part = (bmax[0]  - bmin[0]) / 3;
			int _count = 0;
			int z_i;
			for(double z = bmin[2], z_i = 0; z_i < 3; z+= z_part, ++z_i )
			{
				int y_i;
				for(double y = bmin[1], y_i = 0; y_i < 3; y+= y_part, ++y_i)
				{
					int x_i;
					for(double x = bmin[0], x_i = 0; x_i < 3; x+= x_part, ++x_i)
					{
						int count = (z_i == 1) + (y_i == 1) + (x_i == 1);
						if(count < 2)
						{
							generate_faces(glm::vec3(x,y,z), 
										glm::vec3(x + x_part, y+ y_part ,z + z_part),
										n_obj_vertices,
										n_obj_faces, x_i, y_i, z_i);
						}

					}
				}
			}
		}	
		obj_vertices = n_obj_vertices;
		obj_faces = n_obj_faces;
		--L;
	}
}

void coalesce(int L , std::vector<glm::vec4>& obj_vertices,
                          std::vector<glm::uvec3>& obj_faces)
{
	while(L < 0)
	{
		std::vector<glm::vec4> n_obj_vertices;
		std::vector<glm::uvec3> n_obj_faces;
		//for each cube
		for(int i = 0; i < obj_vertices.size(); i+=8*20)
		{
			glm::vec4 bmin = obj_vertices[i];
			glm::vec4 bmax = obj_vertices[i+8*20 - 2];
			generate_faces(bmin, 
						   bmax,
						   n_obj_vertices,
						   n_obj_faces, 0,0,0);
			
		}	
		obj_vertices = n_obj_vertices;
		obj_faces = n_obj_faces;
		++L;
	}
}

void
Menger::generate_geometry(std::vector<glm::vec4>& obj_vertices,
                          std::vector<glm::uvec3>& obj_faces) const
{
	int current_level = log(obj_vertices.size() / 8) / log(20);
	int L = nesting_level_ - current_level;
	if(L > 0)
	{
		expand(L, obj_vertices, obj_faces);
	} else {
		coalesce(L, obj_vertices, obj_faces);
	}
}



