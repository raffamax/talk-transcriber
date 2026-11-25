# Talk Capture

The JUCE VST3 target is intentionally opt-in during Phase 1 because JUCE is an external dependency.

```sh
cmake -S . -B build -DTALK_TRANSCRIBER_BUILD_PLUGIN=ON -DJUCE_DIR=/path/to/JUCE
cmake --build build --target TalkCapture_VST3
```

O processador envia blocos por uma fila limitada para `127.0.0.1:39741` usando o formato compartilhado.

## Configuração no REAPER

1. Em `Preferences > Plug-ins > VST`, faça `Re-scan`.
2. Crie uma track para o microfone/talkback.
3. Selecione `Input: Mono` com a entrada correta.
4. Arme a track e habilite o monitoramento de entrada.
5. Insira `Talk Capture` como FX.
6. Ative `Capture enabled`.
7. Configure o nome do operador e pressione Enter ou saia do campo para aplicar.

O indicador `Input` deve mostrar o nível em dBFS e `UDP sent` deve aumentar durante a fala.

