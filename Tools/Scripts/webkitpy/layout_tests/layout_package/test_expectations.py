#!/usr/bin/env python
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

"""A helper class for reading in and dealing with tests expectations
for layout tests.
"""

import logging
import os
import re
import sys

import webkitpy.thirdparty.simplejson as simplejson

_log = logging.getLogger("webkitpy.layout_tests.layout_package."
                         "test_expectations")

# Test expectation and modifier constants.
(PASS, FAIL, TEXT, IMAGE, IMAGE_PLUS_TEXT, TIMEOUT, CRASH, SKIP, WONTFIX,
 SLOW, REBASELINE, MISSING, FLAKY, NOW, NONE) = range(15)

# Test expectation file update action constants
(NO_CHANGE, REMOVE_TEST, REMOVE_PLATFORM, ADD_PLATFORMS_EXCEPT_THIS) = range(4)


def result_was_expected(result, expected_results, test_needs_rebaselining,
                        test_is_skipped):
    """Returns whether we got a result we were expecting.
    Args:
        result: actual result of a test execution
        expected_results: set of results listed in test_expectations
        test_needs_rebaselining: whether test was marked as REBASELINE
        test_is_skipped: whether test was marked as SKIP"""
    if result in expected_results:
        return True
    if result in (IMAGE, TEXT, IMAGE_PLUS_TEXT) and FAIL in expected_results:
        return True
    if result == MISSING and test_needs_rebaselining:
        return True
    if result == SKIP and test_is_skipped:
        return True
    return False


def remove_pixel_failures(expected_results):
    """Returns a copy of the expected results for a test, except that we
    drop any pixel failures and return the remaining expectations. For example,
    if we're not running pixel tests, then tests expected to fail as IMAGE
    will PASS."""
    expected_results = expected_results.copy()
    if IMAGE in expected_results:
        expected_results.remove(IMAGE)
        expected_results.add(PASS)
    if IMAGE_PLUS_TEXT in expected_results:
        expected_results.remove(IMAGE_PLUS_TEXT)
        expected_results.add(TEXT)
    return expected_results


class TestExpectations:
    TEST_LIST = "test_expectations.txt"

    def __init__(self, port, tests, expectations, test_platform_name,
                 is_debug_mode, is_lint_mode, overrides=None):
        """Loads and parses the test expectations given in the string.
        Args:
            port: handle to object containing platform-specific functionality
            test: list of all of the test files
            expectations: test expectations as a string
            test_platform_name: name of the platform to match expectations
                against. Note that this may be different than
                port.test_platform_name() when is_lint_mode is True.
            is_debug_mode: whether to use the DEBUG or RELEASE modifiers
                in the expectations
            is_lint_mode: If True, just parse the expectations string
                looking for errors.
            overrides: test expectations that are allowed to override any
                entries in |expectations|. This is used by callers
                that need to manage two sets of expectations (e.g., upstream
                and downstream expectations).
        """
        self._expected_failures = TestExpectationsFile(port, expectations,
            tests, test_platform_name, is_debug_mode, is_lint_mode,
            overrides=overrides)

    # TODO(ojan): Allow for removing skipped tests when getting the list of
    # tests to run, but not when getting metrics.
    # TODO(ojan): Replace the Get* calls here with the more sane API exposed
    # by TestExpectationsFile below. Maybe merge the two classes entirely?

    def get_expectations_json_for_all_platforms(self):
        return (
            self._expected_failures.get_expectations_json_for_all_platforms())

    def get_rebaselining_failures(self):
        return (self._expected_failures.get_test_set(REBASELINE, FAIL) |
                self._expected_failures.get_test_set(REBASELINE, IMAGE) |
                self._expected_failures.get_test_set(REBASELINE, TEXT) |
                self._expected_failures.get_test_set(REBASELINE,
                                                     IMAGE_PLUS_TEXT))

    def get_options(self, test):
        return self._expected_failures.get_options(test)

    def get_expectations(self, test):
        return self._expected_failures.get_expectations(test)

    def get_expectations_string(self, test):
        """Returns the expectatons for the given test as an uppercase string.
        If there are no expectations for the test, then "PASS" is returned."""
        expectations = self.get_expectations(test)
        retval = []

        for expectation in expectations:
            retval.append(self.expectation_to_string(expectation))

        return " ".join(retval)

    def expectation_to_string(self, expectation):
        """Return the uppercased string equivalent of a given expectation."""
        for item in TestExpectationsFile.EXPECTATIONS.items():
            if item[1] == expectation:
                return item[0].upper()
        raise ValueError(expectation)

    def get_tests_with_result_type(self, result_type):
        return self._expected_failures.get_tests_with_result_type(result_type)

    def get_tests_with_timeline(self, timeline):
        return self._expected_failures.get_tests_with_timeline(timeline)

    def matches_an_expected_result(self, test, result,
                                   pixel_tests_are_enabled):
        expected_results = self._expected_failures.get_expectations(test)
        if not pixel_tests_are_enabled:
            expected_results = remove_pixel_failures(expected_results)
        return result_was_expected(result, expected_results,
            self.is_rebaselining(test), self.has_modifier(test, SKIP))

    def is_rebaselining(self, test):
        return self._expected_failures.has_modifier(test, REBASELINE)

    def has_modifier(self, test, modifier):
        return self._expected_failures.has_modifier(test, modifier)

    def remove_platform_from_expectations(self, tests, platform):
        return self._expected_failures.remove_platform_from_expectations(
            tests, platform)


