#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
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

This file was originally directly embedded into the C source.
After it grew more and more, I now prefer to have it as Python file.
The remaining stub loader in the C source is now only a short string.

This version does no longer use an embedded .zip file but is a package.
The old code without a package but with zip compression can still be found
at https://codereview.qt-project.org/#/c/203533/ for reference.
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
def ensure_import_support():
    # Make sure that we always have the shiboken containing package first.
    # This is sometimes hidden by the ctest paths.
    # We adjust the path in a way that the support folder comes first.
    # This can be in "shiboken2/support" or in "shibokenmodule/support",
    # so we use the "support" folder as toplevel.
    sbk_support_dir = os.path.abspath(os.path.join(__file__, "..", "..", ".."))
    sys.path.insert(0, sbk_support_dir)
    sbk = "shiboken2"
    save_sbk = sys.modules.pop(sbk) if sbk in sys.modules else None
    # make sure that we get at the support folder
    try:
        import support
        yield
    except Exception as e:
        print("Problem importing support:")
        print(e)
        traceback.print_exc()
        sys.stdout.flush()
        sys.exit(-1)
    if save_sbk:
        sys.modules[sbk] = save_sbk
    sys.path.pop(0)


# patching inspect's formatting to keep the word "typing":
def formatannotation(annotation, base_module=None):
    # if getattr(annotation, '__module__', None) == 'typing':
    #     return repr(annotation).replace('typing.', '')
    if isinstance(annotation, type):
        if annotation.__module__ in ('builtins', base_module):
            return annotation.__qualname__
        return annotation.__module__+'.'+annotation.__qualname__
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

with ensure_import_support():
    # We store all needed modules in signature_loader.
    # This way, they are always accessible.
    import signature_loader

    if sys.version_info >= (3,):
        import typing
        import inspect
        inspect.formatannotation = formatannotation
    else:
        import inspect
        namespace = inspect.__dict__
        from support.signature import typing27 as typing
        typing.__name__ = "typing"
        from support.signature import backport_inspect as inspect
        _doc = inspect.__doc__
        inspect.__dict__.update(namespace)
        inspect.__doc__ += _doc
        # force inspect to find all attributes. See "heuristic" in pydoc.py!
        inspect.__all__ = list(x for x in dir(inspect) if not x.startswith("_"))
    typing.TypeVar.__repr__ = _typevar__repr__

    def put_into_loader_package(module, loader=signature_loader):
        # Note: the "with" statement hides that we are no longer in a
        # global context, but inside ensure_import_support. Therefore,
        # we need to explicitly pass the signature_loader in.

        # take the last component of the module name
        name = module.__name__.rsplit(".", 1)[-1]
        # allow access as signature_loader.typing
        setattr(loader, name, module)
        # put into sys.modules as a package to allow all import options
        fullname = "{}.{}".format(loader.__name__, name)
        sys.modules[fullname] = module

    put_into_loader_package(typing)
    put_into_loader_package(inspect)
    from support.signature import mapping as sbk_mapping
    sbk_mapping.__name__ = "sbk_mapping"
    put_into_loader_package(sbk_mapping)
    # We may or may not use PySide.
    try:
        from PySide2.support.signature import mapping
    except ModuleNotFoundError:
        mapping = sbk_mapping
        mapping.__name__ = "mapping"
    put_into_loader_package(mapping)
    from support.signature import errorhandler
    put_into_loader_package(errorhandler)
    from support.signature import layout
    put_into_loader_package(layout)
    from support.signature.lib import enum_sig
    put_into_loader_package(enum_sig)
    from support.signature.parser import pyside_type_init


# Note also that during the tests we have a different encoding that would
# break the Python license decorated files without an encoding line.

# name used in signature.cpp
def create_signature(props, key):
    return layout.create_signature(props, key)

# name used in signature.cpp
def seterror_argument(args, func_name):
    return errorhandler.seterror_argument(args, func_name)

# end of file
