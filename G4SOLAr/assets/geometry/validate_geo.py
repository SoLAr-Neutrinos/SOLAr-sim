#!/usr/bin/env python3
"""
validate_geo.py — Validate simulation geometry JSON files against a JSON Schema.

Usage:
    validate_geo.py --schema schemas/tpc.schema.json --key tpc \\
                far-detector/fd_tpc.json single-cube/small_tpc.json

    # Glob expansion (quote the pattern to avoid shell expansion if preferred)
    validate_geo.py --schema schemas/tpc.schema.json --key tpc "far-detector/*.json"

Exit codes:
    0  All files passed (or were cleanly skipped with --no-strict)
    1  One or more validation errors occurred
    2  Script usage / setup error
"""

import argparse
import glob
import json
import sys
import urllib.parse
from pathlib import Path

# ---------------------------------------------------------------------------
# Dependency check — give a helpful message if jsonschema is not installed
# ---------------------------------------------------------------------------
try:
    from jsonschema import Draft7Validator
    from referencing import Registry, Resource
    from referencing.jsonschema import DRAFT7
except ImportError:
    print(
        "ERROR: required packages are not installed.\n"
        "Install them with:  pip install jsonschema referencing",
        file=sys.stderr,
    )
    sys.exit(2)


# ---------------------------------------------------------------------------
# ANSI colours (disabled automatically when not writing to a terminal)
# ---------------------------------------------------------------------------
def _ansi(code: str, text: str) -> str:
    if sys.stdout.isatty():
        return f"\033[{code}m{text}\033[0m"
    return text

GREEN  = lambda t: _ansi("32", t)
RED    = lambda t: _ansi("31", t)
YELLOW = lambda t: _ansi("33", t)
BOLD   = lambda t: _ansi("1",  t)


# ---------------------------------------------------------------------------
# Core validation logic
# ---------------------------------------------------------------------------

def load_json(path: Path) -> dict | list | None:
    """Load a JSON file, returning None and printing an error on failure."""
    try:
        with path.open() as f:
            return json.load(f)
    except json.JSONDecodeError as exc:
        print(f"  {RED('PARSE ERROR')} {exc}", file=sys.stderr)
        return None
    except OSError as exc:
        print(f"  {RED('IO ERROR')} {exc}", file=sys.stderr)
        return None

def check_key_presence(data: dict | list, key: str | None) -> bool:
    """Return True if key is present (or no key is required)."""
    if key is None:
        return True
    return isinstance(data, dict) and key in data

def _is_root_type_mismatch(error) -> bool:
    """
    Return True when the error is purely "instance is the wrong type for this
    branch" at the top level (path length 0, validator == 'type').
 
    These arise from the wrong branch of a oneOf[object, array] and add noise
    rather than pointing at the actual problem.
    """
    return error.validator == "type" and len(error.path) == 0


def leaf_errors(error):
    """
    Recursively expand combiner errors (oneOf / anyOf / allOf) down to the
    leaf ValidationErrors that actually describe what is wrong.

    Strategy:
      1. Filter out context branches that fail only because of a root-level
         type mismatch (e.g. "not of type array" when instance is an object).
         These are the wrong schema branches and produce misleading messages.
      2. Yield all leaf errors from every remaining branch so the user sees
         the full set of problems, not just one.
    """
    if not error.context:
        yield error
        return
 
    candidates = [e for e in error.context if not _is_root_type_mismatch(e)]
    # If filtering removed everything the instance truly is the wrong type
    if not candidates:
        candidates = [min(error.context, key=lambda e: len(e.path))]
 
    for sub in candidates:
        yield from leaf_errors(sub)

def extract_targets(data: dict | list, key: str | None) -> list:
    """
    Return the list of sub-objects to validate.

    - key=None  → validate the entire document as one object.
    - key='TPC' → look for data['TPC']; if it is a list, validate each
                  element individually so you get per-element error paths.
    """
    if key is None:
        return [data]

    if not isinstance(data, dict) or key not in data:
        return []          # caller decides whether this is an error

    value = data[key]
    return value if isinstance(value, list) else [value]


def validate_file(
    path: Path,
    validator: Draft7Validator,
    key: str | None,
    strict: bool,
) -> bool:
    """
    Validate one file.  Returns True if the file passed, False otherwise.
    """
    print(BOLD(f"\n── {path}"))

    data = load_json(path)
    if data is None:
        return False

    targets = extract_targets(data, key)

    if not targets:
        msg = f"  Key '{key}' not found in document."
        if strict:
            print(f"  {RED('MISSING KEY')} {msg}")
            return False
        else:
            print(f"  {YELLOW('SKIP')} {msg} (use --strict to treat as error)")
            return True

    all_passed = True
    for idx, obj in enumerate(targets):
        # print target object
        prefix = f"[{idx}] " if len(targets) > 1 else ""
        raw_errors = sorted(validator.iter_errors(obj), key=lambda e: list(e.path))
        flat_errors = [e for err in raw_errors for e in leaf_errors(err)]
 
        if not flat_errors:
            print(f"  {GREEN('OK')}  valid")
        else:
            print(f"  {RED('FAIL')} {len(flat_errors)} error(s):")
            all_passed = False
            for err in flat_errors:
                path_str = " → ".join(str(p) for p in err.absolute_path) or "(root)"
                print(f"       • {path_str}: {err.message}")
 
    return all_passed


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Validate geometry JSON files against a JSON Schema.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    p.add_argument(
        "--schema", "-s",
        required=True,
        metavar="SCHEMA",
        help="Path to the JSON Schema file (supports $ref resolution relative to the schema directory).",
    )
    p.add_argument(
        "--key", "-k",
        default=None,
        metavar="KEY",
        help=(
            "Top-level key in each JSON file whose value should be validated. "
            "If the value is an array, each element is validated independently. "
            "Omit to validate the entire document."
        ),
    )
    p.add_argument(
        "--strict",
        action="store_true",
        help="Treat a missing --key as a validation failure rather than a warning.",
    )
    p.add_argument(
        "targets",
        nargs="+",
        metavar="FILE",
        help="JSON files (or glob patterns) to validate.",
    )
    return p


