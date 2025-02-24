#!/bin/bash
set -ex

# 컨테이너 ID 추출 (예: /proc/self/cgroup에서 추출)
CONTAINER_ID=$(head -1 /proc/self/cgroup | cut -d/ -f3)
# docer- 접두사, .scope 접미사 제거 (예: docker-5340af7...scope -> docker-5340af7...)
CONTAINER_ID=$(echo "$CONTAINER_ID" | sed -e 's/docker-\(.*\)\.scope/\1/')
echo "Container ID: $CONTAINER_ID"

# 고유 cgroup 디렉터리 생성 (예: /sys/fs/cgroup/sandbox-${CONTAINER_ID})
SANDBOX_CG="/sys/fs/cgroup/sandbox-${CONTAINER_ID}"
if [ ! -d "$SANDBOX_CG" ]; then
    sudo mkdir -p "$SANDBOX_CG" || {
        echo "Failed to create cgroup directory $SANDBOX_CG" >&2
        exit 1
    }
    echo "Created cgroup directory $SANDBOX_CG"
fi

# CONTAINER_ID를 환경 변수로 설정하고 ~/.bashrc에 추가
echo "export CONTAINER_ID=$CONTAINER_ID" >> ~/.bashrc
export CONTAINER_ID
source ~/.bashrc