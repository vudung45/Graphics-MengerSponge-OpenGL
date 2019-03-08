#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <image.h>
#include <jpegio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <unistd.h>
// OpenGL library includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>
#include "menger.h"
#include "camera.h"
#include "cubemap.h" //cubemap config
#include "shaders.h"
#include "wave.h" //wave config
#define WAVES 4
#include <chrono>
int window_width = 800, window_height = 800;

// VBO and VAO descriptors.
enum { kVertexBuffer, kIndexBuffer, kNumVbos };

// These are our VAOs.
enum { kGeometryVao, kFloorVao, kOceanVao, kCubeVao, kNumVaos };

GLuint g_array_objects[kNumVaos];  // This will store the VAO descriptors.
GLuint g_buffer_objects[kNumVaos][kNumVbos];  // These will store VBO descriptors.


unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
		struct Image image;
		
        if (LoadJPEG(faces[i], &image))
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, image.width, image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.bytes.data()
            );
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}  

void
CreateTriangle(std::vector<glm::vec4>& vertices,
        std::vector<glm::uvec3>& indices)
{
	vertices.push_back(glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1.0f));
	vertices.push_back(glm::vec4(0.0f, 0.5f, -0.5f, 1.0f));
	indices.push_back(glm::uvec3(0, 1, 2));
}

void CreateCube(std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& indices){
	vertices.push_back(glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f));
	vertices.push_back(glm::vec4(0.5f, 0.5f, -0.5f, 1.0f));
	vertices.push_back(glm::vec4(0.5f, -0.5f, -0.5f, 1.0f));
	
	vertices.push_back(glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f));
	vertices.push_back(glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f));
	vertices.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	vertices.push_back(glm::vec4(0.5f, -0.5f, 0.5f, 1.0f));

	indices.push_back(glm::uvec3(0, 1, 2));
	indices.push_back(glm::uvec3(0, 2, 3));

	indices.push_back(glm::uvec3(6, 5, 4));
	indices.push_back(glm::uvec3(7, 6, 4));

	indices.push_back(glm::uvec3(3, 2, 6));
	indices.push_back(glm::uvec3(3, 6, 7));

	indices.push_back(glm::uvec3(4, 5, 1));
	indices.push_back(glm::uvec3(4, 1, 0));

	indices.push_back(glm::uvec3(1, 5, 2));
	indices.push_back(glm::uvec3(5, 6, 2));

	indices.push_back(glm::uvec3(4, 0, 3));
	indices.push_back(glm::uvec3(4, 3, 7));
}

void CreatePlane(std::vector<glm::vec4>& vertices, std::vector<glm::uvec3>& indices){
	vertices.push_back(glm::vec4(-20.0f, -4.0f, -20.0f, 1.0f));
	vertices.push_back(glm::vec4(-20.0f, -4.0f, 20.0f, 1.0f));
	vertices.push_back(glm::vec4(20.0f, -4.0f, -20.0f, 1.0f));
	vertices.push_back(glm::vec4(20.0f, -4.0f, 20.0f, 1.0f));

	indices.push_back(glm::uvec3(0, 1, 2));
	indices.push_back(glm::uvec3(1, 3, 2));
}

void CreateOcean(std::vector<glm::vec4>& vertices, std::vector<glm::uvec4>& indices){
	float offset = 40.0 / 16;
	for(int r = 0; r < 16; ++r)
	{
		for(int c = 0; c < 16; ++c)
		{
			int index = vertices.size();
			vertices.push_back(glm::vec4(-20.0f + offset * c, -1.0f, -20.0f + offset * r, 1.0f));
			vertices.push_back(glm::vec4(-20.0f + offset * (c+1), -1.0f, -20.0f + offset * r, 1.0f));
			vertices.push_back(glm::vec4(-20.0f + offset * (c+1), -1.0f, -20.0f + offset * (r+1), 1.0f));
			vertices.push_back(glm::vec4(-20.0f + offset * c, -1.0f, -20.0f + offset * (r+1), 1.0f));
			indices.push_back(glm::uvec4(index,index + 1, index + 2, index +  3));
		}
	}

}

