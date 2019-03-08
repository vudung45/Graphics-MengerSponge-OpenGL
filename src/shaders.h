
const char* vertex_shader =
R"zzz(#version 410 core
layout (location = 0) in vec4 vertex_position;

uniform mat4 view;
uniform vec4 light_position;
out vec4 vs_light_direction;
void main()
{
	gl_Position = vertex_position;
	vs_light_direction =  light_position - gl_Position;
}
)zzz";
	
const char* sky_vertex_shader =
R"zzz(#version 330 core
layout (location = 0) in vec3 vertex_position;

out vec3 vertex_pos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vertex_pos = vertex_position;
    gl_Position = projection * view * vec4(vertex_position,1.0);
}
)zzz"
;

const char* sky_fragment_shader =
R"zzz(#version 330 core
out vec4 fragment_color;

in vec3 vertex_pos;

uniform samplerCube skybox;

void main()
{    
    fragment_color = vec4(texture(skybox, vec3(vertex_pos)).rgb,1.0);
}  
)zzz"
;


const char* TCS_shader =
R"zzz(#version 410 core

layout(vertices = 3) out;
uniform float outer_level;
uniform float inner_level;

void main()
{
	vec4 vertex = gl_in[gl_InvocationID].gl_Position;
	gl_TessLevelOuter[0] = outer_level;
	gl_TessLevelOuter[1] = outer_level;
	gl_TessLevelOuter[2] = outer_level;
	gl_TessLevelInner[0] = inner_level;
	gl_out[gl_InvocationID].gl_Position = vertex;
}
)zzz";

const char* TES_shader =
R"zzz(#version 410 core
layout(triangles, equal_spacing, ccw) in;
uniform vec4 light_position;
out vec4 vs_light_direction;

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	
	gl_Position = u * gl_in[0].gl_Position + v * gl_in[1].gl_Position + (1-u-v) * gl_in[2].gl_Position;

	vs_light_direction =  light_position - gl_Position;
}

)zzz";

const char* ocean_TCS_shader =
R"zzz(#version 410 core

layout(vertices = 4) out;
uniform float outer_level;
uniform float inner_level;
uniform float tidal_x_pos;
void main()
{
	vec4 vertex = gl_in[gl_InvocationID].gl_Position;
	float dist = 0;
	for(int i = 0; i < 4; i++)
	{
		dist += distance(gl_in[i].gl_Position,vec4(tidal_x_pos, gl_in[i].gl_Position[1], 0, 1.0f));
	}
	dist /= 4;
	dist = clamp(4.0f - dist,1.0f,4.0f);
	gl_TessLevelOuter[0] = outer_level * pow(dist,2);
	gl_TessLevelOuter[1] = outer_level * pow(dist,2);
	gl_TessLevelOuter[2] = outer_level * pow(dist,2);
	gl_TessLevelOuter[3] = outer_level * pow(dist,2);
	gl_TessLevelInner[0] = inner_level * pow(dist,2);
	gl_TessLevelInner[1] = inner_level * pow(dist,2);
	gl_out[gl_InvocationID].gl_Position = vertex;
}
)zzz";

const char* ocean_TES_shader =
R"zzz(#version 410 core
layout(quads, equal_spacing, ccw) in;
uniform vec4 light_position;
uniform float timer[4];
uniform float amplitude[4];
uniform float wavelength[4];
uniform float speed[4];
uniform float tidal_x_pos;
uniform vec2 direction[4];
const float pi = 3.14159;
out vec4 vs_light_direction;
out vec4 v_normal; 
float _dx(int n, float x, float y) {
    float frequency = 2*pi/wavelength[n];
    float phase = speed[n] * frequency;
    float theta = dot(direction[n], vec2(x, y));
    float A = amplitude[n] * direction[n].x * frequency;
    return A * cos(theta * frequency + timer[n] * phase); 
}

float _dy(int n, float x, float y) {
    float frequency = 2*pi/wavelength[n];
    float phase = speed[n] * frequency;
    float theta = dot(direction[n], vec2(x, y));
    float A = amplitude[n] * direction[n].y * frequency;
    return A * cos(theta * frequency + timer[n] * phase);
}

