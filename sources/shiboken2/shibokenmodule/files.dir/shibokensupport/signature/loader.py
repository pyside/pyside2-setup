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

from __future__ import print_function, absolute_import

"""
loader.py

The loader has to lazy-load the signature module and also provides a few
Python modules to support Python 2.7 .

This version uses both a normal directory, but has also an embedded zip file
as a fallback solution.
"""

import sys
import os
import traceback
import types
from contextlib import contextmanager

"""
A note on the import problem (solved):

During the tests, the shiboken build structure has the layout

    shiboken2/shibokenmodule/shiboken2.abi3.so

and the name "shiboken2" in sys.modules points directly to the binary
file, hiding the outer shiboken2 module.

To fix that, we temporarily remove the binary from sys.path,
do the needed imports and then restore the binary.
This action was put into a context manager for readability.
"""

# On Python 2, we only have ImportError, which is way too coarse.
# When problems occour, please use Python 3, because it has the finer
# ModuleNotFoundError.

try:
    ModuleNotFoundError
except NameError:
    ModuleNotFoundError = ImportError

@contextmanager
def ensure_import_shibokensupport():
    # Make sure that we always have the shibokensupport containing package first.
    # Also remove any prior loaded module of this name, just in case.
    sbk_support_dir = os.path.abspath(os.path.join(__file__, "..", "..", ".."))
    assert os.path.basename(sbk_support_dir) == "files.dir"
    sys.path.insert(0, sbk_support_dir)

    sbk = "shibokensupport"
    if sbk in sys.modules:
        del sys.modules[sbk]
    for key in list(key for key in sys.modules if key.startswith(sbk + ".")):
        del sys.modules[key]
    try:
        import shibokensupport
        yield
    except Exception as e:
        print("Problem importing shibokensupport:")
        print(e)
        traceback.print_exc()
        sys.stdout.flush()
        sys.exit(-1)
    sys.path.remove(sbk_support_dir)


# patching inspect's formatting to keep the word "typing":
def formatannotation(annotation, base_module=None):
    # if getattr(annotation, '__module__', None) == 'typing':
    #     return repr(annotation).replace('typing.', '')
    if isinstance(annotation, type):
        if annotation.__module__ in ('builtins', base_module):
            return annotation.__qualname__
        return annotation.__module__ + '.' + annotation.__qualname__
    return repr(annotation)

# patching __repr__ to disable the __repr__ of typing.TypeVar:
"""
    def __repr__(self):
        if self.__covariant__:
            prefix = '+'
        elif self.__contravariant__:
            prefix = '-'
        else:
            prefix = '~'
        return prefix + self.__name__
"""
def _typevar__repr__(self):
    return "typing." + self.__name__

# Note also that during the tests we have a different encoding that would
# break the Python license decorated files without an encoding line.

# name used in signature.cpp
def create_signature(props, key):
    return layout.create_signature(props, key)

# name used in signature.cpp
def seterror_argument(args, func_name):
    return errorhandler.seterror_argument(args, func_name)

# name used in signature.cpp
def make_helptext(func):
    return errorhandler.make_helptext(func)

with ensure_import_shibokensupport():
    import signature_loader
    import shibokensupport.signature
    shibokensupport.signature.get_signature = signature_loader.get_signature
    del signature_loader  # protect this dir, too?

    if sys.version_info >= (3,):
        import typing
        import inspect
        inspect.formatannotation = formatannotation
    else:
        import inspect
        namespace = inspect.__dict__
        from shibokensupport.signature import typing27 as typing
        typing.__name__ = "typing"
        # Fix the module names in typing if possible. This is important since
        # the typing names should be I/O compatible, so that typing.Dict
        # shows itself as "typing.Dict".
        for name, obj in typing.__dict__.items():
            if hasattr(obj, "__module__"):
                try:
                    obj.__module__ = "typing"
                except (TypeError, AttributeError):
                    pass
        from shibokensupport.signature import backport_inspect as inspect
        _doc = inspect.__doc__
        inspect.__dict__.update(namespace)
        inspect.__doc__ += _doc
        # force inspect to find all attributes. See "heuristic" in pydoc.py!
        inspect.__all__ = list(x for x in dir(inspect) if not x.startswith("_"))
    typing.TypeVar.__repr__ = _typevar__repr__

    def put_into_package(module, package):
        # take the last component of the module name
        name = module.__name__.rsplit(".", 1)[-1]
        # allow access as {package}.typing
        setattr(package, name, module)
        # put into sys.modules as a package to allow all import options
        fullname = "{}.{}".format(package.__name__, name)
        sys.modules[fullname] = module

    put_into_package(typing, shibokensupport.signature)
    put_into_package(inspect, shibokensupport.signature)
    from shibokensupport.signature import mapping
    from shibokensupport.signature import errorhandler
    from shibokensupport.signature import layout
    from shibokensupport.signature.lib import enum_sig
    from shibokensupport.signature.parser import pyside_type_init

# end of file
