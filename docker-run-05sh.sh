
docker run -it \   
  foldersearch \
  --platform linux/amd64 \
  -e WESTON_DISABLE_COLORMANAGEMENT=1 \
  -v $(pwd):/app \      
  -w /app \      
  --ipc=host
#   bash -c "
#     # Start Weston in background (offscreen/headless mode)
#     # weston --backend=headless-backend.so --width=1920 --height=1080 &
#     # sleep 2 && \
#
#     cd /app
#     ./build/Debug/foldersearch
#   "
