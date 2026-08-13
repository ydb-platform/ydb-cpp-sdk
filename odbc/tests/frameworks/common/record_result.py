#!/usr/bin/env python3

import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="Append a normalized ODBC test result")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--id", required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--status", choices=("passed", "failed", "skipped", "broken"), required=True)
    parser.add_argument("--start", type=int, required=True)
    parser.add_argument("--stop", type=int, required=True)
    parser.add_argument("--message", default="")
    parser.add_argument("--log", type=Path)
    args = parser.parse_args()

    if args.output.exists():
        document = json.loads(args.output.read_text(encoding="utf-8"))
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        document = {"schema_version": 1, "tests": []}

    result = {
        "id": args.id,
        "name": args.name,
        "status": args.status,
        "start": args.start,
        "stop": args.stop,
    }
    if args.message:
        result["message"] = args.message
    if args.log:
        result["attachments"] = [{
            "name": args.log.name,
            "path": args.log.name,
            "type": "text/plain",
        }]
    document["tests"].append(result)
    args.output.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
