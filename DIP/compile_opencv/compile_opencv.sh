#!/bin/bash
set -e

sudo rm -rf ../opencv/build
mkdir -p ../opencv/build
cd ../opencv/build

cmake -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D BUILD_EXAMPLES=ON \
      -D WITH_FFMPEG=OFF \
      -D WITH_GSTREAMER=OFF \
      -D OPENCV_GENERATE_PKGCONFIG=ON ..
make -j$(nproc)
sudo make install
sudo ldconfig