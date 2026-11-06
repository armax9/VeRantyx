# Native Alyx for MoltenVR 0.3.2

Better Quest controller finger animation and corrected fog lighting.

- Read capacitive touches on triggers, thumbsticks, A/B, X/Y and thumbrests separately from button presses, for both hands.
- Fix false resting-hand curl caused by thumbstick drift and grip-based thumb-contact guesses.
- Keep up to eight ragdolls on Low instead of the portable profile’s limit of two. Other graphics settings are unchanged; Steam launch options can override the limit.
- Fix excessive fog illumination from omitted lighting and shadow-mask draws during asynchronous pipeline compilation.
- Keep fog enabled, preserve the selected graphics preset and 4x MSAA, and retain nonblocking compilation for ordinary scene materials. Service passes complete before first use so incomplete lighting data cannot persist.

The orange in-headset preparation display, native-resolution loading screens, startup without waiting for the headset, save/settings handling, gravity gloves and throw timing are retained.

## Install or update

Extract the macOS ARM64 runtime ZIP and run:

```bash
python3 install.py "/path/to/Half-Life Alyx" "/path/to/depot_546564" --notproton
```

Use your own licensed Alyx content and Linux ARM64 depot. Native macOS Steam must be running. Select **Alyx Native - MoltenVR** in Steam compatibility settings, or run `start.command` from the game folder. The installer backs up an existing managed installation and preserves settings, saves and compatible caches.

Steam launch options remain supported, for example `-novid +vr_fidelity_level_auto 0 +vr_fidelity_level 3`. Automatic fidelity is used when no override is specified.

The fixes were tested on the maintainer’s Mac with Quest 3 and MoltenVR. The fog correction was reproduced and compared on the same train save, then confirmed in the headset. No game assets, shader binaries or personal caches are included.
