
docker run -it --rm \
  --name qt-wayland-test \
  -e WAYLAND_DISPLAY=wayland-0 \
  -e XDG_RUNTIME_DIR=/tmp \
  -e QT_QPA_PLATFORM=wayland \
  -e WESTON_DISABLE_COLORMANAGEMENT=1 \
  -v $(pwd):/workspace \
  -w /workspace \
  --ipc=host \
  ubuntu:25.10 \
  bash -c "
    # Install required packages
    apt-get update && apt-get install -y \
      weston \
      wayland-protocols \
      libwayland-client0 \
      qt6-wayland \
      libqt6waylandclient6 \
      libegl1-mesa \
      libgl1-mesa-glx \
      libxkbcommon0 \
      libfontconfig1 \
      wayland-utils \
      qt6-base-dev \
      build-essential && \
    
    # Create runtime directory
    mkdir -p /tmp && \
    
    # Start Weston in background (offscreen/headless mode)
    weston --backend=headless-backend.so --width=1920 --height=1080 &
    sleep 2 && \
    
    # Your Qt app will run here
    echo 'Wayland environment ready. WAYLAND_DISPLAY is set to:' \$WAYLAND_DISPLAY && \
    echo 'You can now build and run your Qt application.' && \
    /bin/bash
  "
