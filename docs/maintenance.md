# psDooM Architecture & Modernization Maintenance Guide

This document records the design decisions, architectural modifications, bug fixes, modern Linux subsystem integrations, and porting changes made to modernize the legacy **psDooM** (process manager Doom) codebase.

---

## 1. Build System & Toolchain Modernization

### Transition to Pure CMake + Ninja + Clang
- **Removed Legacy Build Artifacts**: Deleted obsolete legacy imake/xmkmf/Makefile templates, ancient shell scripts (`Configure`, `makesrc`, `build.sh`), and platform conditional paths targeted at obsolete platforms (SCO, SunOS, Solaris, FreeBSD, OS/2).
- **Modern Build Configuration**:
  - `CMakeLists.txt` configured with C99 (`set(CMAKE_C_STANDARD 99)`).
  - Target build profile: `RelWithDebInfo` using Clang and Ninja.
  - Compile flags: `-Wall -Wextra -Wno-unused-parameter` with **zero warning-suppression flags** (`-Wno-error`, `-Wno-everything`, or broad `-Wno-*`).
  - Achieved **zero compiler warnings** across all 72 compilation units under Clang.

### Warning & Type Cleanups
- **Removed Obsolete `rcsid` Markers**: Stripped unused `#ident` and `static const char rcsid[]` variables from over 66 header and source files.
- **64-bit Pointer Cleanups**:
  - Replaced integer-to-pointer and pointer-to-integer casts with `<stdint.h>` `uintptr_t` / `intptr_t`.
  - Fixed format specifiers (`%p`, `%zu`, `%d`) across logging and debug code.
  - Resolved conflicting definitions and signature mismatches (`D_PostEvent`, `W_InitMultipleFiles`, `I_Error`).

---

## 2. Audio & Video Backend: SDL3 Integration

The original XDoom engine relied on raw X11/XShm and OSS/Linux sound drivers (`/dev/dsp`). These have been completely modernized to **SDL3**:

- **Video Subsystem (`i_video_sdl3.c`)**:
  - Initialized with `SDL_InitSubSystem(SDL_INIT_VIDEO)`.
  - Creates a resizable window with letterboxing (`SDL_SetRenderLogicalPresentation(..., SCREENWIDTH, SCREENHEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX)`).
  - Palette conversion transforms Doom's 8-bit paletted framebuffers (`screens[0]`) to 32-bit ARGB8888 streaming textures via `SDL_UpdateTexture()` and `SDL_RenderPresent()`.
  - Supports scaling parameters (`-1`, `-3`, `-4`, default 2x windowed).
- **Audio Subsystem (`i_sound_sdl3.c`)**:
  - Employs `SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, ...)` running at 11,025 Hz, 8-bit unsigned PCM stereo.
  - Mixes active sound channels directly into internal audio buffers on each tick, streaming them asynchronously through SDL3 without blocking the game loop or requiring legacy external sound daemon processes (`sndserver` / `musserver`).

---

## 3. Controls: WASD Support & Mouse Decoupling

- **WASD + Arrow Key Movement (`g_game.c:G_BuildTiccmd`)**:
  - Player forward and backward movement accepts `W` and `S` in addition to Up and Down arrow keys.
  - Player strafing left and right accepts `A` and `D` in addition to `,` and `.` (or configured strafe keys).
  - Both key schemes work seamlessly in parallel without conflicting with weapon hotkeys or automap controls.
- **Disabled Mouse Movement**:
  - In `i_video_sdl3.c`, suppressed mouse motion events from emitting player look/move delta events.
  - In `g_game.c`, stripped mouse turn/forward accumulators so that mouse motion cannot move or rotate the player.
  - In `m_misc.c`, set default `use_mouse = 0` and `mouseb_forward = -1` to avoid unintended mouse button forward motion. Mouse buttons remain bindable for weapon fire if configured.

---

## 4. Linux Landlock Security Sandbox

To protect the host system from potentially malicious custom WAD files containing malformed lumps, exploit payloads, or malicious scripts:

