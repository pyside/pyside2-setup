# This Python file uses the following encoding: utf-8
# It has been edited by fix-complaints.py .

#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

"""
fix-complaints.py

This module fixes the buildbot messages of external python files.
Run it once after copying a new version. It is idem-potent, unless
you are changing messages (what I did, of course :-) .
"""

import os
import glob

patched_file_patterns = "backport_inspect.py typing27.py python_minilib_*.py"

offending_words = {
    "behavio""ur": "behavior",
    "at""least": "at_least",
    "reali""sed": "realized",
}

utf8_line = "# This Python file uses the following encoding: utf-8\n"
marker_line = f"# It has been edited by {os.path.basename(__file__)} .\n"

def patch_file(fname):
    with open(fname) as f:
        lines = f.readlines()
    dup = lines[:]
    for idx, line in enumerate(lines):
        for word, repl in offending_words.items():
            if word in line:
                lines[idx] = line.replace(word, repl)
                print(f"line:{line!r} {word!r}->{repl!r}")
    if lines[0].strip() != utf8_line.strip():
        lines[:0] = [utf8_line, "\n"]
    if lines[1] != marker_line:
        lines[1:1] = marker_line
    if lines != dup:
        with open(fname, "w") as f:
            f.write("".join(lines))

def doit():
    dirname = os.path.dirname(__file__)
    patched_files = []
    for name in patched_file_patterns.split():
        pattern = os.path.join(dirname, name)
        patched_files += glob.glob(pattern)
    for fname in patched_files:
        print("Working on", fname)
        patch_file(fname)

if __name__ == "__main__":
    doit()

# end of file