def strip_comments(line):
    """Strips comments from a line and return None if the line is empty
    or else the contents of line with leading and trailing spaces removed
    and all other whitespace collapsed"""

    commentIndex = line.find('//')
    if commentIndex is -1:
        commentIndex = len(line)

    line = re.sub(r'\s+', ' ', line[:commentIndex].strip())
    if line == '':
        return None
    else:
        return line


class ParseError(Exception):
    def __init__(self, fatal, errors):
        self.fatal = fatal
        self.errors = errors

    def __str__(self):
        return '\n'.join(map(str, self.errors))

    def __repr__(self):
        return 'ParseError(fatal=%s, errors=%s)' % (fatal, errors)


class ModifiersAndExpectations:
    """A holder for modifiers and expectations on a test that serializes to
    JSON."""

    def __init__(self, modifiers, expectations):
        self.modifiers = modifiers
        self.expectations = expectations


class ExpectationsJsonEncoder(simplejson.JSONEncoder):
    """JSON encoder that can handle ModifiersAndExpectations objects."""
    def default(self, obj):
        # A ModifiersAndExpectations object has two fields, each of which
        # is a dict. Since JSONEncoders handle all the builtin types directly,
        # the only time this routine should be called is on the top level
        # object (i.e., the encoder shouldn't recurse).
        assert isinstance(obj, ModifiersAndExpectations)
        return {"modifiers": obj.modifiers,
                "expectations": obj.expectations}


