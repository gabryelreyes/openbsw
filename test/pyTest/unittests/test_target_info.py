# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import io
import unittest

from target_info import TargetInfo

MINIMAL_TOML = b"""
[freertos.target_process]
command_line = "sleep 1"
"""

FEATURES_TOML = (
    MINIMAL_TOML
    + b"""
[features]
storage = false
ethernet = true
"""
)


def load(toml_bytes):
    return TargetInfo("test_target", io.BytesIO(toml_bytes), cmd_line_no_restart=True)


class Test_TargetInfo_Features(unittest.TestCase):
    """Feature flags from the [features] table of a target .toml"""

    def test_feature_absent_table_defaults_to_true(self):
        info = load(MINIMAL_TOML)
        self.assertEqual(info.features, {})
        self.assertTrue(info.has_feature("storage"))

    def test_feature_absent_key_defaults_to_true(self):
        info = load(FEATURES_TOML)
        self.assertTrue(info.has_feature("middleware"))

    def test_feature_false_is_reported(self):
        info = load(FEATURES_TOML)
        self.assertFalse(info.has_feature("storage"))

    def test_feature_true_is_reported(self):
        info = load(FEATURES_TOML)
        self.assertTrue(info.has_feature("ethernet"))

    def test_features_is_a_copy(self):
        info = load(FEATURES_TOML)
        info.features["storage"] = True
        self.assertFalse(info.has_feature("storage"))


if __name__ == "__main__":
    unittest.main()
