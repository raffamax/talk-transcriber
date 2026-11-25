import type { ReactNode } from "react";
import "./globals.css";
import ThemeRegistry from "./ThemeRegistry";
export default function RootLayout({ children }: { children: ReactNode }) { return <html lang="pt-BR"><body><ThemeRegistry>{children}</ThemeRegistry></body></html>; }

