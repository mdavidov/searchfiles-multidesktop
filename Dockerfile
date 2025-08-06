# Use a base image with Qt pre-installed or install it
FROM ubuntu:25.10

# Install necessary dependencies for a Qt QML application
RUN apt update
RUN apt install -y build-essential
RUN apt install -y gdb
RUN apt install -y valgrind
RUN apt install -y git
RUN apt install -y cmake
RUN apt install -y ninja-build
RUN apt install -y pkg-config
RUN apt install -y qt6-base-dev
RUN apt install -y qt6-base-dev-tools
RUN apt install -y qt6-tools-dev
RUN apt install -y qt6-tools-dev-tools
RUN apt install -y qt6-declarative-dev
RUN apt install -y qt6-wayland
RUN apt install -y qml-module-qtquick-*
RUN apt install -y libqt6gui6
RUN apt install -y libqt6core6
RUN apt install -y libqt6widgets6
RUN apt install -y libqt6waylandclient6
RUN apt install -y libqt6waylandcompositor6
RUN apt install -y libwayland-client0
RUN apt install -y libwayland-cursor0
RUN apt install -y libwayland-server0
RUN apt install -y libqt6opengl6-dev
RUN apt install -y libqt6quickwidgets6
RUN apt install -y libqt6quickcontrols2-6
RUN apt install -y libxkbcommon-x11-0
RUN apt install -y libpulse0
RUN apt install -y libopengl-dev
RUN apt install -y libgl1-mesa-dev
RUN apt install -y libglu1-mesa-dev
RUN apt install -y libgl1-mesa-dri
# RUN apt install -y libgl1-mesa-glx
# RUN apt install -y libegl1-mesa
RUN apt install -y libfontconfig1
RUN apt install -y libxext-dev
RUN apt install -y libxrender-dev
RUN apt install -y libxtst-dev
RUN apt install -y libwayland-dev
RUN apt install -y libwayland-egl1
RUN apt install -y libegl1-mesa-dev
RUN apt install -y libgbm-dev
RUN apt install -y libxkbcommon-dev
RUN apt install -y libdrm-dev
RUN apt install -y libgl1-mesa-dev
RUN apt install -y libgles2-mesa-dev
RUN apt install -y wayland-protocols
RUN apt install -y wayland-utils
RUN apt install -y xwayland
RUN apt install -y libxcb1-dev
RUN apt install -y libxkbcommon0
RUN apt install -y libxcb1
RUN apt install -y libx11-xcb1
RUN apt install -y libx11-dev
RUN apt install -y libxcb-util1
RUN apt install -y libxcb-xinerama0
RUN apt install -y libxcb-icccm4
RUN apt install -y libxcb-image0
RUN apt install -y libxcb-keysyms1
RUN apt install -y libxcb-render0
RUN apt install -y libxcb-shape0
RUN apt install -y libxcb-shm0
RUN apt install -y libxcb-sync1
RUN apt install -y libxcb-xfixes0
RUN apt install -y libxcb-randr0
RUN apt install -y libxcb-glx0
RUN apt install -y libxcb-xkb1
RUN apt install -y libxcb-cursor0
RUN apt install -y libxkbcommon-x11-0
RUN apt install -y libxkbcommon0
RUN apt install -y libpulse0
RUN apt install -y weston
RUN apt install -y sudo
# RUN apt install -y getent

# Set the working directory inside the container
WORKDIR /app

# Copy your application source code into the container
COPY . /app

# Build your Qt application (assuming a CMake-based project)
RUN cd /app
RUN rm -rf build && mkdir build
RUN cmake -S . -B build -G "Ninja Multi-Config"
RUN cmake --build build --config Debug

ENV XDG_RUNTIME_DIR=/tmp/xdg-runtime
RUN mkdir -p $XDG_RUNTIME_DIR
RUN chmod 700 $XDG_RUNTIME_DIR
# RUN sudo usermod -aG input $USER
# RUN sudo usermod -aG video $USER
# RUN sudo chmod 666 /var/run/docker.sock

# Start Weston in background (offscreen/headless mode)
ENV WAYLAND_DISPLAY=wayland-0
ENV XDG_SESSION_TYPE=wayland
ENV QT_QPA_PLATFORM=wayland;xcb
ENV DISPLAY=:0
# ENV QT_QPA_PLATFORM=offscreen
ENV QT_DEBUG_PLUGINS=1
RUN weston --backend=headless-backend.so --width=1920 --height=1080 &

# Expose any necessary ports if your application has network features
# EXPOSE 8080

# Command to run your application when the container starts
CMD ["./build/Debug/foldersearch"]