- **Zero Execution Policy (`i_landlock.c`, `i_landlock.h`)**:
  - Uses the Linux kernel Landlock LSM interface (Linux ABI v1+).
  - Calls `prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)` followed by `landlock_restrict_self()`.
  - Handles `LANDLOCK_ACCESS_FS_EXECUTE` and blocks all file execution throughout the filesystem. Any attempt to invoke `execve`, spawn shells, or launch processes fails with `EACCES` / `EPERM`.
- **Filesystem Access Rules**:
  - **Read-Only**: System libraries (`/usr`, `/lib`, `/lib64`), configurations (`/etc`), devices (`/dev`), kernel metrics (`/proc`, `/sys`), and WAD directories (`$DOOMWADDIR`).
  - **Read-Write**: Configuration and save data (`$HOME`, `~/.psdoom`), temporary directories (`/tmp`), and runtime sockets (`$XDG_RUNTIME_DIR`).
  - Sandbox initialization runs immediately after display and sound hardware initialization in `D_DoomLoop()`.
  - Can be bypassed for debugging with `-nosandbox`.

---

## 5. Direct `/proc` Scanning & Native System Calls

- **Direct `/proc` Scanning (`pr_process.c:pr_check`)**:
  - Replaced legacy `ps` pipes with direct readdir iteration over `/proc/[pid]/stat`.
  - Parses process name, daemon/TTY state, UID, and process start time without spawning external helper processes.
- **Native POSIX Process Actions (`pr_process.c:pr_kill`, `pr_process.c:pr_renice`)**:
  - **Eliminated Shell Commands**: Replaced `system("kill -9 %d")` and `system("renice +5 %d")` with direct POSIX `kill(pid, SIGKILL)` and `getpriority()` / `setpriority(PRIO_PROCESS, pid, prio + 5)`.
  - This eliminates shell dependency (`/bin/sh`) entirely, enabling the engine to function fully under the Landlock zero-execution sandbox.

---

## 6. Process Monster Density Control (`-psmax`)

- **Root Cause of High Density**:
  - Modern Linux systems routinely run hundreds of daemon threads, system services, and background processes. Spawning one Doom demon per process flooded map corridors, causing severe overcrowding and collisions.
- **Configurable Cap**:
  - Implemented the `-psmax <count>` flag (parsed in `d_main.c:942`).
  - **Default**: `32` process monsters (balancing gameplay density and level navigability).
  - Setting `-psmax 0` allows unlimited processes.
  - Processes are sorted by earliest `starttime` so primary system and user daemons appear consistently.

---

## 7. WAD Lump & Renderer Bug Fixes

- **Texture Collision Fix (`8f5da03`)**:
  - `psdoom1.wad` and `psdoom2.wad` contained broken duplicate `TEXTURE1`, `TEXTURE2`, and `PNAMES` lumps defining only 126 textures, eclipsing the base IWAD's texture dictionary and causing crash errors on missing textures (such as `MIDSPACE`).
  - Stripped redundant texture dictionaries from the PWADs to allow normal inheritance from the base game IWADs.
- **Tutti-Frutti Heightmask Segfault (`44f2f1b`)**:
  - In `r_draw.c:R_DrawColumn` and `r_draw.c:R_DrawTranslucentColumn`, power-of-two texture wrapping caused an underflow when `dc_texheight == 0`, leading to an out-of-bounds pointer read and segmentation fault.
  - Added guards checking `if (dc_texheight == 0) return;`.
- **ENDOOM Box-Art UTF-8 Escape Rendering (`a2578dc`)**:
  - Freedoom and vanilla WADs store DOS CP437 box-drawing glyphs in the `ENDOOM` lump.
  - Implemented full CP437-to-Unicode UTF-8 mapping and 16-color ANSI escapes in `endtxt.c`, replacing legacy terminal escape codes with clean modern terminal rendering.

---

## 8. Interactive Launcher (`scripts/run`)

- Pure Bash implementation without external dialog tools (`whiptail`, `dialog`, `fzf`).
- Interactive menu allowing players to choose between:
  1. Freedoom Phase 1 + psDooM 1 (E1M1 process map)
  2. Freedoom Phase 2 + psDooM 2 (MAP01 process map)
  3. Custom IWAD path
- Automatic sandbox runtime staging in `/tmp/psdoom_run_<UID>` and seamless argument pass-through.