void
SaveObj(const std::string& file,
        const std::vector<glm::vec4>& vertices,
        const std::vector<glm::uvec3>& indices)
{
	std::ofstream ofs(file);
	for (const auto& vert : vertices)
		ofs << "v " << vert[0] << " " << vert[1] << " " << vert[2] << std::endl;
	for (const auto& index : indices)
		ofs << "f " << index[0]+1 << " " << index[1]+1 << " " << index[2]+1 << std::endl;
}

void
ErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << "\n";
}

std::shared_ptr<Menger> g_menger;
Camera g_camera;
bool g_save_geo = false;
bool wireframes_toggle = 0;
bool ocean_on = false;
bool fill = true;
void
KeyCallback(GLFWwindow* window,
            int key,
            int scancode,
            int action,
            int mods)
{
	// Note:
	// This is only a list of functions to implement.
	// you may want to re-organize this piece of code.
	if(key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
		outer_level += 1.0;

	if(key == GLFW_KEY_F && action == GLFW_RELEASE)
		wireframes_toggle = !wireframes_toggle;

	if(key == GLFW_KEY_MINUS  && action == GLFW_PRESS)
		outer_level -= 1.0;

	if (key == GLFW_KEY_T && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) 
		tidal_x_pos = 0.0;


	if (key == GLFW_KEY_O && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) 
		ocean_on = !ocean_on;

	if (key == GLFW_KEY_F && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) 
	{
		fill = !fill;
		if(!fill)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if(key == GLFW_KEY_PERIOD  && action == GLFW_PRESS)
		inner_level += 1.0;

	if(key == GLFW_KEY_COMMA  && action == GLFW_PRESS)
		inner_level -= 1.0;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else if (key == GLFW_KEY_S && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
		g_save_geo = true;

	} else if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
		// FIXME: WASD

		g_camera.zoom_up();
	} else if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
		g_camera.zoom_down();

	} else if (key == GLFW_KEY_A && action != GLFW_RELEASE) {
		g_camera.move_left();

	} else if (key == GLFW_KEY_D && action != GLFW_RELEASE) {
		g_camera.move_right();

	} else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
		g_camera.turn_left();

		// FIXME: Left Right Up and Down
	} else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
		g_camera.turn_right();
	} else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
		g_camera.move_down();
	} else if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
		g_camera.move_up();
	} else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
		// FIXME: FPS mode on/off
	}
	if (!g_menger)
		return ; // 0-4 only available in Menger mode.
	if (key == GLFW_KEY_0 && action != GLFW_RELEASE) {
		// FIXME: Change nesting level of g_menger
		g_menger->set_nesting_level(0);
		// Note: GLFW_KEY_0 - 4 may not be continuous.
	} else if (key == GLFW_KEY_1 && action != GLFW_RELEASE) {
		g_menger->set_nesting_level(1);
	} else if (key == GLFW_KEY_2 && action != GLFW_RELEASE) {
		g_menger->set_nesting_level(2);
	} else if (key == GLFW_KEY_3 && action != GLFW_RELEASE) {
		g_menger->set_nesting_level(3);
	} else if (key == GLFW_KEY_4 && action != GLFW_RELEASE) {
		g_menger->set_nesting_level(4);
	}


}

int g_current_button;
bool g_mouse_pressed;
double old_x;
double old_y;
void
MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
	if (!g_mouse_pressed)
	{
		old_x = mouse_x;
		old_y = mouse_y;	
		return;
	}
	if (g_current_button == GLFW_MOUSE_BUTTON_LEFT) {
		double delta_x = (mouse_x - old_x) / window_width; //yaw
		double delta_y = (mouse_y - old_y) / window_height; //pich
		g_camera.turn_mouse(delta_x,delta_y);
		old_x = mouse_x;
		old_y = mouse_y;
		// FIXME: left drag
	} else if (g_current_button == GLFW_MOUSE_BUTTON_RIGHT) {
		// FIXME: middle drag
	} else if (g_current_button == GLFW_MOUSE_BUTTON_MIDDLE) {
		// FIXME: right drag
	}
}

