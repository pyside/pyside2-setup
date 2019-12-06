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
tool.py

Some useful stuff, see below.
On the function with_metaclass see the answer from Martijn Pieters on
https://stackoverflow.com/questions/18513821/python-metaclass-understanding-the-with-metaclass
"""

from textwrap import dedent


class SimpleNamespace(object):
    # From types.rst, because the builtin is implemented in Python 3, only.
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

    def __repr__(self):
        keys = sorted(self.__dict__)
        items = ("{}={!r}".format(k, self.__dict__[k]) for k in keys)
        return "{}({})".format(type(self).__name__, ", ".join(items))

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

try:
    from types import SimpleNamespace
except ImportError:
    pass


def build_brace_pattern(level, separators=""):
    """
    Build a brace pattern upto a given depth

    The brace pattern parses any pattern with round, square, curly, or angle
    brackets. Inside those brackets, any characters are allowed.

    The structure is quite simple and is recursively repeated as needed.
    When separators are given, the match stops at that separator.

    Reason to use this instead of some Python function:
    The resulting regex is _very_ fast!

    A faster replacement would be written in C, but this solution is
    sufficient when the nesting level is not too large.

    Because of the recursive nature of the pattern, the size grows by a factor
    of 4 at every level, as does the creation time. Up to a level of 6, this
    is below 10 ms.

    There are other regex engines available which allow recursive patterns,
    avoiding this problem completely. It might be considered to switch to
    such an engine if the external module is not a problem.
    """
    def escape(str):
        return "".join("\\" + c for c in str)

    ro, rc = round  = "()"
    so, sc = square = "[]"
    co, cc = curly  = "CD"      # we insert "{}", later...
    ao, ac = angle  = "<>"
    qu, bs = '"', "\\"
    all = round + square + curly + angle
    __ = "  "
    ro, rc, so, sc, co, cc, ao, ac, separators, qu, bs, all = map(
        escape, (ro, rc, so, sc, co, cc, ao, ac, separators, qu, bs, all))

    no_brace_sep_q = r"[^{all}{separators}{qu}{bs}]".format(**locals())
    no_quote = r"(?: [^{qu}{bs}] | {bs}. )*".format(**locals())
    pattern = dedent(r"""
        (
          (?: {__} {no_brace_sep_q}
            | {qu} {no_quote} {qu}
            | {ro} {replacer} {rc}
            | {so} {replacer} {sc}
            | {co} {replacer} {cc}
            | {ao} {replacer} {ac}
          )*
        )
        """)
    no_braces_q = "[^{all}{qu}{bs}]*".format(**locals())
    repeated = dedent(r"""
        {indent}  (?: {__} {no_braces_q}
        {indent}    | {qu} {no_quote} {qu}
        {indent}    | {ro} {replacer} {rc}
        {indent}    | {so} {replacer} {sc}
        {indent}    | {co} {replacer} {cc}
        {indent}    | {ao} {replacer} {ac}
        {indent}  )*
        """)
    for idx in range(level):
        pattern = pattern.format(replacer = repeated if idx < level-1 else no_braces_q,
                                 indent = idx * "    ", **locals())
    return pattern.replace("C", "{").replace("D", "}")


# Copied from the six module:
def with_metaclass(meta, *bases):
    """Create a base class with a metaclass."""
    # This requires a bit of explanation: the basic idea is to make a dummy
    # metaclass for one level of class instantiation that replaces itself with
    # the actual metaclass.
    class metaclass(type):

        def __new__(cls, name, this_bases, d):
            return meta(name, bases, d)

        @classmethod
        def __prepare__(cls, name, this_bases):
            return meta.__prepare__(name, bases)
    return type.__new__(metaclass, 'temporary_class', (), {})

# eof
