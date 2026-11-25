import { spawn } from "node:child_process";
const target = process.argv[2];
const script = target === "server" ? "scripts/start-server.sh" : target === "web" ? "scripts/start-dashboard.sh" : null;
if (!script) { console.error("Uso: npm start -- server | npm start -- web"); process.exit(64); }
const child = spawn(`./${script}`, process.argv.slice(3), { stdio: "inherit", shell: true });
child.on("exit", code => process.exit(code ?? 1));
