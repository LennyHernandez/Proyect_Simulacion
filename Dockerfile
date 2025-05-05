# Usar una imagen base de Ubuntu LTS razonablemente reciente
FROM ubuntu:22.04

# --- NUEVA VERSIÓN DEL SDK ---
ARG VULKAN_SDK_VERSION="1.4.309.0"
# --- URL ACTUALIZADA (con versión y formato .tar.xz) ---
ARG VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.xz"

# Etiqueta para identificar la imagen
LABEL maintainer="Tu Nombre <tu_email@example.com>"
LABEL description="Build environment for Particle Simulation with Vulkan SDK ${VULKAN_SDK_VERSION}"

# Evitar prompts interactivos durante apt-get install
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias básicas y de compilación
# Añadir 'xz-utils' necesario para descomprimir .tar.xz
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    tar \
    xz-utils \ # <-- AÑADIDO para .tar.xz
    # Dependencias comunes de X11 para GLFW en Linux
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libxxf86vm-dev \
    # Limpiar caché de apt
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Crear directorio temporal
WORKDIR /tmp/vulkan_install

# Descargar e instalar el Vulkan SDK (con comandos actualizados para .tar.xz)
RUN echo "Downloading Vulkan SDK ${VULKAN_SDK_VERSION} from ${VULKAN_SDK_URL}" && \
    wget --progress=bar:force:noscroll -O vulkansdk.tar.xz ${VULKAN_SDK_URL} && \ # <-- Cambiado a .tar.xz
    mkdir /vulkan_sdk && \
    # Usar tar -xJf para descomprimir .tar.xz
    tar -xJf vulkansdk.tar.xz --strip-components=1 -C /vulkan_sdk && \ # <-- Cambiado comando tar
    rm vulkansdk.tar.xz && \ # <-- Cambiado archivo a borrar
    cd / && rm -rf /tmp/vulkan_install

# Configurar variables de entorno (sin cambios)
ENV VULKAN_SDK=/vulkan_sdk/x86_64
ENV PATH="${VULKAN_SDK}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${VULKAN_SDK}/lib:${LD_LIBRARY_PATH}"
ENV VK_LAYER_PATH="${VULKAN_SDK}/etc/vulkan/explicit_layer.d"

# Establecer directorio de trabajo
WORKDIR /app

# Mensaje final
CMD echo "Vulkan SDK ${VULKAN_SDK_VERSION} build environment ready. Mount your source code to /app and run build commands."