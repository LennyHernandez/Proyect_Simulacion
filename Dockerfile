FROM ubuntu:22.04

# 1. Instalar dependencias del sistema y Vulkan
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake ninja-build git pkg-config doxygen \
    libboost-all-dev libglfw3-dev libx11-dev libgl1-mesa-dev \
    libxrandr-dev libxi-dev libxcursor-dev vulkan-tools libvulkan-dev \
    glslang-tools libxinerama-dev libxext-dev libwayland-dev libwayland-bin \
    libxkbcommon-dev wayland-protocols wget && \
    rm -rf /var/lib/apt/lists/*

# 2. Instalar Vulkan SDK (vía LunarG, descomprimir manualmente)
RUN wget -qO vulkansdk.tar.xz https://sdk.lunarg.com/sdk/download/1.3.275.0/linux/vulkansdk-linux-x86_64-1.3.275.0.tar.xz && \
    mkdir -p /opt/vulkan-sdk && \
    tar -xJf vulkansdk.tar.xz -C /opt/vulkan-sdk --strip-components=1 && \
    rm vulkansdk.tar.xz

# 3. Configurar entorno Vulkan
ENV VULKAN_SDK=/opt/vulkan-sdk/x86_64
ENV PATH=$VULKAN_SDK/bin:$PATH
ENV LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
ENV VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d

# 4. Directorio de trabajo
WORKDIR /app

# 5. Copiar shaders y código
COPY Shaders/*.vert Shaders/*.frag ./Shaders/
RUN glslc Shaders/*.vert -o Shaders/vert.spv && \
    glslc Shaders/*.frag -o Shaders/frag.spv

COPY CMakeLists.txt .
COPY src/ ./src/

# 6. Compilar
RUN cmake -Bbuild -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DSHADER_DIR=/app/Shaders && \
    cmake --build build --verbose

# 7. Ejecutar
CMD ["./build/src/ParticleSimulation"]
