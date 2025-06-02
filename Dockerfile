FROM ubuntu:22.04

# 1. Instalar dependencias básicas (optimizado)
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    doxygen \
    libboost-all-dev \
    libglfw3-dev \
    libx11-dev \
    libgl1-mesa-dev \
    libxrandr-dev \
    libxi-dev \
    libxcursor-dev \
    vulkan-tools \
    libvulkan-dev \
    glslang-tools \
    libxinerama-dev \
    libxext-dev \
    libwayland-dev \
    libwayland-bin \
    libxkbcommon-dev \
    wayland-protocols \
    extra-cmake-modules \
    wget && \
    rm -rf /var/lib/apt/lists/*

# 2. Instalar Vulkan SDK (versión ligera)
RUN wget -qO- https://sdk.lunarg.com/sdk/download/1.4.309.0/linux/vulkansdk-linux-x86_64-1.4.309.0.tar.xz | \
    tar -xJ -C /opt --strip-components=1 && \
    /opt/vulkansdk --default-answer --accept-licenses --disable-updates

# 3. Configurar entorno (simplificado)
ENV VULKAN_SDK=/opt/x86_64
ENV PATH="$VULKAN_SDK/bin:$PATH"
ENV LD_LIBRARY_PATH=/opt/x86_64/lib:/usr/local/lib
ENV VK_LAYER_PATH="$VULKAN_SDK/etc/vulkan/explicit_layer.d"

# 4. Sistema de shaders (2 opciones)
WORKDIR /app

# Opción A: Usar shaders precompilados desde host (recomendado)
COPY Shaders/*.spv ./Shaders/

# Opción B: Compilar shaders durante el build (alternativa)
COPY Shaders/*.vert Shaders/*.frag ./Shaders/
RUN glslc Shaders/*.vert -o Shaders/vert.spv && \
    glslc Shaders/*.frag -o Shaders/frag.spv
# 5. Copiar solo lo necesario (mejor práctica)
COPY CMakeLists.txt .
COPY src/ ./src/

# 6. Construcción del proyecto (con verificación)
RUN cmake -Bbuild -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSHADER_DIR=/app/Shaders && \
    cmake --build build --verbose
    

# 7. Runtime optimizado (multi-stage opcional)
CMD ["./build/src/ParticleSimulation"]