#!/bin/bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
DEPS_DIR="${TALK_TRANSCRIBER_DEPS_DIR:-$HOME}"
JUCE_DIR="${JUCE_DIR:-$DEPS_DIR/JUCE}"
WHISPER_DIR="${WHISPER_DIR:-$DEPS_DIR/whisper.cpp}"
IXWEBSOCKET_DIR="${IXWEBSOCKET_DIR:-$DEPS_DIR/ixwebsocket}"
MODEL_PATH="$PROJECT_DIR/models/ggml-small.bin"

log() { printf '\n==> %s\n' "$1"; }
fail() { printf '\nErro: %s\n' "$1" >&2; exit 1; }

[[ "$(uname -s)" == "Darwin" ]] || fail "este instalador funciona somente no macOS"

if ! command -v brew >/dev/null 2>&1; then
  log "Instalando Homebrew"
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  if [[ -x /opt/homebrew/bin/brew ]]; then
    eval "$(/opt/homebrew/bin/brew shellenv)"
  elif [[ -x /usr/local/bin/brew ]]; then
    eval "$(/usr/local/bin/brew shellenv)"
  fi
fi

command -v brew >/dev/null 2>&1 || fail "Homebrew não está disponível no PATH"

log "Instalando ferramentas do sistema"
brew update
brew install cmake git node

if ! xcode-select -p >/dev/null 2>&1; then
  xcode-select --install || true
  fail "instale o Xcode Command Line Tools e execute este script novamente"
fi

clone_if_missing() {
  local repository="$1"
  local destination="$2"
  if [[ -d "$destination/.git" ]]; then
    printf 'Já existe: %s\n' "$destination"
  elif [[ -e "$destination" ]]; then
    fail "o caminho existe, mas não é um checkout Git válido: $destination"
  else
    git clone --depth 1 "$repository" "$destination"
  fi
}

log "Baixando dependências externas"
clone_if_missing https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
clone_if_missing https://github.com/ggerganov/whisper.cpp.git "$WHISPER_DIR"
clone_if_missing https://github.com/machinezone/IXWebSocket.git "$IXWEBSOCKET_DIR"

log "Baixando o modelo Whisper"
mkdir -p "$PROJECT_DIR/models"
if [[ -f "$MODEL_PATH" ]]; then
  printf 'Já existe: %s\n' "$MODEL_PATH"
else
  curl -fL --retry 3 --progress-bar \
    "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin" \
    -o "$MODEL_PATH"
fi

log "Instalando dependências Node.js"
cd "$PROJECT_DIR"
npm install --ignore-scripts

log "Compilando o servidor"
cmake -S . -B build-runtime \
  -DWHISPER_DIR="$WHISPER_DIR" \
  -DIXWEBSOCKET_DIR="$IXWEBSOCKET_DIR" \
  -DTALK_TRANSCRIBER_BUILD_PLUGIN=OFF \
  -DTALK_TRANSCRIBER_BUILD_TESTS=OFF
cmake --build build-runtime --target talk-transcriber-server -j2

log "Compilando o VST3 universal"
cmake -S . -B build-universal \
  -DJUCE_DIR="$JUCE_DIR" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DTALK_TRANSCRIBER_BUILD_PLUGIN=ON \
  -DTALK_TRANSCRIBER_BUILD_SERVER=OFF \
  -DTALK_TRANSCRIBER_BUILD_TESTS=OFF
cmake --build build-universal --target TalkCapture_VST3 -j2

log "Instalando o VST3"
npm run install-vst

cat <<EOF

Instalação concluída.

Servidor:
  npm run server

Dashboard:
  npm run web

Dependências externas:
  JUCE=$JUCE_DIR
  whisper.cpp=$WHISPER_DIR
  IXWebSocket=$IXWEBSOCKET_DIR
EOF
