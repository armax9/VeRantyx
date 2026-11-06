# Native Alyx for MoltenVR 0.3.0

This update makes the former Test 3 runtime the main native MoltenVR version.

- Prepare known graphics pipelines before play using a recipe reconstructed from your own game files.
- Start preparation without wearing the headset. Once it finishes, the launcher waits for tracking without the old headset timeout, then starts the playable session with the runtime's real pose and resolution.
- Verify shader, runtime and driver identities so changed game files cannot silently reuse an incompatible preparation database.
- Correct missed display-interval counting and provide measured CPU frame-timing history.
- Improve controller release timing with fresh input and bounded measured hand-pose history, without a throw-speed multiplier.
- Keep 4x MSAA at every automatic fidelity level.
- Remove leftover file/settings diagnostics and fix `KL_TRACE_FS=0` enabling file tracing.

Existing graphics settings, saves, working menu compatibility, gravity-glove input, stereo/projection fixes and clean shutdown are retained. No separate launcher app is required.

## Install or update

Extract the macOS ARM64 runtime ZIP and run:

```bash
python3 install.py "/path/to/Half-Life Alyx" "/path/to/depot_546564" --notproton
```

Use your own licensed game and Linux ARM64 depot. Native macOS Steam must be running. Select **Alyx Native - MoltenVR** in Steam compatibility settings, or run `start.command` from the game folder.

The installer reconstructs and verifies the shader recipe locally. The initial graphics preparation can take about a minute or longer. Subsequent launches still replay the list into a fresh process, but can reuse the system Metal compiler cache. New combinations encountered in other scenes can still compile during play. After changing game shader files, rerun the installer to rebuild preparation data.

Validation includes native level loading and normal exit, GPU draw/depth/cache restoration, controller and display-cadence regression tests, sanitizer checks and installer/cache-identity tests. These checks are not a claim of a fixed FPS increase or lower end-to-end streaming latency on every Mac.
