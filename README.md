# Bully NX

Nintendo Switch homebrew port/wrapper for the Android 64-bit release of **Bully: Anniversary Edition**.

This project does not reimplement the game. It loads the original Android native game library, patches the runtime, and runs it inside a minimal Switch-side compatibility layer.

This is still a **work in progress** port. It boots and runs, but it is not finished yet and still contains temporary workarounds that need proper cleanup.

## Credits

- `fgsfds` and `Andy Nguyen` for the original wrapper/loader base used by these ports
- `givethesourceplox` for the Bully AArch64 Switch port work in this repo

## Important

You must use **officially obtained** game files from your own copy of:

- `Bully: Anniversary Edition`
- Android version `v1.4.311`
- with `arm64-v8a` libraries

Do not distribute extracted APK contents with this project.

## Requirements

You need a Switch setup that can run homebrew and this port is intended to be launched through title override / full-memory homebrew, not applet mode.

## Installation

Create this folder on your SD card:

```txt
sdmc:/switch/bully/
```

From an officially obtained `Bully: Anniversary Edition` `v1.4.311` APK, extract:

- `assets/`
- `lib/arm64-v8a/libc++_shared.so`
- `lib/arm64-v8a/libGame.so`

Then place the files like this:

```txt
sdmc:/switch/bully/
- bully_nx.nro
- libc++_shared.so
- libBully.so
- config.txt
- assets/
  - data_0.zip
  - data_1.zip
  - data_2.zip
  - data_3.zip
  - data_4.zip
  - ...
```

Notes:

- `libGame.so` from the APK must be copied as `libBully.so`
- the port checks for `assets/data_0.zip` on startup
- `savegames/` is created automatically on first run
- `config.txt` is created automatically if missing

## Quick Install Steps

1. Copy the built `.nro` into `sdmc:/switch/bully/`
2. Copy `libc++_shared.so` into `sdmc:/switch/bully/`
3. Copy `libGame.so` into `sdmc:/switch/bully/` and rename it to `libBully.so`
4. Copy the full `assets` folder into `sdmc:/switch/bully/assets/`
5. Launch through title override / hbmenu with full memory access

## Configuration

The port reads:

```txt
sdmc:/switch/bully/config.txt
```

If it does not exist, the port creates one with default values on first boot.

Examples:

```txt
screen_width 960
screen_height 540
clarity 0
shadows 1
trilinear_filter 0
timing_workaround_ms 0
debug_log 0
```

`clarity` maps to the native in-game display effects level:

- `0` = Low
- `1` = Medium
- `2` = High

`shadows` controls the native startup shadow setting:

- `0` = Off
- `1` = Low
- `2` = Medium
- `3` = High

`timing_workaround_ms` controls the current compatibility timing workaround used by the port on some consoles.

- `0` = automatic mode
- `-1` = disabled (no added delay at all)
- `2`, `3`, `4`, `5` = fixed manual override

Automatic mode starts with no added delay and raises the workaround only if startup/render progress appears to stall on that console. If you still want to force a fixed value manually, try `2`, `3`, `4`, or `5`.

`debug_log` writes `debug.log` next to the `.nro` (flushed every line, so it survives crashes):

- `0` = off (default, fastest)
- `1` = on; use it to capture crashes and send the log with bug reports

Performance keys:

- `shader_cache 1` (default) keeps compiled shaders in `shader_cache/` next to the `.nro`, so scenes hitch less from the second run on; `0` disables it
- `cpu_boost 0` (default) / `1` raises the CPU to 1785 MHz while a loading screen reads files; FastLoad also drops the GPU clock, so frame rate can dip during loads
- `gl_thread 0` (default) / `1` runs Mesa's GL driver work on its own thread; faster when CPU-bound, still experimental with this engine
- `audio_quality` sets the OpenAL resampler: `0` = point (lightest), `1` = linear (default), `2` = cubic

Graphics keys:

- `phone_mode 1` (default) reports the console as a phone to keep heavy effects off; `0` reports a tablet, which unlocks the native high-end effects and character shadows (heavier)
- `anisotropy 4` (default) sets anisotropic texture filtering (`0`/`1` = off, up to `16`)
- `clarity 2` selects the native High display quality (sharper image, more GPU)

Old wrapper-only keys from earlier builds were removed from the generated config because they were not actually wired in this 64-bit port.

## Building

You need:

- `devkitA64`
- `libnx`
- `SDL2`
- `OpenAL`
- `minizip`
- `zlib`
- `zstd`
- Switch Mesa / Nouveau userspace libraries

Standard build:

```sh
docker run --rm -it \
  -v "$(PWD):/project" \
  --workdir "/project/bully_nx" \
  devkitpro/devkita64 \
  sh -lc 'make clean && make -j$(nproc)'
```

### Mesa 26 build

The stock `devkitpro/devkita64` image ships Mesa 20.1, which does not boot this port. `build-mesa26.sh` builds against Mesa 26.2.4 from `ghcr.io/autorunhq/switch-dev` and writes `bully_nx-mesa26.nro` (plus the matching `.elf` for symbolizing crash addresses):

```sh
./build-mesa26.sh
```

## Troubleshooting

If the port fails very early:

- make sure the APK is really `v1.4.311`
- make sure the libraries came from `lib/arm64-v8a/`
- make sure `libGame.so` was renamed to `libBully.so`
- make sure `assets/data_0.zip` exists in the game folder
- make sure you are not launching in applet mode

If the game reports missing symbols or immediately fails to load:

- verify you extracted the correct 64-bit Android files
- verify `libc++_shared.so` is present beside the `.nro`

## Current Status

Known issues / unfinished parts:

- this is still a work-in-progress port
- there are still some temporary hacks/workarounds in the runtime that should be replaced with cleaner fixes
- more cleanup and stabilization is still needed overall
- Xbox layout on button tips

## Legal

This project is not affiliated with Rockstar Games or Take-Two.

The port should be distributed **without** proprietary game binaries or assets. Users must provide their own legally obtained copy of the Android game files.

Unless stated otherwise, the source code in this repository is released under the MIT License. See [LICENSE](https://github.com/givethesourceplox/bully_nx/blob/master/LICENSE).
