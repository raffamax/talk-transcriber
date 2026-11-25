"use client";
import type { ReactNode } from "react";
import { createTheme, ThemeProvider } from "@mui/material/styles";
const theme = createTheme({ palette: { mode: "dark", background: { default: "#0d1117", paper: "#171e29" }, text: { primary: "#f5f7fa", secondary: "#b8c2d1" }, primary: { main: "#70a7ff" } }, typography: { fontFamily: "Arial, sans-serif" } });
export default function ThemeRegistry({ children }: { children: ReactNode }) { return <ThemeProvider theme={theme}>{children}</ThemeProvider>; }
