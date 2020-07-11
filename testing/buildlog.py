#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python
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

from __future__ import print_function

"""
testing/buildlog.py

A BuildLog holds the tests of a build.

BuildLog.classifiers finds the set of classifier strings.
"""

import os
import sys
import shutil
from collections import namedtuple
from textwrap import dedent

from .helper import script_dir

LogEntry = namedtuple("LogEntry", ["log_dir", "build_dir", "build_classifiers"])


class BuildLog(object):
    """
    This class is a convenience wrapper around a list of log entries.

    The list of entries is sorted by date and checked for consistency.
    For simplicity and readability, the log entries are named tuples.

    """
    def __init__(self):
        history_dir = os.path.join(script_dir, 'build_history')
        build_history = []
        for timestamp in os.listdir(history_dir):
            log_dir = os.path.join(history_dir, timestamp)
            if not os.path.isdir(log_dir):
                continue
            fpath = os.path.join(log_dir, 'build_dir.txt')
            if not os.path.exists(fpath):
                continue
            with open(fpath) as f:
                f_contents = f.read().strip()
                f_contents_split = f_contents.splitlines()
                try:
                    if len(f_contents_split) == 2:
                        build_dir = f_contents_split[0]
                        build_classifiers = f_contents_split[1]
                    else:
                        build_dir = f_contents_split[0]
                        build_classifiers = ""
                except IndexError:
                    print(dedent("""
                        Error: There was an issue finding the build dir and its
                        characteristics, in the following considered file: '{}'
                    """.format(fpath)))
                    sys.exit(1)

                if not os.path.exists(build_dir):
                    rel_dir, low_part = os.path.split(build_dir)
                    rel_dir, two_part = os.path.split(rel_dir)
                    if two_part.startswith("pyside") and two_part.endswith("build"):
                        build_dir = os.path.abspath(os.path.join(two_part, low_part))
                        if os.path.exists(build_dir):
                            print("Note: build_dir was probably moved.")
                        else:
                            print("Warning: missing build dir %s" % build_dir)
                            continue
            entry = LogEntry(log_dir, build_dir, build_classifiers)
            build_history.append(entry)
        # we take the latest build for now.
        build_history.sort()
        self.history = build_history
        self._buildno = None
        self.prune_old_entries(history_dir)

    def prune_old_entries(self, history_dir):
        lst = []
        for timestamp in os.listdir(history_dir):
            log_dir = os.path.join(history_dir, timestamp)
            if not os.path.isdir(log_dir):
                continue
            lst.append(log_dir)
        if lst:
            def warn_problem(func, path, exc_info):
                cls, ins, tb = exc_info
                print("rmtree({}) warning: problem with {}:\n   {}: {}".format(
                    func.__name__, path,
                    cls.__name__, ins.args))

            lst.sort()
            log_dir = lst[-1]
            fname = os.path.basename(log_dir)
            ref_date_str = fname[:10]
            for log_dir in lst:
                fname = os.path.basename(log_dir)
                date_str = fname[:10]
                if date_str != ref_date_str:
                    shutil.rmtree(log_dir, onerror=warn_problem)

    def set_buildno(self, buildno):
        self.history[buildno] # test
        self._buildno = buildno

    @property
    def selected(self):
        if self._buildno is None:
            return None
        if self.history is None:
            return None
        return self.history[self._buildno]

    @property
    def classifiers(self):
        if not self.selected:
            raise ValueError('+++ No build with the configuration found!')
        # Python2 legacy: Correct 'linux2' to 'linux', recommended way.
        platform = 'linux' if sys.platform.startswith('linux') else sys.platform
        res = [platform]
        if self.selected.build_classifiers:
            # Use classifier string encoded into build_dir.txt file.
            res.extend(self.selected.build_classifiers.split('-'))
        else:
            # the rest must be guessed from the given filename
            path = self.selected.build_dir
            base = os.path.basename(path)
            res.extend(base.split('-'))
        # add all the python and qt subkeys
        for entry in res:
            parts = entry.split(".")
            for idx in range(len(parts)):
                key = ".".join(parts[:idx])
                if key not in res:
                    res.append(key)
        return res

builds = BuildLog()
