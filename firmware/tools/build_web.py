#!/usr/bin/env python3
"""
build_web.py — Assemble l'interface web (index.html + style.css + app.js)
en une seule page et la génère sous forme d'en-tête C++ pour l'ESP32.

Pourquoi : on développe l'interface dans de vrais fichiers HTML/CSS/JS
(coloration, formatage, aperçu dans le navigateur avec VS Code), puis on
l'embarque dans le firmware sans système de fichiers (pas de LittleFS).

Utilisation (depuis le dossier firmware/) :
    python tools/build_web.py
    python tools/build_web.py --src web --out boat_control_final/web_page.h --var WEB_PAGE

Le fichier généré ne doit pas être modifié à la main.
"""
import argparse
import re
from pathlib import Path

DELIM = "HTML" 


def inline_assets(src: Path) -> str:
    html = (src / "index.html").read_text(encoding="utf-8")

    def css_repl(m):
        css = (src / m.group(1)).read_text(encoding="utf-8")
        css = re.sub(r"\A/\*.*?\*/\s*", "", css, flags=re.S)  
        return "<style>\n" + css + "</style>"

    def js_repl(m):
        js = (src / m.group(1)).read_text(encoding="utf-8")
        js = re.sub(r"\A(//[^\n]*\n)+\s*", "", js)  
        return "<script>\n" + js + "</script>"

    html = re.sub(r'<link rel="stylesheet" href="([^"]+)">', css_repl, html)
    html = re.sub(r'<script src="([^"]+)"></script>', js_repl, html)
    if f'){DELIM}"' in html:
        raise SystemExit(f'Erreur : la séquence ){DELIM}" apparaît dans la page.')
    return html


def main():
    here = Path(__file__).resolve().parent.parent  # dossier firmware/
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--src", default=str(here / "web"), help="dossier contenant index.html")
    ap.add_argument("--out", default=str(here / "boat_control_final" / "web_page.h"), help="en-tête C++ généré")
    ap.add_argument("--var", default="WEB_PAGE", help="nom de la constante C++")
    args = ap.parse_args()

    src, out = Path(args.src), Path(args.out)
    page = inline_assets(src)
    header = (
        "// =====================================================================\n"
        "// FICHIER GÉNÉRÉ AUTOMATIQUEMENT — NE PAS MODIFIER À LA MAIN\n"
        f"// Source : {src.name}/index.html (+ CSS/JS)  ·  Générateur : tools/build_web.py\n"
        "// =====================================================================\n"
        "#pragma once\n"
        "#include <Arduino.h>\n\n"
        f'const char {args.var}[] PROGMEM = R"{DELIM}(\n{page}){DELIM}";\n'
    )
    out.write_text(header, encoding="utf-8")
    print(f"{out} généré ({len(page.encode('utf-8'))} octets de HTML)")


if __name__ == "__main__":
    main()
