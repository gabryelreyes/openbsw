# NXP S32K344 device headers

Peripheral access layer headers for the NXP S32K344 (version 1.9, 2021-10-27),
licensed under BSD-3-Clause (see the SPDX identifier in each file).

Origin: NXP S32K3 Real-Time Drivers (RTD) `BaseNXP/header` package, taken from
the NXP-maintained mirror at
<https://github.com/zephyrproject-rtos/hal_nxp>
(`s32/drivers/s32k3/BaseNXP/header`, commit
`9f81602140d779d5142bfd2ba41ebe7ba0f38d58`).

Local modifications:

- `S32K344_COMMON.h`: `#include "BasicTypes.h"` replaced with
  `#include <stdint.h>` to decouple the headers from the RTD type system
  (same modification as in `platforms/s32k1xx/bsp/bspMcu`).
