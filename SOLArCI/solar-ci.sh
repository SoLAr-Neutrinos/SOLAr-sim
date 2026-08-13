#!/usr/bin/env bash
set -euo pipefail

# source Geant4 environment if available
if [[ -f /opt/geant4/bin/geant4.sh ]]; then
  # shellcheck disable=SC1091
  source /opt/geant4/bin/geant4.sh
  export G4LEDATA=$( geant4-config --datasets | grep G4LEDATA | cut -d' ' -f3 )
fi

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd .. && pwd)"
SIM_SOURCE_DIR="${BASE_DIR}/G4SOLAr"
UTILS_SOURCE_DIR="${BASE_DIR}/SOLArUtils/source"
ASSETS_DIR="${SIM_SOURCE_DIR}/assets"
GEO_CONFIGS_DIR="${ASSETS_DIR}/geometry"

EXT_BUILD_DIR="${BASE_DIR}/build/ext"
SIM_BUILD_DIR="${BASE_DIR}/build/sim"
UTILS_BUILD_DIR="${BASE_DIR}/build/utils"
LOG_DIR="${BASE_DIR}/build/ci-logs"

EXT_INSTALL_DIR="${BASE_DIR}/install/ext"
SIM_INSTALL_DIR="${BASE_DIR}/install/sim"
UTILS_INSTALL_DIR="${BASE_DIR}/install/utils"
SIM_INSTALL_GEO="${SIM_INSTALL_DIR}/geometry"

# ---- Test geometries ----
GEO_CONFIGS=(
  "singlecube/singlecube_compose.json"
  "solarfd/solar_fd_compose.json"
)

mkdir -p "${SIM_INSTALL_GEO}" "${LOG_DIR}"

run_stage() {
  local name="$1"; shift
  echo "==> ${name}"
  if "$@" >"${LOG_DIR}/${name}.log" 2>&1; then
    echo "✅ ${name}"
  else
    echo "❌ ${name} (log: ${LOG_DIR}/${name}.log)"
    tail -n 80 "${LOG_DIR}/${name}.log" || true
    return 1
  fi
}

echo "==> [1/5] Compose geometries with jcompose"
for cfg in "${GEO_CONFIGS[@]}"; do
  name="$(basename "${cfg}" .json)"
  out="${SIM_INSTALL_GEO}/${name}.geometry.json"
  echo " - composing ${cfg} -> ${out}"
  jcompose --path "${GEO_CONFIGS_DIR}" -o "${out}" "${GEO_CONFIGS_DIR}/${cfg}"
done
cp "${SIM_SOURCE_DIR}/assets/materials/materials_db.json" "${SIM_INSTALL_GEO}/materials_db.json"
cp "${SIM_SOURCE_DIR}/assets/macros/dry_run.mac" "${SIM_INSTALL_DIR}/dry_run.mac"

echo "==> [2/5] Validate geometry JSONs against schemas"
export SLAR_ASSETS_GEO="${GEO_CONFIGS_DIR}"
geo_validation_fail=0

for geom in "${SIM_INSTALL_GEO}"/*.geometry.json; do
  geom_basename="$(basename "${geom}")"
  log_file="${LOG_DIR}/validate_${geom_basename%.json}.log"

  echo " - validating ${geom}"
  if [[ "${geom_basename}" == *"singlecube"* ]]; then
    if ! bash "${SLAR_ASSETS_GEO}/run_validation.sh" --skip PhotoDetectionSystem "${geom}" >"${log_file}" 2>&1; then
      echo "   ❌ Validation failed for ${geom} (log: ${log_file})"
      tail -n 60 "${log_file}" || true
      ((geo_validation_fail++))
    else
      echo "   ✅ Validation passed for ${geom}"
    fi
  else
    if ! bash "${SLAR_ASSETS_GEO}/run_validation.sh" "${geom}" >"${log_file}" 2>&1; then
      echo "   ❌ Validation failed for ${geom} (log: ${log_file})"
      tail -n 60 "${log_file}" || true
      ((geo_validation_fail++))
    else
      echo "   ✅ Validation passed for ${geom}"
    fi
  fi
done

if [[ "${geo_validation_fail}" -ne 0 ]]; then
  echo "❌ ${geo_validation_fail} geometry file(s) failed validation"
  exit 1
fi

echo "==> [3/5] Build simulation"
run_stage "build_ext_configure" cmake -S "${SIM_SOURCE_DIR}/extern" -B "${EXT_BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${EXT_INSTALL_DIR}"
run_stage "build_ext_compile"   cmake --build "${EXT_BUILD_DIR}" -j"$(nproc)"
run_stage "build_ext_install"   cmake --install "${EXT_BUILD_DIR}"

run_stage "build_sim_configure" cmake -S "${SIM_SOURCE_DIR}" -B "${SIM_BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${SIM_INSTALL_DIR}" -DSOLARSIM_EXT_DIR="${EXT_INSTALL_DIR}"
run_stage "build_sim_compile"   cmake --build "${SIM_BUILD_DIR}" -j"$(nproc)"
run_stage "build_sim_install"   cmake --install "${SIM_BUILD_DIR}"

echo "==> [4/5] Build utils"
run_stage "build_utils_configure" cmake -S "${UTILS_SOURCE_DIR}" -B "${UTILS_BUILD_DIR}" -DCMAKE_PREFIX_PATH="${SIM_INSTALL_DIR}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${UTILS_INSTALL_DIR}"
run_stage "build_utils_compile"   cmake --build "${UTILS_BUILD_DIR}" -j"$(nproc)"
run_stage "build_utils_install"   cmake --install "${UTILS_BUILD_DIR}"

echo "==> [5/5] Dry run against composed geometries"
# headless safeguard
unset DISPLAY || true
export QT_QPA_PLATFORM=offscreen
export G4UI_USE_QT=0
export G4VIS_NONE=1

echo "[debug] LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
for geom in "${SIM_INSTALL_GEO}"/*.geometry.json; do
  base="$(basename "${geom}" .json)"
  log_file="${LOG_DIR}/dryrun_${base}.log"
  echo " - dry run with ${geom}"

  set +e
  "${SIM_INSTALL_DIR}/bin/solar_sim" \
    --geometry "${geom}" \
    --materials "${SIM_INSTALL_GEO}/materials_db.json" \
    --macro "${SIM_INSTALL_DIR}/dry_run.mac" \
    2>&1 | tee "${log_file}"
  rc=${PIPESTATUS[0]}
  set -e

  if [[ $rc -ne 0 ]]; then
    echo "   ❌ Dry run failed for ${geom} (exit code: ${rc}, log: ${log_file})"
    if [[ ! -s "${log_file}" ]]; then
      echo "   ⚠️ Log is empty. Possible crash/signal before output."
    fi
    dryrun_fail=1
  else
    echo "   ✅ Dry run passed for ${geom}"
  fi
done

echo "✅ Local CI pipeline completed."
echo "Logs available in: ${LOG_DIR}"
