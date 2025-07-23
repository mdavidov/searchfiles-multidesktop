# Use a base image with Qt pre-installed or install it
FROM ubuntu:22.04

# Install necessary dependencies for a Qt QML application
RUN apt update
RUN apt install -y build-essential
RUN apt install -y gdb valgrind
RUN apt install -y git
RUN apt install -y cmake
RUN apt install -y ninja-build
RUN apt install -y pkg-config
RUN apt install -y qt6-base-dev
RUN apt install -y libqt6opengl6-dev
RUN apt install -y qt6-base-dev-tools
RUN apt install -y qt6-tools-dev
RUN apt install -y qt6-tools-dev-tools
RUN apt install -y qt6-declarative-dev
RUN apt install -y qml-module-qtquick-*
# RUN apt install -y qml6-module-qtquick
# RUN apt install -y qml6-module-qtquick-controls2
# RUN apt install -y qml6-module-qtquick-dialogs
# RUN apt install -y qml6-module-qtquick-layouts
# RUN apt install -y qml6-module-qtquick-window
RUN apt install -y libqt6quickwidgets6
RUN apt install -y libqt6quickcontrols2-6
# RUN apt install -y     # Add any other Qt modules or libraries your application needs
# RUN apt install -y     # e.g., qt6-multimedia-dev, qt6-network-dev, etc.
# RUN apt install -y     # X11 dependencies for GUI applications
RUN apt install -y libxkbcommon-x11-0
RUN apt install -y libpulse0
RUN apt install -y libopengl-dev
RUN apt install -y libgl1-mesa-dev
RUN apt install -y libglu1-mesa-dev
RUN apt install -y libgl1-mesa-glx
RUN apt install -y libgl1-mesa-dri
RUN rm -rf /var/lib/apt/lists/*  # Clean up apt cache to reduce image size

# Set the working directory inside the container
WORKDIR /app

# Copy your application source code into the container
COPY . /app

# Build your Qt application (assuming a CMake-based project)
RUN cd /app
RUN cmake -S . -B build -G "Ninja Multi-Config"
RUN cmake --build build --config Debug

# Set environment variables for Qt and QML (important for GUI applications)
ENV QT_QPA_PLATFORM=xcb
ENV DISPLAY=:0

# Expose any necessary ports if your application has network features
# EXPOSE 8080

# Command to run your application when the container starts
CMD ["./build/Debug/foldersearch.app/Contents/MacOS/foldersearch"]
