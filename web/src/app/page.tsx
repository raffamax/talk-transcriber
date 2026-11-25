"use client";
import {
  Box,
  Button,
  Container,
  FormControl,
  IconButton,
  InputLabel,
  MenuItem,
  Paper,
  Select,
  Stack,
  Tooltip,
  Typography,
} from "@mui/material";
import { useEffect, useMemo, useRef, useState } from "react";
import { useTranscriptions } from "./useTranscriptions";

export default function Home() {
  const { messages, status, clear } = useTranscriptions();
  const [channel, setChannel] = useState("all");
  const channels = useMemo(
    () => [...new Set(messages.map((m) => m.channelId))],
    [messages],
  );
  const visible =
    channel === "all"
      ? messages
      : messages.filter((m) => m.channelId === channel);
  const scrollRef = useRef<HTMLDivElement>(null);
  const stickToBottom = useRef(true);
  useEffect(() => {
    if (scrollRef.current && stickToBottom.current)
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
  }, [visible.length]);
  const color = (id: string) => {
    let hash = 0;
    for (const c of id) hash = (hash * 31 + c.charCodeAt(0)) >>> 0;
    return `hsl(${hash % 360} 78% 68%)`;
  };
  return (
    <Container
      maxWidth="lg"
      sx={{ px: { xs: 1.5, sm: 3 }, py: { xs: 1.5, sm: 3 } }}
    >
      <Stack spacing={{ xs: 1.5, sm: 2.5 }}>
        <Paper elevation={2} sx={{ p: { xs: 1.5, sm: 2 }, borderRadius: 3 }}>
          <Stack
            direction="row"
            spacing={1.5}
            alignItems="center"
            justifyContent="space-between"
            flexWrap="wrap"
            useFlexGap
          >
            <Stack direction="row" spacing={1.5} alignItems="center">
              <Tooltip title="servidor">
                <Box
                  component="span"
                  sx={{ fontSize: 25, lineHeight: 1, cursor: "pointer" }}
                >
                  {status.connected ? "✅" : "⚠️"}
                </Box>
              </Tooltip>
              <Tooltip title="transcrição">
                <Box
                  component="span"
                  sx={{ fontSize: 25, lineHeight: 1, cursor: "pointer" }}
                >
                  {status.whisperReady ? "✅" : "⚠️"}
                </Box>
              </Tooltip>
              <Typography variant="body2" color="text.secondary">
                {status.channels}{" "}
                {status.channels === 1 ? "canal ativo" : "canais ativos"}
              </Typography>
              <FormControl
                size="small"
                sx={{ minWidth: { xs: 0, sm: 190 }, flex: 1 }}
              >
                <InputLabel>Canal</InputLabel>
                <Select
                  value={channel}
                  label="Canal"
                  onChange={(e) => setChannel(e.target.value)}
                >
                  <MenuItem value="all">Todos os canais</MenuItem>
                  {channels.map((c) => (
                    <MenuItem key={c} value={c}>
                      {c}
                    </MenuItem>
                  ))}
                </Select>
              </FormControl>
            </Stack>
            <Stack
              direction="row"
              spacing={1}
              alignItems="center"
              sx={{ width: { xs: "100%", sm: "auto" } }}
            >
              <Tooltip title="Limpar conversa">
                <Button
                  variant="outlined"
                  color="error"
                  onClick={() =>
                    confirm("Limpar o histórico local?") && clear()
                  }
                  aria-label="Limpar conversa"
                  sx={{ textTransform: "none" }}
                >
                  Limpar conversa
                </Button>
              </Tooltip>
            </Stack>
          </Stack>
        </Paper>
        <Paper
          ref={scrollRef}
          onScroll={() => {
            const el = scrollRef.current;
            if (el)
              stickToBottom.current =
                el.scrollHeight - el.scrollTop - el.clientHeight < 48;
          }}
          sx={{
            p: { xs: 1, sm: 2 },
            minHeight: { xs: "70vh", sm: 480 },
            maxHeight: "78vh",
            overflowY: "auto",
            borderRadius: 3,
          }}
        >
          {visible.length === 0 ? (
            <Box
              sx={{
                minHeight: 300,
                display: "grid",
                placeItems: "center",
                px: 2,
              }}
            >
              <Typography
                color="text.secondary"
                textAlign="center"
              ></Typography>
            </Box>
          ) : (
            <Stack spacing={1}>
              {visible.map((m) => (
                <Box
                  key={m.id}
                  sx={{
                    p: { xs: 1.25, sm: 1.5 },
                    borderLeft: 3,
                    borderColor: color(m.channelId),
                    bgcolor: "rgba(255,255,255,.04)",
                    borderRadius: "0 8px 8px 0",
                  }}
                >
                  <Typography
                    variant="caption"
                    sx={{
                      color: color(m.channelId),
                      fontWeight: 700,
                      wordBreak: "break-word",
                    }}
                  >
                    {m.channelId}
                    {m.latencyMs !== undefined ? ` · ${m.latencyMs} ms` : ""}
                  </Typography>
                  <Typography
                    sx={{
                      mt: 0.35,
                      fontSize: { xs: "1rem", sm: "1.05rem" },
                      wordBreak: "break-word",
                    }}
                  >
                    {m.text}
                  </Typography>
                </Box>
              ))}
            </Stack>
          )}
        </Paper>
      </Stack>
    </Container>
  );
}
