#!/bin/bash
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
repo="$(cd "$here/.." && pwd)"
kl="$repo/NativeARM/build/Klepton"
if [[ ! -d "$kl/.git" ]]; then git clone https://github.com/shinyquagsire23/Klepton.git "$kl"; fi
commit=$(cat "$here/klepton/COMMIT")
if [[ -n "$(git -C "$kl" status --porcelain)" ]]; then
 echo 'Use a clean Klepton checkout for this complete patch (not upstream apply.py).' >&2; exit 1
fi
git -C "$kl" checkout "$commit"
git -C "$kl" apply "$here/klepton/working-runtime.patch"
cp -R "$here/klepton/new/runtime/." "$kl/runtime/"
make -C "$kl" mvk
make -C "$repo/NativeARM/host" MVK_DIR='./moltenvk-game'
sdk="$repo/NativeARM/build/OpenXR-SDK"
if [[ ! -d "$sdk/.git" ]]; then git clone --depth 1 --branch release-1.1.49 https://github.com/KhronosGroup/OpenXR-SDK.git "$sdk"; fi
command -v glslangValidator >/dev/null || { echo 'Install glslang (brew install glslang) first.' >&2; exit 1; }
glslangValidator -V "$here/warp.vert" -o "$here/warp.vert.spv"
glslangValidator -V "$here/warp.frag" -o "$here/warp.frag.spv"
clang++ -arch arm64 -std=c++17 -O2 "$here/bridge.cpp" -I"$sdk/include" \
 -I"$kl/vendor-moltenvk/out/include" -L"$kl/vendor-moltenvk/out/macos" -lMoltenVK -o "$here/bridge"
mvk_link=$(otool -L "$here/bridge" | awk '/libMoltenVK.dylib/ {print $1; exit}')
install_name_tool -change "$mvk_link" '@loader_path/libMoltenVK.dylib' "$here/bridge"
codesign --force --sign - "$here/bridge"
echo 'Built alyx_host, bridge and warp shaders. See RELEASE_PACKAGING.md for the binary allowlist.'
