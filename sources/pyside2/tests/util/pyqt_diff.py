#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of the Qt for Python project.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from __future__ import print_function

'''Script to show the difference between PyQt5 and ours'''

import sys

from color import print_colored

def check_module_diff(module_name):
    '''Difference between PySide2 and PyQt5 versions of qt bindings.
    Returns a tuple with the members present only on PySide2 and only on PyQt5'''
    shiboken_module = getattr(__import__('PySide2.' + module_name), module_name)
    orig_module = getattr(__import__('PyQt5.' + module_name), module_name)

    shiboken_set = set(dir(shiboken_module))
    orig_set = set(dir(orig_module))

    return sorted(shiboken_set - orig_set), sorted(orig_set - shiboken_set)


def main(argv=None):
    if argv is None:
        argv = sys.argv

    module_name = argv[1] if len(argv) >= 2 else 'QtCore'

    only_shiboken, only_orig = check_module_diff(module_name)

    print_colored('Only on Shiboken version')
    print(only_shiboken)

    print_colored('Only on SIP version')
    print(only_orig)

if __name__ == '__main__':
    main()
