#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

# This script is used to generate a summary of missing types / classes
# which are present in C++ Qt5, but are missing in PySide6.
#
# Required packages: bs4
# Installed via: pip install bs4
#
# The script uses beautiful soup 4 to parse out the class names from
# the online Qt documentation. It then tries to import the types from
# PySide6.
#
# Example invocation of script:
# python missing_bindings.py --qt-version 5.9 -w all
# --qt-version - specify which version of qt documentation to load.
# -w           - if PyQt5 is an installed package, check if the tested
# class also exists there.

from __future__ import print_function

try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

import argparse
from bs4 import BeautifulSoup
from collections import OrderedDict
from time import gmtime, strftime
import sys
import os.path

modules_to_test = OrderedDict()

# Essentials
modules_to_test['QtCore'] = 'qtcore-module.html'
modules_to_test['QtGui'] = 'qtgui-module.html'
modules_to_test['QtMultimedia'] = 'qtmultimedia-module.html'
modules_to_test['QtMultimediaWidgets'] = 'qtmultimediawidgets-module.html'
modules_to_test['QtNetwork'] = 'qtnetwork-module.html'
modules_to_test['QtQml'] = 'qtqml-module.html'
modules_to_test['QtQuick'] = 'qtquick-module.html'
modules_to_test['QtQuickWidgets'] = 'qtquickwidgets-module.html'
modules_to_test['QtSql'] = 'qtsql-module.html'
modules_to_test['QtTest'] = 'qttest-module.html'
modules_to_test['QtWidgets'] = 'qtwidgets-module.html'

# Addons
modules_to_test['Qt3DCore'] = 'qt3dcore-module.html'
modules_to_test['Qt3DInput'] = 'qt3dinput-module.html'
modules_to_test['Qt3DLogic'] = 'qt3dlogic-module.html'
modules_to_test['Qt3DRender'] = 'qt3drender-module.html'
modules_to_test['Qt3DAnimation'] = 'qt3danimation-module.html'
modules_to_test['Qt3DExtras'] = 'qt3dextras-module.html'
modules_to_test['QtConcurrent'] = 'qtconcurrent-module.html'
#modules_to_test['QtNetworkAuth'] = 'qtnetworkauth-module.html'
modules_to_test['QtHelp'] = 'qthelp-module.html'
modules_to_test['QtLocation'] = 'qtlocation-module.html'
modules_to_test['QtPrintSupport'] = 'qtprintsupport-module.html'
modules_to_test['QtScxml'] = 'qtscxml-module.html'
#modules_to_test['QtSpeech'] = 'qtspeech-module.html'
modules_to_test['QtSvg'] = 'qtsvg-module.html'
modules_to_test['QtUiTools'] = 'qtuitools-module.html'
modules_to_test['QtWebChannel'] = 'qtwebchannel-module.html'
modules_to_test['QtWebEngine'] = 'qtwebengine-module.html'
modules_to_test['QtWebEngineCore'] = 'qtwebenginecore-module.html'
modules_to_test['QtWebEngineWidgets'] = 'qtwebenginewidgets-module.html'
modules_to_test['QtWebSockets'] = 'qtwebsockets-module.html'
modules_to_test['QtMacExtras'] = 'qtmacextras-module.html'
modules_to_test['QtX11Extras'] = 'qtx11extras-module.html'
modules_to_test['QtWinExtras'] = 'qtwinextras-module.html'
modules_to_test['QtXml'] = 'qtxml-module.html'
modules_to_test['QtXmlPatterns'] = 'qtxmlpatterns-module.html'
modules_to_test['QtCharts'] = 'qtcharts-module.html'
modules_to_test['QtDataVisualization'] = 'qtdatavisualization-module.html'
modules_to_test['QtOpenGL'] = 'qtopengl-module.html'
modules_to_test['QtPositioning'] = 'qtpositioning-module.html'
modules_to_test['QtRemoteObjects'] = 'qtremoteobjects-module.html'
modules_to_test['QtScriptTools'] = 'qtscripttools-module.html'
modules_to_test['QtSensors'] = 'qtsensors-module.html'
modules_to_test['QtSerialPort'] = 'qtserialport-module.html'
types_to_ignore = set()
# QtCore
types_to_ignore.add('QFlag')
types_to_ignore.add('QFlags')
types_to_ignore.add('QGlobalStatic')
types_to_ignore.add('QDebug')
types_to_ignore.add('QDebugStateSaver')
types_to_ignore.add('QMetaObject.Connection')
types_to_ignore.add('QPointer')
types_to_ignore.add('QAssociativeIterable')
types_to_ignore.add('QSequentialIterable')
types_to_ignore.add('QStaticPlugin')
types_to_ignore.add('QChar')
types_to_ignore.add('QLatin1Char')
types_to_ignore.add('QHash')
types_to_ignore.add('QMultiHash')
types_to_ignore.add('QLinkedList')
types_to_ignore.add('QList')
types_to_ignore.add('QMap')
types_to_ignore.add('QMultiMap')
types_to_ignore.add('QMap.key_iterator')
types_to_ignore.add('QPair')
types_to_ignore.add('QQueue')
types_to_ignore.add('QScopedArrayPointer')
types_to_ignore.add('QScopedPointer')
types_to_ignore.add('QScopedValueRollback')
types_to_ignore.add('QMutableSetIterator')
types_to_ignore.add('QSet')
types_to_ignore.add('QSet.const_iterator')
types_to_ignore.add('QSet.iterator')
types_to_ignore.add('QExplicitlySharedDataPointer')
types_to_ignore.add('QSharedData')
types_to_ignore.add('QSharedDataPointer')
types_to_ignore.add('QEnableSharedFromThis')
types_to_ignore.add('QSharedPointer')
types_to_ignore.add('QWeakPointer')
types_to_ignore.add('QStack')
types_to_ignore.add('QLatin1String')
types_to_ignore.add('QString')
types_to_ignore.add('QStringRef')
types_to_ignore.add('QStringList')
types_to_ignore.add('QStringMatcher')
types_to_ignore.add('QVarLengthArray')
types_to_ignore.add('QVector')
types_to_ignore.add('QFutureIterator')
types_to_ignore.add('QHashIterator')
types_to_ignore.add('QMutableHashIterator')
types_to_ignore.add('QLinkedListIterator')
types_to_ignore.add('QMutableLinkedListIterator')
types_to_ignore.add('QListIterator')
types_to_ignore.add('QMutableListIterator')
types_to_ignore.add('QMapIterator')
types_to_ignore.add('QMutableMapIterator')
types_to_ignore.add('QSetIterator')
types_to_ignore.add('QMutableVectorIterator')
types_to_ignore.add('QVectorIterator')

