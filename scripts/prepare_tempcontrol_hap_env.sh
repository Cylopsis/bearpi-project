#!/usr/bin/env bash
# Prepare the Linux build environment needed to build the TempControl JS HAP.

set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "${SCRIPT_DIR}/.." && pwd)

SKIP_SSL=0
NO_DOWNLOAD=0
RUN_BUILD=0

usage() {
  cat <<'EOF'
Usage: bash scripts/prepare_tempcontrol_hap_env.sh [--skip-ssl] [--no-download] [--build]

Options:
  --skip-ssl     Pass --skip-ssl to build/prebuilts_download.sh.
  --no-download  Skip prebuilts download and only install/check local tools.
  --build        Build the TempControl HAP after preparing the environment.
  -h, --help     Show this help.
EOF
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --skip-ssl)
      SKIP_SSL=1
      ;;
    --no-download)
      NO_DOWNLOAD=1
      ;;
    --build)
      RUN_BUILD=1
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

cd "${REPO_ROOT}"

if [ ! -f "build/prebuilts_download.sh" ]; then
  echo "This script must be run from a checkout that contains build/prebuilts_download.sh." >&2
  exit 1
fi

case "$(uname -s)" in
  Linux)
    HOST_PLATFORM=linux
    ;;
  *)
    echo "This helper is intended for Linux/VM builds. Current host: $(uname -s)" >&2
    exit 1
    ;;
esac

NODE_DIR="${REPO_ROOT}/prebuilts/build-tools/common/nodejs/node-v12.18.4-${HOST_PLATFORM}-x64"
NODE_BIN="${NODE_DIR}/bin"
export PATH="${HOME}/.local/bin:${NODE_BIN}:${PATH}"

run_step() {
  echo
  echo "==> $*"
}

check_file() {
  local path=$1
  if [ -e "${path}" ]; then
    echo "OK ${path}"
  else
    echo "MISSING ${path}" >&2
    return 1
  fi
}

check_command() {
  local command_name=$1
  if command -v "${command_name}" >/dev/null 2>&1; then
    echo "OK ${command_name}: $(command -v "${command_name}")"
  else
    echo "MISSING command: ${command_name}" >&2
    return 1
  fi
}

if [ "${NO_DOWNLOAD}" -eq 0 ]; then
  run_step "Downloading OpenHarmony prebuilts"
  if [ "${SKIP_SSL}" -eq 1 ]; then
    bash build/prebuilts_download.sh --skip-ssl
  else
    bash build/prebuilts_download.sh
  fi
else
  run_step "Skipping prebuilts download"
fi

run_step "Installing hb from build/lite"
python3 -m pip install --user build/lite

run_step "Checking required tools"
missing=0
check_command hb || missing=1
check_file "${REPO_ROOT}/prebuilts/sdk/js-loader/build-tools/ace-loader/webpack.rich.config.js" || missing=1
check_file "${NODE_BIN}/node" || missing=1
check_file "${REPO_ROOT}/prebuilts/build-tools/common/restool/restool" || missing=1

if [ "${missing}" -ne 0 ]; then
  cat >&2 <<'EOF'

Environment preparation is incomplete. Re-run without --no-download, or inspect
the failing download/install step above before building TempControl.hap.
EOF
  exit 1
fi

run_step "Environment ready"
cat <<EOF
Use this command to build the HAP:

  hb build -T //applications/BearPi/BearPi-HM_Micro/samples/temp_control_frontend:temp_control_hap

Expected output:

  out/<product>/system/internal/TempControl.hap
EOF

if [ "${RUN_BUILD}" -eq 1 ]; then
  run_step "Building TempControl HAP"
  hb build -T //applications/BearPi/BearPi-HM_Micro/samples/temp_control_frontend:temp_control_hap
fi
