# NppMarkdownPanel Qt port

This is Jiang Liwei's independent Qt port of the original NppMarkdownPanel
plugin for
[Notepad++ for Qt](https://github.com/1193561652/notepad-plus-plus/tree/qt-port).
Port source: <https://github.com/1193561652/NppMarkdownPanel/tree/qt-port>.
Original source: <https://github.com/mohzy83/NppMarkdownPanel>. The port uses
the MIT License; see [../License.txt](../License.txt) and
[../QT_PORT_NOTICE.md](../QT_PORT_NOTICE.md).

The renderer uses Qt's CommonMark/GFM parser for the base grammar and implements
the original Markdig `UseAdvancedExtensions()` set, including abbreviations,
heading IDs, citations, containers, definition lists, emphasis extras, figures,
footers, footnotes, grid/pipe tables, math, media, tasks, diagram fences,
autolinks, and generic attributes.

The `0.6.2` source remains the reference. This module preserves the four original
commands, dock preview, CSS, zoom, toolbar, automatic HTML output, and INI keys.
Legacy Notepad++ ABI exports are deliberately empty; functionality uses the new ABI.
