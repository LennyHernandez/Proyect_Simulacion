# Usar una imagen base de Ubuntu LTS razonablemente reciente
FROM ubuntu:22.04

# Argumento para la versión del SDK de Vulkan (ajusta según necesites)
ARG VULKAN_SDK_VERSION="1.3.296.0"
# URL de descarga (verifica si cambia en el sitio de LunarG para futuras versiones)
ARG VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/${VULKAN_SDK_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_SDK_VERSION}.tar.gz"

# Etiqueta para identificar la imagen
LABEL maintainer="Tu Nombre <Lenny >"
LABEL description="Build environment for Particle Simulation with Vulkan SDK ${VULKAN_SDK_VERSION}"

# Evitar prompts interactivos durante apt-get install
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias básicas y de compilación
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    tar \
    # Dependencias comunes de X11 para GLFW en Linux (incluso si no se muestra)
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libxxf86vm-dev \
    # Limpiar caché de apt para reducir tamaño
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Crear directorio temporal para descargar/instalar SDK
WORKDIR /tmp/vulkan_install

# Descargar e instalar el Vulkan SDK
RUN echo "Downloading Vulkan SDK ${VULKAN_SDK_VERSION} from ${VULKAN_SDK_URL}" && \
    wget --progress=bar:force:noscroll -O vulkansdk.tar.gz ${VULKAN_SDK_URL} && \
    # Crear directorio destino para el SDK
    mkdir /vulkan_sdk && \
    # Extraer el SDK al directorio destino
    tar -xzf vulkansdk.tar.gz --strip-components=1 -C /vulkan_sdk && \
    # Limpiar el directorio temporal
    rm vulkansdk.tar.gz && \
    # Opcional: Ejecutar el script de setup del SDK si es necesario (generalmente no para build)
    # cd /vulkan_sdk && ./vulkansdk --accept-licenses --default-answer --confirm-command install && \
    cd / && rm -rf /tmp/vulkan_install

# Configurar variables de entorno para que CMake/compilador encuentren el SDK
ENV VULKAN_SDK=/vulkan_sdk/x86_64
ENV PATH="${VULKAN_SDK}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${VULKAN_SDK}/lib:${LD_LIBRARY_PATH}"
ENV VK_LAYER_PATH="${VULKAN_SDK}/etc/vulkan/explicit_layer.d"

# Establecer directorio de trabajo para el código de la aplicación
WORKDIR /app

# (Opcional) Establecer un punto de entrada por defecto si corres el contenedor interactivamente
# ENTRYPOINT ["/bin/bash"]

# Mensaje final
CMD echo "Vulkan SDK ${VULKAN_SDK_VERSION} build environment ready. Mount your source code to /app and run build commands."