vec3 get_normal(float x, float y) {
	float dx = 0;
	float dy = 0;
	for(int n = 0; n < 4; ++n)
	{
	    dx += _dx(n, x, y);
	    dy += _dy(n, x, y);
	}
    vec3 _n = vec3(-dx, 1.0, -dy);
    return normalize(_n);
}

vec3 get_normal_gaussian(float x, float y)
{
	float dx = 4.0*-8*(-tidal_x_pos + x)*exp(-pow(2*pow(-tidal_x_pos + x,2) + 2 * pow(y,2),2)) * (2* pow(-tidal_x_pos + x,2) + 2 * pow(y,2));
	float dy = 4.0*-8*y*exp(-pow(2*pow(-tidal_x_pos + x,2) + 2 * pow(y,2),2))  * (2* pow(-tidal_x_pos + x,2) + 2 * pow(y,2));
	vec3 _n = vec3(-dx, 1.0, -dy);
	return normalize(_n);
}

float getHeight(int i, float x, float y)
{
	float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
	return amplitude[i] * sin(theta * frequency + timer[i] * phase);
}

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	
	gl_Position = (1-u) * (1-v) * gl_in[0].gl_Position 
				+ u * (1-v) * gl_in[1].gl_Position
				+ u * v * gl_in[2].gl_Position 
				+ (1-u) * v * gl_in[3].gl_Position;

	float d = distance(gl_Position,vec4(tidal_x_pos, gl_Position.y, 0, 1.0f));
	
	if(d > 1.0f) 
	{
		for(int i = 0; i < 4 ; ++i)
		{
			gl_Position.y += getHeight(i, gl_Position.x, gl_Position.z);
			
		}
		v_normal = vec4(get_normal(gl_Position.x, gl_Position.z),0.0f);
	}else 
	{
		gl_Position.y += 4.0 * exp(-pow(pow(gl_Position.x - tidal_x_pos,2.0)/0.5 + pow(gl_Position.z,2.0)/0.5 ,2.0));	
		v_normal = vec4(get_normal_gaussian(gl_Position.x, gl_Position.z),0.0f);
	}

	vs_light_direction =  light_position - gl_Position;
}

)zzz";


