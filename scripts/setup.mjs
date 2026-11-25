import { cp, mkdir, access } from "node:fs/promises";
import { constants } from "node:fs";
import { join } from "node:path";
import { homedir, platform } from "node:os";
import { fileURLToPath } from "node:url";

const root = fileURLToPath(new URL("..", import.meta.url));
const vst = join(root, "build-universal/plugin/TalkCapture_artefacts/VST3/Talk Capture.vst3");
const model = join(root, "models/ggml-small.bin");
const destination = join(homedir(), "Library/Audio/Plug-Ins/VST3/Talk Capture.vst3");
const vstOnly = process.argv.includes("--vst-only");

async function exists(path) { try { await access(path, constants.F_OK); return true; } catch { return false; } }

if (platform() !== "darwin") console.warn("Aviso: a instalação automática do VST3 foi preparada para macOS.");
if (!vstOnly && !(await exists(model))) console.warn(`Modelo não encontrado: ${model}`);
if (!(await exists(vst))) {
  console.error(`VST3 não encontrado: ${vst}`);
  console.error("Compile primeiro com: cmake --build build-universal --target TalkCapture_VST3");
  process.exit(1);
}
await mkdir(join(homedir(), "Library/Audio/Plug-Ins/VST3"), { recursive: true });
await cp(vst, destination, { recursive: true, force: true });
console.log(`VST3 instalado em: ${destination}`);
if (!vstOnly) console.log("Dashboard: dependências instaladas pelo workspace web.");
