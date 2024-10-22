# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Unit tests for json_results_generator.py."""

import unittest
import optparse
import random
import shutil
import tempfile

from webkitpy.layout_tests.layout_package import json_results_generator
from webkitpy.layout_tests.layout_package import test_expectations


class JSONGeneratorTest(unittest.TestCase):
    def setUp(self):
        self.builder_name = 'DUMMY_BUILDER_NAME'
        self.build_name = 'DUMMY_BUILD_NAME'
        self.build_number = 'DUMMY_BUILDER_NUMBER'

        # For archived results.
        self._json = None
        self._num_runs = 0
        self._tests_set = set([])
        self._test_timings = {}
        self._failed_count_map = {}

        self._PASS_count = 0
        self._DISABLED_count = 0
        self._FLAKY_count = 0
        self._FAILS_count = 0
        self._fixable_count = 0

    def _test_json_generation(self, passed_tests_list, failed_tests_list):
        tests_set = set(passed_tests_list) | set(failed_tests_list)

        DISABLED_tests = set([t for t in tests_set
                             if t.startswith('DISABLED_')])
        FLAKY_tests = set([t for t in tests_set
                           if t.startswith('FLAKY_')])
        FAILS_tests = set([t for t in tests_set
                           if t.startswith('FAILS_')])
        PASS_tests = tests_set - (DISABLED_tests | FLAKY_tests | FAILS_tests)

        failed_tests = set(failed_tests_list) - DISABLED_tests
        failed_count_map = dict([(t, 1) for t in failed_tests])

        test_timings = {}
        i = 0
        for test in tests_set:
            test_timings[test] = float(self._num_runs * 100 + i)
            i += 1

        test_results_map = dict()
        for test in tests_set:
            test_results_map[test] = json_results_generator.TestResult(test,
                failed=(test in failed_tests),
                elapsed_time=test_timings[test])

        generator = json_results_generator.JSONResultsGeneratorBase(
            self.builder_name, self.build_name, self.build_number,
            '',
            None,   # don't fetch past json results archive
            test_results_map)

        failed_count_map = dict([(t, 1) for t in failed_tests])

        # Test incremental json results
        incremental_json = generator.get_json(incremental=True)
        self._verify_json_results(
            tests_set,
            test_timings,
            failed_count_map,
            len(PASS_tests),
            len(DISABLED_tests),
            len(FLAKY_tests),
            len(DISABLED_tests | failed_tests),
            incremental_json,
            1)

        # Test aggregated json results
        generator.set_archived_results(self._json)
        json = generator.get_json(incremental=False)
        self._json = json
        self._num_runs += 1
        self._tests_set |= tests_set
        self._test_timings.update(test_timings)
        self._PASS_count += len(PASS_tests)
        self._DISABLED_count += len(DISABLED_tests)
        self._FLAKY_count += len(FLAKY_tests)
        self._fixable_count += len(DISABLED_tests | failed_tests)

        get = self._failed_count_map.get
        for test in failed_count_map.iterkeys():
            self._failed_count_map[test] = get(test, 0) + 1

        self._verify_json_results(
            self._tests_set,
            self._test_timings,
            self._failed_count_map,
            self._PASS_count,
            self._DISABLED_count,
            self._FLAKY_count,
            self._fixable_count,
            self._json,
            self._num_runs)

    def _verify_json_results(self, tests_set, test_timings, failed_count_map,
                             PASS_count, DISABLED_count, FLAKY_count,
                             fixable_count,
                             json, num_runs):
        # Aliasing to a short name for better access to its constants.
        JRG = json_results_generator.JSONResultsGeneratorBase

        self.assertTrue(JRG.VERSION_KEY in json)
        self.assertTrue(self.builder_name in json)

        buildinfo = json[self.builder_name]
        self.assertTrue(JRG.FIXABLE in buildinfo)
        self.assertTrue(JRG.TESTS in buildinfo)
        self.assertEqual(len(buildinfo[JRG.BUILD_NUMBERS]), num_runs)
        self.assertEqual(buildinfo[JRG.BUILD_NUMBERS][0], self.build_number)

        if tests_set or DISABLED_count:
            fixable = {}
            for fixable_items in buildinfo[JRG.FIXABLE]:
                for (type, count) in fixable_items.iteritems():
                    if type in fixable:
                        fixable[type] = fixable[type] + count
                    else:
                        fixable[type] = count

            if PASS_count:
                self.assertEqual(fixable[JRG.PASS_RESULT], PASS_count)
            else:
                self.assertTrue(JRG.PASS_RESULT not in fixable or
                                fixable[JRG.PASS_RESULT] == 0)
            if DISABLED_count:
                self.assertEqual(fixable[JRG.SKIP_RESULT], DISABLED_count)
            else:
                self.assertTrue(JRG.SKIP_RESULT not in fixable or
                                fixable[JRG.SKIP_RESULT] == 0)
            if FLAKY_count:
                self.assertEqual(fixable[JRG.FLAKY_RESULT], FLAKY_count)
            else:
                self.assertTrue(JRG.FLAKY_RESULT not in fixable or
                                fixable[JRG.FLAKY_RESULT] == 0)

        if failed_count_map:
            tests = buildinfo[JRG.TESTS]
            for test_name in failed_count_map.iterkeys():
                self.assertTrue(test_name in tests)
                test = tests[test_name]

                failed = 0
                for result in test[JRG.RESULTS]:
                    if result[1] == JRG.FAIL_RESULT:
                        failed += result[0]
                self.assertEqual(failed_count_map[test_name], failed)

                timing_count = 0
                for timings in test[JRG.TIMES]:
                    if timings[1] == test_timings[test_name]:
                        timing_count = timings[0]
                self.assertEqual(1, timing_count)

        if fixable_count:
            self.assertEqual(sum(buildinfo[JRG.FIXABLE_COUNT]), fixable_count)

    def test_json_generation(self):
        self._test_json_generation([], [])
        self._test_json_generation(['A1', 'B1'], [])
        self._test_json_generation([], ['FAILS_A2', 'FAILS_B2'])
        self._test_json_generation(['DISABLED_A3', 'DISABLED_B3'], [])
        self._test_json_generation(['A4'], ['B4', 'FAILS_C4'])
        self._test_json_generation(['DISABLED_C5', 'DISABLED_D5'], ['A5', 'B5'])
        self._test_json_generation(
            ['A6', 'B6', 'FAILS_C6', 'DISABLED_E6', 'DISABLED_F6'],
            ['FAILS_D6'])

        # Generate JSON with the same test sets. (Both incremental results and
        # archived results must be updated appropriately.)
        self._test_json_generation(
            ['A', 'FLAKY_B', 'DISABLED_C'],
            ['FAILS_D', 'FLAKY_E'])
        self._test_json_generation(
            ['A', 'DISABLED_C', 'FLAKY_E'],
            ['FLAKY_B', 'FAILS_D'])
        self._test_json_generation(
            ['FLAKY_B', 'DISABLED_C', 'FAILS_D'],
            ['A', 'FLAKY_E'])

if __name__ == '__main__':
    unittest.main()