const char* geometry_shader =
R"zzz(#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
in vec4 vs_light_direction[];
flat out vec4 normal;
out vec4 normalized_coords;
flat out vec4 f_gl_positions[3]; 
out vec4 proj_coord;
out vec4 light_direction;
out vec4 world_position;
flat out float area;
void main()
{
	int n = 0;
	vec3 b_coords = vec3(gl_in[1].gl_Position);
	vec3 a_coords = vec3(gl_in[0].gl_Position);
	vec3 c_coords = vec3(gl_in[2].gl_Position);
	vec3 norm = cross(b_coords - a_coords, c_coords - a_coords);
	norm = normalize(norm);
	normal = vec4(norm, 0.0);
	for (n = 0; n < gl_in.length(); n++) {
		light_direction = vs_light_direction[n];
		gl_Position = projection * view * gl_in[n].gl_Position;
		f_gl_positions[n] = gl_Position;
		proj_coord = gl_Position; 
		world_position = gl_in[n].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)zzz";


const char* ocean_geometry_shader =
R"zzz(#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
uniform vec4 light_position;
in vec4 v_normal[];
in vec4 vs_light_direction[];
out vec4 normal;
flat out vec4 f_gl_positions[3]; 
out vec4 proj_coord;
out vec4 light_direction;
out vec4 world_position;
flat out float area;

void main()
{
	int n = 0;
	vec3 b_coords = vec3(gl_in[1].gl_Position);
	vec3 a_coords = vec3(gl_in[0].gl_Position);
	vec3 c_coords = vec3(gl_in[2].gl_Position);
	for (n = 0; n < gl_in.length(); n++) {
		light_direction = vs_light_direction[n];
		normal = v_normal[n];
		gl_Position = projection * view * gl_in[n].gl_Position;
		f_gl_positions[n] = gl_Position;
		proj_coord = gl_Position; 
		world_position = gl_in[n].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)zzz";

const char* ocean_fragment_shader =
R"zzz(#version 410 core
in vec4 normal;
flat in float area;
uniform samplerCube skybox;
uniform bool wireframes;
uniform vec3 eye_coord;
in vec4 light_direction;
in vec4 world_position;
out vec4 fragment_color;
in vec4 proj_coord;
flat in vec4 f_gl_positions[];

float distance(vec2 p1, vec2 p2, vec2 point)
{
	return abs((p2.y - p1.y)*point.x - (p2.x - p1.x)*point.y + p2.x*p1.y - p2.y*p1.x)
          / sqrt(pow(p2.y - p1.y,2.0)+ pow(p2.x - p1.x,2.0));
}
void main()
{
	vec2 a_coords = f_gl_positions[0].xy;
	vec2 b_coords = f_gl_positions[1].xy;
	vec2 c_coords = f_gl_positions[2].xy;
	vec2 point = proj_coord.xy;
	float dist1 = distance(a_coords, b_coords, point);
	float dist2 = distance(b_coords, c_coords, point);
	float dist3 = distance(a_coords, c_coords, point);
	if(wireframes && (dist1 < 0.01f || dist2 < 0.01f  || dist3  < 0.01f))
	{
		fragment_color = vec4(0, 1.0, 0.0,1.0);
	} else {
		vec3 kd_color = vec3(0.15, 0.2, 0.23);
		vec3 ks_color = vec3(1.0, 1.0, 1.0);
		vec3 ambient = vec3(0.15, 0.2, 0.23);
		float shininess = 7000;
		float kr = 0.7;
		vec4 v = normalize(vec4(eye_coord,0.0f)-world_position);
		float dot_nl = dot(normalize(light_direction), normalize(normal));
		float dot_vr = dot(normalize(reflect(-light_direction,normalize(normal))), v);
		dot_nl = max(dot_nl,0);
		dot_vr = max(dot_vr, 0.0);

		vec4 R = reflect(-v, normalize(normal));
		vec3 skybox_color = texture(skybox, vec3(R)).rgb * kr;
		fragment_color = vec4(clamp(skybox_color + ambient + dot_nl * kd_color  + pow(dot_vr * ks_color , vec3(7000,7000,7000)), 0.0, 1.0),0.3);
	}
}
)zzz";

;

const char* fragment_shader =
R"zzz(#version 410 core
flat in vec4 normal;
flat in float area;
uniform bool wireframes;
in vec4 light_direction;
in vec4 world_position;
flat in vec4 f_gl_positions[3]; 
in vec4 proj_coord;
out vec4 fragment_color;

float distance(vec2 p1, vec2 p2, vec2 point)
{
	return abs((p2.y - p1.y)*point.x - (p2.x - p1.x)*point.y + p2.x*p1.y - p2.y*p1.x)
          / sqrt(pow(p2.y - p1.y,2.0)+ pow(p2.x - p1.x,2.0));
}
void main()
{

	vec2 a_coords = f_gl_positions[0].xy;
	vec2 b_coords = f_gl_positions[1].xy;
	vec2 c_coords = f_gl_positions[2].xy;
	vec2 point = proj_coord.xy;
	float dist1 = distance(a_coords, b_coords, point);
	float dist2 = distance(b_coords, c_coords, point);
	float dist3 = distance(a_coords, c_coords, point);
	if(wireframes && (dist1 < 0.01f || dist2 < 0.01f  || dist3  < 0.01f))
	{
		fragment_color = vec4(0, 1.0, 0.0,1.0);
	} else {
		vec3 color = vec3(float(abs(normal[0]) > 0.0), float(abs(normal[1]) > 0.0), 
								float(abs(normal[2]) > 0));

		float dot_nl = dot(normalize(light_direction), normalize(normal));

		dot_nl = clamp(dot_nl, 0.0, 1.0);

		fragment_color = vec4(clamp(dot_nl * color, 0.0, 1.0),1.0);
	}

}
)zzz";

// FIXME: Implement shader effects with an alternative shader.
const char* floor_fragment_shader =
R"zzz(#version 410 core
flat in vec4 normal;
flat in float area;
const float pi = 3.14159;
uniform bool wireframes;
uniform float timer[4];
uniform float amplitude[4];
uniform float wavelength[4];
uniform float speed[4];
uniform vec2 direction[4];
uniform bool ocean_on;
uniform vec4 light_position;
in vec4 light_direction;
in vec4 world_position;
out vec4 fragment_color;
uniform samplerCube skybox;
in vec4 proj_coord;
flat in vec4 f_gl_positions[];
uniform float tidal_x_pos;