class TestExpectationsFile:
    """Test expectation files consist of lines with specifications of what
    to expect from layout test cases. The test cases can be directories
    in which case the expectations apply to all test cases in that
    directory and any subdirectory. The format of the file is along the
    lines of:

      LayoutTests/fast/js/fixme.js = FAIL
      LayoutTests/fast/js/flaky.js = FAIL PASS
      LayoutTests/fast/js/crash.js = CRASH TIMEOUT FAIL PASS
      ...

    To add other options:
      SKIP : LayoutTests/fast/js/no-good.js = TIMEOUT PASS
      DEBUG : LayoutTests/fast/js/no-good.js = TIMEOUT PASS
      DEBUG SKIP : LayoutTests/fast/js/no-good.js = TIMEOUT PASS
      LINUX DEBUG SKIP : LayoutTests/fast/js/no-good.js = TIMEOUT PASS
      LINUX WIN : LayoutTests/fast/js/no-good.js = TIMEOUT PASS

    SKIP: Doesn't run the test.
    SLOW: The test takes a long time to run, but does not timeout indefinitely.
    WONTFIX: For tests that we never intend to pass on a given platform.
    DEBUG: Expectations apply only to the debug build.
    RELEASE: Expectations apply only to release build.
    LINUX/WIN/WIN-XP/WIN-VISTA/WIN-7/MAC: Expectations apply only to these
        platforms.

    Notes:
      -A test cannot be both SLOW and TIMEOUT
      -A test should only be one of IMAGE, TEXT, IMAGE+TEXT, or FAIL. FAIL is
       a migratory state that currently means either IMAGE, TEXT, or
       IMAGE+TEXT. Once we have finished migrating the expectations, we will
       change FAIL to have the meaning of IMAGE+TEXT and remove the IMAGE+TEXT
       identifier.
      -A test can be included twice, but not via the same path.
      -If a test is included twice, then the more precise path wins.
      -CRASH tests cannot be WONTFIX
    """

    EXPECTATIONS = {'pass': PASS,
                    'fail': FAIL,
                    'text': TEXT,
                    'image': IMAGE,
                    'image+text': IMAGE_PLUS_TEXT,
                    'timeout': TIMEOUT,
                    'crash': CRASH,
                    'missing': MISSING}

    EXPECTATION_DESCRIPTIONS = {SKIP: ('skipped', 'skipped'),
                                PASS: ('pass', 'passes'),
                                FAIL: ('failure', 'failures'),
                                TEXT: ('text diff mismatch',
                                       'text diff mismatch'),
                                IMAGE: ('image mismatch', 'image mismatch'),
                                IMAGE_PLUS_TEXT: ('image and text mismatch',
                                                  'image and text mismatch'),
                                CRASH: ('DumpRenderTree crash',
                                        'DumpRenderTree crashes'),
                                TIMEOUT: ('test timed out', 'tests timed out'),
                                MISSING: ('no expected result found',
                                          'no expected results found')}

    EXPECTATION_ORDER = (PASS, CRASH, TIMEOUT, MISSING, IMAGE_PLUS_TEXT,
       TEXT, IMAGE, FAIL, SKIP)

    BUILD_TYPES = ('debug', 'release')

    MODIFIERS = {'skip': SKIP,
                 'wontfix': WONTFIX,
                 'slow': SLOW,
                 'rebaseline': REBASELINE,
                 'none': NONE}

    TIMELINES = {'wontfix': WONTFIX,
                 'now': NOW}

    RESULT_TYPES = {'skip': SKIP,
                    'pass': PASS,
                    'fail': FAIL,
                    'flaky': FLAKY}

    def __init__(self, port, expectations, full_test_list, test_platform_name,
        is_debug_mode, is_lint_mode, overrides=None):
        """
        expectations: Contents of the expectations file
        full_test_list: The list of all tests to be run pending processing of
            the expections for those tests.
        test_platform_name: name of the platform to match expectations
            against. Note that this may be different than
            port.test_platform_name() when is_lint_mode is True.
        is_debug_mode: Whether we testing a test_shell built debug mode.
        is_lint_mode: Whether this is just linting test_expecatations.txt.
        overrides: test expectations that are allowed to override any
            entries in |expectations|. This is used by callers
            that need to manage two sets of expectations (e.g., upstream
            and downstream expectations).
        """

        self._port = port
        self._expectations = expectations
        self._full_test_list = full_test_list
        self._test_platform_name = test_platform_name
        self._is_debug_mode = is_debug_mode
        self._is_lint_mode = is_lint_mode
        self._overrides = overrides
        self._errors = []
        self._non_fatal_errors = []

        # Maps relative test paths as listed in the expectations file to a
        # list of maps containing modifiers and expectations for each time
        # the test is listed in the expectations file.
        self._all_expectations = {}

        # Maps a test to its list of expectations.
        self._test_to_expectations = {}

        # Maps a test to its list of options (string values)
        self._test_to_options = {}

        # Maps a test to its list of modifiers: the constants associated with
        # the options minus any bug or platform strings
        self._test_to_modifiers = {}

        # Maps a test to the base path that it was listed with in the list.
        self._test_list_paths = {}

        self._modifier_to_tests = self._dict_of_sets(self.MODIFIERS)
        self._expectation_to_tests = self._dict_of_sets(self.EXPECTATIONS)
        self._timeline_to_tests = self._dict_of_sets(self.TIMELINES)
        self._result_type_to_tests = self._dict_of_sets(self.RESULT_TYPES)

        self._read(self._get_iterable_expectations(self._expectations),
                   overrides_allowed=False)

        # List of tests that are in the overrides file (used for checking for
        # duplicates inside the overrides file itself). Note that just because
        # a test is in this set doesn't mean it's necessarily overridding a
        # expectation in the regular expectations; the test might not be
        # mentioned in the regular expectations file at all.
        self._overridding_tests = set()

        if overrides:
            self._read(self._get_iterable_expectations(self._overrides),
                       overrides_allowed=True)

        self._handle_any_read_errors()
        self._process_tests_without_expectations()

    def _handle_any_read_errors(self):
        if len(self._errors) or len(self._non_fatal_errors):
            if self._is_debug_mode:
                build_type = 'DEBUG'
            else:
                build_type = 'RELEASE'
            _log.error('')
            _log.error("FAILURES FOR PLATFORM: %s, BUILD_TYPE: %s" %
                       (self._test_platform_name.upper(), build_type))

            for error in self._errors:
                _log.error(error)
            for error in self._non_fatal_errors:
                _log.error(error)

            if len(self._errors):
                raise ParseError(fatal=True, errors=self._errors)
            if len(self._non_fatal_errors) and self._is_lint_mode:
                raise ParseError(fatal=False, errors=self._non_fatal_errors)

    def _process_tests_without_expectations(self):
        expectations = set([PASS])
        options = []
        modifiers = []
        if self._full_test_list:
            for test in self._full_test_list:
                if not test in self._test_list_paths:
                    self._add_test(test, modifiers, expectations, options,
                        overrides_allowed=False)

    def _dict_of_sets(self, strings_to_constants):
        """Takes a dict of strings->constants and returns a dict mapping
        each constant to an empty set."""
        d = {}
        for c in strings_to_constants.values():
            d[c] = set()
        return d

    def _get_iterable_expectations(self, expectations_str):
        """Returns an object that can be iterated over. Allows for not caring
        about whether we're iterating over a file or a new-line separated
        string."""
        iterable = [x + "\n" for x in expectations_str.split("\n")]
        # Strip final entry if it's empty to avoid added in an extra
        # newline.
        if iterable[-1] == "\n":
            return iterable[:-1]
        return iterable

    def get_test_set(self, modifier, expectation=None, include_skips=True):
        if expectation is None:
            tests = self._modifier_to_tests[modifier]
        else:
            tests = (self._expectation_to_tests[expectation] &
                self._modifier_to_tests[modifier])

        if not include_skips:
            tests = tests - self.get_test_set(SKIP, expectation)

        return tests

    def get_tests_with_result_type(self, result_type):
        return self._result_type_to_tests[result_type]

    def get_tests_with_timeline(self, timeline):
        return self._timeline_to_tests[timeline]

    def get_options(self, test):
        """This returns the entire set of options for the given test
        (the modifiers plus the BUGXXXX identifier). This is used by the
        LTTF dashboard."""
        return self._test_to_options[test]

    def has_modifier(self, test, modifier):
        return test in self._modifier_to_tests[modifier]

    def get_expectations(self, test):
        return self._test_to_expectations[test]

    def get_expectations_json_for_all_platforms(self):
        # Specify separators in order to get compact encoding.
        return ExpectationsJsonEncoder(separators=(',', ':')).encode(
            self._all_expectations)

    def get_non_fatal_errors(self):
        return self._non_fatal_errors

    def remove_platform_from_expectations(self, tests, platform):
        """Returns a copy of the expectations with the tests matching the
        platform removed.

        If a test is in the test list and has an option that matches the given
        platform, remove the matching platform and save the updated test back
        to the file. If no other platforms remaining after removal, delete the
        test from the file.

        Args:
          tests: list of tests that need to update..
          platform: which platform option to remove.

        Returns:
          the updated string.
        """

        assert(platform)
        f_orig = self._get_iterable_expectations(self._expectations)
        f_new = []

        tests_removed = 0
        tests_updated = 0
        lineno = 0
        for line in f_orig:
            lineno += 1
            action = self._get_platform_update_action(line, lineno, tests,
                                                      platform)
            assert(action in (NO_CHANGE, REMOVE_TEST, REMOVE_PLATFORM,
                              ADD_PLATFORMS_EXCEPT_THIS))
            if action == NO_CHANGE:
                # Save the original line back to the file
                _log.debug('No change to test: %s', line)
                f_new.append(line)
            elif action == REMOVE_TEST:
                tests_removed += 1
                _log.info('Test removed: %s', line)
            elif action == REMOVE_PLATFORM:
                parts = line.split(':')
                new_options = parts[0].replace(platform.upper() + ' ', '', 1)
                new_line = ('%s:%s' % (new_options, parts[1]))
                f_new.append(new_line)
                tests_updated += 1
                _log.info('Test updated: ')
                _log.info('  old: %s', line)
                _log.info('  new: %s', new_line)
            elif action == ADD_PLATFORMS_EXCEPT_THIS:
                parts = line.split(':')
                new_options = parts[0]
                for p in self._port.test_platform_names():
                    p = p.upper()
                    # This is a temp solution for rebaselining tool.
                    # Do not add tags WIN-7 and WIN-VISTA to test expectations
                    # if the original line does not specify the platform
                    # option.
                    # TODO(victorw): Remove WIN-VISTA and WIN-7 once we have
                    # reliable Win 7 and Win Vista buildbots setup.
                    if not p in (platform.upper(), 'WIN-VISTA', 'WIN-7'):
                        new_options += p + ' '
                new_line = ('%s:%s' % (new_options, parts[1]))
                f_new.append(new_line)
                tests_updated += 1
                _log.info('Test updated: ')
                _log.info('  old: %s', line)
                _log.info('  new: %s', new_line)

        _log.info('Total tests removed: %d', tests_removed)
        _log.info('Total tests updated: %d', tests_updated)

        return "".join(f_new)

    def parse_expectations_line(self, line, lineno):
        """Parses a line from test_expectations.txt and returns a tuple
        with the test path, options as a list, expectations as a list."""
        line = strip_comments(line)
        if not line:
            return (None, None, None)

        options = []
        if line.find(":") is -1:
            test_and_expectation = line.split("=")
        else:
            parts = line.split(":")
            options = self._get_options_list(parts[0])
            test_and_expectation = parts[1].split('=')

        test = test_and_expectation[0].strip()
        if (len(test_and_expectation) is not 2):
            self._add_error(lineno, "Missing expectations.",
                           test_and_expectation)
            expectations = None
        else:
            expectations = self._get_options_list(test_and_expectation[1])

        return (test, options, expectations)

    def _get_platform_update_action(self, line, lineno, tests, platform):
        """Check the platform option and return the action needs to be taken.

        Args:
          line: current line in test expectations file.
          lineno: current line number of line
          tests: list of tests that need to update..
          platform: which platform option to remove.

        Returns:
          NO_CHANGE: no change to the line (comments, test not in the list etc)
          REMOVE_TEST: remove the test from file.
          REMOVE_PLATFORM: remove this platform option from the test.
          ADD_PLATFORMS_EXCEPT_THIS: add all the platforms except this one.
        """
        test, options, expectations = self.parse_expectations_line(line,
                                                                   lineno)
        if not test or test not in tests:
            return NO_CHANGE

        has_any_platform = False
        for option in options:
            if option in self._port.test_platform_names():
                has_any_platform = True
                if not option == platform:
                    return REMOVE_PLATFORM

        # If there is no platform specified, then it means apply to all
        # platforms. Return the action to add all the platforms except this
        # one.
        if not has_any_platform:
            return ADD_PLATFORMS_EXCEPT_THIS

        return REMOVE_TEST

    def _has_valid_modifiers_for_current_platform(self, options, lineno,
        test_and_expectations, modifiers):
        """Returns true if the current platform is in the options list or if
        no platforms are listed and if there are no fatal errors in the
        options list.

        Args:
          options: List of lowercase options.
          lineno: The line in the file where the test is listed.
          test_and_expectations: The path and expectations for the test.
          modifiers: The set to populate with modifiers.
        """
        has_any_platform = False
        has_bug_id = False
        for option in options:
            if option in self.MODIFIERS:
                modifiers.add(option)
            elif option in self._port.test_platform_names():
                has_any_platform = True
            elif re.match(r'bug\d', option) != None:
                self._add_error(lineno, 'Bug must be either BUGCR, BUGWK, or BUGV8_ for test: %s' %
                                option, test_and_expectations)
            elif option.startswith('bug'):
                has_bug_id = True
            elif option not in self.BUILD_TYPES:
                self._add_error(lineno, 'Invalid modifier for test: %s' %
                                option, test_and_expectations)

        if has_any_platform and not self._match_platform(options):
            return False

        if not has_bug_id and 'wontfix' not in options:
            # TODO(ojan): Turn this into an AddError call once all the
            # tests have BUG identifiers.
            self._log_non_fatal_error(lineno, 'Test lacks BUG modifier.',
                test_and_expectations)

        if 'release' in options or 'debug' in options:
            if self._is_debug_mode and 'debug' not in options:
                return False
            if not self._is_debug_mode and 'release' not in options:
                return False

        if self._is_lint_mode and 'rebaseline' in options:
            self._add_error(lineno,
                'REBASELINE should only be used for running rebaseline.py. '
                'Cannot be checked in.', test_and_expectations)

        return True

    def _match_platform(self, options):
        """Match the list of options against our specified platform. If any
        of the options prefix-match self._platform, return True. This handles
        the case where a test is marked WIN and the platform is WIN-VISTA.

        Args:
          options: list of options
        """
        for opt in options:
            if self._test_platform_name.startswith(opt):
                return True
        return False

    def _add_to_all_expectations(self, test, options, expectations):
        # Make all paths unix-style so the dashboard doesn't need to.
        test = test.replace('\\', '/')
        if not test in self._all_expectations:
            self._all_expectations[test] = []
        self._all_expectations[test].append(
            ModifiersAndExpectations(options, expectations))

    def _read(self, expectations, overrides_allowed):
        """For each test in an expectations iterable, generate the
        expectations for it."""
        lineno = 0
        for line in expectations:
            lineno += 1

            test_list_path, options, expectations = \
                self.parse_expectations_line(line, lineno)
            if not expectations:
                continue

            self._add_to_all_expectations(test_list_path,
                                          " ".join(options).upper(),
                                          " ".join(expectations).upper())

            modifiers = set()
            if options and not self._has_valid_modifiers_for_current_platform(
                options, lineno, test_list_path, modifiers):
                continue

            expectations = self._parse_expectations(expectations, lineno,
                test_list_path)

            if 'slow' in options and TIMEOUT in expectations:
                self._add_error(lineno,
                    'A test can not be both slow and timeout. If it times out '
                    'indefinitely, then it should be just timeout.',
                    test_list_path)

            full_path = os.path.join(self._port.layout_tests_dir(),
                                     test_list_path)
            full_path = os.path.normpath(full_path)
            # WebKit's way of skipping tests is to add a -disabled suffix.
            # So we should consider the path existing if the path or the
            # -disabled version exists.
            if (not self._port.path_exists(full_path)
                and not self._port.path_exists(full_path + '-disabled')):
                # Log a non fatal error here since you hit this case any
                # time you update test_expectations.txt without syncing
                # the LayoutTests directory
                self._log_non_fatal_error(lineno, 'Path does not exist.',
                                       test_list_path)
                continue

            if not self._full_test_list:
                tests = [test_list_path]
            else:
                tests = self._expand_tests(test_list_path)

            self._add_tests(tests, expectations, test_list_path, lineno,
                           modifiers, options, overrides_allowed)

    def _get_options_list(self, listString):
        return [part.strip().lower() for part in listString.strip().split(' ')]

    def _parse_expectations(self, expectations, lineno, test_list_path):
        result = set()
        for part in expectations:
            if not part in self.EXPECTATIONS:
                self._add_error(lineno, 'Unsupported expectation: %s' % part,
                    test_list_path)
                continue
            expectation = self.EXPECTATIONS[part]
            result.add(expectation)
        return result

    def _expand_tests(self, test_list_path):
        """Convert the test specification to an absolute, normalized
        path and make sure directories end with the OS path separator."""
        # FIXME: full_test_list can quickly contain a big amount of
        # elements. We should consider at some point to use a more
        # efficient structure instead of a list. Maybe a dictionary of
        # lists to represent the tree of tests, leaves being test
        # files and nodes being categories.

        path = os.path.join(self._port.layout_tests_dir(), test_list_path)
        path = os.path.normpath(path)
        if self._port.path_isdir(path):
            # this is a test category, return all the tests of the category.
            path = os.path.join(path, '')

            return [test for test in self._full_test_list if test.startswith(path)]

        # this is a test file, do a quick check if it's in the
        # full test suite.
        result = []
        if path in self._full_test_list:
            result = [path, ]
        return result

    def _add_tests(self, tests, expectations, test_list_path, lineno,
                   modifiers, options, overrides_allowed):
        for test in tests:
            if self._already_seen_test(test, test_list_path, lineno,
                                       overrides_allowed):
                continue

            self._clear_expectations_for_test(test, test_list_path)
            self._add_test(test, modifiers, expectations, options,
                           overrides_allowed)

    def _add_test(self, test, modifiers, expectations, options,
                  overrides_allowed):
        """Sets the expected state for a given test.

        This routine assumes the test has not been added before. If it has,
        use _ClearExpectationsForTest() to reset the state prior to
        calling this.

        Args:
          test: test to add
          modifiers: sequence of modifier keywords ('wontfix', 'slow', etc.)
          expectations: sequence of expectations (PASS, IMAGE, etc.)
          options: sequence of keywords and bug identifiers.
          overrides_allowed: whether we're parsing the regular expectations
              or the overridding expectations"""
        self._test_to_expectations[test] = expectations
        for expectation in expectations:
            self._expectation_to_tests[expectation].add(test)

        self._test_to_options[test] = options
        self._test_to_modifiers[test] = set()
        for modifier in modifiers:
            mod_value = self.MODIFIERS[modifier]
            self._modifier_to_tests[mod_value].add(test)
            self._test_to_modifiers[test].add(mod_value)

        if 'wontfix' in modifiers:
            self._timeline_to_tests[WONTFIX].add(test)
        else:
            self._timeline_to_tests[NOW].add(test)

        if 'skip' in modifiers:
            self._result_type_to_tests[SKIP].add(test)
        elif expectations == set([PASS]):
            self._result_type_to_tests[PASS].add(test)
        elif len(expectations) > 1:
            self._result_type_to_tests[FLAKY].add(test)
        else:
            self._result_type_to_tests[FAIL].add(test)

        if overrides_allowed:
            self._overridding_tests.add(test)

    def _clear_expectations_for_test(self, test, test_list_path):
        """Remove prexisting expectations for this test.
        This happens if we are seeing a more precise path
        than a previous listing.
        """
        if test in self._test_list_paths:
            self._test_to_expectations.pop(test, '')
            self._remove_from_sets(test, self._expectation_to_tests)
            self._remove_from_sets(test, self._modifier_to_tests)
            self._remove_from_sets(test, self._timeline_to_tests)
            self._remove_from_sets(test, self._result_type_to_tests)

        self._test_list_paths[test] = os.path.normpath(test_list_path)

    def _remove_from_sets(self, test, dict):
        """Removes the given test from the sets in the dictionary.

        Args:
          test: test to look for
          dict: dict of sets of files"""
        for set_of_tests in dict.itervalues():
            if test in set_of_tests:
                set_of_tests.remove(test)

    def _already_seen_test(self, test, test_list_path, lineno,
                           allow_overrides):
        """Returns true if we've already seen a more precise path for this test
        than the test_list_path.
        """
        if not test in self._test_list_paths:
            return False

        prev_base_path = self._test_list_paths[test]
        if (prev_base_path == os.path.normpath(test_list_path)):
            if (not allow_overrides or test in self._overridding_tests):
                if allow_overrides:
                    expectation_source = "override"
                else:
                    expectation_source = "expectation"
                self._add_error(lineno, 'Duplicate %s.' % expectation_source,
                                   test)
                return True
            else:
                # We have seen this path, but that's okay because its
                # in the overrides and the earlier path was in the
                # expectations.
                return False

        # Check if we've already seen a more precise path.
        return prev_base_path.startswith(os.path.normpath(test_list_path))

    def _add_error(self, lineno, msg, path):
        """Reports an error that will prevent running the tests. Does not
        immediately raise an exception because we'd like to aggregate all the
        errors so they can all be printed out."""
        self._errors.append('Line:%s %s %s' % (lineno, msg, path))

    def _log_non_fatal_error(self, lineno, msg, path):
        """Reports an error that will not prevent running the tests. These are
        still errors, but not bad enough to warrant breaking test running."""
        self._non_fatal_errors.append('Line:%s %s %s' % (lineno, msg, path))
