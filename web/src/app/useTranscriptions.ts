"use client";
import { useCallback, useEffect, useMemo, useRef, useState } from "react";
export type Transcription = { id: string; channelId: string; sequence: number; latencyMs?: number; text: string; receivedAt: number };
const STORAGE_KEY = "talk-transcriber.transcriptions";
const MAX_MESSAGES = 500;
const INACTIVITY_TIMEOUT_MS = 5 * 60 * 1000;
function displayText(text: string) {
  return text.replace(/\[[^\]]*\]|\([^)]*\)/g, "").replace(/\s{2,}/g, " ").trim();
}
export function useTranscriptions(url?: string) {
  const endpoint = url ?? (typeof window === "undefined" ? "ws://127.0.0.1:39742" : `ws://${window.location.hostname === "localhost" ? "127.0.0.1" : window.location.hostname}:39742`);
  const [messages, setMessages] = useState<Transcription[]>([]);
  const [status, setStatus] = useState({ connected: false, whisperReady: false, channels: 0 });
  const [version, setVersion] = useState(0);
  const socketRef = useRef<WebSocket | undefined>(undefined);
  useEffect(() => { try { setMessages(JSON.parse(localStorage.getItem(STORAGE_KEY) || "[]").map((message: Transcription) => ({ ...message, text: displayText(message.text) })).filter((message: Transcription) => message.text)); } catch { setMessages([]); } }, []);
  useEffect(() => { localStorage.setItem(STORAGE_KEY, JSON.stringify(messages.slice(-MAX_MESSAGES))); }, [messages]);
  useEffect(() => {
    if (messages.length === 0) return;
    const timer = setTimeout(() => {
      setMessages([]);
      localStorage.removeItem(STORAGE_KEY);
    }, INACTIVITY_TIMEOUT_MS);
    return () => clearTimeout(timer);
  }, [messages]);
  useEffect(() => {
    let socket: WebSocket | undefined; let retry = 500; let timer: ReturnType<typeof setTimeout>;
    const connect = () => { socket = new WebSocket(endpoint); socketRef.current = socket; socket.onopen = () => { setStatus(s => ({ ...s, connected: true })); retry = 500; socket?.send(JSON.stringify({ version: 1, type: "server.status.request" })); }; socket.onclose = () => { setStatus(s => ({ ...s, connected: false })); timer = setTimeout(connect, retry); retry = Math.min(retry * 2, 8000); }; socket.onmessage = event => { try { const data = JSON.parse(event.data); if (data.type === "server.status") setStatus({ connected: true, whisperReady: !!data.whisperReady, channels: data.channels || 0 }); if (data.type === "transcription.final") { const text = displayText(data.text); if (!text) return; setMessages(current => { const id = `${data.channelId}:${data.sequence}`; if (current.some(m => m.id === id)) return current; return [...current, { id, channelId: data.channelId, sequence: data.sequence, latencyMs: data.latencyMs, text, receivedAt: Date.now() }].slice(-MAX_MESSAGES); }); } } catch { /* ignore malformed events */ } }; };
    connect(); return () => { clearTimeout(timer); socket?.close(); };
  }, [endpoint, version]);
  const clear = useCallback(() => { setMessages([]); localStorage.removeItem(STORAGE_KEY); }, []);
  const reconnect = useCallback(() => setVersion(v => v + 1), []);
  const setVadConfig = useCallback((channelId: string, thresholdDb: number, releaseMs: number) => socketRef.current?.readyState === WebSocket.OPEN && socketRef.current.send(JSON.stringify({ version: 1, type: "vad.config", channelId, thresholdDb, releaseMs })), []);
  return useMemo(() => ({ messages, status, clear, reconnect, setVadConfig }), [messages, status, clear, reconnect, setVadConfig]);
}