float _dx(int n, float x, float y) {
    float frequency = 2*pi/wavelength[n];
    float phase = speed[n] * frequency;
    float theta = dot(direction[n], vec2(x, y));
    float A = amplitude[n] * direction[n].x * frequency;
    return A * cos(theta * frequency + timer[n] * phase); 
}

float _dy(int n, float x, float y) {
    float frequency = 2*pi/wavelength[n];
    float phase = speed[n] * frequency;
    float theta = dot(direction[n], vec2(x, y));
    float A = amplitude[n] * direction[n].y * frequency;
    return A * cos(theta * frequency + timer[n] * phase);
}

vec3 get_normal(float x, float y) {
	float dx = 0;
	float dy = 0;
	for(int n = 0; n < 4; ++n)
	{
	    dx += _dx(n, x, y);
	    dy += _dy(n, x, y);
	}
    vec3 _n = vec3(-dx, 1.0, -dy);
    return normalize(_n);
}

vec3 get_normal_gaussian(float x, float y)
{
	float dx = 4.0*-8*(-tidal_x_pos + x)*exp(-pow(2*pow(-tidal_x_pos + x,2) + 2 * pow(y,2),2)) * (2* pow(-tidal_x_pos + x,2) + 2 * pow(y,2));
	float dy = 4.0*-8*y*exp(-pow(2*pow(-tidal_x_pos + x,2) + 2 * pow(y,2),2))  * (2* pow(-tidal_x_pos + x,2) + 2 * pow(y,2));
	vec3 _n = vec3(-dx, 1.0, -dy);
	return normalize(_n);
}


float _distance(vec2 p1, vec2 p2, vec2 point)
{
	return abs((p2.y - p1.y)*point.x - (p2.x - p1.x)*point.y + p2.x*p1.y - p2.y*p1.x)
          / sqrt(pow(p2.y - p1.y,2.0)+ pow(p2.x - p1.x,2.0));
}
void main()
{

	vec2 a_coords = f_gl_positions[0].xy;
	vec2 b_coords = f_gl_positions[1].xy;
	vec2 c_coords = f_gl_positions[2].xy;
	vec2 point = proj_coord.xy;
	float dist1 = _distance(a_coords, b_coords, point);
	float dist2 = _distance(b_coords, c_coords, point);
	float dist3 = _distance(a_coords, c_coords, point);
	if(wireframes && (dist1 < 0.01f || dist2 < 0.01f  || dist3  < 0.01f))
	{
		fragment_color = vec4(0, 1.0, 0.0,1.0);
	} else {
		
		float c = float((int(world_position.x - (-1000.0))  + int(world_position.z - (-1000.0)))%2);
		float dot_nl = dot(normalize(light_direction), normalize(normal));
		dot_nl = max(dot_nl, 0.0);
		vec3 color = vec3(c,c,c);
		if(ocean_on)
		{
			vec3 water_normal;
			//generate causetic ray
			float d = distance(world_position ,vec4(tidal_x_pos, world_position.y, 0, 1.0f));
			if(d > 1.0f)
			{
				water_normal = get_normal(world_position.x,world_position.z);
			} else 
			{
				water_normal = get_normal_gaussian(world_position.x,world_position.z);

			}

			float ratio =  1.52 / 1.00;
			vec3 R = normalize(refract(vec3(0,1.0,0), -water_normal, ratio)); //refraction ray from the bottom
			vec3 _l = normalize(vec3(light_position) - vec3(world_position.x,-1.0f,world_position.z)); //intercept point to light
			float light_map_factor = 5.0f;
			vec3 caustic_color = vec3(1.0,1.0,1.0) * max(pow((1-dot(normalize(vec3(_l)),R)),light_map_factor),0);
			fragment_color = vec4(clamp(dot_nl * color + caustic_color , 0.0, 1.0),1.0);
		} 
		else 
		{
			fragment_color = vec4(clamp(dot_nl * color , 0.0, 1.0),1.0);

		}
	}	
}

)zzz";
