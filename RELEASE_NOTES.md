# Native Alyx for MoltenVR 0.3.1

Launch Alyx from macOS Steam, let it prepare, then put on your headset and play.

- Start the game and shader preparation without waiting for the headset, in a single game session.
- See shader-cache checking and preparation progress directly in VR, with an orange Alyx-style interface and the game’s own fonts.
- Restore loading rooms and chapter maps using assets from your installed game, rendered at MoltenVR’s recommended resolution.
- Join tracking after launch with a consistent starting height and position.
- Prevent duplicate native launches and conflicting shader-cache writes.
- Keep existing save handling, graphics settings, controller input, gravity gloves, throw timing and 4x MSAA.

Steam launch options remain supported, including `-novid +vr_fidelity_level_auto 0 +vr_fidelity_level 3`. Automatic fidelity is used when no override is specified.

## Install or update

Extract the macOS ARM64 runtime ZIP and run:

```bash
python3 install.py "/path/to/Half-Life Alyx" "/path/to/depot_546564" --notproton
```

Use your own licensed Alyx content and Linux ARM64 depot. Native macOS Steam must be running. Select **Alyx Native - MoltenVR** in Steam compatibility settings, or run `start.command` from the game folder. The installer backs up an existing managed installation and preserves settings, saves and compatible caches.

Shader preparation uses your installed game files. Later launches reuse compatible caches but still replay the pipeline list; previously unseen combinations can compile during play. No game assets or shader binaries are included.