void
MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	g_mouse_pressed = (action == GLFW_PRESS);
	g_current_button = button;
}

int main(int argc, char* argv[])
{
	std::string window_title = "Menger";
	if (!glfwInit()) exit(EXIT_FAILURE);
	g_menger = std::make_shared<Menger>();
	glfwSetErrorCallback(ErrorCallback);

	// Ask an OpenGL 4.1 core profile context
	// It is required on OSX and non-NVIDIA Linux
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(window_width, window_height,
			&window_title[0], nullptr, nullptr);
	CHECK_SUCCESS(window != nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MousePosCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSwapInterval(1);
	const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);    // version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	std::vector<glm::vec4> obj_vertices;
	std::vector<glm::uvec3> obj_faces;

        //FIXME: Create the geometry from a Menger object.
        CreateCube(obj_vertices, obj_faces);

	g_menger->set_nesting_level(0);
	glm::vec4 min_bounds = glm::vec4(std::numeric_limits<float>::max());
	glm::vec4 max_bounds = glm::vec4(-std::numeric_limits<float>::max());
	for (const auto& vert : obj_vertices) {
		min_bounds = glm::min(vert, min_bounds);
		max_bounds = glm::max(vert, max_bounds);
	}
	std::cout << "min_bounds = " << glm::to_string(min_bounds) << "\n";
	std::cout << "max_bounds = " << glm::to_string(max_bounds) << "\n";

	// Setup our VAO array.
	CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &g_array_objects[0]));

	// Switch to the VAO for Geometry.
	CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kGeometryVao]));

	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kGeometryVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * obj_vertices.size() * 4, obj_vertices.data(),
				GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint32_t) * obj_faces.size() * 3,
				obj_faces.data(), GL_STATIC_DRAW));

	/*
 	 * By far, the geometry is loaded into g_buffer_objects[kGeometryVao][*].
	 * These buffers are binded to g_array_objects[kGeometryVao]
	 */

	// FIXME: load the floor into g_buffer_objects[kFloorVao][*],
	//        and bind these VBO to g_array_objects[kFloorVao]

	std::vector<glm::vec4> plane_vertices;
	std::vector<glm::uvec3> plane_indices;
	CreatePlane(plane_vertices, plane_indices);

	// CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &g_array_objects[0]));

	// Switch to the VAO for Geometry.
	CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kFloorVao]));

	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kFloorVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kFloorVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * plane_vertices.size() * 4, plane_vertices.data(),
				GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kFloorVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint32_t) * plane_indices.size() * 3,
				plane_indices.data(), GL_STATIC_DRAW));

	/*
	*	Ocean
	*
	*/
	std::vector<glm::vec4> ocean_vertices;
	std::vector<glm::uvec4> ocean_indices;
	CreateOcean(ocean_vertices, ocean_indices);

	// CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &g_array_objects[0]));

	// Switch to the VAO for Geometry.
	CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kOceanVao]));
	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kOceanVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kOceanVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
				sizeof(float) * ocean_vertices.size() * 4, ocean_vertices.data(),
				GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kOceanVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint32_t) * ocean_indices.size() * 4,
				ocean_indices.data(), GL_STATIC_DRAW));


	/*
	*	Sky
	*
	*/
	//setup vbo and vao for cube

    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kCubeVao]));

    // Generate buffer objects
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kCubeVao][0]));

    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kCubeVao][kVertexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));
    CHECK_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));


    /*
    *
    * Initialize shaders
    */
	// Setup vertex shader.
	GLuint vertex_shader_id = 0;
	const char* vertex_source_pointer = vertex_shader;
	CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
	CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr));
	glCompileShader(vertex_shader_id);
	CHECK_GL_SHADER_ERROR(vertex_shader_id);


	// Setup vertex shader.
	GLuint sky_vertex_shader_id = 0;
	const char* sky_vertex_source_pointer = sky_vertex_shader;
	CHECK_GL_ERROR(sky_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
	CHECK_GL_ERROR(glShaderSource(sky_vertex_shader_id, 1, &sky_vertex_source_pointer, nullptr));
	glCompileShader(sky_vertex_shader_id);
	CHECK_GL_SHADER_ERROR(sky_vertex_shader_id);

	// Setup geometry shader.
	GLuint geometry_shader_id = 0;
	const char* geometry_source_pointer = geometry_shader;
	CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
	CHECK_GL_ERROR(glShaderSource(geometry_shader_id, 1, &geometry_source_pointer, nullptr));
	glCompileShader(geometry_shader_id);
	CHECK_GL_SHADER_ERROR(geometry_shader_id);

	// Setup ocean geometry shader.
	GLuint ocean_geometry_shader_id = 0;
	const char* ocean_geometry_source_pointer = ocean_geometry_shader;
	CHECK_GL_ERROR(ocean_geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
	CHECK_GL_ERROR(glShaderSource(ocean_geometry_shader_id, 1, &ocean_geometry_source_pointer, nullptr));
	glCompileShader(ocean_geometry_shader_id);
	CHECK_GL_SHADER_ERROR(ocean_geometry_shader_id);

	// Setup fragment shader.
	GLuint fragment_shader_id = 0;
	const char* fragment_source_pointer = fragment_shader;
	CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr));
	glCompileShader(fragment_shader_id);
	CHECK_GL_SHADER_ERROR(fragment_shader_id);

	// Setup fragment shader.
	GLuint sky_fragment_shader_id = 0;
	const char* sky_fragment_source_pointer = sky_fragment_shader;
	CHECK_GL_ERROR(sky_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(sky_fragment_shader_id, 1, &sky_fragment_source_pointer, nullptr));
	glCompileShader(sky_fragment_shader_id);
	CHECK_GL_SHADER_ERROR(sky_fragment_shader_id);

	// Setup ocean fragment shader.
	GLuint ocean_fragment_shader_id = 0;
	const char* ocean_fragment_source_pointer = ocean_fragment_shader;
	CHECK_GL_ERROR(ocean_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(ocean_fragment_shader_id, 1, &ocean_fragment_source_pointer, nullptr));
	glCompileShader(ocean_fragment_shader_id);
	CHECK_GL_SHADER_ERROR(ocean_fragment_shader_id);

	// Setup tcs  shader.
	GLuint TCS_shader_id = 0;
	const char* TCS_shader_source_id = TCS_shader;
	CHECK_GL_ERROR(TCS_shader_id = glCreateShader(GL_TESS_CONTROL_SHADER));
	CHECK_GL_ERROR(glShaderSource(TCS_shader_id, 1, &TCS_shader_source_id, nullptr));
	glCompileShader(TCS_shader_id);
	CHECK_GL_SHADER_ERROR(TCS_shader_id);


		// Setup tes  shader.
	GLuint TES_shader_id = 0;
	const char* TES_shader_source_id = TES_shader;
	CHECK_GL_ERROR(TES_shader_id = glCreateShader(GL_TESS_EVALUATION_SHADER));
	CHECK_GL_ERROR(glShaderSource(TES_shader_id, 1, &TES_shader_source_id, nullptr));
	glCompileShader(TES_shader_id);
	CHECK_GL_SHADER_ERROR(TES_shader_id);

	// Setup tcs  shader.
	GLuint ocean_TCS_shader_id = 0;
	const char* ocean_TCS_shader_source_id = ocean_TCS_shader;
	CHECK_GL_ERROR(ocean_TCS_shader_id = glCreateShader(GL_TESS_CONTROL_SHADER));
	CHECK_GL_ERROR(glShaderSource(ocean_TCS_shader_id, 1, &ocean_TCS_shader_source_id, nullptr));
	glCompileShader(ocean_TCS_shader_id);
	CHECK_GL_SHADER_ERROR(ocean_TCS_shader_id);


	// Setup tes  shader.
	GLuint ocean_TES_shader_id = 0;
	const char* ocean_TES_shader_source_id = ocean_TES_shader;
	CHECK_GL_ERROR(ocean_TES_shader_id = glCreateShader(GL_TESS_EVALUATION_SHADER));
	CHECK_GL_ERROR(glShaderSource(ocean_TES_shader_id, 1, &ocean_TES_shader_source_id, nullptr));
	glCompileShader(ocean_TES_shader_id);
	CHECK_GL_SHADER_ERROR(ocean_TES_shader_id);

	// Let's create our program.
	GLuint program_id = 0;
	CHECK_GL_ERROR(program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(program_id, vertex_shader_id));
	// CHECK_GL_ERROR(glAttachShader(program_id, TCS_shader_id));
	// CHECK_GL_ERROR(glAttachShader(program_id, TES_shader_id));
	CHECK_GL_ERROR(glAttachShader(program_id, fragment_shader_id));
	CHECK_GL_ERROR(glAttachShader(program_id, geometry_shader_id));

	// Bind attributes.
	CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
	glLinkProgram(program_id);
	CHECK_GL_PROGRAM_ERROR(program_id);

	// Get the uniform locations.
	GLint projection_matrix_location = 0;
	CHECK_GL_ERROR(projection_matrix_location =
			glGetUniformLocation(program_id, "projection"));

	GLint view_matrix_location = 0;
	CHECK_GL_ERROR(view_matrix_location =
			glGetUniformLocation(program_id, "view"));
	GLint light_position_location = 0;
	CHECK_GL_ERROR(light_position_location =
			glGetUniformLocation(program_id, "light_position"));
	GLint wireframe_toggle = 0;
	CHECK_GL_ERROR(wireframe_toggle =
			glGetUniformLocation(program_id, "wireframes"));
	GLint num_inner_level = 0;
	CHECK_GL_ERROR(num_inner_level =
			glGetUniformLocation(program_id, "inner_level"));
	GLint num_outer_level = 0;
	CHECK_GL_ERROR(num_outer_level =
			glGetUniformLocation(program_id, "outer_level"));

	// Setup fragment shader for the floor
	GLuint floor_fragment_shader_id = 0;
	const char* floor_fragment_source_pointer = floor_fragment_shader;
	CHECK_GL_ERROR(floor_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(floor_fragment_shader_id, 1,
				&floor_fragment_source_pointer, nullptr));
	glCompileShader(floor_fragment_shader_id);
	CHECK_GL_SHADER_ERROR(floor_fragment_shader_id);


	// FIXME: Setup another program for the floor, and get its locations.
	// Note: you can reuse the vertex and geometry shader objects

	// Let's create our program.
	GLuint floor_program_id = 0;
	CHECK_GL_ERROR(floor_program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(floor_program_id, vertex_shader_id));
	CHECK_GL_ERROR(glAttachShader(floor_program_id, TCS_shader_id));
	CHECK_GL_ERROR(glAttachShader(floor_program_id, TES_shader_id));
	CHECK_GL_ERROR(glAttachShader(floor_program_id, floor_fragment_shader_id));
	CHECK_GL_ERROR(glAttachShader(floor_program_id, geometry_shader_id));

	// Bind attributes.
	CHECK_GL_ERROR(glBindFragDataLocation(floor_program_id, 0, "fragment_color"));
	glLinkProgram(floor_program_id);
	CHECK_GL_PROGRAM_ERROR(floor_program_id);

	// Get the uniform locations.
	GLint floor_projection_matrix_location = 0;
	CHECK_GL_ERROR(floor_projection_matrix_location =
			glGetUniformLocation(floor_program_id, "projection"));
	GLint floor_view_matrix_location = 0;
	CHECK_GL_ERROR(floor_view_matrix_location =
			glGetUniformLocation(floor_program_id, "view"));
	GLint floor_light_position_location = 0;
	CHECK_GL_ERROR(floor_light_position_location =
			glGetUniformLocation(floor_program_id, "light_position"));
	GLint floor_wireframe_toggle = 0;
	CHECK_GL_ERROR(floor_wireframe_toggle =
			glGetUniformLocation(floor_program_id, "wireframes"));
	GLint floor_ocean_timer = 0;
	CHECK_GL_ERROR(floor_ocean_timer =
			glGetUniformLocation(floor_program_id, "timer"));

	GLint floor_ocean_on = 0;
	CHECK_GL_ERROR(floor_ocean_on =
			glGetUniformLocation(floor_program_id, "ocean_on"));

	GLint floor_num_inner_level = 0;
	CHECK_GL_ERROR(floor_num_inner_level =
			glGetUniformLocation(floor_program_id, "inner_level"));
	GLint floor_num_outer_level = 0;
	CHECK_GL_ERROR(floor_num_outer_level =
			glGetUniformLocation(floor_program_id, "outer_level"));

	GLint floor_eye_coord = 0;
	CHECK_GL_ERROR(floor_eye_coord =
			glGetUniformLocation(floor_program_id, "eye_coord"));
	GLint floor_ocean_wavelength = 0;
	CHECK_GL_ERROR(floor_ocean_wavelength =
			glGetUniformLocation(floor_program_id, "wavelength"));

	GLint floor_ocean_amplitude = 0;
	CHECK_GL_ERROR(floor_ocean_amplitude =
			glGetUniformLocation(floor_program_id, "amplitude"));
	GLint floor_ocean_direction = 0;
	CHECK_GL_ERROR(floor_ocean_direction =
			glGetUniformLocation(floor_program_id, "direction"));

	GLint floor_ocean_speed = 0;
	CHECK_GL_ERROR(floor_ocean_speed =
			glGetUniformLocation(floor_program_id, "speed"));

	GLint floor_ocean_tidal_x_pos = 0;
	CHECK_GL_ERROR(floor_ocean_tidal_x_pos =
			glGetUniformLocation(floor_program_id, "tidal_x_pos"));

	// Let's create our program.
	GLuint ocean_program_id = 0;
	CHECK_GL_ERROR(ocean_program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(ocean_program_id, vertex_shader_id));
	CHECK_GL_ERROR(glAttachShader(ocean_program_id, ocean_TCS_shader_id));
	CHECK_GL_ERROR(glAttachShader(ocean_program_id, ocean_TES_shader_id));
	CHECK_GL_ERROR(glAttachShader(ocean_program_id, ocean_fragment_shader_id));
	CHECK_GL_ERROR(glAttachShader(ocean_program_id, ocean_geometry_shader_id));

	// Bind attributes.
	CHECK_GL_ERROR(glBindFragDataLocation(ocean_program_id, 0, "fragment_color"));
	glLinkProgram(ocean_program_id);
	CHECK_GL_PROGRAM_ERROR(ocean_program_id);

	// UNIFORMs for ocean.
	GLint ocean_projection_matrix_location = 0;
	CHECK_GL_ERROR(ocean_projection_matrix_location =
			glGetUniformLocation(ocean_program_id, "projection"));
	GLint ocean_view_matrix_location = 0;
	CHECK_GL_ERROR(ocean_view_matrix_location =
			glGetUniformLocation(ocean_program_id, "view"));
	GLint ocean_light_position_location = 0;
	CHECK_GL_ERROR(ocean_light_position_location =
			glGetUniformLocation(ocean_program_id, "light_position"));
	GLint ocean_wireframe_toggle = 0;
	CHECK_GL_ERROR(ocean_wireframe_toggle =
			glGetUniformLocation(ocean_program_id, "wireframes"));
	GLint ocean_num_inner_level = 0;
	CHECK_GL_ERROR(ocean_num_inner_level =
			glGetUniformLocation(ocean_program_id, "inner_level"));
	GLint ocean_num_outer_level = 0;
	CHECK_GL_ERROR(ocean_num_outer_level =
			glGetUniformLocation(ocean_program_id, "outer_level"));
	GLint ocean_timer = 0;
	CHECK_GL_ERROR(ocean_timer =
			glGetUniformLocation(ocean_program_id, "timer"));

	GLint eye_coord = 0;
	CHECK_GL_ERROR(eye_coord =
			glGetUniformLocation(ocean_program_id, "eye_coord"));

	GLint ocean_wavelength = 0;
	CHECK_GL_ERROR(ocean_wavelength =
			glGetUniformLocation(ocean_program_id, "wavelength"));

	GLint ocean_amplitude = 0;
	CHECK_GL_ERROR(ocean_amplitude =
			glGetUniformLocation(ocean_program_id, "amplitude"));
	GLint ocean_direction = 0;
	CHECK_GL_ERROR(ocean_direction =
			glGetUniformLocation(ocean_program_id, "direction"));

	GLint ocean_speed = 0;
	CHECK_GL_ERROR(ocean_speed =
			glGetUniformLocation(ocean_program_id, "speed"));

	GLint ocean_tidal_x_pos = 0;
	CHECK_GL_ERROR(ocean_tidal_x_pos =
			glGetUniformLocation(ocean_program_id, "tidal_x_pos"));

	// Let's create our program.
	GLuint skybox_program_id = 0;
	CHECK_GL_ERROR(skybox_program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(skybox_program_id, sky_vertex_shader_id));
	CHECK_GL_ERROR(glAttachShader(skybox_program_id, sky_fragment_shader_id));

	// Bind attributes.
	glLinkProgram(skybox_program_id);

	// Get the uniform locations.
	GLint skybox_projection_matrix_location = 0;
	CHECK_GL_ERROR(skybox_projection_matrix_location =
			glGetUniformLocation(skybox_program_id, "projection"));
	GLint skybox_view_matrix_location = 0;
	CHECK_GL_ERROR(skybox_view_matrix_location =
			glGetUniformLocation(skybox_program_id, "view"));
	
	unsigned int textureID = loadCubemap(faces);

	glm::vec4 light_position = glm::vec4(-10.0f, 10.0f, 0.0f, 1.0f);
	float aspect = static_cast<float>(window_width) / window_height;
	glm::mat4 projection_matrix =
		glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);


	g_camera.set_proj(projection_matrix);

	//setup time
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);

		// Switch to the Geometry VAO.
		CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kGeometryVao]));

		if (g_menger && g_menger->is_dirty()) {
			g_menger->generate_geometry(obj_vertices, obj_faces);
			g_menger->set_clean();
			// Setup vertex data in a VBO.
			CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kVertexBuffer]));
			// NOTE: We do not send anything right now, we just describe it to OpenGL.
			CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
						sizeof(float) * obj_vertices.size() * 4, obj_vertices.data(),
						GL_STATIC_DRAW));

			// Setup element array buffer.
			CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_buffer_objects[kGeometryVao][kIndexBuffer]));
			CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						sizeof(uint32_t) * obj_faces.size() * 3,
						obj_faces.data(), GL_STATIC_DRAW));
			// FIXME: Upload your vertex data here.
		}

		float t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count();
		tidal_x_pos += 0.01f;
		//Save the geometry to a .obj file if needed
		if (g_save_geo) {
			SaveObj("geometry.obj", obj_vertices, obj_faces);
			g_save_geo = false;
		}

		// // Compute the projection matrix.
		// aspect = static_cast<float>(window_width) / window_height;
		// glm::mat4 projection_matrix =
		// 	glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);

		// Compute the view matrix
		glm::mat4 view_matrix = g_camera.get_view_matrix();

		// Use our program.
		CHECK_GL_ERROR(glUseProgram(program_id));

		// Pass uniforms in.
		CHECK_GL_ERROR(glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE,
					&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE,
					&view_matrix[0][0]));
		CHECK_GL_ERROR(glUniform4fv(light_position_location, 1, &light_position[0]));
		CHECK_GL_ERROR(glUniform1i(wireframe_toggle, wireframes_toggle));
		CHECK_GL_ERROR(glUniform1f(num_outer_level, outer_level));
		CHECK_GL_ERROR(glUniform1f(num_inner_level, inner_level));

		// Draw our triangles.
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));
		//glPatchParameteri(GL_PATCH_VERTICES, 3);
		//CHECK_GL_ERROR(glDrawElements(GL_PATCHES, obj_faces.size() * 3, GL_UNSIGNED_INT, 0));

		//render floor
		CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kFloorVao]));

		CHECK_GL_ERROR(glUseProgram(floor_program_id));
		CHECK_GL_ERROR(glUniformMatrix4fv(floor_projection_matrix_location, 1, GL_FALSE,
					&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniform1i(floor_wireframe_toggle, wireframes_toggle));
		CHECK_GL_ERROR(glUniformMatrix4fv(floor_view_matrix_location, 1, GL_FALSE,
					&view_matrix[0][0]));

		CHECK_GL_ERROR(glUniform1f(floor_num_outer_level, outer_level));
		CHECK_GL_ERROR(glUniform1f(floor_num_inner_level, inner_level));
		CHECK_GL_ERROR(glUniform1fv(floor_ocean_wavelength, WAVES, &wavelength[0]));
		CHECK_GL_ERROR(glUniform1fv(floor_ocean_amplitude, WAVES, &amplitude[0]));
		CHECK_GL_ERROR(glUniform1fv(floor_ocean_speed, WAVES, &speed[0]));
		CHECK_GL_ERROR(glUniform1f(floor_ocean_timer, t));
		CHECK_GL_ERROR(glUniform1i(floor_ocean_on, ocean_on));

		CHECK_GL_ERROR(glUniform2fv(floor_ocean_direction, WAVES, &direction[0]));
		CHECK_GL_ERROR(glUniform1f(floor_ocean_tidal_x_pos, tidal_x_pos));

		CHECK_GL_ERROR(glUniform4fv(floor_light_position_location, 1, &light_position[0]));
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		CHECK_GL_ERROR(glDrawElements(GL_PATCHES, plane_indices.size() * 3, GL_UNSIGNED_INT, 0));


		if(ocean_on)
		{
			//render Ocean
			CHECK_GL_ERROR(glUseProgram(ocean_program_id));
			
			CHECK_GL_ERROR(glUniformMatrix4fv(ocean_projection_matrix_location, 1, GL_FALSE, &projection_matrix[0][0]));
			CHECK_GL_ERROR(glUniformMatrix4fv(ocean_view_matrix_location, 1, GL_FALSE,
						&view_matrix[0][0]));
			CHECK_GL_ERROR(glUniform1i(ocean_wireframe_toggle, wireframes_toggle));
			CHECK_GL_ERROR(glUniform1f(ocean_num_outer_level, outer_level));
			CHECK_GL_ERROR(glUniform1f(ocean_num_inner_level, inner_level));
			CHECK_GL_ERROR(glUniform1fv(ocean_wavelength, WAVES, &wavelength[0]));
			CHECK_GL_ERROR(glUniform1fv(ocean_amplitude, WAVES, &amplitude[0]));
			CHECK_GL_ERROR(glUniform1fv(ocean_speed, WAVES, &speed[0]));
			CHECK_GL_ERROR(glUniform1f(ocean_tidal_x_pos, tidal_x_pos));
			CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

			CHECK_GL_ERROR(glUniform2fv(ocean_direction, WAVES, &direction[0]));

			CHECK_GL_ERROR(glUniform3fv(eye_coord, 1, &g_camera.get_eye()[0]));
			CHECK_GL_ERROR(glUniform1f(ocean_timer, t));
			CHECK_GL_ERROR(glUniform4fv(ocean_light_position_location, 1, &light_position[0]));
			
			CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kOceanVao]));

			glPatchParameteri(GL_PATCH_VERTICES, 4);
			CHECK_GL_ERROR(glDrawElements(GL_PATCHES, ocean_indices.size() * 4, GL_UNSIGNED_INT, 0));
		}

		//draw skybox
		glDepthMask(GL_FALSE);
		CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kCubeVao]));
		CHECK_GL_ERROR(glUseProgram(skybox_program_id));
		CHECK_GL_ERROR(glUniformMatrix4fv(skybox_projection_matrix_location, 1, GL_FALSE, &projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(skybox_view_matrix_location, 1, GL_FALSE,
					&view_matrix[0][0]));
        CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));
        CHECK_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, 36));
		glDepthMask(GL_TRUE);
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
