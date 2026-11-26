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

run_check() {
    local key="$1" schema="$2" file="$3"
    local schema_path="${SCHEMA_DIR}/${schema}"

    # Skip silently if the schema file does not exist yet
    if [[ ! -f "${schema_path}" ]]; then
        echo -e "  ${BOLD}[${key}]${RESET} schema not found (${schema}), skipping"
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
if [[ $# -eq 0 ]]; then
    echo "Usage: $0 <json_file> [<json_file> ...]" >&2
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