# QtGui
types_to_ignore.add('QIconEnginePlugin')
types_to_ignore.add('QImageIOPlugin')
types_to_ignore.add('QGenericPlugin')
types_to_ignore.add('QGenericPluginFactory')
types_to_ignore.add('QGenericMatrix')
types_to_ignore.add('QOpenGLExtraFunctions')
types_to_ignore.add('QOpenGLFunctions')
types_to_ignore.add('QOpenGLFunctions_1_0')
types_to_ignore.add('QOpenGLFunctions_1_1')
types_to_ignore.add('QOpenGLFunctions_1_2')
types_to_ignore.add('QOpenGLFunctions_1_3')
types_to_ignore.add('QOpenGLFunctions_1_4')
types_to_ignore.add('QOpenGLFunctions_1_5')
types_to_ignore.add('QOpenGLFunctions_2_0')
types_to_ignore.add('QOpenGLFunctions_2_1')
types_to_ignore.add('QOpenGLFunctions_3_0')
types_to_ignore.add('QOpenGLFunctions_3_1')
types_to_ignore.add('QOpenGLFunctions_3_2_Compatibility')
types_to_ignore.add('QOpenGLFunctions_3_2_Core')
types_to_ignore.add('QOpenGLFunctions_3_3_Compatibility')
types_to_ignore.add('QOpenGLFunctions_3_3_Core')
types_to_ignore.add('QOpenGLFunctions_4_0_Compatibility')
types_to_ignore.add('QOpenGLFunctions_4_0_Core')
types_to_ignore.add('QOpenGLFunctions_4_1_Compatibility')
types_to_ignore.add('QOpenGLFunctions_4_1_Core')
types_to_ignore.add('QOpenGLFunctions_4_2_Compatibility')
types_to_ignore.add('QOpenGLFunctions_4_2_Core')
types_to_ignore.add('QOpenGLFunctions_4_3_Compatibility')
types_to_ignore.add('QOpenGLFunctions_4_3_Core')
types_to_ignore.add('QOpenGLFunctions_4_4_Compatibility')
types_to_ignore.add('QOpenGLFunctions_4_4_Core')
types_to_ignore.add('QOpenGLFunctions_4_5_Compatibility')
types_to_ignore.add('QOpenGLFunctions_4_5_Core')
types_to_ignore.add('QOpenGLFunctions_ES2')

