# Particle Life Engine

[![Docker](https://github.com/nenuadrian/particle-life-engine/actions/workflows/docker.yml/badge.svg)](https://github.com/nenuadrian/particle-life-engine/actions/workflows/docker.yml)
[![Cross-env Builds](https://github.com/nenuadrian/particle-life-engine/actions/workflows/build.yml/badge.svg)](https://github.com/nenuadrian/particle-life-engine/actions/workflows/build.yml)
[![Docs](https://github.com/nenuadrian/particle-life-engine/actions/workflows/docs.yml/badge.svg)](https://github.com/nenuadrian/particle-life-engine/actions/workflows/docs.yml)

GPU-accelerated Particle Life simulation using Vulkan compute shaders. Multiple particle types interact via configurable attraction/repulsion rules, producing emergent life-like behavior.

![Particle Life Engine](assets/example.png)

![Particle Life Engine 2](assets/example2.png)

## Prerequisites

- CMake 3.20+
- C++17 compiler
- Vulkan SDK

### macOS

```bash
brew install vulkan-loader vulkan-headers molten-vk shaderc
```

### Ubuntu

```bash
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list https://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list
sudo apt-get update
sudo apt-get install vulkan-sdk libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev
```

### Windows

Download and install the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows).

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

```bash
./build/particle-life-engine
```

On Windows: `build\Release\particle-life-engine.exe`

## Docker

```bash
docker build -t particle-life-engine .
```

To run with display forwarding (Linux):

```bash
docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix particle-life-engine
```

## How It Works

- Tens of thousands of particles across multiple types, each with a random attraction/repulsion matrix
- A Vulkan compute shader calculates pairwise forces every frame
- Particles wrap around a toroidal space (edges connect)
- Close-range repulsion prevents collapse; medium-range attraction/repulsion drives emergent patterns
