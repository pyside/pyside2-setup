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
module_collector.py

Collect a number of modules listed on the command line.

The purpose of this script is to generate the scripts needed for
a complete isolation of the signature extension.

Usage:

Run this script in one of the used python versions.
It will create an executable archive of the files on the command line.
"""

import sys
import os
import argparse
import pickle
from textwrap import dedent

def source_archive(module, modname):
    fname = os.path.splitext(module.__file__)[0] + ".py"
    with open(fname) as source:
        text = source.read()
    encoded = text.replace("'''", "(triple_single)")
    # modname = module.__name__
    # Do not use: Some modules rename themselves!
    version = ".".join(map(str, sys.version_info[:3]))
    shortname = os.path.basename(fname)
    preamble = dedent(r"""
        # BEGIN SOURCE ARCHIVE    Python {version}  module {modname}

        sources = {{}} if "sources" not in globals() else sources
        sources["{modname}"] = '''\
        {encoded}'''.replace("(triple_single)", "'''")

        # END   SOURCE ARCHIVE    Python {version}  module {modname}
        """).format(**locals())
    return preamble

def read_all(modules):
    collected = ""
    for modname in modules:
        mod = __import__(modname)
        collected += source_archive(mod, modname)
    return collected

def license_header():
    license = os.path.join(os.path.dirname(__file__), "qt_python_license.txt")
    with open(license) as f:
        return f.read()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('modules', nargs="+")
    args = parser.parse_args()
    print("modules:", args.modules)
    ret = license_header() + read_all(args.modules)
    ma_mi = "_".join(map(str, sys.version_info[:2]))
    outpath = os.path.join(os.path.dirname(__file__), "..", "..", "shibokenmodule",
        "files.dir", "shibokensupport", "python_minilib_{ma_mi}.py".format(**locals()))
    with open(outpath, "w") as f:
        f.write(ret)
