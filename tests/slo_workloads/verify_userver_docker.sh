#!/usr/bin/env bash
# Smoke-test the userver SLO workload Docker image against a local YDB instance.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
COMPOSE_FILE="${REPO_ROOT}/examples/otel_tracing/docker-compose.yml"
IMAGE_NAME="${IMAGE_NAME:-ydb-slo-userver-verify}"
PRESET="${PRESET:-release-test-clang}"
CONNECTION_STRING="${CONNECTION_STRING:-grpc://127.0.0.1:2136/?database=/local}"
YDB_IMAGE="${YDB_IMAGE:-cr.yandex/yc/yandex-docker-local-ydb:25.2.1}"
YDB_CONTAINER="${YDB_CONTAINER:-ydb-slo-userver-verify-ydb}"

failures=0
USE_COMPOSE=0

log() {
  echo "[verify_userver_docker] $*"
}

run_step() {
  local name="$1"
  shift
  log "Running: ${name}"
  if "$@"; then
    log "PASS: ${name}"
  else
    log "FAIL: ${name} (exit $?)"
    failures=$((failures + 1))
  fi
}

have_docker_compose() {
  docker compose version >/dev/null 2>&1
}

start_ydb() {
  if have_docker_compose; then
    USE_COMPOSE=1
    log "Starting local YDB via docker compose..."
    docker compose -f "${COMPOSE_FILE}" up -d ydb
    return
  fi

  USE_COMPOSE=0
  log "Starting local YDB via docker run (docker compose not available)..."
  docker rm -f "${YDB_CONTAINER}" >/dev/null 2>&1 || true
  docker run -d --name "${YDB_CONTAINER}" --network host \
    -e GRPC_TLS_PORT=2135 \
    -e GRPC_PORT=2136 \
    -e MON_PORT=8765 \
    -e YDB_DEFAULT_LOG_LEVEL=NOTICE \
    -e YDB_USE_IN_MEMORY_PDISKS=true \
    "${YDB_IMAGE}" >/dev/null
}

wait_for_ydb() {
  log "Waiting for YDB to become healthy..."
  for _ in $(seq 1 60); do
    if [ "${USE_COMPOSE}" -eq 1 ]; then
      if docker compose -f "${COMPOSE_FILE}" ps ydb 2>/dev/null | grep -q "(healthy)"; then
        return 0
      fi
    elif docker exec "${YDB_CONTAINER}" /bin/sh -c \
      "/ydb -e grpc://localhost:2136 -d /local scheme ls" >/dev/null 2>&1; then
      return 0
    fi
    sleep 2
  done
  return 1
}

cleanup() {
  log "Stopping local YDB..."
  if [ "${USE_COMPOSE}" -eq 1 ]; then
    docker compose -f "${COMPOSE_FILE}" stop ydb 2>/dev/null || true
  else
    docker rm -f "${YDB_CONTAINER}" >/dev/null 2>&1 || true
  fi
}

trap cleanup EXIT

cd "${REPO_ROOT}"

if [ "${SKIP_BUILD:-0}" != "1" ]; then
  log "Building userver SLO Docker image (preset=${PRESET})..."
  cp tests/slo_workloads/.dockerignore .dockerignore
  docker build -t "${IMAGE_NAME}" \
    --network=host \
    --build-arg REF=local \
    --build-arg PRESET="${PRESET}" \
    -f tests/slo_workloads/Dockerfile.userver .
  rm -f .dockerignore
else
  log "Skipping Docker image build (SKIP_BUILD=1)..."
fi

start_ydb

if ! wait_for_ydb; then
  log "FAIL: YDB did not become healthy in time"
  if [ "${USE_COMPOSE}" -eq 1 ]; then
    docker compose -f "${COMPOSE_FILE}" logs ydb || true
  else
    docker logs "${YDB_CONTAINER}" || true
  fi
  exit 1
fi

run_workload() {
  docker run --rm --network host "${IMAGE_NAME}" \
    --connection-string "${CONNECTION_STRING}" "$@"
}

run_step "create" \
  run_workload create --dont-push

run_step "run" \
  run_workload run \
    --time 30 \
    --read-rps 50 \
    --write-rps 10 \
    --read-timeout 100 \
    --write-timeout 100 \
    --dont-push

run_step "cleanup" \
  run_workload cleanup

if [ "${failures}" -ne 0 ]; then
  log "${failures} step(s) failed"
  exit 1
fi

log "All steps passed"
