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
###############

"""
This is a troubleshooting script that assists finding out which DLLs or
which symbols in a DLL are missing when executing a PySide2 python
script.
It can also be used with any other non Python executable.

Usage: python debug_windows.py
       When no arguments are given the script will try to import
       PySide2.QtCore.

Usage: python debug_windows.py python -c "import PySide2.QtWebEngine"
       python debug_windows.py my_executable.exe arg1 arg2 --arg3=4
       Any arguments given after the script name will be considered
       as the target executable and the arguments passed to that
       executable.

The script requires administrator privileges.

The script uses cdb.exe and gflags.exe under the hood, which are
installed together with the Windows Kit found at:
https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk

"""

from __future__ import print_function

import sys
import re
import subprocess
import ctypes
import logging
import argparse
from os import path
from textwrap import dedent

is_win = sys.platform == "win32"
is_py_3 = sys.version_info[0] == 3
if is_win:
    if is_py_3:
        import winreg
    else:
        import _winreg as winreg
        import exceptions


def get_parser_args():
    desc_msg = "Run an executable under cdb with loader snaps set."
    help_msg = "Pass the executable and the arguments passed to it as a list."
    parser = argparse.ArgumentParser(description=desc_msg)
    parser.add_argument('args', nargs='*', help=help_msg)
    # Prepend -- so that python options like '-c' are ignored by
    # argparse.
    massaged_args = ['--'] + sys.argv[1:]
    return parser.parse_args(massaged_args)


parser_args = get_parser_args()
verbose_log_file_name = path.join(path.dirname(path.abspath(__file__)),
                                  'log_debug_windows.txt')


def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except Exception as e:
        log.error("is_admin: Exception error: {}".format(e))
        return False


def get_verbose_logger():
    handler = logging.FileHandler(verbose_log_file_name, mode='w')
    main_logger = logging.getLogger('main')
    main_logger.setLevel(logging.INFO)
    main_logger.addHandler(handler)
    return main_logger


def get_non_verbose_logger():
    handler = logging.StreamHandler()
    main_logger = logging.getLogger('main.non_verbose')
    main_logger.setLevel(logging.INFO)
    main_logger.addHandler(handler)
    return main_logger


big_log = get_verbose_logger()
log = get_non_verbose_logger()


def sub_keys(key):
    i = 0
    while True:
        try:
            sub_key = winreg.EnumKey(key, i)
            yield sub_key
            i += 1
        except WindowsError as e:
            log.error(e)
            break


def sub_values(key):
    i = 0
    while True:
        try:
            v = winreg.EnumValue(key, i)
            yield v
            i += 1
        except WindowsError as e:
            log.error(e)
            break


def get_installed_windows_kits():
    roots_key = r"SOFTWARE\Microsoft\Windows Kits\Installed Roots"
    log.info("Searching for Windows kits in registry path: "
             "{}".format(roots_key))
    roots = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, roots_key, 0,
                           winreg.KEY_READ)
    kits = []
    pattern = re.compile(r'KitsRoot(\d+)')

    for (name, value, value_type) in sub_values(roots):
        if value_type == winreg.REG_SZ and name.startswith('KitsRoot'):
            match = pattern.search(name)
            if match:
                version = match.group(1)
                kits.append({'version': version, 'value': value})

    if not kits:
        log.error(dedent("""
            No windows kits found in the registry.
            Consider downloading and installing the latest kit, either from
            https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools
            or from
            https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk
        """))
        exit(1)
    return kits


def get_appropriate_kit(kits):
    # Fixme, figure out if there is a more special way to choose a kit
    # and not just latest version.
    log.info("Found Windows kits are: {}".format(kits))
    chosen_kit = {'version': "0", 'value': None}
    for kit in kits:
        if (kit['version'] > chosen_kit['version'] and
                # version 8.1 is actually '81', so consider everything
                # above version 20, as '2.0', etc.
                kit['version'] < "20"):
            chosen_kit = kit
    first_kit = kits[0]
    return first_kit


def get_cdb_and_gflags_path(kits):
    first_kit = get_appropriate_kit(kits)
    first_path_path = first_kit['value']
    log.info('Using kit found at {}'.format(first_path_path))
    bits = 'x64' if (sys.maxsize > 2 ** 32) else 'x32'
    debuggers_path = path.join(first_path_path, 'Debuggers', bits)
    cdb_path = path.join(debuggers_path, 'cdb.exe')
    if not path.exists(cdb_path): # Try for older "Debugging Tools" packages
        debuggers_path = "C:\\Program Files\\Debugging Tools for Windows (x64)"
        cdb_path = path.join(debuggers_path, 'cdb.exe')

    if not path.exists(cdb_path):
        log.error("Couldn't find cdb.exe at: {}.".format(cdb_path))
        exit(1)
    else:
        log.info("Found cdb.exe at: {}.".format(cdb_path))

    gflags_path = path.join(debuggers_path, 'gflags.exe')

    if not path.exists(gflags_path):
        log.error('Couldn\'t find gflags.exe at: {}.'.format(gflags_path))
        exit(1)
    else:
        log.info('Found gflags.exe at: {}.'.format(cdb_path))

    return cdb_path, gflags_path


