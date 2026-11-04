# Native ARM64 Alyx runtime

This directory combines VeRantyx's native runtime updates through upstream main commit `421b682` (November 4, 2026) with the MoltenVR integration described in the repository README.

It loads the user's Linux ARM64 Alyx libraries through a patched Klepton runtime, supplies Linux host APIs and OpenVR interfaces, and publishes stereo frames and tracking data through the existing native MoltenVR bridge.

The MoltenVR-specific changes retain cropped projection metadata, handle stale viewports after fidelity changes, preserve graphics-preset storage, map the left stick click to crouch, and follow bridge tracking cadence. The main-branch runtime supplies persistent rewritten-shader archives, thread-synchronization fixes, cooperative quit, display-colour matching, shadow compatibility and safer frame/pose handling.

Use `MoltenVRBridge/build-release.sh` from the repository root to build the runtime. See `host/Makefile` and `Makefile` for host and synthetic runtime checks. Tests use synthetic input, resources and shaders; GPU cache tests additionally use a local MoltenVK library. No proprietary game files are included.
