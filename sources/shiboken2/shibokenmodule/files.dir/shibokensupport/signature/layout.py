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
layout.py

The signature module now has the capability to configure
differently formatted versions of signatures. The default
layout is known from the "__signature__" attribute.

The function "get_signature(ob, modifier=None)" produces the same
signatures by default. By passing different modifiers, you
can select different layouts.

This module configures the different layouts which can be used.
It also implements them in this file. The configurations are
used literally as strings like "signature", "existence", etc.
"""

from textwrap import dedent
from shibokensupport.signature import inspect
from shibokensupport.signature.mapping import ellipsis
from shibokensupport.signature.lib.tool import SimpleNamespace


class SignatureLayout(SimpleNamespace):
    """
    Configure a signature.

    The layout of signatures can have different layouts which are
    controlled by keyword arguments:

    definition=True         Determines if self will generated.
    defaults=True
    ellipsis=False          Replaces defaults by "...".
    return_annotation=True
    parameter_names=True    False removes names before ":".
    """
    allowed_keys = SimpleNamespace(definition=True,
                                   defaults=True,
                                   ellipsis=False,
                                   return_annotation=True,
                                   parameter_names=True)
    allowed_values = True, False

    def __init__(self, **kwds):
        args = SimpleNamespace(**self.allowed_keys.__dict__)
        args.__dict__.update(kwds)
        self.__dict__.update(args.__dict__)
        err_keys = list(set(self.__dict__) - set(self.allowed_keys.__dict__))
        if err_keys:
            self._attributeerror(err_keys)
        err_values = list(set(self.__dict__.values()) - set(self.allowed_values))
        if err_values:
            self._valueerror(err_values)

    def __setattr__(self, key, value):
        if key not in self.allowed_keys.__dict__:
            self._attributeerror([key])
        if value not in self.allowed_values:
            self._valueerror([value])
        self.__dict__[key] = value

    def _attributeerror(self, err_keys):
        err_keys = ", ".join(err_keys)
        allowed_keys = ", ".join(self.allowed_keys.__dict__.keys())
        raise AttributeError(dedent("""\
            Not allowed: '{err_keys}'.
            The only allowed keywords are '{allowed_keys}'.
            """.format(**locals())))

    def _valueerror(self, err_values):
        err_values = ", ".join(map(str, err_values))
        allowed_values = ", ".join(map(str, self.allowed_values))
        raise ValueError(dedent("""\
            Not allowed: '{err_values}'.
            The only allowed values are '{allowed_values}'.
            """.format(**locals())))

# The following names are used literally in this module.
# This way, we avoid the dict hashing problem.
signature = SignatureLayout()

existence = SignatureLayout(definition=False,
                            defaults=False,
                            return_annotation=False,
                            parameter_names=False)

hintingstub = SignatureLayout(ellipsis=True)

typeerror = SignatureLayout(definition=False,
                            return_annotation=False,
                            parameter_names=False)


def define_nameless_parameter():
    """
    Create Nameless Parameters

    A nameless parameter has a reduced string representation.
    This is done by cloning the parameter type and overwriting its
    __str__ method. The inner structure is still a valid parameter.
    """
    def __str__(self):
        # for Python 2, we must change self to be an instance of P
        klass = self.__class__
        self.__class__ = P
        txt = P.__str__(self)
        self.__class__ = klass
        txt = txt[txt.index(":") + 1:].strip() if ":" in txt else txt
        return txt

    P = inspect.Parameter
    newname = "NamelessParameter"
    bases = P.__bases__
    body = dict(P.__dict__) # get rid of mappingproxy
    if "__slots__" in body:
        # __slots__ would create duplicates
        for name in body["__slots__"]:
            del body[name]
    body["__str__"] = __str__
    return type(newname, bases, body)


NamelessParameter = define_nameless_parameter()


def make_signature_nameless(signature):
    """
    Make a Signature Nameless

    We use an existing signature and change the type of its parameters.
    The signature looks different, but is totally intact.
    """
    for key in signature.parameters.keys():
        signature.parameters[key].__class__ = NamelessParameter


def create_signature(props, key):
    if not props:
        # empty signatures string
        return
    if isinstance(props["multi"], list):
        # multi sig: call recursively
        return list(create_signature(elem, key)
                    for elem in props["multi"])
    if type(key) is tuple:
        sig_kind, modifier = key
    else:
        sig_kind, modifier = key, "signature"

    layout = globals()[modifier]  # lookup of the modifier in this module
    if not isinstance(layout, SignatureLayout):
        raise SystemError("Modifiers must be names of a SignatureLayout "
                          "instance")

    # this is the basic layout of a signature
    varnames = props["varnames"]
    if layout.definition:
        if sig_kind == "function":
            pass
        elif sig_kind == "method":
            varnames = ("self",) + varnames
        elif sig_kind == "staticmethod":
            pass
        elif sig_kind == "classmethod":
            varnames = ("klass",) + varnames
        else:
            raise SystemError("Methods must be function, method, staticmethod or "
                              "classmethod")
    # calculate the modifications
    defaults = props["defaults"][:]
    if not layout.defaults:
        defaults = ()
    if layout.ellipsis:
        defaults = (ellipsis,) * len(defaults)
    annotations = props["annotations"].copy()
    if not layout.return_annotation and "return" in annotations:
        del annotations["return"]

    # attach parameters to a fake function and build a signature
    argstr = ", ".join(varnames)
    fakefunc = eval("lambda {}: None".format(argstr))
    fakefunc.__name__ = props["name"]
    fakefunc.__defaults__ = defaults
    fakefunc.__kwdefaults__ = props["kwdefaults"]
    fakefunc.__annotations__ = annotations
    sig = inspect._signature_from_function(inspect.Signature, fakefunc)

    # the special case of nameless parameters
    if not layout.parameter_names:
        make_signature_nameless(sig)
    return sig

# end of file
