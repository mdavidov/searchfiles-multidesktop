
docker run -it --rm \
  --name foldersearch-app \
  -e WAYLAND_DISPLAY=wayland-0 \
  -e XDG_RUNTIME_DIR=/run/user/1000 \
  -e QT_QPA_PLATFORM=wayland \
  -e QT_WAYLAND_DISABLE_WINDOWDECORATION=1 \
  -v /run/user/$(id -u)/wayland-0:/run/user/1000/wayland-0 \
  -v /run/user/$(id -u)/pulse:/run/user/1000/pulse \
  --user $(id -u):$(id -g) \
  --ipc=host \
  --security-opt apparmor=unconfined \
  foldersearch
