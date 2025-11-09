#!/usr/bin/env python3
"""A lightweight Notepad-like text editor for EDHM.

Features:
    * New, Open, Save, and Save As operations.
    * Basic clipboard actions (cut, copy, paste).
    * Optional command-line argument to open a file at startup.
    * Status bar showing current line and column.
"""

from __future__ import annotations

import sys
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox


class NotepadApp:
    """Tkinter-based text editor inspired by Windows Notepad."""

    def __init__(self, root: tk.Tk, initial_path: Path | None = None) -> None:
        self.root = root
        self.root.title("EDHM Notepad")
        self.root.geometry("800x600")
        self.root.protocol("WM_DELETE_WINDOW", self.on_exit)

        self._file_path: Path | None = None

        self._create_widgets()
        self._create_menus()
        self._bind_shortcuts()

        if initial_path and initial_path.exists():
            self._load_file(initial_path)
        self._update_title()

    def _create_widgets(self) -> None:
        self.text = tk.Text(
            self.root,
            wrap=tk.WORD,
            undo=True,
            font=("Courier New", 12),
        )
        self.text.pack(fill=tk.BOTH, expand=True)
        self.text.focus_set()

        self.status_var = tk.StringVar(value="Ligne 1, Colonne 1")
        status_bar = tk.Label(
            self.root,
            textvariable=self.status_var,
            anchor=tk.W,
            relief=tk.SUNKEN,
            padx=5,
        )
        status_bar.pack(fill=tk.X, side=tk.BOTTOM)

        self.text.bind("<KeyRelease>", lambda event: self._update_status())
        self.text.bind("<ButtonRelease>", lambda event: self._update_status())
        self.text.edit_modified(False)

    def _create_menus(self) -> None:
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)

        file_menu = tk.Menu(menubar, tearoff=False)
        file_menu.add_command(label="Nouveau", command=self.new_file, accelerator="Ctrl+N")
        file_menu.add_command(label="Ouvrir...", command=self.open_file, accelerator="Ctrl+O")
        file_menu.add_command(label="Enregistrer", command=self.save_file, accelerator="Ctrl+S")
        file_menu.add_command(label="Enregistrer sous...", command=self.save_file_as)
        file_menu.add_separator()
        file_menu.add_command(label="Quitter", command=self.on_exit, accelerator="Ctrl+Q")
        menubar.add_cascade(label="Fichier", menu=file_menu)

        edit_menu = tk.Menu(menubar, tearoff=False)
        edit_menu.add_command(label="Annuler", command=self._event_generate("<<Undo>>"), accelerator="Ctrl+Z")
        edit_menu.add_command(label="Rétablir", command=self._event_generate("<<Redo>>"), accelerator="Ctrl+Y")
        edit_menu.add_separator()
        edit_menu.add_command(label="Couper", command=self._event_generate("<<Cut>>"), accelerator="Ctrl+X")
        edit_menu.add_command(label="Copier", command=self._event_generate("<<Copy>>"), accelerator="Ctrl+C")
        edit_menu.add_command(label="Coller", command=self._event_generate("<<Paste>>"), accelerator="Ctrl+V")
        edit_menu.add_command(label="Tout sélectionner", command=self._event_generate("<<SelectAll>>"), accelerator="Ctrl+A")
        menubar.add_cascade(label="Édition", menu=edit_menu)

        help_menu = tk.Menu(menubar, tearoff=False)
        help_menu.add_command(label="À propos", command=self.show_about)
        menubar.add_cascade(label="Aide", menu=help_menu)

    def _bind_shortcuts(self) -> None:
        self.root.bind("<Control-n>", lambda event: self.new_file())
        self.root.bind("<Control-o>", lambda event: self.open_file())
        self.root.bind("<Control-s>", lambda event: self.save_file())
        self.root.bind("<Control-q>", lambda event: self.on_exit())

    def _event_generate(self, sequence: str):
        return lambda: self.text.event_generate(sequence)

    def _update_title(self) -> None:
        name = self._file_path.name if self._file_path else "Sans titre"
        self.root.title(f"{name} - EDHM Notepad")

    def _update_status(self) -> None:
        index = self.text.index(tk.INSERT)
        line, column = map(int, index.split("."))
        self.status_var.set(f"Ligne {line}, Colonne {column + 1}")

    def new_file(self) -> None:
        if self._confirm_discard_changes():
            self.text.delete("1.0", tk.END)
            self._file_path = None
            self._update_title()
            self._update_status()
            self.text.edit_modified(False)

    def open_file(self) -> None:
        if not self._confirm_discard_changes():
            return

        path_str = filedialog.askopenfilename(
            title="Ouvrir un fichier",
            filetypes=[("Fichiers texte", "*.txt"), ("Tous les fichiers", "*.*")],
        )
        if path_str:
            self._load_file(Path(path_str))

    def save_file(self) -> None:
        if self._file_path is None:
            self.save_file_as()
        else:
            self._write_to_path(self._file_path)

    def save_file_as(self) -> None:
        path_str = filedialog.asksaveasfilename(
            title="Enregistrer sous",
            defaultextension=".txt",
            filetypes=[("Fichiers texte", "*.txt"), ("Tous les fichiers", "*.*")],
        )
        if path_str:
            self._file_path = Path(path_str)
            self._write_to_path(self._file_path)
            self._update_title()

    def on_exit(self) -> None:
        if self._confirm_discard_changes():
            self.root.destroy()

    def _load_file(self, path: Path) -> None:
        try:
            content = path.read_text(encoding="utf-8")
        except OSError as error:
            messagebox.showerror("Erreur", f"Impossible d'ouvrir le fichier:\n{error}")
            return

        self.text.delete("1.0", tk.END)
        self.text.insert("1.0", content)
        self._file_path = path
        self._update_title()
        self._update_status()
        self.text.edit_modified(False)

    def _write_to_path(self, path: Path) -> None:
        try:
            content = self.text.get("1.0", "end-1c")
            path.write_text(content, encoding="utf-8")
        except OSError as error:
            messagebox.showerror("Erreur", f"Impossible d'enregistrer le fichier:\n{error}")
        else:
            self.text.edit_modified(False)

    def _confirm_discard_changes(self) -> bool:
        if self._is_modified():
            response = messagebox.askyesnocancel(
                "Modifications non enregistrées",
                "Le document a été modifié. Voulez-vous enregistrer avant de continuer ?",
            )
            if response is None:
                return False
            if response:
                self.save_file()
                return not self._is_modified()
        return True

    def _is_modified(self) -> bool:
        return bool(self.text.edit_modified())

    def show_about(self) -> None:
        messagebox.showinfo(
            "À propos",
            "EDHM Notepad\nUn éditeur de texte léger inspiré de Notepad.",
        )


def main(argv: list[str]) -> int:
    initial_path: Path | None = None
    missing_path: Path | None = None
    if len(argv) > 1:
        candidate = Path(argv[1])
        if candidate.exists():
            initial_path = candidate
        else:
            missing_path = candidate

    root = tk.Tk()
    app = NotepadApp(root, initial_path=initial_path)
    if missing_path is not None:
        messagebox.showwarning(
            "Fichier introuvable",
            f"Le fichier '{missing_path}' n'existe pas. Un nouveau document sera créé.",
        )
    root.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
