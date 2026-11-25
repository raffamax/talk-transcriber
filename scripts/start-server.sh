#!/bin/sh
set -eu

PROJECT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
MODEL_PATH=${1:-"$PROJECT_DIR/models/ggml-small.bin"}
exec "$PROJECT_DIR/build-runtime/server/talk-transcriber-server" \
  --model "$MODEL_PATH" --cpu --verbose