# QtWidgets
types_to_ignore.add('QItemEditorCreator')
types_to_ignore.add('QStandardItemEditorCreator')
types_to_ignore.add('QStylePlugin')

# QtSql
types_to_ignore.add('QSqlDriverCreator')
types_to_ignore.add('QSqlDriverPlugin')

qt_documentation_website_prefixes = OrderedDict()
qt_documentation_website_prefixes['5.6'] = 'http://doc.qt.io/qt-5.6/'
qt_documentation_website_prefixes['5.8'] = 'http://doc.qt.io/qt-5.8/'
qt_documentation_website_prefixes['5.9'] = 'http://doc.qt.io/qt-5.9/'
qt_documentation_website_prefixes['5.10'] = 'http://doc.qt.io/qt-5.10/'
qt_documentation_website_prefixes['5.11'] = 'http://doc.qt.io/qt-5.11/'
qt_documentation_website_prefixes['5.11'] = 'http://doc.qt.io/qt-5.11/'
qt_documentation_website_prefixes['5.12'] = 'http://doc.qt.io/qt-5.12/'
qt_documentation_website_prefixes['5.13'] = 'http://doc.qt.io/qt-5.13/'
qt_documentation_website_prefixes['5.14'] = 'http://doc.qt.io/qt-5.14/'
qt_documentation_website_prefixes['5.15'] = 'http://doc.qt.io/qt-5/'
qt_documentation_website_prefixes['dev'] = 'http://doc-snapshots.qt.io/qt5-dev/'


def qt_version_to_doc_prefix(version):
    if version in qt_documentation_website_prefixes:
        return qt_documentation_website_prefixes[version]
    else:
        raise RuntimeError("The specified qt version is not supported")


def create_doc_url(module_doc_page_url, version):
    return qt_version_to_doc_prefix(version) + module_doc_page_url

parser = argparse.ArgumentParser()
parser.add_argument("module",
                    default='all',
                    choices=list(modules_to_test.keys()).append('all'),
                    nargs='?',
                    type=str,
                    help="the Qt module for which to get the missing types")
parser.add_argument("--qt-version",
                    "-v",
                    default='5.15',
                    choices=['5.6', '5.9', '5.11', '5.12', '5.13', '5.14', '5.15', 'dev'],
                    type=str,
                    dest='version',
                    help="the Qt version to use to check for types")
parser.add_argument("--which-missing",
                    "-w",
                    default='all',
                    choices=['all', 'in-pyqt', 'not-in-pyqt'],
                    type=str,
                    dest='which_missing',
                    help="Which missing types to show (all, or just those "
                        "that are not present in PyQt)")

args = parser.parse_args()

if hasattr(args, "module") and args.module != 'all':
    saved_value = modules_to_test[args.module]
    modules_to_test.clear()
    modules_to_test[args.module] = saved_value

pyside_package_name = "PySide6"
pyqt_package_name = "PyQt5"

total_missing_types_count = 0
total_missing_types_count_compared_to_pyqt = 0
total_missing_modules_count = 0

wiki_file = open('missing_bindings_for_wiki_qt_io.txt', 'w')
wiki_file.truncate()


def log(*pargs, **kw):
    print(*pargs)

    computed_str = ''
    for arg in pargs:
        computed_str += str(arg)

    style = 'text'
    if 'style' in kw:
        style = kw['style']

    if style == 'heading1':
        computed_str = '= ' + computed_str + ' ='
    elif style == 'heading5':
        computed_str = '===== ' + computed_str + ' ====='
    elif style == 'with_newline':
        computed_str += '\n'
    elif style == 'bold_colon':
        computed_str = computed_str.replace(':', ":'''")
        computed_str += "'''"
        computed_str += '\n'
    elif style == 'error':
        computed_str = "''" + computed_str.strip('\n') + "''\n"
    elif style == 'text_with_link':
        computed_str = computed_str
    elif style == 'code':
        computed_str = ' ' + computed_str
    elif style == 'end':
        return

    print(computed_str, file=wiki_file)

log('PySide6 bindings for Qt {}'.format(args.version), style='heading1')

log("""Using Qt version {} documentation to find public API Qt types and test
if the types are present in the PySide6 package.""".format(args.version))

log("""Results are usually stored at
https://wiki.qt.io/PySide6_Missing_Bindings
so consider taking the contents of the generated
missing_bindings_for_wiki_qt_io.txt file and updating the linked wiki page.""",
style='end')

