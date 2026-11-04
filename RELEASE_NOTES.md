# Native Alyx for MoltenVR 0.2.0

Native runtime updates from VeRantyx main through November 4, 2026, adapted to the existing MoltenVR launch path.

- Persistent caching of rewritten shaders and improved background compilation/cache synchronization.
- Normal game shutdown, allowing settings and caches to finish saving.
- Darker shadows and midtones, plus the upstream shadow-flicker fix.
- Thread-synchronization and frame/pose handling fixes.
- Compatibility with newer Alyx submenus and native save-slot names.
- Existing MoltenVR controls, gravity-glove interaction, stereo correction and Steam launch integration retained.

## Install or update

Download and extract `verantyx-moltenvr-macos-arm64-v0.2.0.zip`, then run:

```bash
python3 install.py "/path/to/Half-Life Alyx" "/path/to/depot_546564" --notproton
```

The same command updates a previous managed installation while preserving settings and shader caches. Use your own game content and Linux ARM64 depot. Native macOS Steam and MoltenVR must be running when playing. Omit `--notproton` for direct launcher use.

Select **Alyx Native - MoltenVR** in Steam. Existing launch options remain supported:

```text
-novid +vr_fidelity_level_auto 0 +vr_fidelity_level 3
```

Tested by the maintainer on M3 Max / Quest 3. Installer-update, runtime, cache and shutdown checks pass. Previously unseen shaders can still need compilation on first use.

Based on [VeRantyx](https://github.com/Ag3497120/VeRantyx), including native refinements by [medved91](https://github.com/medved91). No game files or MoltenVR runtime are included.
