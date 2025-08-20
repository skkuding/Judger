#!/bin/bash
set -euo pipefail

JUDGER=./output/libjudger.so
TEST_DIR=./tests/general_seccomp
BUILD_DIR=./build
COMPILED_EXES=()

cleanup() {
  echo
  echo "[CLEANUP] 테스트 실행 파일 삭제"
  for e in "${COMPILED_EXES[@]:-}"; do
    [ -f "$e" ] && rm -f "$e"
  done
}
trap cleanup EXIT

if [ ! -f "$JUDGER" ]; then
  echo "[BUILD] libjudger.so"
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  ( cd "$BUILD_DIR" && cmake .. && make -j"$(nproc)" )
fi

echo "[COMPILE] test programs"
for src in "$TEST_DIR"/*.c; do
  exe="${src%.c}"
  gcc -O2 -o "$exe" "$src"
  COMPILED_EXES+=("$exe")
done

echo
echo "===== [EXECUTE] (seccomp_rule_name=general) ====="
run_one () {
  local exe=$1
  echo
  echo "[RUN] $(basename "$exe")"
  ${SUDO_CMD:-} "$JUDGER" \
    --exe_path="$exe" \
    --seccomp_rule_name=general \
    --max_cpu_time=1000 \
    --max_real_time=2000 \
    --max_memory=134217728 \
    --max_output_size=1048576 || true
}

run_one "$TEST_DIR/fork_test"
run_one "$TEST_DIR/getdents64_test"
run_one "$TEST_DIR/open_test"
run_one "$TEST_DIR/socket_test"
run_one "$TEST_DIR/write_open_test"