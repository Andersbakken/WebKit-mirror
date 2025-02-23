# Copyright (C) 2009, Google Inc. All rights reserved.
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
#
# WebKit's Python module for understanding the various ports

import os
import platform

from webkitpy.common.system.executive import Executive


class WebKitPort(object):

    # We might need to pass scm into this function for scm.checkout_root
    @classmethod
    def script_path(cls, script_name):
        return os.path.join("Tools", "Scripts", script_name)

    @staticmethod
    def port(port_name):
        ports = {
            "chromium": ChromiumPort,
            "chromium-xvfb": ChromiumXVFBPort,
            "gtk": GtkPort,
            "mac": MacPort,
            "win": WinPort,
            "qt": QtPort,
            "efl": EflPort,
        }
        default_port = {
            "Windows": WinPort,
            "Darwin": MacPort,
        }
        # Do we really need MacPort as the ultimate default?
        return ports.get(port_name, default_port.get(platform.system(), MacPort))

    @staticmethod
    def makeArgs():
        args = '--makeargs="-j%s"' % Executive().cpu_count()
        if os.environ.has_key('MAKEFLAGS'):
            args = '--makeargs="%s"' % os.environ['MAKEFLAGS']
        return args

    @classmethod
    def name(cls):
        raise NotImplementedError("subclasses must implement")

    @classmethod
    def flag(cls):
        raise NotImplementedError("subclasses must implement")

    @classmethod
    def update_webkit_command(cls):
        return [cls.script_path("update-webkit")]

    @classmethod
    def build_webkit_command(cls, build_style=None):
        command = [cls.script_path("build-webkit")]
        if build_style == "debug":
            command.append("--debug")
        if build_style == "release":
            command.append("--release")
        return command

    @classmethod
    def run_javascriptcore_tests_command(cls):
        return [cls.script_path("run-javascriptcore-tests")]

    @classmethod
    def run_webkit_tests_command(cls):
        return [cls.script_path("run-webkit-tests")]

    @classmethod
    def run_python_unittests_command(cls):
        return [cls.script_path("test-webkitpy")]

    @classmethod
    def run_perl_unittests_command(cls):
        return [cls.script_path("test-webkitperl")]

    @classmethod
    def layout_tests_results_path(cls):
        return "/tmp/layout-test-results/results.html"


class MacPort(WebKitPort):

    @classmethod
    def name(cls):
        return "Mac"

    @classmethod
    def flag(cls):
        return "--port=mac"

    @classmethod
    def _system_version(cls):
        version_string = platform.mac_ver()[0]  # e.g. "10.5.6"
        version_tuple = version_string.split('.')
        return map(int, version_tuple)

    @classmethod
    def is_leopard(cls):
        return tuple(cls._system_version()[:2]) == (10, 5)


class WinPort(WebKitPort):

    @classmethod
    def name(cls):
        return "Win"

    @classmethod
    def flag(cls):
        # FIXME: This is lame.  We should autogenerate this from a codename or something.
        return "--port=win"


class GtkPort(WebKitPort):

    @classmethod
    def name(cls):
        return "Gtk"

    @classmethod
    def flag(cls):
        return "--port=gtk"

    @classmethod
    def build_webkit_command(cls, build_style=None):
        command = WebKitPort.build_webkit_command(build_style=build_style)
        command.append("--gtk")
        command.append(WebKitPort.makeArgs())
        return command

    @classmethod
    def run_webkit_tests_command(cls):
        command = WebKitPort.run_webkit_tests_command()
        command.append("--gtk")
        return command


class QtPort(WebKitPort):

    @classmethod
    def name(cls):
        return "Qt"

    @classmethod
    def flag(cls):
        return "--port=qt"

    @classmethod
    def build_webkit_command(cls, build_style=None):
        command = WebKitPort.build_webkit_command(build_style=build_style)
        command.append("--qt")
        command.append(WebKitPort.makeArgs())
        return command


class EflPort(WebKitPort):

    @classmethod
    def name(cls):
        return "Efl"

    @classmethod
    def flag(cls):
        return "--port=efl"

    @classmethod
    def build_webkit_command(cls, build_style=None):
        command = WebKitPort.build_webkit_command(build_style=build_style)
        command.append("--efl")
        command.append(WebKitPort.makeArgs())
        return command


class ChromiumPort(WebKitPort):

    @classmethod
    def name(cls):
        return "Chromium"

    @classmethod
    def flag(cls):
        return "--port=chromium"

    @classmethod
    def update_webkit_command(cls):
        command = WebKitPort.update_webkit_command()
        command.append("--chromium")
        return command

    @classmethod
    def build_webkit_command(cls, build_style=None):
        command = WebKitPort.build_webkit_command(build_style=build_style)
        command.append("--chromium")
        command.append("--update-chromium")
        return command

    @classmethod
    def run_webkit_tests_command(cls):
        return [
            cls.script_path("new-run-webkit-tests"),
            "--chromium",
            "--no-pixel-tests",
        ]

    @classmethod
    def run_javascriptcore_tests_command(cls):
        return None


class ChromiumXVFBPort(ChromiumPort):

    @classmethod
    def flag(cls):
        return "--port=chromium-xvfb"

    @classmethod
    def run_webkit_tests_command(cls):
        # FIXME: We should find a better way to do this.
        return ["xvfb-run"] + ChromiumPort.run_webkit_tests_command()
