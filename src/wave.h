#define WAVES 4

float outer_level = 7.0;
float inner_level = 7.0;
float water_height = 3.0f;
float amplitude[WAVES] = {0.2f,0.5f,0.1f,0.3f};
float tidal_x_pos = 30.0f;
float wavelength[WAVES] = {4.0f,9.0f,5.0f,5.0f};
float speed[WAVES] = {0.01f,0.01f,0.01f,0.01f};
float direction[WAVES*2] = {0.3,-0.8,-0.5,0.4,-0.2,0.4,-0.8,-0.8};