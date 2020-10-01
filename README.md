# mod1 [[42](https://www.42.fr/) project]

![demo](../assets/readme_assets/screenshot.png?raw=true)

## 🌊 Project

A simple water flow simulation over a terrain.

The project is fully written in C++ with the OpenGL framework.

## 💻 Installation

You need `CMake` 3.13 or above
```bash
git clone --recursive https://github.com/zer0nim/mod1
cd mod1
mkdir build && cd build
cmake ..
make
```

 ## 🚀 Usage

The program uses at least one **map file** as input.
The file contains some points height to define were the Terrain should pass.
You can view  some example maps format in the `asset/map` folder.

```usage
usage: ./mod1 <map1.mod1> <map2.mod1> ...
```

 ```bash
 ./mod1 asset/map/example1.mod1 asset/map/example2.mod1 asset/map/example3.mod1 asset/map/example4.mod1
 ```

If you want, you can edit some settings (resolution, keys, ...). After starting the program at least once, modify the `configs/settings.json` and/or `configs/controls.json` files.

### Wave demo
![demo wave 1](../assets/readme_assets/wave1.gif?raw=true)
![demo wave 2](../assets/readme_assets/wave2.gif?raw=true)
### Sandbox demo, add water were you want
![demo sandbox](../assets/readme_assets/sandbox1.gif?raw=true)
### Drain demo, water is drained from the ground below a certain height
![demo drain](../assets/readme_assets/drain1.gif?raw=true)

## 📔 References

### Water flow simulation
For the water simulation, I used the Pipe Method. Only the water surface is represented with a height field, modeling the flow between adjacent columns of fluid by using a system of virtual pipe connection.

[Large-Scale Water Simulation in Games. Kellomäki, Timo (2015)](https://tutcris.tut.fi/portal/files/4312220/kellomaki_1354.pdf)

[Real-Time Fluid Simulation Using Height Fields, Bálint Miklós, Advisor Dr. Matthias Müller, Prof Dr, Markus Gross, Eth Zürich](http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=B33B5964453724398C32E8B7FEE26AEC?doi=10.1.1.138.5153&rep=rep1&type=pdf)

[Dynamic Simulation of Splashing Fluids, James F. O’Brien and Jessica K. Hodgins](http://graphics.berkeley.edu/papers/Obrien-DSS-1995-04/Obrien-DSS-1995-04.pdf)

### Terrain interpolation

The Terrain is represented by a height field, the terrain should cross some points defined by a map file (example in the asset/map folder).
Then I interpolate the other points by using an Inverse Distance Weighting Interpolation algorithm.

### Terrain/water rendering

I render the terrain and water height field using a single OpenGL triangle strip.
[triangle strip to render surface, Kevin](https://www.learnopengles.com/tag/triangle-strips/)

The water use the skybox environment mapping to simulate reflection.

[learnopengl.com cubemaps, Joey de Vries](https://learnopengl.com/Advanced-OpenGL/Cubemaps)

### Mouse Picking with Ray Casting
For the sandbox simulation, I used raycast to detect were to put water in the ground.
[OpenGL 3D Mouse Picking Tutorial, ThinMatrix](https://youtu.be/DLKN0jExRIM)
[Mouse Picking with Ray Casting, Anton Gerdelan](https://antongerdelan.net/opengl/raycasting.html)