log("""Similar report:
https://gist.github.com/ethanhs/6c626ca4e291f3682589699296377d3a""",
style='text_with_link')

python_executable = os.path.basename(sys.executable or '')
command_line_arguments = ' '.join(sys.argv)
report_date = strftime("%Y-%m-%d %H:%M:%S %Z", gmtime())

log("""
This report was generated by running the following command:
 {} {}
on the following date:
 {}
""".format(python_executable, command_line_arguments, report_date))

for module_name in modules_to_test.keys():
    log(module_name, style='heading5')

    url = create_doc_url(modules_to_test[module_name], args.version)
    log('Documentation link: {}\n'.format(url), style='text_with_link')

    # Import the tested module
    try:
        pyside_tested_module = getattr(__import__(pyside_package_name,
            fromlist=[module_name]), module_name)
    except Exception as e:
        log('\nCould not load {}.{}. Received error: {}. Skipping.\n'.format(
            pyside_package_name, module_name, str(e).replace("'", '')),
            style='error')
        total_missing_modules_count += 1
        continue

    try:
        pyqt_module_name = module_name
        if module_name == "QtCharts":
            pyqt_module_name = module_name[:-1]

        pyqt_tested_module = getattr(__import__(pyqt_package_name,
            fromlist=[pyqt_module_name]), pyqt_module_name)
    except Exception as e:
        log("\nCould not load {}.{} for comparison. "
            "Received error: {}.\n".format(pyqt_package_name, module_name,
                str(e).replace("'", '')), style='error')

    # Get C++ class list from documentation page.
    page = urllib2.urlopen(url)
    soup = BeautifulSoup(page, 'html.parser')

    #  Extract the Qt type names from the documentation classes table
    links = soup.body.select('.annotated a')
    types_on_html_page = []

    for link in links:
        link_text = link.text
        link_text = link_text.replace('::', '.')
        if link_text not in types_to_ignore:
            types_on_html_page.append(link_text)

    log('Number of types in {}: {}'.format(module_name,
        len(types_on_html_page)), style='bold_colon')

    missing_types_count = 0
    missing_types_compared_to_pyqt = 0
    missing_types = []
    for qt_type in types_on_html_page:
        try:
            pyside_qualified_type = 'pyside_tested_module.'

            if "QtCharts" == module_name:
                pyside_qualified_type += 'QtCharts.'
            elif "DataVisualization" in module_name:
                pyside_qualified_type += 'QtDataVisualization.'

            pyside_qualified_type += qt_type
            eval(pyside_qualified_type)
        except:
            missing_type = qt_type
            missing_types_count += 1
            total_missing_types_count += 1

            is_present_in_pyqt = False
            try:
                pyqt_qualified_type = 'pyqt_tested_module.'

                if "Charts" in module_name:
                    pyqt_qualified_type += 'QtCharts.'
                elif "DataVisualization" in module_name:
                    pyqt_qualified_type += 'QtDataVisualization.'

                pyqt_qualified_type += qt_type
                eval(pyqt_qualified_type)
                missing_type += " (is present in PyQt5)"
                missing_types_compared_to_pyqt += 1
                total_missing_types_count_compared_to_pyqt += 1
                is_present_in_pyqt = True
            except:
                pass

            if args.which_missing == 'all':
                missing_types.append(missing_type)
            elif args.which_missing == 'in-pyqt' and is_present_in_pyqt:
                missing_types.append(missing_type)
            elif (args.which_missing == 'not-in-pyqt' and
                    not is_present_in_pyqt):
                missing_types.append(missing_type)

    if len(missing_types) > 0:
        log('Missing types in {}:'.format(module_name), style='with_newline')
        missing_types.sort()
        for missing_type in missing_types:
            log(missing_type, style='code')
        log('')

    log('Number of missing types: {}'.format(missing_types_count),
        style='bold_colon')
    if len(missing_types) > 0:
        log('Number of missing types that are present in PyQt5: {}'
            .format(missing_types_compared_to_pyqt), style='bold_colon')
        log('End of missing types for {}\n'.format(module_name), style='end')
    else:
        log('', style='end')

log('Summary', style='heading5')
log('Total number of missing types: {}'.format(total_missing_types_count),
    style='bold_colon')
log('Total number of missing types that are present in PyQt5: {}'
    .format(total_missing_types_count_compared_to_pyqt), style='bold_colon')
log('Total number of missing modules: {}'
    .format(total_missing_modules_count), style='bold_colon')
wiki_file.close()
