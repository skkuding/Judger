#!/bin/bash
set -ex

# 이전 build 폴더 삭제 (root 소유여도 sudo를 사용)
if [ -d build ]; then
  sudo rm -rf build
fi

# 이전 설치된 output 디렉터리도 삭제 (root 소유일 수 있으므로 sudo 사용)
if [ -d output ]; then
  sudo rm -rf output
fi

mkdir build
cd build
cmake ..
make -j$(nproc)
sudo make install