def toggle_loader_snaps(executable_name, gflags_path, enable=True):
    arg = '+sls' if enable else '-sls'
    gflags_args = [gflags_path, '-i', executable_name, arg]
    try:
        log.info('Invoking gflags: {}'.format(gflags_args))
        output = subprocess.check_output(gflags_args, stderr=subprocess.STDOUT,
                                         universal_newlines=True)
        log.info(output)
    except exceptions.WindowsError as e:
        log.error("\nRunning {} exited with exception: "
                  "\n{}".format(gflags_args, e))
        exit(1)
    except subprocess.CalledProcessError as e:
        log.error("\nRunning {} exited with: {} and stdout was: "
                  "{}".format(gflags_args, e.returncode, e.output))
        exit(1)


def find_error_like_snippets(content):
    snippets = []
    lines = content.splitlines()
    context_lines = 4

    def error_predicate(l):
        # A list of mostly false positives are filtered out.
        # For deeper inspection, the full log exists.
        errors = {'errorhandling',
                  'windowserrorreporting',
                  'core-winrt-error',
                  'RtlSetLastWin32Error',
                  'RaiseInvalid16BitExeError',
                  'BaseWriteErrorElevationRequiredEvent',
                  'for DLL "Unknown"',
                  'LdrpGetProcedureAddress',
                  'X509_STORE_CTX_get_error',
                  'ERR_clear_error',
                  'ERR_peek_last_error',
                  'ERR_error_string',
                  'ERR_get_error',
                  ('ERROR: Module load completed but symbols could '
                   'not be loaded')}
        return (re.search('error', l, re.IGNORECASE)
                and all(e not in errors for e in errors))

    for i in range(1, len(lines)):
        line = lines[i]
        if error_predicate(line):
            snippets.append(lines[i - context_lines:i + context_lines + 1])

    return snippets


def print_error_snippets(snippets):
    if len(snippets) > 0:
        log.info("\nThe following possible errors were found:\n")

        for i in range(1, len(snippets)):
            log.info("Snippet {}:".format(i))
            for line in snippets[i]:
                log.info(line)
        log.info("")


def call_command_under_cdb_with_gflags(executable_path, args):
    executable_name = path.basename(executable_path)
    invocation = [executable_path] + args

    kits = get_installed_windows_kits()
    cdb_path, gflags_path = get_cdb_and_gflags_path(kits)

    toggle_loader_snaps(executable_name, gflags_path, enable=True)

    log.info("Debugging the following command invocation: "
             "{}".format(invocation))

    cdb_args = [cdb_path] + invocation

    log.info('Invoking cdb: {}'.format(cdb_args))

    p = subprocess.Popen(cdb_args,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE,
                         stdin=subprocess.PIPE,
                         shell=False)

    # Symbol fix, start process, print all thread stack traces, exit.
    cdb_commands = ['.symfix', 'g', '!uniqstack', 'q']
    cdb_commands_text = '\n'.join(cdb_commands)
    out, err = p.communicate(input=cdb_commands_text.encode('utf-8'))

    out_decoded = out.decode('utf-8')
    big_log.info('stdout: {}'.format(out_decoded))
    if err:
        big_log.info('stderr: {}'.format(err.decode('utf-8')))

    log.info('Finished execution of process under cdb.')

    toggle_loader_snaps(executable_name, gflags_path, enable=False)

    snippets = find_error_like_snippets(out_decoded)
    print_error_snippets(snippets)

    log.info("Finished processing.\n !!! Full log can be found at: "
             "{}".format(verbose_log_file_name))


def test_run_import_qt_core_under_cdb_with_gflags():
    # The weird characters are there for faster grepping of the output
    # because there is a lot of content in the full log.
    # The 2+2 is just ensure that Python itself works.
    python_code = """
print(">>>>>>>>>>>>>>>>>>>>>>> Test computation of 2+2 is: {}".format(2+2))
import PySide2.QtCore
print(">>>>>>>>>>>>>>>>>>>>>>> QtCore object instance: {}".format(PySide2.QtCore))
"""
    call_command_under_cdb_with_gflags(sys.executable, ["-c", python_code])


def handle_args():
    if not parser_args.args:
        test_run_import_qt_core_under_cdb_with_gflags()
    else:
        call_command_under_cdb_with_gflags(parser_args.args[0],
                                           parser_args.args[1:])


if __name__ == '__main__':
    if not is_win:
        log.error("This script only works on Windows.")
        exit(1)

    if is_admin():
        handle_args()
    else:
        log.error("Please rerun the script with administrator privileges. "
                  "It is required for gflags.exe to work. ")
        exit(1)
