#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
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
Tool to run qtattributionsscanner and convert its output to rst
"""

import os, json, subprocess, sys

def indent(lines, indent):
    result = ''
    for l in lines:
        result += "{}{}\n".format(indent, l)
    return result

rstHeader="""Licenses Used in Qt for Python
******************************

Qt for Python contains some code that is not provided under the
GNU Lesser General Public License (LGPL) or the Qt Commercial License,
but rather under specific licenses from the original authors.

Qt for Python contains some code that is not provided under the
GNU Lesser General Public License (LGPL) or the Qt Commercial License,
but rather under specific licenses from the original authors.
The Qt Company gratefully acknowledges these and other contributions
to Qt for Python. We recommend that programs that use Qt for Python
also acknowledge these contributions, and quote these license
statements in an appendix to the documentation.

Note: You only need to comply with (and acknowledge) the licenses of
the third-party components that you are using with your application.
Click the name of the component to see the licensing details.

Third-party Licenses
^^^^^^^^^^^^^^^^^^^^

The following table lists parts of Qt for Python that incorporate code
licensed under third-party open-source licenses:

"""

rstEntryFormat = """{}

{}

{}

{}

`Project Homepage <{}>`_ , upstream version: {}

::

{}


"""

# Extract the license which is embedded in triple quotes
def extractLicense(fileName):
    result = []
    with open(fileName, 'r') as file:
        recording = False
        for line in file.readlines():
            line = line.strip("\n\r\xc2\xb6")
            if line == '"""':
                if recording:
                    break
                else:
                    recording = True
            elif recording:
               result.append(line)
    return result

def runScanner(directory, targetFileName):
    # qtattributionsscanner recursively searches for qt_attribution.json files
    # and outputs them in JSON with the paths of the 'LicenseFile' made absolute
    command = 'qtattributionsscanner --output-format json {}'.format(directory)
    jsonS = subprocess.check_output(command, shell=True)
    if not jsonS:
        raise RuntimeError('{} failed to produce output.'.format(command))

    with open(targetFileName, 'w') as targetFile:
        targetFile.write(rstHeader)
        for entry in json.loads(jsonS):
            name = entry['Name']
            title = "{}\n{}".format(name, '-' * len(name))
            brief = entry['License']
            url = entry['Homepage']
            version = entry['Version']
            description = entry['Description']
            qtUsage = entry['QtUsage']
            content = extractLicense(entry['LicenseFile'])
            rst = rstEntryFormat.format(title, brief, description, qtUsage,
                                        url, version, indent(content, '    '))
            targetFile.write(rst)

if len(sys.argv) < 3:
    print("Usage: qtattributionsscannertorst [directory] [file]'")
    sys.exit(0)

directory = sys.argv[1]
targetFileName = sys.argv[2]
runScanner(directory, targetFileName)
