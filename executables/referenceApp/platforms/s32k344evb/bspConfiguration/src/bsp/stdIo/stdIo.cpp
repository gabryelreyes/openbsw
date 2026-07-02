/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "platform/estdint.h"

// TODO (Phase 2, doc/dev/s32k344_integration_plan.md): route the console
// through the LPUART driver, as done on the S32K148EVB.

extern "C" void putByteToStdout(uint8_t const byte) { (void)byte; }

extern "C" int32_t getByteFromStdin() { return -1; }
