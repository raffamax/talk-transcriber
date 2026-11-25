# Local server

O servidor nativo C++20 recebe áudio UDP em localhost e publica eventos finais por WebSocket.

```sh
cmake -S . -B build
cmake --build build --target talk-transcriber-server
./build/server/talk-transcriber-server
```

To configure whisper.cpp from a local checkout with Metal enabled:

```sh
cmake -S . -B build-whisper \
  -DWHISPER_DIR="$HOME/whisper.cpp" \
  -DGGML_METAL=ON
cmake --build build-whisper --target talk-transcriber-server
```

The server accepts the model path for the upcoming inference worker:

```sh
./build-whisper/server/talk-transcriber-server --model models/ggml-base.bin
```

Em Intel ou quando Metal não estiver disponível, use o fallback CPU:

```sh
./build-whisper/server/talk-transcriber-server \
  --model models/ggml-base.bin --cpu
```

Por padrão, as portas são UDP `39741` e WebSocket `39742`. Para testes isolados:

```sh
./build-runtime/server/talk-transcriber-server \
  --model models/ggml-base.bin --cpu \
  --udp-port 39744 --ws-port 39743
```

O dashboard conecta automaticamente ao WebSocket no mesmo endereço do navegador, na porta `39742`. O servidor aceita conexões na rede local; use o IP do Mac, por exemplo `http://192.168.0.246:3000`.

Atalhos de execução a partir da raiz do projeto:

```sh
./scripts/start-server.sh
./scripts/start-dashboard.sh
```

Ou, usando o comando unificado na raiz:

```sh
./start server
./start web
```

Para digitar exatamente `start server`, adicione a raiz do projeto ao `PATH` da sessão:

```sh
export PATH="$PWD:$PATH"
start server
start web
```

Para diagnosticar a chegada de áudio UDP, acrescente `--verbose`. O servidor exibirá canal, sequência, quantidade de amostras e taxa dos pacotes recebidos:

```sh
./build-runtime/server/talk-transcriber-server \
  --model models/ggml-base.bin --verbose
```

The initial VAD implementation is energy-based, with configurable threshold, pre-roll and silence duration. It is intentionally isolated so it can later be replaced by Silero VAD without changing the audio receiver contract.

