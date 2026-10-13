#!/bin/bash
set -euo pipefail
# Forward Steam launch options without shell evaluation.
has_auto=0; has_level=0; has_map=0
for argument in "$@"; do
 case "$argument" in
  +vr_fidelity_level_auto) has_auto=1 ;;
  +vr_fidelity_level) has_level=1 ;;
  +map) has_map=1 ;;
 esac
done
if (( !has_auto )); then
 if (( has_level )); then set -- +vr_fidelity_level_auto 0 "$@"
 else set -- +vr_fidelity_level_auto 1 "$@"; fi
fi
if (( !has_map )); then set -- +map "${ALYX_MAP:-startup}" "$@"; fi
if [[ "${ALYX_NATIVE_DRY_RUN:-0}" == 1 ]]; then
 printf 'native-argument: <%s>\n' "$@"
 exit 0
fi
install="$(cd "$(dirname "$0")" && pwd)"
base="$install/verantyx"
game="$base/native-game"
[[ -d "$game/game/bin/linuxsteamrtarm64" && -d "$base/guest-lib" ]] || { echo 'Run install.py with your ARM64 depot first.' >&2; exit 1; }
logs=$(mktemp -d "${TMPDIR:-/tmp}/alyx-native.XXXXXX")
if pgrep -f "$base/alyx_host" >/dev/null; then
 echo 'Close the previous Alyx session and retry.';exit 1
fi
if pgrep -f '[/]HardwareEncoder' >/dev/null; then
 echo 'Close the previous VeRantyx encoder session and retry.';exit 1
fi
export KL_MVK_DIR="$base/moltenvk-game"
export VERANTYX_SHADER_DIR="$base"
export VERANTYX_SETTINGS_FILE="$base/native-vr-settings.txt"
export VERANTYX_SHARED_FRAME="$game/vr_shared_frame.dat"
export VERANTYX_BRIDGE_READY="$logs/ready"
export MVK_CONFIG_LOG_LEVEL=0
export ALYX_ENGINE_LOG=0
export VERANTYX_QUIET=1
export KL_VK_TRACE=0
export KL_TRACE=0
export KL_TRACE_FS=0
unset ALYX_PIPELINE_RECORDER_LIBRARY ALYX_PIPELINE_RECORD_FILE
# Report the actual Mac memory through the Linux compatibility layer.
mem_bytes=$(/usr/sbin/sysctl -n hw.memsize)
export KL_MEM_TOTAL_MB="$((mem_bytes / 1048576))"
export KL_VK_ASYNC_CACHE_DIR="$base/pipeline-cache"
mkdir -p "$KL_VK_ASYNC_CACHE_DIR"
unset KL_VK_WAIT_FIRST_USE
export VERANTYX_BENCH=0
unset VERANTYX_EYE_WIDTH VERANTYX_EYE_HEIGHT
unset VERANTYX_OUTPUT_EYE_WIDTH VERANTYX_OUTPUT_EYE_HEIGHT
export VERANTYX_RUNTIME_RESOLUTION_FILE="$logs/runtime-resolution.txt"
export VERANTYX_DISPLAY_HZ=72
export VERANTYX_SYNC_TRACKING=1
export VERANTYX_LOCAL_ROTATION_WARP="${VERANTYX_LOCAL_ROTATION_WARP:-1}"
export VB_FPS=72
export MOLTENVR_FPS_FILE="$logs/runtime-fps"
printf '%s\n' "$VERANTYX_DISPLAY_HZ" > "$MOLTENVR_FPS_FILE"
unset VERANTYX_SUBMIT_CURRENT_POSE
export VERANTYX_SHARED_FRUSTUM=1
# Leave IPD at runtime defaults; no maintainer headset calibration is shipped.
rm -f "$VERANTYX_BRIDGE_READY" "$VERANTYX_RUNTIME_RESOLUTION_FILE"
bridge_pid='';game_pid='';settings_pid=''

cleanup(){
 if [[ -n "$game_pid" ]] && kill -0 "$game_pid" 2>/dev/null;then
  echo 'Stopping Alyx and saving shader cache...';kill -TERM "$game_pid" 2>/dev/null || true
  for _ in {1..30};do kill -0 "$game_pid" 2>/dev/null || break;sleep 1;done
  kill -KILL "$game_pid" 2>/dev/null || true
 fi
 if [[ -n "$settings_pid" ]];then kill -TERM "$settings_pid" 2>/dev/null || true;wait "$settings_pid" 2>/dev/null || true;fi
 if [[ -n "$bridge_pid" ]];then kill -TERM "$bridge_pid" 2>/dev/null || true;wait "$bridge_pid" 2>/dev/null || true;fi
 rm -rf "$logs"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
"$base/bridge" > /dev/null 2>&1 &
bridge_pid=$!
echo 'Waiting for MoltenVR tracking. Wear the Quest headset.'
for _ in {1..60};do
 [[ -f "$VERANTYX_BRIDGE_READY" ]] && break
 if ! kill -0 "$bridge_pid" 2>/dev/null;then echo "VR bridge exited before tracking became ready.";exit 1;fi
 sleep 1
done
if [[ ! -f "$VERANTYX_BRIDGE_READY" ]];then echo "Tracking timeout.";exit 1;fi
read -r runtime_width runtime_height < "$VERANTYX_RUNTIME_RESOLUTION_FILE"
if ! [[ "$runtime_width" =~ ^[0-9]+$ && "$runtime_height" =~ ^[0-9]+$ ]] || (( runtime_width < 1 || runtime_width > 8192 || runtime_height < 1 || runtime_height > 8192 )); then
 echo 'Invalid MoltenVR runtime resolution.'; exit 1
fi
export VERANTYX_OUTPUT_EYE_WIDTH="$runtime_width"
export VERANTYX_OUTPUT_EYE_HEIGHT="$runtime_height"
echo "MoltenVR resolution: ${runtime_width}x${runtime_height} per eye"
cd "$game"
"$base/alyx_host" --libdir "$base/guest-lib" --game "$game" -- -vr -steam -novid -noasserts -nopassiveasserts -vulkan -nowindow "$@" > /dev/null 2>&1 &
game_pid=$!
echo "Alyx native → MoltenVR. Steam launch parameters supported."
echo 'Stop with the game menu or Ctrl+C.'
while kill -0 "$game_pid" 2>/dev/null && kill -0 "$bridge_pid" 2>/dev/null;do
 sleep 5

done
echo 'Alyx session ended.'
