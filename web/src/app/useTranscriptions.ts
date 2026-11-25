"use client";
import { useCallback, useEffect, useMemo, useRef, useState } from "react";
export type Transcription = { id: string; channelId: string; channelName?: string; sequence: number; latencyMs?: number; text: string; receivedAt: number };
type Channel = { id: string; name: string };
type ServerStatus = { connected: boolean; whisperReady: boolean; channels: number; activeChannels: Channel[] };
const STORAGE_KEY = "talk-transcriber.transcriptions";
const MAX_MESSAGES = 500;
const INACTIVITY_TIMEOUT_MS = 5 * 60 * 1000;
function displayText(text: string) {
  return text.replace(/\[[^\]]*\]|\([^)]*\)/g, "").replace(/\s{2,}/g, " ").trim();
}
export function useTranscriptions(url?: string) {
  const endpoint = url ?? (typeof window === "undefined" ? "ws://127.0.0.1:39742" : `ws://${window.location.hostname === "localhost" ? "127.0.0.1" : window.location.hostname}:39742`);
  const [messages, setMessages] = useState<Transcription[]>([]);
  const [status, setStatus] = useState<ServerStatus>({ connected: false, whisperReady: false, channels: 0, activeChannels: [] });
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
    let socket: WebSocket | undefined;
    let retry = 500;
    let timer: ReturnType<typeof setTimeout>;
    const connect = () => {
      const connection = new WebSocket(endpoint);
      socket = socketRef.current = connection;
      connection.onopen = () => {
        setStatus(s => ({ ...s, connected: true }));
        retry = 500;
        connection.send(JSON.stringify({ version: 1, type: "server.status.request" }));
      };
      connection.onclose = () => {
        setStatus(s => ({ ...s, connected: false, channels: 0, activeChannels: [] }));
        timer = setTimeout(connect, retry);
        retry = Math.min(retry * 2, 8000);
      };
      connection.onmessage = event => {
        try {
          const data = JSON.parse(event.data);
          if (data.type === "server.status") {
            const entries: unknown = data.activeChannels;
            const activeChannels = Array.isArray(entries)
              ? entries.filter((channel): channel is Channel => typeof channel?.id === "string" && typeof channel?.name === "string")
              : [];
            setStatus({ connected: true, whisperReady: !!data.whisperReady, channels: data.channels || 0, activeChannels });
          }
          if (data.type === "transcription.final") {
            const text = displayText(data.text);
            if (!text) return;
            setMessages(current => {
              const id = `${data.channelId}:${data.sequence}`;
              if (current.some(m => m.id === id)) return current;
              return [...current, {
                id, channelId: data.channelId, channelName: data.channelName,
                sequence: data.sequence, latencyMs: data.latencyMs, text, receivedAt: Date.now(),
              }].slice(-MAX_MESSAGES);
            });
          }
        } catch { /* ignore malformed events */ }
      };
    };
    connect();
    return () => {
      clearTimeout(timer);
      if (socket) {
        socket.onopen = socket.onclose = socket.onmessage = null;
        socket.close();
      }
    };
  }, [endpoint, version]);
  const clear = useCallback(() => { setMessages([]); localStorage.removeItem(STORAGE_KEY); }, []);
  const reconnect = useCallback(() => setVersion(v => v + 1), []);
  const setVadConfig = useCallback((channelId: string, thresholdDb: number, releaseMs: number) => socketRef.current?.readyState === WebSocket.OPEN && socketRef.current.send(JSON.stringify({ version: 1, type: "vad.config", channelId, thresholdDb, releaseMs })), []);
  return useMemo(() => ({ messages, status, clear, reconnect, setVadConfig }), [messages, status, clear, reconnect, setVadConfig]);
}
