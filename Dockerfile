# Usar una imagen base de Ubuntu LTS razonablemente reciente
FROM ubuntu:22.04

# --- SDK VERSION ---
ARG VULKAN_SDK_VERSION="1.4.309.0"
ARG VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.xz"

# Etiqueta
LABEL maintainer="Tu Nombre <tu_email@example.com>"
LABEL description="Build environment for Particle Simulation with Vulkan SDK ${VULKAN_SDK_VERSION}"

# Evitar prompts
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    tar \
    xz-utils \
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libxxf86vm-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Crear directorio temporal
WORKDIR /tmp/vulkan_install

# Descargar e instalar el Vulkan SDK (Añadido --no-check-certificate)
RUN echo "Downloading Vulkan SDK ${VULKAN_SDK_VERSION} from ${VULKAN_SDK_URL}" && \
    wget --no-check-certificate --progress=bar:force:noscroll -O vulkansdk.tar.xz ${VULKAN_SDK_URL} && \ 
    echo "Creating directory /vulkan_sdk" && \
    mkdir /vulkan_sdk && \
    echo "Extracting SDK..." && \
    tar -xJf vulkansdk.tar.xz --strip-components=1 -C /vulkan_sdk && \
    echo "Cleaning up..." && \
    rm vulkansdk.tar.xz && \
    cd / && rm -rf /tmp/vulkan_install && \
    echo "Vulkan SDK installed."
# Configurar variables de entorno
ENV VULKAN_SDK=/vulkan_sdk/x86_64
ENV PATH="${VULKAN_SDK}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${VULKAN_SDK}/lib:${LD_LIBRARY_PATH}"
ENV VK_LAYER_PATH="${VULKAN_SDK}/etc/vulkan/explicit_layer.d"

# Establecer directorio de trabajo
WORKDIR /app

# Mensaje final
CMD echo "Vulkan SDK ${VULKAN_SDK_VERSION} build environment ready. Mount source code to /app."