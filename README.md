# Menger Sponge Milestone 1

**Ryan Poznich**

**David Vu**

A nontrivial OpenGL program that shows a procedurally-generated fractal, the Menger sponge, with full user interaction using mouse and keyboard camera controls

**Implemented features**

* Camera movement

* Infinite ground plane with a checker board pattern

* Shaders to handle to colors of the cube and the ground plane

* Generated the geometry of a Menger sponge


# Menger Sponge Milestone 2

**Ryan Poznich**

**David Vu**

A nontrivial OpenGL program that shows a procedurally-generated fractal, the Menger sponge, with full user interaction using mouse and keyboard camera controls

**Implemented features**

* Default camera position views the whole scene
* Green Wireframes on the edges of the triangles
    - the F key toggles the wireframes on and off
    - Ctrl + F to remove all texture and just color the edges
* Tessallation Controls to increase the inner and outer tessellation levels with the keyboard
* Tessellation Evaluation for the floor
* Tessellation from Quad Mesh 
    - Created a second pair of TCS and TES to generated tessellated quads rather than triangles
    - Ctrl-O toggles the ocean on and off
* Displacement Mapping for the waves
* Adaptive Tessellation
    - creates a massive Gaussian tidal wave the moves in the positive x direction when you press ctrl-T
* Implemented a shading model for the ocean that includes specular terms

**Extra Credit**

* Made constant width lines for the wireframes
    - Done by passing in the pre-interpolated triangles to the shader when we calculate the distance to the edge of the triangles
* Created a skybox that wraps around the scene
* Implemented reflections of the skybox on the water
    - In our ocean fragment shader we're shooting a reflected ray from the ocean, then we add the color from the skybox to the ocean
* Implemented an ocean floor under ocean with caustic affects
    - Happens in the ocean floor fragment shader
    - To make the caustics we perform the following steps:
        1. Shoot a ray from the floor to the water
        2. Refract it by the normal of the water
        3. The caustic on the floor is attenuated by the angle of that ray vs the point of intersection (on the water) to the light
            - larger angle will produce a dimmer light
    - Very noticable underneath the tidal wave


**How to Add Cube Map**

* We added a file `config.h`, please look at that file in order to edit the path to your cubemap

    ```std::vector<std::string> faces
        {
                "../cubemap_autumn/autumn_positive_x.jpg",
                "../cubemap_autumn/autumn_negative_x.jpg",
                "../cubemap_autumn/autumn_positive_y.jpg",
                "../cubemap_autumn/autumn_negative_y.jpg",
                "../cubemap_autumn/autumn_positive_z.jpg",
                "../cubemap_autumn/autumn_negative_z.jpg"
        };
    ```

    - The path the the cubemap can be absolute or relative to your current terminal directory.


**Installation**

*  `mkdir build` to setup build directory
*  `cd build`
* `CMAKE ..`, if you want to build it on release build then `CMAKE --DCMAKE_BUILD_TYPE=Release ..`
* `make -j8` to build the program
* The binary should be generated inside `./build/bin`



*Water Caustics*
![Alt Text](https://media.giphy.com/media/vFKqnCdLPNOKc/giphy.gif)

*Water + Tidal Wave + Reflection*
![Alt Text](https://media.giphy.com/media/XJeM1TiY3jI6JZKI4Z/giphy-downsized-large.gif)