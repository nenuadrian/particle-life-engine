# Architecture

## Runtime Flow

The application is built around a small set of subsystems coordinated by `Application`.

1. Initialize Vulkan context, particle buffers, compute pipeline, and graphics pipeline.
2. Enter the main loop.
3. Process input and UI changes.
4. Run the compute pass to update particle positions and velocities.
5. Render the latest particle buffer to the swapchain.
6. Present the frame and repeat.

## Core Components

### `Application`

`Application` owns the top-level runtime state:

- `VulkanContext` for device and swapchain management
- `ParticleSystem` for simulation data
- `ComputePipeline` for simulation updates
- `GraphicsPipeline` for drawing

It also manages FPS tracking, UI visibility, zoom state, and per-frame flags such as reinitialization and attraction-matrix updates.

### `ParticleSystem`

`ParticleSystem` manages:

- Double-buffered particle storage on the GPU
- Simulation parameters like `deltaTime`, `frictionFactor`, and world bounds
- The attraction matrix for pairwise type interactions
- Reinitialization and world-size rescaling

### `ComputePipeline`

The compute pipeline dispatches the simulation shader each frame. It reads the current particle buffer, applies the interaction model, and writes the next particle state into the alternate buffer.

### `GraphicsPipeline`

The graphics pipeline renders the active particle buffer after compute has finished. It is responsible for drawing the simulation output onto the current swapchain image.

### `VulkanContext`

The context layer hides the Vulkan setup complexity:

- instance and physical device selection
- logical device creation
- surface and swapchain setup
- command buffers and synchronization primitives

## Data Flow

```text
ParticleSystem buffer A -> ComputePipeline -> ParticleSystem buffer B
ParticleSystem buffer B -> GraphicsPipeline -> Swapchain image
swap A/B and repeat
```

That ping-pong model keeps simulation updates and rendering decoupled while avoiding read-write hazards on a single particle buffer.

## Source Layout

| Path | Purpose |
| --- | --- |
| `src/application.*` | Frame loop, input, and UI orchestration |
| `src/vulkan_context.*` | Vulkan bootstrap and swapchain/device state |
| `src/particle_system.*` | Particle storage and simulation parameter management |
| `src/compute_pipeline.*` | Compute shader bindings and dispatch |
| `src/graphics_pipeline.*` | Graphics pipeline and rendering |
| `src/shader_utils.*` | Shader loading/helpers |
| `shaders/` | GLSL compute and graphics shader sources |
| `tests/` | Unit tests for math and particle-system logic |
