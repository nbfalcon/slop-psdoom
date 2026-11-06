# psDooM Command-Line Flags Reference

This document provides a comprehensive reference of all command-line flags supported by the modernized **psDooM** engine.

---

## Process Management Flags (`psDooM` specific)

| Flag | Argument | Description | Default |
| :--- | :--- | :--- | :--- |
| `-psmax` | `<count>` | Caps the maximum number of process monsters spawned on custom management levels. Use `0` for unlimited. | `32` |
| `-psuser` | `[user1 ...]` | Limits process monitoring to specified usernames. If invoked without usernames, monitors the current `$USER`. | Current user (if not root) |
| `-psnotuser` | `[user1 ...]` | Excludes specified usernames from process monitoring. If invoked without usernames, excludes current `$USER`. | None |
| `-psallusers` | *(none)* | Monitors processes from all users across the system. | `root` only |
| `-nopsmon` | *(none)* | Disables the process scanner entirely; game behaves like standard Doom without spawning PID monsters. | Active |
| `-nopsact` | *(none)* | Disables process actions. Shooting or killing PID monsters will not issue actual `kill` or `renice` system calls. Useful for safe testing. | Active |
| `-nopssafety` | *(none)* | Disables protection on PID monsters, allowing them to take damage and die from infighting, slime, crushers, or barrels (instead of player weapon attacks only). | Protected |
| `-nopslev` | *(none)* | Prevents automatic loading of custom management levels (`psdoom1.wad` or `psdoom2.wad`). | Active |

---

## Security & Sandboxing Flags

| Flag | Argument | Description | Default |
| :--- | :--- | :--- | :--- |
| `-nosandbox` | *(none)* | Disables the Linux Landlock filesystem sandbox. By default, psDooM applies a zero-execution Landlock sandbox forbidding all `execve` / binary execution to protect the host against malicious WAD files. | Enabled |

---

## Video & Display Flags (SDL3)

| Flag | Argument | Description | Default |
| :--- | :--- | :--- | :--- |
| `-1` | *(none)* | Render at 1x resolution (320x200). | Disabled |
| `-3` | *(none)* | Render at 3x resolution (960x600). | Disabled |
| `-4` | *(none)* | Render at 4x resolution (1280x800). | Disabled |
| *(default)* | *(none)* | Borderless window defaulting to 2x resolution (640x400), resizable with integer aspect ratio preservation. | Enabled (2x borderless) |
| `-nograb` | *(none)* | Disables mouse grabbing / relative mouse capture mode. | Grabbed |
| `-notrans` | *(none)* | Disables translucency rendering effects. | Translucency enabled |
| `-noendtxt` | *(none)* | Suppresses rendering the ENDOOM text screen upon game exit. | ENDOOM enabled |

---

## Audio & Music Flags (SDL3)

| Flag | Argument | Description | Default |
| :--- | :--- | :--- | :--- |
| `-nosound` | *(none)* | Disables all audio playback (SFX). | Sound enabled |
| `-nomusic` | *(none)* | Disables music playback. | Music enabled |
| `-cdaudio` | *(none)* | Enables legacy CD audio driver mode if supported. | Disabled |

---

## Game Setup & WAD Flags

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `-iwad` | `<path>` | Specifies the IWAD to load (e.g. `-iwad freedoom1.wad` or via `$DOOMWADDIR`). |
| `-file` | `<path ...>` | Adds one or more PWAD files to load. |
| `-warp` | `[e m]` or `[map]` | Warps directly to episode/map (Doom 1: `-warp 1 1`, Doom 2: `-warp 01`). |
| `-skill` | `<1-5>` | Sets game difficulty (1: I'm too young to die, 5: Nightmare). |
| `-nomonsters` | *(none)* | Disables spawning of standard game monsters. |
| `-fast` | *(none)* | Enables fast monsters (like Nightmare difficulty). |
| `-respawn` | *(none)* | Enables monster respawning. |
| `-loadgame` | `<slot>` | Loads savegame directly from slot `0-5`. |
| `-config` | `<file>` | Specifies custom configuration file instead of `~/.psdoom/.doomrc`. |

---

## Demo Recording & Playback Flags

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `-record` | `<name>` | Records game session to `<name>.lmp`. |
| `-playdemo` | `<name>` | Plays demo `<name>.lmp` and exits. |
| `-timedemo` | `<name>` | Plays demo `<name>.lmp` at maximum possible speed and outputs frame rate benchmark statistics. |
| `-maxdemo` | `<kb>` | Sets demo recording buffer size in kilobytes (default 128 KB). |

---

## Multiplayer & Networking Flags

| Flag | Argument | Description |
| :--- | :--- | :--- |
| `-net` | `<p1> ...` | Configures multiplayer network nodes. |
| `-deathmatch` | *(none)* | Enables standard deathmatch mode. |
| `-altdeath` | *(none)* | Enables deathmatch 2.0 rules (weapons stay, items respawn). |
| `-altcoop` | *(none)* | Enables cooperative mode with deathmatch item respawns. |
| `-timer` | `<minutes>` | Sets level time limit in deathmatch. |
| `-avg` | *(none)* | Sets level time limit to 20 minutes (Austin Virtual Gaming rules). |
| `-dup` | `<1-9>` | Reduces network packet rate by combining tics. |
| `-extratic` | *(none)* | Sends duplicate tic commands in each network packet to mitigate packet loss. |
| `-port` | `<num>` | Specifies network UDP port. |
