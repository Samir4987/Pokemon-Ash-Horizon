#!/usr/bin/env python3
"""Read-only validator for Pokémon Essentials-style PBS text databases."""
from __future__ import annotations
import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

SECTION_RE = re.compile(r"^\s*\[([^\]]+)\]\s*(?:[#;].*)?$")
KEY_RE = re.compile(r"^\s*([A-Za-z0-9_]+)\s*=\s*(.*?)\s*$")

def scan_file(path: Path, root: Path) -> dict:
    rel = path.relative_to(root).as_posix()
    result = {"file": rel, "sections": [], "issues": [], "section_count": 0}
    current = None
    seen_sections, seen_keys = {}, {}
    try:
        lines = path.read_text(encoding="utf-8-sig").splitlines()
    except (UnicodeDecodeError, OSError) as exc:
        result["issues"].append({"severity":"error","line":1,"code":"read_error","message":str(exc)})
        return result
    for number, raw in enumerate(lines, 1):
        stripped = raw.strip()
        if not stripped or stripped.startswith(("#", ";")):
            continue
        section = SECTION_RE.match(raw)
        if section:
            name = section.group(1).strip()
            normalized = name.casefold()
            if normalized in seen_sections:
                result["issues"].append({"severity":"warning","line":number,"code":"duplicate_section",
                    "message":f"Section [{name}] first declared on line {seen_sections[normalized]}."})
            else:
                seen_sections[normalized] = number
            current = {"name":name,"line":number,"keys":0}
            result["sections"].append(current)
            continue
        if stripped.startswith("["):
            result["issues"].append({"severity":"error","line":number,"code":"malformed_section",
                                     "message":"Malformed section header."})
            continue
        if "=" in raw:
            match = KEY_RE.match(raw)
            if not match:
                result["issues"].append({"severity":"warning","line":number,"code":"malformed_key",
                                         "message":"Expected a simple key=value record."})
                continue
            if current is None:
                result["issues"].append({"severity":"warning","line":number,"code":"key_outside_section",
                                         "message":"Key/value record appears before the first section."})
                continue
            key = match.group(1).casefold()
            identity = (current["name"].casefold(), key)
            if identity in seen_keys:
                result["issues"].append({"severity":"warning","line":number,"code":"duplicate_key",
                    "message":f"Key '{match.group(1)}' repeats in [{current['name']}], first seen on line {seen_keys[identity]}."})
            else:
                seen_keys[identity] = number
            current["keys"] += 1
        elif current is None:
            result["issues"].append({"severity":"warning","line":number,"code":"text_outside_section",
                                     "message":"Non-comment text before the first section."})
    result["section_count"] = len(result["sections"])
    return result

def scan_database(root: Path) -> dict:
    root = root.resolve()
    pbs = root / "PBS" if (root / "PBS").is_dir() else root
    paths = sorted(pbs.rglob("*.txt")) if pbs.exists() else []
    reports = [scan_file(path, pbs) for path in paths]
    counts = Counter(issue["severity"] for report in reports for issue in report["issues"])
    return {"root":str(pbs),"files_scanned":len(reports),
            "sections_scanned":sum(r["section_count"] for r in reports),
            "errors":counts["error"],"warnings":counts["warning"],"files":reports}

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=Path("."),
                        help="Database folder or project root containing PBS/")
    parser.add_argument("--json",type=Path,help="Write full report to JSON")
    args = parser.parse_args()
    report = scan_database(args.root)
    rendered = json.dumps(report,indent=2,ensure_ascii=False)
    if args.json:
        args.json.parent.mkdir(parents=True,exist_ok=True)
        args.json.write_text(rendered+"\n",encoding="utf-8")
    print(f"Scanned {report['files_scanned']} text file(s), {report['sections_scanned']} section(s): {report['errors']} error(s), {report['warnings']} warning(s).")
    for item in report["files"]:
        for issue in item["issues"]:
            print(f"{issue['severity'].upper()}: {item['file']}:{issue['line']} [{issue['code']}] {issue['message']}")
    if not report["files_scanned"]:
        print("No PBS .txt files found; no source database was available to scan.")
    return 1 if report["errors"] else 0

if __name__ == "__main__":
    sys.exit(main())
