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

from __future__ import print_function

from argparse import ArgumentParser, RawTextHelpFormatter
import os
import re
import subprocess
import sys
import warnings

desc = """
Utility script for working with Qt for Python.

Feel free to extend!

qp5_tool.py can be configured by creating a configuration file
in the format key=value:
    "%CONFIGFILE%"

It is possible to use repository-specific values
by adding a key postfixed by a dash and the repository folder base name, eg:
Modules-pyside-setup512=Core,Gui,Widgets,Network,Test

Configuration keys:
Modules          Comma separated list of modules to be built
                 (for --module-subset=)
BuildArguments   Arguments to setup.py
Python           Python executable (Use python_d for debug builds on Windows)

Arbitrary keys can be defined and referenced by $(name):

MinimalModules=Core,Gui,Widgets,Network,Test
Modules=$(MinimalModules),Multimedia
Modules-pyside-setup-minimal=$(MinimalModules)
"""

def which(needle):
    """Perform a path search"""
    needles = [needle]
    if is_windows:
        for ext in ("exe", "bat", "cmd"):
            needles.append("{}.{}".format(needle, ext))

    for path in os.environ.get("PATH", "").split(os.pathsep):
        for n in needles:
            binary = os.path.join(path, n)
            if os.path.isfile(binary):
                return binary
    return None

def execute(args):
    """Execute a command and print to log"""
    log_string = '[{}] {}'.format(os.path.basename(os.getcwd()), ' '.join(args))
    print(log_string)
    exit_code = subprocess.call(args)
    if exit_code != 0:
        raise RuntimeError('FAIL({}): {}'.format(exit_code, log_string))

def run_git(args):
    """Run git in the current directory and its submodules"""
    args.insert(0, git) # run in repo
    execute(args)  # run for submodules
    module_args = [git, "submodule", "foreach"]
    module_args.extend(args)
    execute(module_args)

def expand_reference(dict, value):
    """Expand references to other keys in config files $(name) by value."""
    pattern = re.compile(r"\$\([^)]+\)")
    while True:
       match = pattern.match(value)
       if not match:
           break
       key = match.group(0)[2:-1]
       value = value[:match.start(0)] + dict[key] + value[match.end(0):]
    return value

"""
Config file handling, cache and read function
"""

config_dict = {}

def read_config_file(fileName):
    global config_dict
    keyPattern = re.compile(r'^\s*([A-Za-z0-9\_\-]+)\s*=\s*(.*)$')
    with open(config_file) as f:
        while True:
            line = f.readline().rstrip()
            if not line:
                break
            match = keyPattern.match(line)
            if match:
                key = match.group(1)
                value = match.group(2)
                while value.endswith('\\'):
                    value = value.rstrip('\\')
                    value += f.readline().rstrip()
                config_dict[key] = expand_reference(config_dict, value)

def read_tool_config(key):
    """
    Read a value from the '$HOME/.qp5_tool' configuration file. When given
    a key 'key' for the repository directory '/foo/qt-5', check for the
    repo-specific value 'key-qt5' and then for the general 'key'.
    """
    if not config_dict:
        read_config_file(config_file)
    repo_value = config_dict.get(key + '-' + base_dir)
    return repo_value if repo_value else config_dict.get(key)

def read_config_build_arguments():
    value = read_tool_config('BuildArguments')
    if value:
        return re.split(r'\s+', value)
    return default_build_args;

def read_config_modules_argument():
    value = read_tool_config('Modules')
    if value and value != '' and value != 'all':
        return '--module-subset=' + value
    return None

def read_config_python_binary():
    binary = read_tool_config('Python')
    return binary if binary else 'python'

def get_config_file():
    home = os.getenv('HOME')
    if is_windows:
        # Set a HOME variable on Windows such that scp. etc.
        # feel at home (locating .ssh).
        if not home:
            home = os.getenv('HOMEDRIVE') + os.getenv('HOMEPATH')
            os.environ['HOME'] = home
        user = os.getenv('USERNAME')
        config_file = os.path.join(os.getenv('APPDATA'), config_file_name)
    else:
        user = os.getenv('USER')
        config_dir = os.path.join(home, '.config')
        if os.path.exists(config_dir):
            config_file = os.path.join(config_dir, config_file_name)
        else:
            config_file = os.path.join(home, '.' + config_file_name)
    return config_file

def get_options(desc):
    parser = ArgumentParser(description=desc, formatter_class=RawTextHelpFormatter)
    parser.add_argument('--reset', '-r', action='store_true',
                        help='Git reset hard to upstream state')
    parser.add_argument('--clean', '-c', action='store_true',
                        help='Git clean')
    parser.add_argument('--pull', '-p', action='store_true',
                        help='Git pull')
    parser.add_argument('--build', '-b', action='store_true',
                        help='Build (configure + build)')
    parser.add_argument('--make', '-m', action='store_true', help='Make')
    parser.add_argument('--Make', '-M', action='store_true',
                        help='cmake + Make (continue broken build)')
    parser.add_argument('--version', '-v', action='version', version='%(prog)s 1.0')

    return parser.parse_args()

if __name__ == '__main__':

    git = None
    base_dir = None
    default_build_args = ['--build-tests', '--skip-docs', '--quiet']
    is_windows = sys.platform == 'win32'
    config_file_name = 'qp5_tool.conf'
    config_file = None
    user = None
    default_config_file = """
    Modules=Core,Gui,Widgets,Network,Test,Qml,Quick,Multimedia,MultimediaWidgets
    BuildArguments={}
    # Python executable (python_d for debug builds)
    Python=python
    """

    config_file = get_config_file()
    desc = desc.replace('%CONFIGFILE%', config_file)
    options = get_options(desc)

    git = which('git')
    if git is None:
        warnings.warn('Unable to find git', RuntimeWarning)
        sys.exit(-1)

    if not os.path.exists(config_file):
        print('Create initial config file ', config_file, " ..")
        with open(config_file, 'w') as f:
            f.write(default_config_file.format(' '.join(default_build_args)))

    while not os.path.exists('.gitmodules'):
        cwd = os.getcwd()
        if cwd == '/' or (is_windows and len(cwd) < 4):
            warnings.warn('Unable to find git root', RuntimeWarning)
            sys.exit(-1)
        os.chdir(os.path.dirname(cwd))

    base_dir = os.path.basename(os.getcwd())

    if options.clean:
        run_git(['clean', '-dxf'])

    if options.reset:
        run_git(['reset', '--hard', '@{upstream}'])

    if options.pull:
        run_git(['pull', '--rebase'])

    if options.build or options.make or options.Make:
        arguments = [read_config_python_binary(), 'setup.py', 'install']
        arguments.extend(read_config_build_arguments())
        if options.make or options.Make:
            arguments.extend(['--reuse-build', '--ignore-git'])
            if not options.Make:
                arguments.append('--skip-cmake')
        modules = read_config_modules_argument()
        if modules:
            arguments.append(modules)
        execute(arguments)
    sys.exit(0)
