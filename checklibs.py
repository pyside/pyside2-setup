#############################################################################
##
## Copyright (C) 2017 The Qt Company Ltd.
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

#!/usr/bin/env python
#
# checklibs.py
#
# Check Mach-O dependencies.
#
# See http://www.entropy.ch/blog/Developer/2011/03/05/2011-Update-to-checklibs-Script-for-dynamic-library-dependencies.html
#
# Written by Marc Liyanage <http://www.entropy.ch>
#
#

import subprocess, sys, re, os.path, optparse, collections
from pprint import pprint


class MachOFile:

    def __init__(self, image_path, arch, parent = None):
        self.image_path = image_path
        self._dependencies = []
        self._cache = dict(paths = {}, order = [])
        self.arch = arch
        self.parent = parent
        self.header_info = {}
        self.load_info()
        self.add_to_cache()

    def load_info(self):
        if not self.image_path.exists():
            return
        self.load_header()
        self.load_rpaths()

    def load_header(self):
        # Get the mach-o header info, we're interested in the file type
        # (executable, dylib)
        cmd = 'otool -arch {0} -h "{1}"'
        output = self.shell(cmd, [self.arch, self.image_path.resolved_path],
            fatal = True)
        if not output:
            print("Unable to load mach header for {} ({}), architecture "
                "mismatch? Use --arch option to pick architecture".format(
                self.image_path.resolved_path, self.arch), file=sys.stderr)
            exit()
        (keys, values) = output.splitlines()[2:]
        self.header_info = dict(zip(keys.split(), values.split()))

    def load_rpaths(self):
        output = self.shell('otool -arch {0} -l "{1}"',
            [self.arch, self.image_path.resolved_path], fatal = True)
        # skip file name on first line
        load_commands = re.split('Load command (\d+)', output)[1:]
        self._rpaths = []
        load_commands = collections.deque(load_commands)
        while load_commands:
            load_commands.popleft() # command index
            command = load_commands.popleft().strip().splitlines()
            if command[0].find('LC_RPATH') == -1:
                continue

            path = re.findall('path (.+) \(offset \d+\)$', command[2])[0]
            image_path = self.image_path_for_recorded_path(path)
            image_path.rpath_source = self
            self._rpaths.append(image_path)

    def ancestors(self):
        ancestors = []
        parent = self.parent
        while parent:
            ancestors.append(parent)
            parent = parent.parent

        return ancestors

    def self_and_ancestors(self):
        return [self] + self.ancestors()

    def rpaths(self):
        return self._rpaths

    def all_rpaths(self):
        rpaths = []
        for image in self.self_and_ancestors():
            rpaths.extend(image.rpaths())
        return rpaths

    def root(self):
        if not self.parent:
            return self
        return self.ancestors()[-1]

    def executable_path(self):
        root = self.root()
        if root.is_executable():
            return root.image_path
        return None

    def filetype(self):
        return long(self.header_info.get('filetype', 0))

    def is_dylib(self):
        return self.filetype() == MachOFile.MH_DYLIB

    def is_executable(self):
        return self.filetype() == MachOFile.MH_EXECUTE

    def all_dependencies(self):
        self.walk_dependencies()
        return self.cache()['order']

    def walk_dependencies(self, known = {}):
        if known.get(self.image_path.resolved_path):
            return

        known[self.image_path.resolved_path] = self

        for item in self.dependencies():
            item.walk_dependencies(known)

    def dependencies(self):
        if not self.image_path.exists():
            return []

        if self._dependencies:
            return self._dependencies

        output = self.shell('otool -arch {0} -L "{1}"',
            [self.arch, self.image_path.resolved_path], fatal = True)
        output = [line.strip() for line in output.splitlines()]
        del(output[0])
        if self.is_dylib():
            # In the case of dylibs, the first line is the id line
            del(output[0])

        self._dependencies = []
        for line in output:
            match = re.match('^(.+)\s+(\(.+)\)$', line)
            if not match:
                continue
            recorded_path = match.group(1)
            image_path = self.image_path_for_recorded_path(recorded_path)
            image = self.lookup_or_make_item(image_path)
            self._dependencies.append(image)

        return self._dependencies

    # The root item holds the cache, all lower-level requests bubble up
    # the parent chain
    def cache(self):
        if self.parent:
            return self.parent.cache()
        return self._cache

    def add_to_cache(self):
        cache = self.cache()
        cache['paths'][self.image_path.resolved_path] = self
        cache['order'].append(self)

    def cached_item_for_path(self, path):
        if not path:
            return None
        return self.cache()['paths'].get(path)

    def lookup_or_make_item(self, image_path):
        image = self.cached_item_for_path(image_path.resolved_path)
        if not image: # cache miss
            image = MachOFile(image_path, self.arch, parent = self)
        return image

    def image_path_for_recorded_path(self, recorded_path):
        path = ImagePath(None, recorded_path)

        # handle @executable_path
        if recorded_path.startswith(ImagePath.EXECUTABLE_PATH_TOKEN):
            executable_image_path = self.executable_path()
            if executable_image_path:
                path.resolved_path = os.path.normpath(
                    recorded_path.replace(
                        ImagePath.EXECUTABLE_PATH_TOKEN,
                        os.path.dirname(executable_image_path.resolved_path)))

        # handle @loader_path
        elif recorded_path.startswith(ImagePath.LOADER_PATH_TOKEN):
            path.resolved_path = os.path.normpath(recorded_path.replace(
                ImagePath.LOADER_PATH_TOKEN,
                os.path.dirname(self.image_path.resolved_path)))

        # handle @rpath
        elif recorded_path.startswith(ImagePath.RPATH_TOKEN):
            for rpath in self.all_rpaths():
                resolved_path = os.path.normpath(recorded_path.replace(
                    ImagePath.RPATH_TOKEN, rpath.resolved_path))
                if os.path.exists(resolved_path):
                    path.resolved_path = resolved_path
                    path.rpath_source = rpath.rpath_source
                    break

        # handle absolute path
        elif recorded_path.startswith('/'):
            path.resolved_path = recorded_path

        return path

    def __repr__(self):
        return str(self.image_path)

    def dump(self):
        print(self.image_path)
        for dependency in self.dependencies():
            print('\t{0}'.format(dependency))

    @staticmethod
    def shell(cmd_format, args, fatal = False):
        cmd = cmd_format.format(*args)
        popen = subprocess.Popen(cmd, shell = True, stdout = subprocess.PIPE)
        output = popen.communicate()[0]
        if popen.returncode and fatal:
            print("Nonzero exit status for shell command '{}'".format(cmd),
                file=sys.stderr)
            sys.exit(1)

        return output

    @classmethod
    def architectures_for_image_at_path(cls, path):
        output = cls.shell('file "{}"', [path])
        file_architectures = re.findall(r' executable (\w+)', output)
        ordering = 'x86_64 i386'.split()
        file_architectures = sorted(file_architectures, lambda a, b: cmp(
            ordering.index(a), ordering.index(b)))
        return file_architectures

    MH_EXECUTE = 0x2
    MH_DYLIB = 0x6
    MH_BUNDLE = 0x8


