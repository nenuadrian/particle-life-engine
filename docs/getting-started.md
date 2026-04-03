# Getting Started

## Prerequisites

- CMake `3.20+`
- A C++17 compiler
- Vulkan SDK or platform Vulkan loader/toolchain

## Install Dependencies

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

Install the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows).

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The build also compiles the GLSL shaders in `shaders/` into SPIR-V for runtime use.

## Run

### macOS and Linux

```bash
./build/particle-life-engine
```

### Windows

```powershell
build\Release\particle-life-engine.exe
```

## Docker

Build the image:

```bash
docker build -t particle-life-engine .
```

Run with X11 display forwarding on Linux:

```bash
docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix particle-life-engine
```

## Controls

- `Tab` toggles the UI
- Zoom controls are available in the application loop
- The ImGui panel exposes particle count, type count, and interaction tuning

## Local Docs Preview

Install the docs toolchain and run the development server:

```bash
python3 -m pip install -r requirements-docs.txt
python3 -m mkdocs serve
```
