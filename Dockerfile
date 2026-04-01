FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    gnupg2 \
    software-properties-common \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libwayland-dev \
    libxkbcommon-dev \
    vulkan-tools \
    libvulkan-dev \
    vulkan-validationlayers \
    glslang-tools \
    spirv-tools \
    shaderc \
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
