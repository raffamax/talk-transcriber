# Talk Transcriber

O Talk Transcriber transforma o áudio recebido de uma track do REAPER em texto em português em tempo real.

O projeto é composto por três partes:

- Um plugin VST3 para capturar o áudio e identificar o operador.
- Um servidor local em C++ que recebe o áudio por UDP, detecta as falas e usa `whisper.cpp` para transcrever.
- Um dashboard web que recebe as transcrições por WebSocket.

O processamento é local. O áudio não precisa ser enviado para um serviço externo.

## Requisitos

- macOS Intel ou Apple Silicon
- Node.js 20 ou superior
- CMake 3.22 ou superior
- Xcode Command Line Tools
- REAPER
- JUCE
- whisper.cpp
- IXWebSocket

As dependências externas podem ser baixadas assim:

```sh
git clone https://github.com/juce-framework/JUCE.git "$HOME/JUCE"
git clone https://github.com/ggerganov/whisper.cpp.git "$HOME/whisper.cpp"
git clone https://github.com/machinezone/IXWebSocket.git "$HOME/ixwebsocket"
```

## Instalação

Para executar toda a instalação automaticamente no macOS, a partir da raiz do projeto:

```sh
./install-macos.sh
```

O script instala as ferramentas necessárias via Homebrew, baixa as dependências externas e o modelo, instala os pacotes Node.js, compila o servidor e o VST3 universal e instala o plugin no diretório de plugins do usuário.

O Homebrew, o Xcode Command Line Tools e o download do modelo podem solicitar autorização ou interação do macOS.

Para escolher outro diretório para as dependências externas:

```sh
TALK_TRANSCRIBER_DEPS_DIR="$HOME/Developer" ./install-macos.sh
```

O script pode ser executado novamente. Dependências e modelos que já existirem serão reutilizados.

Entre na pasta do projeto e baixe o modelo local `small`:

```sh
cd /caminho/para/talk-transcriber
mkdir -p models
curl -L "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin" \
  -o models/ggml-small.bin
```

Compile o servidor:

```sh
cmake -S . -B build-runtime \
  -DWHISPER_DIR="$HOME/whisper.cpp" \
  -DIXWEBSOCKET_DIR="$HOME/ixwebsocket" \
  -DTALK_TRANSCRIBER_BUILD_PLUGIN=OFF \
  -DTALK_TRANSCRIBER_BUILD_TESTS=OFF

cmake --build build-runtime --target talk-transcriber-server -j2
```

Compile o VST3 universal para Intel e Apple Silicon:

```sh
cmake -S . -B build-universal \
  -DJUCE_DIR="$HOME/JUCE" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DTALK_TRANSCRIBER_BUILD_PLUGIN=ON \
  -DTALK_TRANSCRIBER_BUILD_SERVER=OFF \
  -DTALK_TRANSCRIBER_BUILD_TESTS=OFF

cmake --build build-universal --target TalkCapture_VST3 -j2
```

Instale as dependências do dashboard e copie o VST3 para a pasta de plugins do usuário:

```sh
npm install
```

Para reinstalar somente o VST3 depois de uma nova compilação:

```sh
npm run install-vst
```

## Execução local

Abra um terminal e inicie o servidor:

```sh
npm run server
```

Abra outro terminal e inicie o dashboard:

```sh
npm run web
```

O navegador padrão será aberto automaticamente em [http://localhost:3000](http://localhost:3000).

O servidor usa as portas UDP `39741` e WebSocket `39742`.

## Configuração no REAPER

1. Crie uma track no REAPER.
2. Adicione o plugin `Talk Capture` como efeito VST3.
3. Digite o nome do operador, incluindo espaços se necessário, como `talk 01`.
4. Clique em **Salvar**.
5. Mantenha **Capture enabled** ativado.
6. Inicie o servidor e o dashboard.
7. Fale no canal da track e acompanhe o resultado no navegador.

O plugin mantém o áudio em pass-through. A captura é enviada ao servidor local em uma thread separada.

## Acesso pela rede local

O dashboard pode ser acessado por outro dispositivo conectado à mesma rede. Use o endereço IP do Mac:

```text
http://IP_DO_MAC:3000
```

O servidor WebSocket aceita conexões na rede local pela porta `39742`. O firewall do macOS precisa permitir as conexões quando solicitado.

## Estrutura principal

```text
plugin/     Plugin JUCE/VST3
server/     Servidor nativo e integração com whisper.cpp
protocol/   Protocolo dos pacotes de áudio UDP
web/        Dashboard Next.js
scripts/    Comandos de instalação e execução
models/     Modelos locais do Whisper, ignorados pelo Git
```
