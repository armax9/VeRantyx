# VeRantyx — native Half-Life: Alyx for MoltenVR

Run Valve’s Linux ARM64 build of Half-Life: Alyx on an Apple Silicon Mac and stream it to a Quest headset through MoltenVR.

This fork adapts [VeRantyx’s native branch](https://github.com/Ag3497120/VeRantyx/tree/native) for MoltenVR, with controller input, corrected stereo presentation, persistent graphics settings and fixes for object throwing. It is based on upstream commit `12e7826e489a3f7c8526e623e4ca3c2c6d6bb62a`.

## Install the runtime release

Download the ARM64 runtime archive from [Releases](https://github.com/armax9/VeRantyx/releases), extract it, and run:

```bash
python3 install.py "/path/to/Half-Life Alyx" "/path/to/depot_546564" --notproton
```

Use your own installed Alyx content and SteamCMD ARM64 depot. The installer downloads Debian dependencies from the official mirror; it never downloads or includes game files. It creates a separate merged native game root and library links, backs up a previous managed runtime and launcher, and preserves native settings/cache on updates. It refuses to replace an unmanaged `verantyx` folder; rename that folder first if upgrading from a manual installation. Omit `--notproton` for direct launches.

To obtain the depot, install SteamCMD and use `NativeARM/scripts/fetch_alyx_arm64.sh <your-steam-login>` from the source repository. Steam handles authentication. Build instructions are in `MoltenVRBridge/build-release.sh`; the complete working Klepton patch is in `MoltenVRBridge/klepton/`.

**Status:** experimental, based on a working M3 Max / Quest 3 installation. Packaging and installer checks pass; a fresh-machine install and complete Steam Play session still require validation.

## How it works

```text
Valve’s Linux ARM64 Alyx
    → VeRantyx / Klepton on macOS
    → MoltenVK rendering and our native OpenXR bridge
    → MoltenVR
    → Quest headset
```

The game’s CPU code runs as ARM64 through the upstream compatibility layer. Vulkan rendering uses MoltenVK. MoltenVR handles streaming to the headset. This launch path does not use the Windows Alyx executable, Wine, D3DMetal, OpenComposite or SteamVR.

An existing Windows installation of Alyx, including one installed through Steam in CrossOver, can supply the shared game content. The required Linux ARM64 binaries are downloaded separately through SteamCMD using the owner’s Steam account.

## What upstream already provides

VeRantyx and Klepton provide the foundation: loading Valve’s Linux ARM64 binaries on macOS, Linux API compatibility, graphics integration, a native VR interface implementation, macOS Steam communication and tools for downloading and arranging game files. This fork builds on that work; it does not claim to have created the native Alyx port.

The original native setup documents a Vision Pro streaming route. This fork adds a MoltenVR route for Quest use.

## Changes in this fork

### MoltenVR output and headset tracking

- A native OpenXR bridge connects directly to the installed MoltenVR runtime.
- Headset tracking and runtime eye projection data reach the game through the shared frame/pose bridge.
- Output resolution is read from MoltenVR’s runtime recommendation at startup, instead of requiring a resolution specific to one user’s headset configuration.
- Startup waits for live tracking before launching the game.
- Frame presentation carries the pose used to render the image, rather than attaching a newer pose to an older image.
- Local rotation correction uses the source image’s actual field of view. This addresses image stretching and skew during head movement and changes to graphics settings or fidelity.
- Texture bounds are validated and clipped consistently before splitting the shared render frustum into the two eyes.

### Quest controller input

- Touch-controller grip poses, triggers, grips, thumbsticks, stick clicks/touch, A/B/X/Y and the menu button are connected through OpenXR actions.
- The native input implementation supplies Alyx’s action and pose queries.
- Gravity-glove interaction works in the tested setup.
- Controller-derived finger animation is supported. Full optical finger tracking is not part of this release.

### Object throwing and release timing

Earlier test builds could drop held objects at the player’s feet, or require an unnaturally late trigger release to transfer momentum. The working version corrects the input/pose information supplied to Alyx:

- Preserve the hand’s input origin when a digital action changes to released.
- Refresh input when the game updates its action state.
- Timestamp action changes using the tracking packet’s time when valid.
- Keep a short history of measured controller poses and velocities for historical pose queries.
- Honor the relative time requested by the game, while preserving compositor poses for next-frame queries.

These fixes restored throwing and normal release timing in the maintainer’s tests. They do not add an artificial throw multiplier or peak-speed boost.

### Graphics settings and everyday use

- Preserve the native build’s Low / Medium / High graphics preset through the engine’s own archived settings path.
- Add persistent storage for VR settings instead of accepting writes and discarding them.
- Support `fopen64` through the host’s file-opening implementation.
- Supply the Mac’s actual memory size to the compatibility layer.
- Preserve a pipeline cache between sessions. This is not a complete precompiled shader cache: new content can still need compilation or resource loading.
- Forward launch parameters as separate arguments without evaluating them as shell commands.
- Default to automatic fidelity; allow an explicit fidelity level through launch options.
- Normal launches suppress diagnostic output and remove temporary session files. Persistent settings and pipeline caches are retained.

## Requirements

- An Apple Silicon Mac.
- [MoltenVR](https://github.com/MoltenVR/MoltenVR-Preview) installed and configured with a compatible headset.
- A licensed copy of Half-Life: Alyx and its installed game content.
- **Native Steam for macOS, running and signed in to the account that owns Alyx.** Steam inside CrossOver alone does not satisfy the current native Steam bridge.
- Valve’s Linux ARM64 Alyx depot, downloaded with the user’s own account through SteamCMD.
- Build dependencies described in upstream [NATIVE_SETUP.md](https://github.com/Ag3497120/VeRantyx/blob/native/docs/NATIVE_SETUP.md) when building from source.

CrossOver is useful if it already holds your game installation, but it is not the execution layer for native Alyx. NotProton is optional for direct script launches.

Tested hardware: M3 Max and Quest 3. Performance depends on resolution, graphics preset, scene complexity and streaming configuration. No fixed FPS or latency is promised for other systems.

## Launch an existing native installation

Start native macOS Steam and the MoltenVR session, then run the launcher installed alongside Alyx:

```bash
bash "/path/to/Half-Life Alyx/start.command"
```

An explicit fidelity level can be passed like this:

```bash
bash "/path/to/Half-Life Alyx/start.command" -novid +vr_fidelity_level_auto 0 +vr_fidelity_level 3
```

Automatic fidelity is used when no fidelity override is supplied. If a level is provided without an auto flag, the launcher disables automatic fidelity for that launch. Graphics presets remain the game’s own saved settings; a preset is not forced on every startup.

Exit through the game menu where possible. For a terminal launch, Ctrl+C stops the session and allows time to save the pipeline cache.

## Optional: launch from macOS Steam with NotProton

[NotProton](https://github.com/NotProtonNot/NotProton) exposes Steam Play functionality in the native macOS Steam client. It is recommended here for convenience: you can use Alyx’s library entry and its normal launch-options field.

Our integration is a separate compatibility tool named **Alyx Native - MoltenVR**. NotProton exposes the selection UI; this tool launches the native runtime directly. Selecting a CrossOver tool instead would launch the Windows version. NotProton does not improve the native game’s rendering speed by itself.

1. Install and configure NotProton according to its own instructions.
2. Ensure Alyx appears as installed in the native Steam library. Installing this tool does not import a CrossOver library or download the game.
3. Install our integration, after the native runtime and `start.command` are installed:

   ```bash
   bash steam/install-notproton.sh "/path/to/Half-Life Alyx"
   ```

4. Fully quit and restart macOS Steam.
5. Open **Alyx → Properties → Compatibility**. Enable the forced compatibility-tool selection and choose **Alyx Native - MoltenVR**.
6. Remove Windows-specific launch settings such as `XR_RUNTIME_JSON='C:\openxr\...'`, `CX_GRAPHICS_BACKEND=d3dmetal`, `D3DM_ENABLE_METALFX=1` and custom `%command%` wrappers.
7. Put game arguments directly into Steam’s launch-options field. For example:

   ```text
   -novid +vr_fidelity_level_auto 0 +vr_fidelity_level 3
   ```

8. Start MoltenVR, wear the headset and press **Play**.

The installer backs up an existing copy of this compatibility tool before updating it. It does not patch NotProton’s source code or modify other compatibility tools. Re-run it if the game folder moves.

The integration has passed local argument-forwarding and installer checks. A fresh installation and a complete Steam Play session on another Mac still need validation before calling the release installation-tested.

## Known limitations

- Experimental, tested on a limited hardware setup.
- Full shader precompilation and removal of all first-use stalls are not implemented.
- Rotation correction helps head movement but does not remove rendering, readback, transfer or streaming latency; it is not positional reprojection.
- Runtime resolution is selected at launch. Restart the session after changing MoltenVR’s output resolution.
- The native build’s graphics menu differs from Windows Alyx and offers three presets.
- One tested save showed a missing pistol icon in the weapon selector; another save displayed it correctly. No universal icon fix is claimed.
- Crouch-related input/configuration changes require further confirmation.
- The graphics-preset archive fix checks a particular native client binary before patching it. Future Valve updates may need a revised fix.
- Launching native Alyx from Windows Steam inside CrossOver is outside this integration.

## Distribution and credits

No Valve game files, Steam credentials, MoltenVR binaries or CrossOver files should be included in the release. Users obtain proprietary components from their owners and download Valve’s ARM64 depot using their own Steam accounts.

Preserve upstream MIT notices and include the applicable licenses for bundled third-party libraries. Credit [VeRantyx](https://github.com/Ag3497120/VeRantyx), Klepton, [MoltenVK](https://github.com/KhronosGroup/MoltenVK), [MoltenVR](https://github.com/MoltenVR/MoltenVR-Preview), Valve and [NotProton](https://github.com/NotProtonNot/NotProton).
