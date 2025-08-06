
docker run -it \
  -e XDG_RUNTIME_DIR=/run/user/$(id -u) \
  -e WAYLAND_DISPLAY=wayland-0 \
  -e XDG_SESSION_TYPE=wayland \
  -e QT_QPA_PLATFORM=wayland \
  -v /run/user/$(id -u)/wayland-0:/run/user/$(id -u)/wayland-0 \
  --user $(id -u):$(id -g) \
  --ipc=host \
  --security-opt apparmor=unconfined \
  foldersearch

docker run -it \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e DISPLAY=:0 \
    -v /dev/snd:/dev/snd \
    -v /dev/video0:/dev/video0 \
    foldersearch
