# This file is part of PySide: Python for Qt
#
# Copyright (C) 2016 The Qt Company Ltd.
#
# Contact: http://www.qt.io/licensing/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

from __future__ import print_function

"""
This script adds a constructor to wrappers where the constructor
of the base class is not reachable (private).

It is run via the equivalent of an execfile command.
This patch became necessary when VS2015 became the standard compiler.
"""

import os

def patch(fname, snippet, path=target_dir):
    fpath = os.path.join(path, fname)
    with open(fpath, 'r') as f:
        lines = f.readlines()
    for idx, line in enumerate(lines):
        if line.rstrip() == "public:":
            break
    else:
        raise SyntaxError("no public section found")
    lines[idx+1:idx+1] = snippet
    with open(fpath, 'w') as f:
        f.writelines(lines)
    print("+++ patched file:", fpath)

snippets = {
    "qclipboard_wrapper.h" : """\
    QClipboardWrapper(QObject *parent);
    ~QClipboardWrapper();
""",
    "qsessionmanager_wrapper.h" : """\
    QSessionManagerWrapper(QGuiApplication *app, QString &id, QString &key);
    ~QSessionManagerWrapper();
""",
}

for snippet, addition in snippets.items():
    patch(snippet, addition)
