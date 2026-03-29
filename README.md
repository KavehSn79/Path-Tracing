# Path Tracing

This project implements a **physically-based path tracer**. In contrast to rasterization, this renderer simulates **global illumination** by tracing light paths and approximating the rendering equation using **Monte Carlo integration**.

Rays are cast from the camera into the scene, where **ray–triangle intersections** are computed to find visible surfaces. At each intersection, light transport is evaluated recursively, allowing light to bounce between objects. The implementation supports both **uniform hemisphere sampling** and **BRDF importance sampling**, including **diffuse** and **mirror materials**.

To reduce noise, the renderer uses **multiple samples per pixel** with stochastic sampling and supports **progressive accumulation** for improved image quality over time. Performance is further optimized using **bounding sphere acceleration**, reducing unnecessary intersection tests.

The application includes an interactive GUI (ImGui) to control rendering parameters such as **max depth**, **samples per pixel**, **sampling strategy**, and **bounding volume usage**. A split-screen interface shows a rasterized preview (left) and the final path-traced result (right).

---

## Result

<p align="center">
  <img src="output.png" width="600"/>
</p>

---

## Features

- Physically-based path tracing  
- Monte Carlo integration  
- Recursive light transport  
- BRDF importance sampling (diffuse & mirror)  
- Multiple samples per pixel (noise reduction)  
- Bounding sphere acceleration  
- Interactive GUI (ImGui)  
- Rasterized preview + path traced output

## **🛠️ Building the Project**

The project uses **CMake** for building. Ensure you have **CMake version**  installed and a C++ compiler capable of compiling C++20 (e.g., GCC , Clang , or MSVC ).

### **Steps to Build and Run**

Run these commands in the root directory of the project:

**1\. Configure the project:**

cmake . \-B build \-DCMAKE\_BUILD\_TYPE=Release

This command creates a build directory and configures the project files.

**2\. Build the executable:**

cmake \--build build \--parallel \--config Release

This builds the project executables, placing them inside the build directory.

3\. Run the application:  
To Run the program:
./src/main  
\# or (example for Windows)  
.\\src\\main.exe

***Note: The exact path to the executable may vary based on your CMake setup.***
