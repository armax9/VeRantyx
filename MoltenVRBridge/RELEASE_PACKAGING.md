# Runtime packaging

Version 0.2.0 integrates VeRantyx main through `421b682218dc8ff27ea3c73e9deb6567a00aec78`, retaining the fork's MoltenVR projection and input integration.

The archive contains the ARM64 host, native OpenXR bridge, two warp SPIR-V shaders, two MoltenVK builds, license notices, installer helpers, launcher and optional Steam integration. The installed host was validated by the maintainer before release; the portable host build changes the fallback MoltenVK path, strips debug symbols and uses ad-hoc signing.

The bridge and bundled MoltenVK libraries retain the working installation's versions. Building with another MoltenVK version requires separate gameplay validation.

Do not include `guest-lib`, game assets, native-game, settings, saves, logs, shader caches, shared frame mappings, backups or credentials. UI resources are patched locally by `prepare_native_ui.py` using the owner's VPK; no compiled game UI resources are redistributed. Debian libraries are downloaded from their official mirror by the installer.

The complete pinned Klepton baseline patch is in `klepton/`. The build script applies it and then the current NativeARM Klepton refinements. Preserve all third-party notices.
