#!/bin/sh
set -eu

PROJECT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$PROJECT_DIR/web"
export PATH="${HOME}/.nvm/versions/node/v24.22.0/bin:$PATH"
npm run dev -- -H 0.0.0.0 &
WEB_PID=$!
stop_web() {
  trap - INT TERM EXIT
  kill "$WEB_PID" 2>/dev/null || true
  wait "$WEB_PID" 2>/dev/null || true
  exit 130
}
trap stop_web INT TERM
trap 'kill "$WEB_PID" 2>/dev/null || true' EXIT

for attempt in $(seq 1 60); do
  if curl -fsS "http://localhost:3000" >/dev/null 2>&1; then
    if command -v open >/dev/null 2>&1; then
      open "http://localhost:3000" >/dev/null 2>&1 || true
    elif command -v xdg-open >/dev/null 2>&1; then
      xdg-open "http://localhost:3000" >/dev/null 2>&1 || true
    fi
    break
  fi
  sleep 0.5
done

wait "$WEB_PID"
