#!/usr/bin/env bash

######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : run_validation.sh
# @created     : Friday Jun 12, 2026 10:53:19 CEST
#
# @description : Run the JSON schema validator for each subsystem 
#                defined in a geometry file.
#                Usage:
#                  ./run_validation.sh <json_file> [<json_file> ...]
#
#                Example:
#                  ./run_validation.sh LArCh/larch_tpc_tub.json
######################################################################

set -euo pipefail

# Environment -----------------------------------------------------------------
if [[ -z "${VIRTUAL_ENV:-}" ]]; then
  for venv_dir in "/opt/solarsim-venv" "${SLAR_ASSETS_GEO:-}/.venv" ".venv"; do
    if [[ -f "${venv_dir}/bin/activate" ]]; then
      source "${venv_dir}/bin/activate"
      echo "Using Python venv: ${venv_dir}"
      break
    fi
  done
fi

# if SLAR_ASSETS_GEO is not set, set it to the current directory
if [[ -z "${SLAR_ASSETS_GEO:-}" ]]; then
    SLAR_ASSETS_GEO="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
fi

echo "Using SLAR_ASSETS_GEO=${SLAR_ASSETS_GEO}"

VALIDATOR="python3 ${SLAR_ASSETS_GEO}/validate_geo.py"
SCHEMA_DIR="${SLAR_ASSETS_GEO}/schemas"

# Subsystem registry ----------------------------------------------------------
# Each entry is "KEY:schema_file". Add a line here whenever you introduce a
# new subsystem schema.
declare -a SUBSYSTEMS=(
    "TPC:tpc.schema.json"
    "Cryostat:cryostat.schema.json"
    "Anode:anode.schema.json"
    "Cathode:cathode.schema.json"
    "ReadoutTile:readouttile.schema.json"
    "OpDetModules:opdet.schema.json"
    "PhotoDetectionSystem:pds.schema.json"
)

# Helpers ---------------------------------------------------------------------
RED='\033[0;31m'; GREEN='\033[0;32m'; BOLD='\033[1m'; RESET='\033[0m'

pass=0; fail=0; skip=0

# Command-line options -------------------------------------------------------
# Global skips: --skip KEY (repeatable)
declare -a GLOBAL_SKIPS=()

usage() {
  cat <<EOF
Usage:
  $0 [options] <json_file> [<json_file> ...]

Options:
  -s, --skip KEY             Skip subsystem KEY for all files (repeatable)
  -h, --help                 Show this help

Examples:
  $0 --skip PhotoDetectionSystem geom1.json geom2.json
EOF
}

# Parse command-line options ------------------------------------------------
contains_word() {
  local needle="$1"; shift
  local x
  for x in "$@"; do [[ "$x" == "$needle" ]] && return 0; done
  return 1
}

should_skip_for_file() {
  local file="$1" key="$2"
  local base="${file##*/}"

  contains_word "$key" "${GLOBAL_SKIPS[@]:-}"
}

# Run check for a single subsystem ------------------------------------------------
run_check() {
    local key="$1" schema="$2" file="$3"
    local schema_path="${SCHEMA_DIR}/${schema}"

    # Skip silently if the schema file does not exist yet
    if [[ ! -f "${schema_path}" ]]; then
        echo -e "  ${BOLD}[${key}]${RESET} schema not found (${schema}), skipping"
        (( skip++ )) || true
        return
    fi

    if should_skip_for_file "$file" "$key"; then
      echo -e "  ${BOLD}[${key}]${RESET} skipped"
      (( skip++ )) || true
      return
    fi

    echo -e "${BOLD}── ${key} │ ${file}${RESET}"
    if ${VALIDATOR} --strict --schema "${schema_path}" --key "${key}" "${file}"; then
        (( pass++ )) || true
    else
        (( fail++ )) || true
    fi
}

# Main ------------------------------------------------------------------------
# --- getopt parsing ---
SHORT_OPTS="hs:"
LONG_OPTS="help,skip:"

PARSED="$(getopt -o "$SHORT_OPTS" -l "$LONG_OPTS" -- "$@")" || {
  usage >&2; exit 2;
}
eval set -- "$PARSED"

while true; do
  case "$1" in
    -h|--help)
      usage; exit 0 ;;
    -s|--skip)
      GLOBAL_SKIPS+=("$2"); shift 2 ;;
    --)
      shift; break ;;
    *)
      echo "Internal option parsing error: $1" >&2
      exit 2 ;;
  esac
done

if [[ $# -eq 0 ]]; then
  usage >&2
  exit 2
fi

for file in "$@"; do
    echo -e "\n${BOLD}════ ${file} ════${RESET}"
    for entry in "${SUBSYSTEMS[@]}"; do
        key="${entry%%:*}"
        schema="${entry##*:}"
        run_check "${key}" "${schema}" "${file}"
    done
done

# Summary ---------------------------------------------------------------------
echo -e "\n$(printf '─%.0s' {1..50})"
echo -e "Passed: ${GREEN}${pass}${RESET}  Failed: ${RED}${fail}${RESET}  Skipped: ${skip}"
[[ ${fail} -eq 0 ]]   # exits 0 if all passed, 1 if any failed


