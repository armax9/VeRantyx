# Native Half-Life: Alyx for MoltenVR

Play Valve's Linux ARM64 version of Half-Life: Alyx on an Apple Silicon Mac with MoltenVR and a Quest headset. The game runs on the Mac; MoltenVR streams it to the headset.

This is a MoltenVR adaptation of [VeRantyx](https://github.com/Ag3497120/VeRantyx), built on Klepton and MoltenVK. Version 0.3.2 includes this fork's tested pipeline preparation, input and frame-timing updates on top of the native upstream runtime through `421b682`.

## Download and install

Download and extract the latest **macOS ARM64 runtime ZIP** from [Releases](https://github.com/armax9/VeRantyx/releases). Then run:

```bash
python3 install.py "/path/to/Half-Life Alyx" "/path/to/depot_546564" --notproton
```

Use your installed Alyx content and your Linux ARM64 depot. To download that depot with your own Steam account, use `NativeARM/scripts/fetch_alyx_arm64.sh <your-steam-login>` from this repository. The installer fetches Linux dependencies, sets up the native runtime, and backs up an existing managed installation. Updates preserve settings and shader caches. Omit `--notproton` if you prefer the launcher directly.

You need:

- An Apple Silicon Mac running macOS 14.2 or newer.
- [MoltenVR](https://github.com/MoltenVR/MoltenVR-Preview) configured with your headset.
- Your own copy of Half-Life: Alyx, its installed content and Valve's Linux ARM64 depot.
- **Native Steam for macOS, running and signed in.** Steam inside CrossOver alone is insufficient for this runtime.

A Windows Alyx installation can supply the shared content. This native launch route does not use CrossOver, OpenComposite or SteamVR.

## Play from Steam

[NotProton](https://github.com/NotProtonNot/NotProton) is recommended for convenient launching: it exposes compatibility-tool selection in macOS Steam, allowing the existing Alyx library entry and launch-options field to start the native game.

1. Install NotProton and use the installer command above with `--notproton`.
2. Fully restart Steam.
3. Open **Alyx → Properties → Compatibility** and select **Alyx Native - MoltenVR**.
4. Remove Windows-specific launch wrappers and variables such as `%command%`, `CX_GRAPHICS_BACKEND` and the Windows `XR_RUNTIME_JSON` path.
5. Start MoltenVR and press **Play** in Steam. You can put on the headset later: the game prepares shaders and loads without waiting for the headset.

Example launch options:

```text
-novid +vr_fidelity_level_auto 0 +vr_fidelity_level 3
```

Without a fidelity override, automatic fidelity is used. Your Low / Medium / High graphics preset is saved by the game.

For a direct launch:

```bash
bash "/path/to/Half-Life Alyx/start.command"
```

Arguments can be appended to that command. Exit through the game menu, or use Ctrl+C for a terminal launch; the launcher requests a normal game shutdown and waits for saving to finish.

## What works

Tested by the maintainer on an M3 Max with Quest 3:

- Native ARM64 Alyx rendering and streaming through MoltenVR.
- Head tracking, stereo projection and correction during head rotation.
- Quest controller input, gravity gloves and object interaction.
- Persistent graphics settings and native save-slot handling.
- Steam launching with the optional NotProton compatibility tool.

Output resolution follows MoltenVR's recommendation at launch. Restart the game after changing it. Controller-driven finger animation is supported; optical finger tracking is not implemented.

## New in 0.3.2

- Quest controller finger animation now uses capacitive touches on triggers, thumbsticks, A/B, X/Y and thumbrests, independently for each hand and separately from presses.
- Resting hands no longer curl from small thumbstick drift, and grip pressure no longer substitutes for thumb contact.
- Keep up to eight ragdolls on Low instead of the portable profile’s limit of two. Other graphics settings are unchanged; Steam launch options can override the limit.
- Fix excessive fog illumination caused by skipping lighting and shadow-mask draws while their pipelines compiled. Depth, offscreen and single-sample service passes complete before first use; ordinary 4x-MSAA scene materials retain nonblocking compilation.
- Fog remains enabled with its original settings. The fix does not lower the graphics preset or disable the effect.

Includes the in-headset orange preparation display, native-resolution loading screens, headset-independent startup, persistent settings, gravity gloves, throw timing and 4x MSAA from the previous release.

The first preparation may take a minute or longer. A new process still replays its pipeline list on later launches, usually faster when the system Metal compiler cache is warm. The list does not cover every combination in every scene. Performance depends on your Mac, resolution, graphics settings and streaming connection.

No separate app is needed. Exit through the game menu or Ctrl+C so settings and caches can finish saving.

## Building and credits

Build instructions: [MoltenVRBridge/build-release.sh](MoltenVRBridge/build-release.sh). Runtime packaging: [MoltenVRBridge/RELEASE_PACKAGING.md](MoltenVRBridge/RELEASE_PACKAGING.md).

Thanks to [VeRantyx](https://github.com/Ag3497120/VeRantyx), [medved91](https://github.com/medved91) for the native runtime refinements, [Klepton](https://github.com/shinyquagsire23/Klepton), [MoltenVK](https://github.com/KhronosGroup/MoltenVK), [MoltenVR](https://github.com/MoltenVR/MoltenVR-Preview), Valve, ALVR and NotProton. Shader-recipe reconstruction and replay are adapted from [Glovebox](https://github.com/glovebox-vr/glovebox) using [Fossilize](https://github.com/ValveSoftware/Fossilize). Cache-policy research also compared [Alyx Jailbreak](https://github.com/alferiko/alyx-jailbreak-releases) and [Qualyx](https://github.com/tinsarfal/Qualyx); their Android game libraries and private binary patches are not bundled.

No game files, Steam credentials, MoltenVR runtime or CrossOver files are included. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for licenses.