# ANSI terminal coloring sequences
class Color:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    ENDC = '\033[0m'

    @staticmethod
    def red(string):
        return Color.wrap(string, Color.RED)

    @staticmethod
    def blue(string):
        return Color.wrap(string, Color.BLUE)

    @staticmethod
    def wrap(string, color):
        return Color.HEADER + color + string + Color.ENDC


# This class holds path information for a mach-0 image file.
# It holds the path as it was recorded in the loading binary as well as
# the effective, resolved file system path.
# The former can contain @-replacement tokens.
# In the case where the recorded path contains an @rpath token that was
# resolved successfully, we also capture the path of the binary that
# supplied the rpath value that was used.
# That path itself can contain replacement tokens such as @loader_path.
class ImagePath:

    def __init__(self, resolved_path, recorded_path = None):
        self.recorded_path = recorded_path
        self.resolved_path = resolved_path
        self.rpath_source = None

    def __repr__(self):
        description = None

        if self.resolved_equals_recorded() or self.recorded_path == None:
            description = self.resolved_path
        else:
            description = '{0} ({1})'.format(self.resolved_path,
                self.recorded_path)

        if (not self.is_system_location()) and (not self.uses_dyld_token()):
            description = Color.blue(description)

        if self.rpath_source:
            description += ' (rpath source: {0})'.format(
                self.rpath_source.image_path.resolved_path)

        if not self.exists():
            description += Color.red(' (missing)')

        return description

    def exists(self):
        return self.resolved_path and os.path.exists(self.resolved_path)

    def resolved_equals_recorded(self):
        return (self.resolved_path and self.recorded_path and
            self.resolved_path == self.recorded_path)

    def uses_dyld_token(self):
        return self.recorded_path and self.recorded_path.startswith('@')

    def is_system_location(self):
        system_prefixes = ['/System/Library', '/usr/lib']
        for prefix in system_prefixes:
            if self.resolved_path and self.resolved_path.startswith(prefix):
                return True

    EXECUTABLE_PATH_TOKEN = '@executable_path'
    LOADER_PATH_TOKEN = '@loader_path'
    RPATH_TOKEN = '@rpath'


# Command line driver
parser = optparse.OptionParser(
    usage = "Usage: %prog [options] path_to_mach_o_file")
parser.add_option(
    "--arch", dest = "arch", help = "architecture", metavar = "ARCH")
parser.add_option(
    "--all", dest = "include_system_libraries",
    help = "Include system frameworks and libraries", action="store_true")
(options, args) = parser.parse_args()

if len(args) < 1:
    parser.print_help()
    sys.exit(1)

archs = MachOFile.architectures_for_image_at_path(args[0])
if archs and not options.arch:
    print('Analyzing architecture {}, override with --arch if needed'.format(
        archs[0]), file=sys.stderr)
    options.arch = archs[0]

toplevel_image = MachOFile(ImagePath(args[0]), options.arch)

for dependency in toplevel_image.all_dependencies():
    if (dependency.image_path.exists() and
            (not options.include_system_libraries) and
            dependency.image_path.is_system_location()):
        continue

    dependency.dump()
    print("\n")

