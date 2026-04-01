FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Add LunarG Vulkan SDK repository (provides glslc via vulkan-sdk)
RUN apt-get update && apt-get install -y wget gnupg2 && \
    wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc \
        | tee /etc/apt/trusted.gpg.d/lunarg.asc && \
    wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list \
        https://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list && \
    apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libwayland-dev \
    libxkbcommon-dev \
    vulkan-sdk \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-DNDEBUG" \
    && cmake --build build --parallel $(nproc)

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libvulkan1 \
    mesa-vulkan-drivers \
    libwayland-client0 \
    libx11-6 \
    libxrandr2 \
    libxinerama1 \
    libxcursor1 \
    libxi6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/particle-life-engine .
COPY --from=builder /app/build/shaders ./shaders

# For headless/CI validation. To run with display:
# docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix particle-life-engine
ENV VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.x86_64.json

ENTRYPOINT ["./particle-life-engine"]
