# T5L + SDCC Starter Template

> Open-source firmware template for DWIN T5L/T5L51 panels using SDCC — clean, portable, and Keil-free.

A production‑ready starter for building firmware on DWIN T5L/T5L51 controllers using the open‑source SDCC toolchain. It includes a clean project layout, startup code, portable libraries (UART, CRC, timer, RTC, SYS), and a reproducible Makefile that auto‑detects SDCC include/lib paths and prints a concise memory‑usage report after each build.

> Why this exists: DWIN officially recommends Keil C (uVision5) for firmware development, which is paid and runs only on Windows. This project provides a fully open-source alternative using SDCC that works on all major operating systems (macOS, Linux, and Windows). It also preserves full compatibility with the UART functions provided by DWIN’s original SDK, enabling developers to build and test firmware without Keil or any proprietary toolchain.

---

## 🎥 Video Tutorial

A complete video walkthrough of the setup and build process for this template is available on YouTube:

👉 [Watch on YouTube](https://www.youtube.com/watch?v=ZnZoBKaqA0A)

This video demonstrates SDCC and Make installation on macOS and Windows, project structure overview, and how to compile your code using `make`.

---

## Features
- Structured C project with clear separation of **startup**, **libs**, and **app** layers
- SDCC **large model** configuration with XRAM placed at `0x8000`
- Startup for T5L with ISR support
- Portable libraries: `uart`, `sys`, `crc16`, `timer`,
- `Makefile` that:
  - auto‑detects SDCC mcs51 include and large‑model lib folders
  - builds `.rel` objects and links with map output
  - converts IHX → HEX → BIN (`T5L51.bin`)
  - prints a deterministic **memory usage** summary (CODE, XDATA, DATA, IDATA, BIT)
- Example application scaffold under `src/app`
- Provides a fully open DGUS core layer with RAM/NOR access, page control, and graph update routines — equivalent to vendor SDK, written from scratch.

---

## Repository Layout
```
.
├─ include/                # Shared headers (project‑wide)
├─ lib/
│  ├─ uart/                # UART driver (multi‑port ready)
│  ├─ sys/                 # System init, clock, low‑level utils
│  ├─ crc16/               # CRC utilities
│  ├─ timer/               # Timer helpers
│  └─ rtc/                 # RTC helpers
├─ src/
│  ├─ app/
│  │  ├─ app_defs/         # App‑specific definitions
│  │  └─ functions/        # App logic utilities
│  └─ main.c               # Entry point
├─ startup/
│  └─ startup_T5L.s        # Reset vector, ISRs, segments
├─ artifacts/            # Prebuilt binaries for quick flashing
│  └─ v0.1.1/            # Versioned folder (e.g., 9600/115200, CRC on/off)
├─ build/
│  ├─ obj/                 # Compiled objects (.rel)
│  └─ dist/                # Final artifacts (.ihx/.hex/.bin/.map)
├─ Makefile                # Build rules, memory summary
└─ ReadMe.md               # This file
```

---

## Prerequisites
- **SDCC** 4.5.0+ (mcs51 target)
- **GNU make**
- **sdas8051** (assembler packaged with SDCC)
- `packihx`, `makebin` (bundled with SDCC on most platforms)

### macOS (Homebrew)
```bash
brew install sdcc make
```
> The Makefile auto‑detects SDCC include/lib paths. If Homebrew layout changes, it also falls back to a standard path.

### Windows
#### Install SDCC
1. Install **SDCC** → https://sourceforge.net/projects/sdcc/

#### Install MSYS2
1. Install **MSYS2** → https://www.msys2.org/
2. Open *MSYS2 MinGW 64‑bit* shell and install:
   ```bash
    pacman -S make
   ```

3. Ensure `C:\msys64\usr\bin` and `C:\msys64\mingw64\bin` are in PATH.

### Linux (Debian/Ubuntu)
```bash
sudo apt-get update
sudo apt-get install sdcc make
```

---

## Building
### One‑line build
```bash
make
```
Artifacts go to `build/dist/`:
- `output.ihx`  (linked image with map)
- `output.hex`  (Intel HEX)
- `T5L51.bin`   (final binary)
- `output.map`  (for memory analysis)

### Clean
```bash
make clean
```

### Manual (reference)
```bash
sdcc -mmcs51 --model-large --xram-loc 0x8000 --xram-size 0x8000 \
-Isrc -Isrc/app -Isrc/app/app_defs -Iinclude -Istartup -Ilib/uart -Ilib/sys \
-Ilib/crc16 -Ilib/timer -Ilib/rtc -Isrc/app/functions \
-c src/main.c -o build/obj/main.rel
sdas8051 -plos build/obj/startup_T5L.rel startup/startup_T5L.s
# ... compile other libs ...
cd build/dist && sdcc -mmcs51 --model-large --xram-loc 0x8000 --xram-size 0x8000 -Wl-m -o output.ihx ../obj/*.rel
cd ../..
packihx build/dist/output.ihx > build/dist/output.hex
makebin -p build/dist/output.hex build/dist/T5L51.bin
```

---

## Configuration
Compiler and linker flags are centralized in the **Makefile**:
- Model: `--model-large`
- XRAM base/size: `--xram-loc 0x8000 --xram-size 0x8000`
- Include paths: `-Isrc -Iinclude -Istartup -Ilib/...`

Adjust these if your hardware variant changes the memory map.

---

## Flashing

DWIN panels accept two common flows:

1) **SD card (recommended, OS‑agnostic)**  
   Copy `T5L51.bin` to an SD card under `DWIN_SET/` (FAT32). Insert the card and power‑cycle the panel; the loader picks it up automatically.

2) **Serial download (Windows only)**  
   Use **DownLoadFor8051‑V1.4** to program via **Serial1** of the MCU. Not every model exposes Serial1 on a header; many reserve it for the on‑board Wi‑Fi footprint. If your model has that Wi‑Fi footprint, you can wire a USB‑UART there and program with DownLoadFor8051.

   *Important wiring note:* on some boards the Wi‑Fi pads are labeled from the MCU’s point of view. That means you should connect **TX→TX** and **RX→RX** on those pads (counter‑intuitive), plus common **GND**. Use **3.3V TTL** levels (not RS‑232). Always confirm your model’s schematic/manual.

This template only produces the **BIN**; choose either method above based on your hardware access.

---

## Debugging
- Use a serial console to validate `uart` output and ISRs.
- If you rely on `printf`-style logs, ensure stack/heap are sized appropriately in large model.
- Review `build/dist/output.map` to verify segment placement.

---

## Memory Usage Report
After `make`, a summary like below is printed from the linker map:
```
📊 Memory usage (build/dist/output.map):
  CODE      22299 / 65536 (34.0%)
  XDATA      5841 / 32768 (17.8%)
  DATA        123 / 256   (48.0%)
  IDATA         0 / 256   (0.0%)
  BIT           2 / 256   (0.8%)
```
If DATA+IDATA exceeds 256 bytes or CODE > 64KB, the Makefile emits warnings.

---
