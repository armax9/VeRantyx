# Release allowlist

The source branch is based on upstream native commit `12e7826e489a3f7c8526e623e4ca3c2c6d6bb62a`.

The ARM64 runtime archive contains only `alyx_host`, `bridge`, two warp SPIR-V files, two MoltenVK builds, license notices, the installer, launcher, optional Steam integration and Debian dependency downloader. The existing tested runtime binaries are used for 0.1.0; a complete Klepton source delta against the pinned commit is provided in `klepton/`. New builds may use a different MoltenVK version and need gameplay validation.

Never copy the installed `guest-lib` directory: it mixes Valve game libraries with Debian dependencies. Never include native-game, Steam files, cfg, vrpreferences, logs, pipeline cache, shared frame mappings, backups or local user paths. The installer builds library links and a merged game root locally from the user's own installation and ARM64 depot. Debian dependencies are fetched from their official distribution instead of redistributed.

VeRantyx and Klepton are MIT; the OpenVR SDK is BSD-3-Clause; OpenXR SDK is Apache-2.0 OR MIT; MoltenVK is Apache-2.0; the ALVR skeleton data is MIT. Preserve their notices. Valve game files and MoltenVR runtime binaries are not distributed. Licensed OpenVR headers inherited from upstream remain in the source fork, not proprietary Valve binaries.

0.1.0 is unsigned by a Developer ID; binaries use local ad-hoc signing. Installation and argument forwarding are tested with fixture directories. Full fresh-machine installation and macOS Steam Play have not been verified.
