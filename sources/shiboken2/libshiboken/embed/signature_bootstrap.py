#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of PySide2.
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
signature_bootstrap.py
----------------------

This file was originally directly embedded into the C source.
After it grew more and more, I now prefer to have it as Python file.

Meanwhile, there is also no more a stub loader necessary:
Because we meanwhile have embedding support, we could also load this file
directly from a .pyc file.

This file replaces the hard to read Python stub in 'signature.cpp', and we
could distinguish better between bootstrap related functions and loader
functions.
It is embedded into 'signature.cpp' as "embed/signature_bootstrap.inc".
"""

from __future__ import print_function, absolute_import

recursion_trap = 0

# We avoid real imports in phase 1 that could fail (simply removed all).
# Python 2 is not able to import when the extension import is still active.
# Phase 1 simply defines the functions, which will be used in Phase 2.

def bootstrap():
    import sys
    import os
    import tempfile
    import traceback
    from contextlib import contextmanager

    global recursion_trap
    if recursion_trap:
        # we are probably called from outside, already
        print("Recursion occurred in Bootstrap. Did you start by hand? Then it's ok.")
        print("But you should trigger start by 'type.__signature__', only!")
    recursion_trap += 1

    @contextmanager
    def ensure_shibokensupport(support_path):
        # Make sure that we always have the shibokensupport containing package first.
        # Also remove any prior loaded module of this name, just in case.
        sys.path.insert(0, support_path)

        sbks = "shibokensupport"
        if sbks in sys.modules:
            del sys.modules[sbks]
        prefix = sbks + "."
        for key in list(key for key in sys.modules if key.startswith(prefix)):
            del sys.modules[key]
        try:
            import shibokensupport
            yield
        except Exception as e:
            print("Problem importing shibokensupport:")
            print(e)
            traceback.print_exc()
            print("sys.path:")
            for p in sys.path:
                print("  " + p)
            sys.stdout.flush()
            sys.exit(-1)
        sys.path.remove(support_path)

    try:
        import shiboken2 as root
    except ImportError:
        # uninstalled case without ctest, try only this one which has __init__:
        import shibokenmodule as root
    rp = os.path.realpath(os.path.dirname(root.__file__))
    # This can be the shiboken2 directory or the binary module, so search.
    look_for = "files.dir"
    while len(rp) > 3 and not os.path.exists(os.path.join(rp, look_for)):
        rp = os.path.abspath(os.path.join(rp, ".."))

    # Here we decide if we work embedded or not.
    embedding_var = "pyside_uses_embedding"
    use_embedding = bool(getattr(sys, embedding_var, False))
    # We keep the zip file for inspection if the sys variable has been set.
    keep_zipfile = hasattr(sys, embedding_var)
    real_dir = os.path.join(rp, look_for)

    # We report in sys what we used. We could put more here as well.
    if not os.path.exists(real_dir):
        use_embedding = True
    support_path = prepare_zipfile() if use_embedding else real_dir
    setattr(sys, embedding_var, use_embedding)

    try:
        with ensure_shibokensupport(support_path):
            from shibokensupport.signature import loader

    except Exception as e:
        print('Exception:', e)
        traceback.print_exc(file=sys.stdout)

    finally:
        if use_embedding and not keep_zipfile:
            # clear the temp zipfile
            try:
                os.remove(support_path)
            except OSError as e:
                print(e)
                print("Error deleting {support_path}, ignored".format(**locals()))
        return loader

# New functionality: Loading from a zip archive.
# There exists the zip importer, but as it is written, only real zip files are
# supported. Before I will start an own implementation, it is easiest to use
# a temporary zip file.

def prepare_zipfile():
    """
    Write the zip file to a real file and return its name.
    It will be implicitly opened as such when we add the name to sys.path .
    """
    import base64
    import tempfile
    import os
    import zipfile

    # 'zipstring_sequence' comes from signature.cpp
    zipbytes = base64.b64decode(''.join(zipstring_sequence))
    fd, fname = tempfile.mkstemp(prefix='embedded.', suffix='.zip')
    os.write(fd, zipbytes)
    os.close(fd)
    # Let us test the zipfile if it really is one.
    # Otherwise, zipimporter would simply ignore it without notice.
    try:
        z = zipfile.ZipFile(fname)
        z.close()
    except zipfile.BadZipFile as e:
        print('Broken Zip File:', e)
        traceback.print_exc(file=sys.stdout)
    finally:
        return fname

# eof
