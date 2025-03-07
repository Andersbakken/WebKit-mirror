# Copyright (C) 2010 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import unittest

from webkitpy.common.config import build


class ShouldBuildTest(unittest.TestCase):
    _should_build_tests = [
        (["Websites/bugs.webkit.org/foo", "WebCore/bar"], ["*"]),
        (["Websites/bugs.webkit.org/foo"], []),
        (["JavaScriptCore/JavaScriptCore.xcodeproj/foo"], ["mac-leopard", "mac-snowleopard"]),
        (["Sources/JavaScriptGlue/foo", "WebCore/bar"], ["*"]),
        (["Sources/JavaScriptGlue/foo"], ["mac-leopard", "mac-snowleopard"]),
        (["LayoutTests/foo"], ["*"]),
        (["LayoutTests/platform/chromium-linux/foo"], ["chromium-linux"]),
        (["LayoutTests/platform/chromium-win/fast/compact/001-expected.txt"], ["chromium-win"]),
        (["LayoutTests/platform/mac-leopard/foo"], ["mac-leopard"]),
        (["LayoutTests/platform/mac-snowleopard/foo"], ["mac-snowleopard", "win"]),
        (["LayoutTests/platform/mac-wk2/Skipped"], ["mac-snowleopard", "win"]),
        (["LayoutTests/platform/mac/foo"], ["mac-leopard", "mac-snowleopard", "win"]),
        (["LayoutTests/platform/win-xp/foo"], ["win"]),
        (["LayoutTests/platform/win-wk2/foo"], ["win"]),
        (["LayoutTests/platform/win/foo"], ["win"]),
        (["WebCore/mac/foo"], ["chromium-mac", "mac-leopard", "mac-snowleopard"]),
        (["WebCore/win/foo"], ["chromium-win", "win"]),
        (["WebCore/platform/graphics/gpu/foo"], ["mac-leopard", "mac-snowleopard"]),
        (["WebCore/platform/wx/wxcode/win/foo"], []),
        (["WebCore/rendering/RenderThemeMac.mm", "WebCore/rendering/RenderThemeMac.h"], ["mac-leopard", "mac-snowleopard"]),
        (["WebCore/rendering/RenderThemeChromiumLinux.h"], ["chromium-linux"]),
        (["WebCore/rendering/RenderThemeWinCE.h"], []),
    ]

    def test_should_build(self):
        for files, platforms in self._should_build_tests:
            # FIXME: We should test more platforms here once
            # build._should_file_trigger_build is implemented for them.
            for platform in ["win"]:
                should_build = platform in platforms or "*" in platforms
                self.assertEqual(build.should_build(platform, files), should_build, "%s should%s have built but did%s (files: %s)" % (platform, "" if should_build else "n't", "n't" if should_build else "", str(files)))


if __name__ == "__main__":
    unittest.main()
