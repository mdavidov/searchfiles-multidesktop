# Use a base image with Qt pre-installed or install it
FROM ubuntu:25.10

# Install necessary dependencies for a Qt QML application
RUN apt update && apt install -y \
    build-essential \
    gdb valgrind \
    git \
    cmake \
    ninja-build \
    pkg-config \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-declarative-dev \
    qt6-wayland \
    qml-module-qtquick-* \
    libqt6gui6 \
    libqt6core6 \
    libqt6widgets6 \
    libwayland-client0 \
    libqt6opengl6-dev \
    libqt6quickwidgets6 \
    libqt6quickcontrols2-6 \
    libxkbcommon-x11-0 \
    libpulse0 \
    libopengl-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libgl1-mesa-dri \
    # libgl1-mesa-glx \
    # libegl1-mesa \
    libxext-dev \
    libxrender-dev \
    libxtst-dev \
    libwayland-dev \
    libwayland-egl1 \
    libegl1-mesa-dev \
    libgbm-dev \
    libxkbcommon-dev \
    libdrm-dev \
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    wayland-protocols \
    libxkbcommon0 \
    libxcb1 \
    libx11-xcb1 \
    libx11-dev \
    libxcb-util1 \
    libxcb-xinerama0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render0 \
    libxcb-shape0 \
    libxcb-shm0 \
    libxcb-sync1 \
    libxcb-xfixes0 \
    libxcb-randr0 \
    libxcb-glx0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0 \
    libxkbcommon0

# Set the working directory inside the container
WORKDIR /app

# Copy your application source code into the container
COPY . /app

# Build your Qt application (assuming a CMake-based project)
RUN cd /app
RUN rm -rf build && mkdir build
RUN cmake -S . -B build -G "Ninja Multi-Config"
RUN cmake --build build --config Debug

# Set environment variables
ENV XDG_RUNTIME_DIR=/tmp/xdg-runtime
RUN mkdir -p /tmp/xdg-runtime && chmod 700 /tmp/xdg-runtime
ENV DISPLAY=:0
# ENV QT_QPA_PLATFORM=wayland
ENV QT_QPA_PLATFORM=offscreen
ENV QT_DEBUG_PLUGINS=1

# Expose any necessary ports if your application has network features
# EXPOSE 8080

# Command to run your application when the container starts
CMD ["./build/Debug/foldersearch"]
