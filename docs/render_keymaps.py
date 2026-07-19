#!/usr/bin/env python3
"""Render the FLB36 firmware keymap as dependency-free SVG diagrams."""

from html import escape
from pathlib import Path

POSITIONS = [
    (0, 75), (100, 25), (200, 0), (300, 25), (400, 75),
    (800, 75), (900, 25), (1000, 0), (1100, 25), (1200, 75),
    (0, 175), (100, 125), (200, 100), (300, 125), (400, 175),
    (800, 175), (900, 125), (1000, 100), (1100, 125), (1200, 175),
    (0, 275), (100, 225), (200, 200), (300, 225), (400, 275),
    (800, 275), (900, 225), (1000, 200), (1100, 225), (1200, 275),
    (150, 400), (250, 400), (350, 400), (475, 350),
    (725, 350), (850, 400), (950, 400), (1050, 400),
]

LAYERS = {
    "base": (
        "Base",
        ["Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
         "A", "S", "D", "F", "G", "H", "J", "K", "L", ";",
         "Z|Shift", "X|Alt", "C|Ctrl", "V|GUI", "B",
         "N", "M|GUI", ",|Ctrl", ".|Alt", "/|Shift",
         "Alt", "Tab|GUI", "Space|Lower", "Raise|toggle",
         "Mute", "Space|Raise", "Backspace", "Enter|Shift"],
        "Left encoder: Tab / Shift+Tab   •   Right encoder: Volume",
    ),
    "lower": (
        "Lower — hold left Space",
        ["!", "@", "*", "`", "~", "7", "8", "9", "+", "'",
         "#", "'", '"', "-", "=", "4", "5", "6", "-", ":",
         "_", "$", "%", "^", "&", "1", "2", "3", "\\", "Ctrl+Del",
         "▽", "▽", "▽", "▽", "▽", "Enter", "0", "."],
        "Left encoder: ← / →   •   Right encoder: Previous / Next track",
    ),
    "raise": (
        "Raise — hold right Space",
        ["1", "2", "3", "4", "5", "Ctrl+Shift+Tab", "⌘←", "⌘→", "Ctrl+Tab", "⇧⌘4",
         "Caps", "Scroll ←", "Scroll ↓", "Scroll ↑", "Scroll →", "←", "↑", "↓", "→", "⇧⌘5",
         "Shift", "—", "\\", "Mouse 2", "Mouse 1", "Mouse ←", "Mouse ↓", "Mouse ↑", "Mouse →", "⌘V",
         "Delete", "▽", "▽", "▽", "▽", "▽", "Backspace", "▽"],
        "Left encoder: ↓ / ↑   •   Right encoder: Screen brightness − / +",
    ),
    "adjust": (
        "Adjust — hold left Space, then right Space",
        ["F1", "Studio|unlock", "F3", "F4", "RGB|toggle", "1", "2", "3", "4", "5",
         "F6", "F7", "F8", "F9", "F10", "Mute", "Vol −", "Vol +", "Play/Pause", "—",
         "BT 1", "BT 2", "BT 3", "Clear BT", "—", "—", "Previous", "Next", "Stop", "—",
         "▽", "▽", "▽", "▽", "▽", "▽", "USB output", "▽"],
        "Left encoder: ← / →   •   Right encoder: Volume − / +",
    ),
}


def color(label: str) -> str:
    if "BT" in label or "Studio" in label or "USB" in label:
        return "#2f6feb"
    if "RGB" in label:
        return "#b94fd8"
    if any(word in label for word in ("Mouse", "Scroll", "Volume", "Vol ", "Mute", "Play", "Previous", "Next", "Stop")):
        return "#25845b"
    if any(word in label for word in ("Lower", "Raise", "Space")):
        return "#d97706"
    if any(word in label for word in ("Shift", "Ctrl", "Alt", "GUI", "⌘")):
        return "#7657c8"
    if label in ("▽", "—"):
        return "#3b4252"
    return "#202938"


def render(slug: str, title: str, labels: list[str], encoder: str) -> None:
    scale, ox, oy = 0.62, 60, 92
    key = 58
    parts = [f'''<svg xmlns="http://www.w3.org/2000/svg" width="920" height="450" viewBox="0 0 920 450">
<rect width="920" height="450" rx="24" fill="#0d1117"/>
<text x="460" y="38" text-anchor="middle" fill="#f0f6fc" font-family="system-ui,sans-serif" font-size="24" font-weight="700">{escape(title)}</text>
<path d="M38 70 H370 Q405 70 420 105 V404 H38 Z" fill="#161b22" stroke="#30363d" stroke-width="2"/>
<path d="M500 105 Q515 70 550 70 H882 V404 H500 Z" fill="#161b22" stroke="#30363d" stroke-width="2"/>''']
    for (px, py), label in zip(POSITIONS, labels):
        x, y = ox + px * scale, oy + py * scale
        fill = color(label)
        parts.append(f'<rect x="{x:.1f}" y="{y:.1f}" width="{key}" height="{key}" rx="9" fill="{fill}" stroke="#8b949e" stroke-width="1"/>')
        lines = label.split("|")
        start = y + 27 - (len(lines) - 1) * 8
        size = 11 if len(label) > 10 else 13
        for index, line in enumerate(lines):
            parts.append(f'<text x="{x + key / 2:.1f}" y="{start + index * 17:.1f}" text-anchor="middle" fill="white" font-family="system-ui,sans-serif" font-size="{size}" font-weight="600">{escape(line)}</text>')
    parts.append(f'''<circle cx="105" cy="55" r="20" fill="#21262d" stroke="#8b949e"/><circle cx="815" cy="55" r="20" fill="#21262d" stroke="#8b949e"/>
<text x="460" y="436" text-anchor="middle" fill="#b1bac4" font-family="system-ui,sans-serif" font-size="14">{escape(encoder)}</text>
</svg>''')
    Path(__file__).with_name(f"keymap-{slug}.svg").write_text("\n".join(parts) + "\n")


if __name__ == "__main__":
    for name, values in LAYERS.items():
        render(name, *values)
