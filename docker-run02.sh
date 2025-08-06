
docker run -it --rm \
  --name foldersearch \
  -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
  -e XDG_RUNTIME_DIR=/run/user/$(id -u) \
  -e QT_QPA_PLATFORM=wayland \
  -v /run/user/$(id -u):/run/user/$(id -u) \
  --user $(id -u):$(id -g) \
  --ipc=host \
  --security-opt apparmor=unconfined \
  foldersearch
  