#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of PySide6.
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
embedding_generator.py

This file takes the content of the two supported directories and inserts
it into a zip file. The zip file is then converted into a C++ source
file that can easily be unpacked again with Python (see signature.cpp,
constant 'PySide_PythonCode').

Note that this _is_ a zipfile, but since it is embedded into the shiboken
binary, we cannot use the zipimport module from Python.
But a similar solution is possible that allows for normal imports.

See signature_bootstrap.py for details.
"""

import sys
import os
import subprocess
import textwrap
import tempfile
import argparse
import marshal
import traceback

# work_dir is set to the source for testing, onl.
# It can be overridden in the command line.
work_dir = os.path.abspath(os.path.dirname(__file__))
embed_dir = work_dir
cur_dir = os.getcwd()
source_dir = os.path.normpath(os.path.join(work_dir, "..", "..", ".."))
assert os.path.basename(source_dir) == "sources"
build_script_dir = os.path.normpath(os.path.join(work_dir, "..", "..", "..", ".."))
assert os.path.exists(os.path.join(build_script_dir, "build_scripts"))

sys.path.insert(0, build_script_dir)

from build_scripts import utils


def runpy(cmd, **kw):
    subprocess.call([sys.executable, '-E'] + cmd.split(), **kw)


def create_zipfile(limited_api):
    """
    Collect all Python files, compile them, create a zip file
    and make a chunked base64 encoded file from it.
    """
    zip_name = "signature.zip"
    inc_name = "signature_inc.h"
    flag = '-b'
    os.chdir(work_dir)

    # Remove all left-over py[co] and other files first, in case we use '--reuse-build'.
    # Note that we could improve that with the PyZipfile function to use .pyc files
    # in different folders, but that makes only sense when COIN allows us to have
    # multiple Python versions in parallel.
    from os.path import join, getsize
    for root, dirs, files in os.walk(work_dir):
        for name in files:
            fpath = os.path.join(root, name)
            ew = name.endswith
            if ew(".pyc") or ew(".pyo") or ew(".zip") or ew(".inc"):
                os.remove(fpath)
    # We copy every Python file into this dir, but only for the right version.
    # For testing in the source dir, we need to filter.
    ignore = []
    utils.copydir(os.path.join(source_dir, "shiboken6", "shibokenmodule", "files.dir", "shibokensupport"),
                  os.path.join(work_dir, "shibokensupport"),
                  ignore=ignore, file_filter_function=lambda name, n2: name.endswith(".py"))
    if embed_dir != work_dir:
        utils.copyfile(os.path.join(embed_dir, "signature_bootstrap.py"), work_dir)

    if limited_api:
        pass   # We cannot compile, unless we have folders per Python version
    else:
        files = ' '.join(fn for fn in os.listdir('.'))
        runpy(f'-m compileall -q {flag} {files}')
    files = ' '.join(fn for fn in os.listdir('.') if not fn == zip_name)
    runpy(f'-m zipfile -c {zip_name} {files}')
    tmp = tempfile.TemporaryFile(mode="w+")
    runpy(f'-m base64 {zip_name}', stdout=tmp)
    # now generate the include file
    tmp.seek(0)
    with open(inc_name, "w") as inc:
        _embed_file(tmp, inc)
    tmp.close()
    # also generate a simple embeddable .pyc file for signature_bootstrap.pyc
    boot_name = "signature_bootstrap.py" if limited_api else "signature_bootstrap.pyc"
    with open(boot_name, "rb") as ldr, open("signature_bootstrap_inc.h", "w") as inc:
        _embed_bytefile(ldr, inc, limited_api)
    os.chdir(cur_dir)


def _embed_file(fin, fout):
    """
    Format a text file for embedding in a C++ source file.
    """
    # MSVC has a 64k string limitation. In C, it would be easy to create an
    # array of 64 byte strings and use them as one big array. In C++ this does
    # not work, since C++ insists in having the terminating nullbyte.
    # Therefore, we split the string after an arbitrary number of lines
    # (chunked file).
    limit = 50
    text = fin.readlines()
    print(textwrap.dedent("""
        /*
         * This is a ZIP archive of all Python files in the directory
         *         "shiboken6/shibokenmodule/files.dir/shibokensupport/signature"
         * There is also a toplevel file "signature_bootstrap.py[c]" that will be
         * directly executed from C++ as a bootstrap loader.
         */
         """).strip(), file=fout)
    block, blocks = 0, len(text) // limit + 1
    for idx, line in enumerate(text):
        if idx % limit == 0:
            comma = "," if block else ""
            block += 1
            print(file=fout)
            print(f'/* Block {block} of {blocks} */{comma}', file=fout)
        print(f'\"{line.strip()}\"', file=fout)
    print(f'/* Sentinel */, \"\"', file=fout)


def _embed_bytefile(fin, fout, is_text):
    """
    Format a binary file for embedding in a C++ source file.
    This version works directly with a single .pyc file.
    """
    fname = fin.name
    remark = ("No .pyc file because '--LIMITED-API=yes'" if is_text else
              "The .pyc header is stripped away")
    print(textwrap.dedent(f"""
        /*
         * This is the file "{fname}" as a simple byte array.
         * It can be directly embedded without any further processing.
         * {remark}.
         */
         """), file=fout)
    headsize = ( 0 if is_text else
                16 if sys.version_info >= (3, 7) else 12 if sys.version_info >= (3, 3) else 8)
    binstr = fin.read()[headsize:]
    if is_text:
        try:
            compile(binstr, fin.name, "exec")
        except SyntaxError as e:
            print(e)
            traceback.print_exc(file=sys.stdout)
            print(textwrap.dedent(f"""
                *************************************************************************
                ***
                *** Could not compile the boot loader '{fname}'!
                ***
                *************************************************************************
                """))
            raise SystemError
    else:
        try:
            marshal.loads(binstr)
        except ValueError as e:
            print(e)
            traceback.print_exc(file=sys.stdout)
            version = sys.version_info[:3]
            print(textwrap.dedent(f"""
                *************************************************************************
                ***
                *** This Python version {version} seems to have a new .pyc header size.
                *** Please correct the 'headsize' constant ({headsize}).
                ***
                *************************************************************************
                """))
            raise SystemError

    print(file=fout)
    use_ord = sys.version_info[0] == 2
    for i in range(0, len(binstr), 16):
        for c in bytes(binstr[i : i + 16]):
            print("{:#4},".format(ord(c) if use_ord else c), file=fout, end="")
        print(file=fout)
    print("/* End Of File */", file=fout)


def str2bool(v):
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--cmake-dir', nargs="?")
    parser.add_argument('--limited-api', type=str2bool)
    args = parser.parse_args()
    if args.cmake_dir:
        work_dir = os.path.abspath(args.cmake_dir)
    create_zipfile(args.limited_api)
