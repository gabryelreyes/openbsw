# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

# Test applicable for targets with a hardware tester, i.e. a [hw_tester_serial]
# section in the target's .toml file. The [adc] section names the channel the
# tester's DAC output is wired to:
#
#   [adc]
#   channel = 1              # argument of "adc get"
#   name = "AiEval_ADC"      # channel name printed by "adc get"
#   tolerance_mv = 100       # accepted difference between DAC and ADC in mV

import pytest


def test_adc_channel(target_session, hw_tester):
    adc = target_session.target_info.adc
    if not adc:
        pytest.skip("No [adc] section in the target's .toml file")
    channel = adc["channel"]
    name = adc["name"]
    tolerance_mv = adc.get("tolerance_mv", 100)

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    analog_ip_mv = 2700
    hw_tester.write(f"dac write {analog_ip_mv}\n".encode())
    (stm_write_ok, stm_out, _) = hw_tester.read_until(b"returned 0", timeout=1)
    assert stm_write_ok

    expected = [f"Adc Channel {channel} : {name}".encode(), b"ok"]
    capserial.clear()
    capserial.write(f"adc get {channel}\n".encode())
    (success, lines, _) = capserial.read_until(expected, timeout=3)
    assert success

    scaled_mv = [
        int(line.decode().split()[-3])
        for line in lines
        if "scaled" in line.decode()
    ]
    assert scaled_mv, "No scaled ADC value in the console output"
    assert abs(scaled_mv[0] - analog_ip_mv) <= tolerance_mv
