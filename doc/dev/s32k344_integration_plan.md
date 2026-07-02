# S32K344 Integration Plan

This document details the phased integration of the NXP **S32K344** microcontroller
(Arm Cortex-M7, S32K3 family) into Eclipse OpenBSW.

## 1. Overview and Scope

### Goal

The OpenBSW `referenceApp` runs on the NXP **S32K3X4EVB-T172** evaluation board
(S32K344, 172-pin HDQFP) with:

- Boot from flash, clock configuration, and stable operation on FreeRTOS
- Interactive UART console (same command set as on the S32K148EVB)
- CAN communication and a working UDS stack (docan transport)
- Unit tests, on-target pytest suites, CI, and documentation for the new platform

### Out of scope (planned as future extensions, see section 5)

- Ethernet (the S32K344 uses a GMAC controller, a different IP than the
  S32K148's ENET)
- ThreadX support
- Rust support and the Bazel build for the new platform
- Safety hardening (SWT watchdog, Cortex-M7 MPU, FCCU/ERM fault handling,
  lockstep monitoring)
- CAN FD (classic CAN first; the S32K344 FlexCAN supports FD as a later step)
- Full IO/ADC/PWM support (eMIOS) and persistent storage (C40 flash EEPROM
  emulation)

### Assumptions

- Debug and flash tooling: PE Micro `pegdbserver` (already used for the
  S32K148EVB, see `tools/gdb/pegdbserver.gdb`), with SEGGER J-Link or the NXP
  S32 Debug Probe as documented fallbacks.
- The device is in its delivery lifecycle; the HSE_B security subsystem and
  UTEST flash area are never written or reconfigured.
- Toolchain: existing `arm-none-eabi-gcc` from `cmake/toolchains/`; GCC first,
  clang preset as a future extension.

## 2. Architectural Decisions

### D1 — New `platforms/s32k3xx/` platform directory

The S32K3 family shares almost no peripheral IP with the S32K1 family:

| Function      | S32K148 (s32k1xx)   | S32K344 (s32k3xx)      |
| ------------- | ------------------- | ---------------------- |
| Core          | Cortex-M4F, 80 MHz  | Cortex-M7, 160 MHz, I/D-cache, TCM |
| GPIO / pinmux | PORT + GPIO         | SIUL2                  |
| Clocking      | SCG + PCC           | MC_ME + MC_CGM + MC_RGM + PLL |
| Flash         | FTFC                | C40                    |
| Timers / PWM  | FTM                 | eMIOS, PIT, STM        |
| CAN           | FlexCAN             | FlexCAN (similar IP, more instances, FD-capable) |
| UART          | LPUART              | LPUART (similar IP)    |
| Ethernet      | ENET                | GMAC                   |
| RAM           | plain SRAM          | ECC SRAM + DTCM/ITCM (must be initialized before use) |
| Boot          | Flash config field  | BAF (Boot Assist Firmware) + IVT |

Extending `platforms/s32k1xx` would create false sharing; only FlexCAN and
LPUART are similar enough to port rather than rewrite. A new
`platforms/s32k3xx/` directory therefore mirrors the *module layout* of
`platforms/s32k1xx` (developers can navigate by analogy) while adopting the
per-chip dispatch pattern introduced by `platforms/stm32`:
`platforms/s32k3xx/CMakeLists.txt` switches on an `S32K3_CHIP` variable and
includes `platforms/s32k3xx/cmake/s32k344.cmake`. This keeps the door open for
other S32K3 derivatives (S32K312, S32K358, ...) later.

### D2 — Parameterize the ARM toolchain instead of forking it

`cmake/toolchains/ArmNoneEabi.cmake` currently hardcodes
`-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16` (lines 19–24 and 76) and
its idempotency guards match the literal string `-mcpu=cortex-m4`
(lines 80 and 92). Introduce cache variables with backward-compatible
defaults:

```cmake
set(OPENBSW_ARM_CPU "cortex-m4"   CACHE STRING "ARM -mcpu value")
set(OPENBSW_ARM_FPU "fpv4-sp-d16" CACHE STRING "ARM -mfpu value")
```

used in `_ARCH_FLAGS`, `_ASM_FLAGS`, and both guard checks
(`string(FIND ... "-mcpu=${OPENBSW_ARM_CPU}")`). The S32K344 presets in
`CMakePresets.json` pass `-DOPENBSW_ARM_CPU=cortex-m7 -DOPENBSW_ARM_FPU=fpv5-d16`.
When the variables are unset, behavior is bit-identical to today — this is the
regression-safety property the CI gate in Phase 1 verifies. The change is
shared automatically by `ArmNoneEabi-gcc.cmake` and `ArmNoneEabi-clang.cmake`.

### D3 — Vendor NXP device headers, s32k1xx-style

Vendor the NXP SVD-derived S32K344 device headers into
`platforms/s32k3xx/bsp/bspMcu/include/3rdparty/nxp/` (per-peripheral
`S32K344_*.h` files plus the SVD), with an `include/mcu/mcu.h` aggregator —
exactly mirroring `platforms/s32k1xx/bsp/bspMcu/include/3rdparty/nxp/`.
Source: the NXP S32K3 header package (BSD-3-Clause). Replicate the license
notice pattern used for the S32K148 headers and flag the addition for Eclipse
IP review. NXP RTD driver *sources* are used as reference reading only, never
vendored — OpenBSW drivers are hand-written against the register headers.

Add `core_cm7.h` to `libs/3rdparty/cmsis/` from the same ARM CMSIS origin
(Apache-2.0) as the existing `core_cm4.h`; `bspMcu` already adds the CMSIS
include directories.

### D4 — Board directory owns all boot complexity

`executables/referenceApp/platforms/s32k344evb/` is a new sibling of
`s32k148evb/` with the same structure (`Options.cmake`, `main/`,
`bspConfiguration/`, `freeRtosCoreConfiguration/`). Everything specific to the
S32K3 boot flow lives here: the IVT boot header, the ECC RAM initialization,
the vector table, and the linker script. `Options.cmake` sets
`OPENBSW_PLATFORM s32k3xx` and starts with all `PLATFORM_SUPPORT_*` features
`OFF`; features are switched on phase by phase.

### D5 — FreeRTOS Cortex-M7 port

New `platforms/s32k3xx/3rdparty/freertos_cm7_sysTick/`, derived from the
upstream FreeRTOS `GCC/ARM_CM7/r0p1` port and packaged like
`platforms/s32k1xx/3rdparty/freertos_cm4_sysTick/` (LICENSE.md, README.md, a
`ManualChanges.diff` documenting any deltas from upstream). The shared kernel
in `libs/3rdparty/freeRtos/` and the shared configuration in
`libs/bsw/asyncFreeRtos/` are reused unchanged; board tuning goes into
`executables/referenceApp/platforms/s32k344evb/freeRtosCoreConfiguration/include/os/FreeRtosPlatformConfig.h`
(`configCPU_CLOCK_HZ 160000000`, heap size, priorities).

## 3. Integration Phases

### Phase 1 — Build-system enablement and skeleton (no hardware required)

> **Status: implemented (2026-07).** The `s32k344-freertos-gcc` preset builds
> a linking ELF (v7E-M, FPv5-D16) with zero regression to the existing
> presets. Beyond the original scope, the skeleton already contains a working
> CM7 startup with ECC RAM initialization, the FreeRTOS CM7 port, and a
> UART placeholder driver, so the ELF is debugger-loadable for Phase 2.

**Goal:** `cmake --preset s32k344-freertos-gcc` configures and cross-compiles a
skeleton `referenceApp` with `-mcpu=cortex-m7`, with zero regression to the
S32K148 build.

**Tasks**

- Parameterize `cmake/toolchains/ArmNoneEabi.cmake` per D2.
- Root `CMakeLists.txt`: extend the `BUILD_TARGET_PLATFORM` branch at
  lines 120–125 so `S32K344EVB` maps `OPENBSW_PLATFORM_DIR` to
  `platforms/s32k3xx`; extend the Rust branch at line 92 to
  `S32K148EVB OR S32K344EVB` (both use the `thumbv7em-none-eabihf` triple;
  actually building Rust is a future extension).
- Create `platforms/s32k3xx/` skeleton: `CMakeLists.txt` with `S32K3_CHIP`
  dispatch (template: `platforms/stm32/CMakeLists.txt`),
  `cmake/s32k344.cmake`, and `bsp/bspMcu/` with the vendored headers and
  `mcu.h` (D3).
- Add `core_cm7.h` to `libs/3rdparty/cmsis/`.
- Create `executables/referenceApp/platforms/s32k344evb/` skeleton:
  `Options.cmake` (all `PLATFORM_SUPPORT_*` OFF), `CMakeLists.txt`, empty
  `main/` scaffold (template: `s32k148evb/`).
- Add the `s32k344-freertos-gcc` configure/build presets to
  `CMakePresets.json`.

**Deliverables:** a cross-compiling skeleton ELF (not yet expected to run);
vendored headers with license notes.

**Verification**

- The new preset compiles; `arm-none-eabi-readelf -A` on the ELF shows
  `Tag_CPU_arch: v7E-M` with an FPv5 FPU.
- All pre-existing presets (`s32k148-*`, `posix-*`, `tests-*`) still build
  green in CI — ideally with an unchanged `application.map` for the S32K148 as
  proof of D2's no-regression property.

**Dependencies:** none.

### Phase 2 — Minimal bring-up: boot, ECC RAM, clock, UART console, FreeRTOS

**Goal:** the board boots from flash through BAF, runs FreeRTOS, and serves the
interactive OpenBSW console over LPUART. This is deliberately the single
highest-risk phase; everything after it is incremental.

**Tasks**

- *First task, before any driver work:* validate flash/debug tooling — confirm
  the available PE Micro `pegdbserver` version supports the S32K344's C40
  flash; adapt `tools/gdb/` configs; fall back to J-Link if needed. Document
  the working procedure.
- `main/src/bsp/bootHeader.S`: S32K3 IVT (boot marker, boot-core
  configuration, application start pointer) placed by the linker at the
  program-flash base — replaces the S32K148 flash-config-field
  `bootHeader.S` concept.
- `main/src/bsp/startUp.S`: vector table for the S32K344 IRQ map (generate
  from the device headers rather than hand-typing; assert the vector count
  against the header's interrupt count at compile time), ECC SRAM/DTCM
  initialization (64-bit aligned writes over the full RAM map, registers only
  — no stack before RAM is initialized), FPU and I/D-cache enable, then C
  runtime init.
- `main/linkerscript/application.dld.in`: S32K344 memory map (program flash at
  `0x0040_0000`, SRAM, DTCM/ITCM, IVT section), keeping the existing
  C-preprocessor `.dld.in` convention and CMake generation step from
  `s32k148evb/main/CMakeLists.txt`.
- `platforms/s32k3xx/bsp/bspClock`: MC_ME mode transitions, MC_CGM muxes and
  dividers, PLL to 160 MHz; MC_RGM reset-cause reporting.
- `platforms/s32k3xx/bsp/bspInterruptsImpl`: CMSIS CM7 NVIC wrapper (mirror
  `platforms/s32k1xx/bsp/bspInterruptsImpl`).
- `platforms/s32k3xx/hardFaultHandler`: port and extend for Cortex-M7 fault
  registers.
- Minimal SIUL2 pinmux support (enough for UART now, CAN in Phase 3) in
  `platforms/s32k3xx/bsp/bspIo`, plus board pin tables in
  `bspConfiguration/`.
- `platforms/s32k3xx/bsp/bspUart` (LPUART — port from
  `platforms/s32k1xx/bsp/bspUart`, similar IP) and console wiring
  (`bspConfiguration` stdIo).
- FreeRTOS CM7 port (D5) and `freeRtosCoreConfiguration/`.
- Board `main/src/main.cpp`, lifecycle wiring (`StaticBsp`), `os/` hooks, and
  a minimal `systems/BspSystem`.
- `test/pyTest/target_s32k344.toml` (modeled on `target_s32k148.toml`) with
  serial port, flash/reset gdb scripts, and boot-log markers.

**Deliverables:** a bootable image and a documented flash/debug procedure.

**Verification**

- Power-cycle boot through BAF (not only debugger load) reaches the console
  prompt; `help` and lifecycle commands work; OS tick statistics advance.
- The console pytest suite (`test/pyTest/console`) passes against
  `target_s32k344.toml`.
- ECC coverage check: confirm the init loop covers the entire RAM map
  (deliberate boundary read test).

**Dependencies:** Phase 1.

### Phase 3 — CAN and UDS

**Goal:** `PLATFORM_SUPPORT_CAN`, `PLATFORM_SUPPORT_TRANSPORT`, and
`PLATFORM_SUPPORT_UDS` are ON; the docan/UDS stack is operational on the
board's CAN interface.

**Tasks**

- `platforms/s32k3xx/bsp/bspFlexCan` and a transceiver class implementing
  `ICanTransceiver` — ported from `platforms/s32k1xx/bsp/bspFlexCan` and
  `canflex2Transceiver` (similar FlexCAN IP; differences: instance count,
  clocking via MC_CGM, larger message-buffer RAM). Classic CAN first; CAN FD
  deferred.
- CAN pinmux and on-board CAN PHY enable for the S32K3X4EVB-T172 in
  `bspConfiguration/`.
- Board `main/src/systems/CanSystem` (template:
  `s32k148evb/main/src/systems/CanSystem.cpp`) and the CAN ISRs in
  `main/src/os/isr/`.
- Note for later phases: FlexCAN message buffers live in module RAM (device
  memory, no D-cache coherency issue), but any future DMA descriptor in SRAM
  needs a non-cacheable MPU region — reserve a place for it in the linker
  script now so Ethernet/eDMA work does not require a memory-map change.

**Deliverables:** CAN transmit/receive and UDS diagnostics working on target.

**Verification**

- FlexCAN loopback self-test (pre-hardware gate, runs without a bus).
- `test/pyTest/can` suite via a USB-CAN adapter against
  `target_s32k344.toml`.
- UDS smoke test over docan: session control and a read-DID request.

**Dependencies:** Phase 2.

### Phase 4 — Consolidation: tests, CI, documentation

**Goal:** the S32K344 is a first-class platform: unit-tested, CI-gated, and
documented.

**Tasks**

- Unit tests: add an `elseif (OPENBSW_PLATFORM STREQUAL "s32k3xx")` branch to
  the root `CMakeLists.txt` test section (lines 190–208 pattern), per-module
  `test/` directories for the pure-logic parts of the new BSP drivers
  (bspUart, bspFlexCan/transceiver, bspIo — mirroring the s32k1xx test set),
  and `tests-s32k3xx-debug/release` presets.
- CI: add the `s32k344-freertos-gcc` preset to
  `.github/workflows/build.yml` and `s32k3xx` to the `clang-tidy.yml`
  platform matrix.
- Documentation: `doc/dev/modules/s32k3xx.rst` (toctree over the new BSP
  module docs, following `stm32.rst`),
  `doc/dev/platforms/s32k344evb/index.rst` (board overview, pin tables,
  flash/debug options: PE Micro, J-Link, S32 Debug Probe — following
  `s32k148evb/index.rst`), setup guide additions under
  `doc/dev/learning/setup/`, and entries in `doc/dev/index.rst`.
- Finalize `test/pyTest/target_s32k344.toml`; document a hardware-tester
  variant if applicable (compare `target_s32k148_with_hwtester.toml`).

**Deliverables:** green CI including the new platform; rendered documentation.

**Verification**

- Full CI matrix green (all old and new presets).
- Sphinx documentation build passes (`sphinx-doc-build.yml`).
- Complete on-target pytest run (console + can + uds) recorded as acceptance
  evidence.

**Dependencies:** Phases 2–3 (documentation and CI items should also land
incrementally inside each phase; this phase closes the remaining gaps).

## 4. Sequencing Summary

```
Phase 1 (build skeleton, no HW)
   └─► Phase 2 (boot + clock + console + FreeRTOS)   ◄── minimal demonstrable milestone
          └─► Phase 3 (CAN + UDS)
                 └─► Phase 4 (tests, CI, docs)        ◄── "core comms" done
```

The minimal demonstrable milestone is the end of Phase 2: the board boots from
flash and serves the console on FreeRTOS. "Done" for this integration is the
end of Phase 4.

## 5. Future Extensions (out of scope)

- **IO/ADC/PWM:** full SIUL2 GPIO driver behind the `libs/bsp` input/output
  manager interfaces, S32K3 ADC, and eMIOS-based PWM replacing the FTM-based
  `bspFtm`/`bspFtmPwm` role.
- **Persistent storage:** C40 flash driver plus an EEPROM-emulation layer
  behind `IEepromDriver` (`libs/bsw/bsp/include/bsp/eeprom/IEepromDriver.h`) —
  the S32K3 has no FTFC/FlexRAM EEPROM.
- **Ethernet:** GMAC driver (descriptor-DMA style, entirely different from the
  S32K148 ENET) with a non-cacheable DMA region, board PHY driver, reuse of
  `lwipSysArch` glue, DoIP.
- **Safety:** SWT watchdog behind the existing watchdog interfaces, Cortex-M7
  core MPU in `safeMemory`, ERM/FCCU ECC and fault reaction, lockstep fault
  reporting (the S32K344 runs lockstep in hardware by default).
- **ThreadX:** Cortex-M7 port under `platforms/s32k3xx/3rdparty/threadx/` plus
  a `threadXCoreConfiguration/` in the board directory and
  `s32k344-threadx-gcc` preset.
- **Rust:** `s32k344-rust-gcc` preset — the target triple
  (`thumbv7em-none-eabihf`) and the root `CMakeLists.txt` branch are already
  in place after Phase 1.
- **Bazel:** `soc:s32k344` constraint in `bazel/platform/`, a cortex-m7
  parameterization of the hardcoded flags in
  `bazel/toolchain/arm_none_eabi/gcc/`, `--config=s32k344` in `.bazelrc`, and
  `BUILD.bazel` files for all new modules.
- **Clang preset** (`s32k344-freertos-clang`) — the toolchain parameterization
  from Phase 1 already covers it.
- **CAN FD** on the FlexCAN instances that support it.

## 6. Risks and Mitigations

| # | Risk | Impact | Mitigation |
| - | ---- | ------ | ---------- |
| R1 | **ECC RAM init**: any SRAM/DTCM read before ECC initialization hard-faults; a stack in uninitialized RAM fails pre-`main` and is hard to debug | Bring-up blocker, opaque failures | Init in `startUp.S` using registers only (no stack); cover the entire RAM map; provide a debugger script that pre-inits RAM for JTAG-load workflows |
| R2 | **IVT/BAF boot**: a wrong IVT means BAF never jumps to the application — symptoms look like a dead board | Bring-up blocker | Bring up via debugger-load first (bypassing BAF), add the IVT afterwards; keep a known-good NXP example IVT as reference; the Phase 2 gate explicitly requires power-cycle boot |
| R3 | **C40 flash tooling**: PE Micro support for S32K344 flash algorithms must be confirmed; some S32K3 flashing paths need utility images | Blocks all on-target work | Validate tooling as the *first* task of Phase 2, before driver work; J-Link fallback documented |
| R4 | **HSE_B / UTEST bricking**: writing UTEST/IVT security fields or advancing the device lifecycle can permanently lock debug access | Bricked boards | Never address UTEST/HSE ranges in drivers or flash scripts; code-review gate on anything touching those address ranges |
| R5 | **Toolchain regression on S32K148**: D2 touches files shared by all ARM builds | Breaks the existing platform | Default-preserving cache variables; Phase 1 CI gate requires all existing presets green (map-file comparison for the S32K148 build) |
| R6 | **New-IP driver effort underestimated**: SIUL2, MC_ME/MC_CGM, and C40 are new implementations, not ports | Schedule risk | Phase ordering puts similar-IP wins first (LPUART, FlexCAN); NXP RTD sources used as reference reading only |
| R7 | **Vector table / IRQ map errors**: the S32K344 has a much larger IRQ map than the S32K148; hand-typed tables drift | Subtle runtime faults | Generate the table from the device headers; compile-time assert of the vector count against the header interrupt count |
| R8 | **Cache coherency with DMA** (future Ethernet/eDMA) | Data corruption, heisenbugs | FlexCAN is unaffected (module RAM); reserve a non-cacheable MPU region in the linker script during Phase 3 so later DMA users have a defined home |

## 7. Reference Points in the Repository

| Concern | Template / hook point |
| ------- | --------------------- |
| Per-chip platform dispatch | `platforms/stm32/CMakeLists.txt`, `platforms/stm32/cmake/<chip>.cmake` |
| Full-featured MCU platform layout | `platforms/s32k1xx/` (bsp modules, 3rdparty RTOS port, hardFaultHandler) |
| Complete board directory | `executables/referenceApp/platforms/s32k148evb/` |
| Platform selection | root `CMakeLists.txt` lines 92, 120–125, 190–208; `executables/referenceApp/platforms/<board>/Options.cmake` |
| ARM toolchain flags | `cmake/toolchains/ArmNoneEabi.cmake` (shared by gcc and clang variants) |
| Build presets | `CMakePresets.json`, `cmake/presets/config-base.json` |
| CAN interface contract | `libs/bsw/cpp2can/include/can/transceiver/ICanTransceiver.h` |
| BSP interface contracts | `libs/bsw/bsp/include/bsp/` |
| On-target tests | `test/pyTest/` + `target_s32k148.toml`, `flash.gdb`, `reset.gdb` |
| Debugger configs | `tools/gdb/pegdbserver.gdb`, `tools/vscode/dot_vscode/launch.json` |
| CI | `.github/workflows/build.yml`, `clang-tidy.yml` |