def resolve_targets(raw: list[str]) -> list[Path]:
    """Expand globs and deduplicate, preserving order."""
    seen: set[Path] = set()
    result: list[Path] = []
    for pattern in raw:
        matches = [Path(p) for p in glob.glob(pattern, recursive=True)]
        # Fall back to treating the argument as a literal path when glob matches nothing
        if not matches:
            matches = [Path(pattern)]
        for p in matches:
            if p not in seen:
                seen.add(p)
                result.append(p)
    return result


def build_registry(schema_path: Path, schema: dict) -> Registry:
    """
    Build a referencing.Registry anchored at *schema_path*.

    The retrieve callable is invoked lazily whenever the validator encounters
    a $ref URI it hasn't seen yet, loading the target file from disk.
    This replaces the deprecated jsonschema.RefResolver.
    """
    schema_dir = schema_path.parent

    # Pre-index all schemas in the directory by their $id for reliable lookup
    id_to_path: dict[str, Path] = {}
    for p in schema_dir.glob("*.json"):
        try:
            with p.open() as f:
                s = json.load(f)
            if "$id" in s:
                id_to_path[s["$id"]] = p
        except (json.JSONDecodeError, OSError):
            pass
 
    def retrieve(uri: str) -> Resource:
        # Try $id index first, then fall back to bare filename in schema_dir
        parsed_name = Path(urllib.parse.urlparse(uri).path).name
        source_path = id_to_path.get(uri) or id_to_path.get(parsed_name)
        if source_path is None:
            # Last resort: treat uri as a filename relative to schema_dir
            source_path = schema_dir / parsed_name
        if not source_path.exists():
            raise FileNotFoundError(
                f"Cannot resolve schema URI '{uri}'.\n"
                f"  Looked up: {list(id_to_path.keys())}\n"
                f"  Schema dir: {schema_dir}"
            )
        with source_path.open() as f:
            contents = json.load(f)
        return Resource.from_contents(contents, default_specification=DRAFT7)
 
    main_resource = Resource.from_contents(schema, default_specification=DRAFT7)
    return Registry(retrieve=retrieve).with_resource(schema_path.as_uri(), main_resource)


def main() -> int:
    args = build_parser().parse_args()

    # ── Load schema ──────────────────────────────────────────────────────────
    schema_path = Path(args.schema).resolve()
    if not schema_path.exists():
        print(f"ERROR: schema file not found: {schema_path}", file=sys.stderr)
        return 2

    with schema_path.open() as f:
        schema = json.load(f)

    registry = build_registry(schema_path, schema)
    validator = Draft7Validator(schema, registry=registry)

    # print the loaded schema for debugging
    print("Loaded schema:")
    print(json.dumps(schema, indent=4))

    try:
        Draft7Validator.check_schema(schema)
        # Walk all $refs to trigger resolution errors immediately
        for error in Draft7Validator(schema, registry=registry).iter_errors({}):
            pass  # we only care about RefResolutionErrors here, not validation failures
    except Exception as exc:
        print(f"ERROR: schema is broken: {exc}", file=sys.stderr)
        sys.exit(2)

    # ── Resolve target files ─────────────────────────────────────────────────
    targets = resolve_targets(args.targets)
    if not targets:
        print("ERROR: no target files found.", file=sys.stderr)
        return 2

    print(
        f"Schema : {schema_path.name}\n"
        f"Key    : {args.key or '(entire document)'}\n"
        f"Files  : {len(targets)}"
    )

    # ── Validate ─────────────────────────────────────────────────────────────
    results = []
    for target in targets:
        passed = validate_file(target, validator, args.key, args.strict)
        results.append((target, passed))

    # ── Summary ──────────────────────────────────────────────────────────────
    passed_count = sum(1 for _, ok in results if ok)
    failed_count = len(results) - passed_count

    print(f"\n{'─' * 50}")
    print(f"Results: {GREEN(f'{passed_count} passed')}  {RED(f'{failed_count} failed')}  (total: {len(results)})")

    if failed_count:
        print(RED("\nValidation FAILED."))
        return 1

    print(GREEN("\nAll files valid."))
    return 0


if __name__ == "__main__":
    sys.exit(main())
