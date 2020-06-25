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

import os
import distutils.log as log


class Config(object):
    def __init__(self):
        # Constants
        self._build_type_all = "all"
        self._invocation_type_top_level = "top-level"
        self._invocation_type_internal = "internal"

        # The keyword arguments which will be given to setuptools.setup
        self.setup_kwargs = {}

        # The setup.py invocation type.
        # top-level
        # internal
        self.invocation_type = None

        # The type of the top-level build.
        # all - build shiboken2 module, shiboken2-generator and PySide2
        #       modules
        # shiboken2 - build only shiboken2 module
        # shiboken2-generator - build only the shiboken2-generator
        # pyside2 - build only PySide2 modules
        self.build_type = None

        # The internal build type, used for internal invocations of
        # setup.py to build a specific module only.
        self.internal_build_type = None

        # Options that can be given to --build-type and
        # --internal-build-type
        self.shiboken_module_option_name = "shiboken2"
        self.shiboken_generator_option_name = "shiboken2-generator"
        self.pyside_option_name = "pyside2"

        # Names to be passed to setuptools.setup() name key,
        # so not package name, but rather project name as it appears
        # in the wheel name and on PyPi.
        self.shiboken_module_st_name = "shiboken2"
        self.shiboken_generator_st_name = "shiboken2-generator"
        self.pyside_st_name = "PySide2"

        # Used by check_allowed_python_version to validate the
        # interpreter version.
        self.python_version_classifiers = [
            'Programming Language :: Python',
            'Programming Language :: Python :: 2',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3',
            'Programming Language :: Python :: 3.5',
            'Programming Language :: Python :: 3.6',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: 3.8',
        ]

        self.setup_script_dir = None

    def init_config(self, build_type=None, internal_build_type=None,
                    cmd_class_dict=None, package_version=None,
                    ext_modules=None, setup_script_dir=None,
                    quiet=False):
        """
        Sets up the global singleton config which is used in many parts
        of the setup process.
        """

        # if --internal-build-type was passed, it means that this is a
        # sub-invocation to build a specific package.
        if internal_build_type:
            self.set_is_internal_invocation()
            self.set_internal_build_type(internal_build_type)
        else:
            self.set_is_top_level_invocation()

        # --build-type was specified explicitly, so set it. Otherwise
        # default to all.
        if build_type:
            self.build_type = build_type
        else:
            self.build_type = self._build_type_all

        self.setup_script_dir = setup_script_dir

        setup_kwargs = {}
        setup_kwargs['long_description'] = self.get_long_description()
        setup_kwargs['long_description_content_type'] = 'text/markdown'
        setup_kwargs['keywords'] = 'Qt'
        setup_kwargs['author'] = 'Qt for Python Team'
        setup_kwargs['author_email'] = 'pyside@qt-project.org'
        setup_kwargs['url'] = 'https://www.pyside.org'
        setup_kwargs['download_url'] = 'https://download.qt.io/official_releases/QtForPython'
        setup_kwargs['license'] = 'LGPL'
        setup_kwargs['zip_safe'] = False
        setup_kwargs['cmdclass'] = cmd_class_dict
        setup_kwargs['version'] = package_version
        setup_kwargs['python_requires'] = ">=2.7, !=3.0.*, !=3.1.*, !=3.2.*, !=3.3.*, !=3.4.*, <3.9"


        if quiet:
            # Tells distutils / setuptools to be quiet, and only print warnings or errors.
            # Makes way less noise in the terminal when building.
            setup_kwargs['verbose'] = 0

        # Setting these two keys is still a bit of a discussion point.
        # In general not setting them will allow using "build" and
        # "bdist_wheel" just fine. What they do, is they specify to the
        # setuptools.command.build_py command that certain pure python
        # modules (.py files) exist in the specified package location,
        # and that they should be copied over to the setuptools build
        # dir.
        # But it doesn't really make sense for us, because we copy all
        # the necessary files to the build dir via prepare_packages()
        # function anyway.
        # If we don't set them, the build_py sub-command will be
        # skipped, but the build command will still be executed, which
        # is where we run cmake / make.
        # The only plausible usage of it, is if we will implement a
        # correctly functioning setup.py develop command (or bdist_egg).
        # But currently that doesn't seem to work.
        setup_kwargs['packages'] = self.get_setup_tools_packages_for_current_build()
        setup_kwargs['package_dir'] = self.get_package_name_to_dir_path_mapping()

        # Add a bogus extension module (will never be built here since
        # we are overriding the build command to do it using cmake) so
        # things like bdist_egg will know that there are extension
        # modules and will name the dist with the full platform info.
        setup_kwargs['ext_modules'] = ext_modules

        common_classifiers = [
            'Development Status :: 5 - Production/Stable',
            'Environment :: Console',
            'Environment :: MacOS X',
            'Environment :: X11 Applications :: Qt',
            'Environment :: Win32 (MS Windows)',
            'Intended Audience :: Developers',
            'License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)',
            'License :: Other/Proprietary License',
            'Operating System :: MacOS :: MacOS X',
            'Operating System :: POSIX',
            'Operating System :: POSIX :: Linux',
            'Operating System :: Microsoft',
            'Operating System :: Microsoft :: Windows',
            'Programming Language :: C++']
        common_classifiers.extend(self.python_version_classifiers)
        common_classifiers.extend([
            'Topic :: Database',
            'Topic :: Software Development',
            'Topic :: Software Development :: Code Generators',
            'Topic :: Software Development :: Libraries :: Application Frameworks',
            'Topic :: Software Development :: User Interfaces',
            'Topic :: Software Development :: Widget Sets'])
        setup_kwargs['classifiers'] = common_classifiers

        if self.internal_build_type == self.shiboken_module_option_name:
            setup_kwargs['name'] = self.shiboken_module_st_name
            setup_kwargs['description'] = "Python / C++ bindings helper module"
            setup_kwargs['entry_points'] = {}

        elif self.internal_build_type == self.shiboken_generator_option_name:
            setup_kwargs['name'] = self.shiboken_generator_st_name
            setup_kwargs['description'] = "Python / C++ bindings generator"
            setup_kwargs['install_requires'] = ["{}=={}".format(self.shiboken_module_st_name, package_version)]
            setup_kwargs['entry_points'] = {
                'console_scripts': [
                    'shiboken2 = {}.scripts.shiboken_tool:main'.format(self.package_name()),
                ]
            }

        elif self.internal_build_type == self.pyside_option_name:
            setup_kwargs['name'] = self.pyside_st_name
            setup_kwargs['description'] = "Python bindings for the Qt cross-platform application and UI framework"
            setup_kwargs['install_requires'] = ["{}=={}".format(self.shiboken_module_st_name, package_version)]
            setup_kwargs['entry_points'] = {
                'console_scripts': [
                    'pyside2-uic = {}.scripts.pyside_tool:uic'.format(self.package_name()),
                    'pyside2-rcc = {}.scripts.pyside_tool:rcc'.format(self.package_name()),
                    'pyside2-designer= {}.scripts.pyside_tool:designer'.format(self.package_name()),
                    'pyside2-lupdate = {}.scripts.pyside_tool:main'.format(self.package_name()),
                ]
            }
        self.setup_kwargs = setup_kwargs

    def get_long_description(self):
        readme_filename = 'README.md'
        changes_filename = 'CHANGES.rst'

        if self.is_internal_shiboken_module_build():
            readme_filename = 'README.shiboken2.md'
        elif self.is_internal_shiboken_generator_build():
            readme_filename = 'README.shiboken2-generator.md'
        elif self.is_internal_pyside_build():
            readme_filename = 'README.pyside2.md'

        content = ''
        changes = ''
        try:
            with open(os.path.join(self.setup_script_dir, readme_filename)) as f:
                readme = f.read()
        except Exception as e:
            log.error("Couldn't read contents of {}.".format(readme_filename))
            raise

        # Don't include CHANGES.rst for now, because we have not decided
        # how to handle change files yet.
        include_changes = False
        if include_changes:
            try:
                with open(os.path.join(self.setup_script_dir, changes_filename)) as f:
                    changes = f.read()
            except Exception as e:
                log.error("Couldn't read contents of {}".format(changes_filename))
                raise
        content += readme

        if changes:
            content += "\n\n" + changes

        return content

    def package_name(self):
        """
        Returns package name as it appears in Python's site-packages
        directory.

        Package names can only be delimited by underscores, and not by
        dashes.
        """
        if self.is_internal_shiboken_module_build():
            return "shiboken2"
        elif self.is_internal_shiboken_generator_build():
            return "shiboken2_generator"
        elif self.is_internal_pyside_build():
            return "PySide2"
        else:
            return None

    def get_setup_tools_packages_for_current_build(self):
        """
        Returns a list of packages for setup tools to consider in the
        build_py command, so that it can copy the pure python files.
        Not really necessary because it's done in prepare_packages()
        anyway.

        This is really just to satisfy some checks in setuptools
        build_py command, and if we ever properly implement the develop
        command.
        """
        if self.internal_build_type == self.pyside_option_name:
            return [
                config.package_name(),
            ]
        elif self.internal_build_type == self.shiboken_module_option_name:
            return [self.package_name()]
        else:
            return []

    def get_package_name_to_dir_path_mapping(self):
        """
        Used in setuptools.setup 'package_dir' argument to specify where
        the actual module packages are located.

        For example when building the shiboken module, setuptools will
        expect to find the "shiboken2" module sources under
        "sources/shiboken2/shibokenmodule".

        This is really just to satisfy some checks in setuptools
        build_py command, and if we ever properly implement the develop
        command.
        """
        if self.is_internal_shiboken_module_build():
            return {
                self.package_name(): "sources/shiboken2/shibokenmodule"
            }
        elif self.is_internal_shiboken_generator_build():
            # This is left empty on purpose, because the shiboken
            # generator doesn't have a python module for now.
            return {}
        elif self.is_internal_pyside_build():
            return {
                self.package_name(): "sources/pyside2/PySide2",
            }
        else:
            return {}

    def get_buildable_extensions(self):
        """
        Used by PysideBuild.run to build the CMake projects.
        :return: A list of directory names under the sources directory.
        """
        if self.is_internal_shiboken_module_build() or self.is_internal_shiboken_generator_build():
            return ['shiboken2']
        elif self.is_internal_pyside_build():
            return ['pyside2']
        return None

    def set_is_top_level_invocation(self):
        self.invocation_type = self._invocation_type_top_level

    def set_is_internal_invocation(self):
        self.invocation_type = self._invocation_type_internal

    def is_top_level_invocation(self):
        return self.invocation_type == self._invocation_type_top_level

    def is_internal_invocation(self):
        return self.invocation_type == self._invocation_type_internal

    def is_top_level_build_all(self):
        return self.build_type == self._build_type_all

    def is_top_level_build_shiboken_module(self):
        return self.build_type == self.shiboken_module_option_name

    def is_top_level_build_shiboken_generator(self):
        return self.build_type == self.shiboken_generator_option_name

    def is_top_level_build_pyside(self):
        return self.build_type == self.pyside_option_name

    def set_internal_build_type(self, internal_build_type):
        self.internal_build_type = internal_build_type

    def is_internal_shiboken_module_build(self):
        return self.internal_build_type == self.shiboken_module_option_name

    def is_internal_shiboken_generator_build(self):
        return self.internal_build_type == self.shiboken_generator_option_name

    def is_internal_pyside_build(self):
        return self.internal_build_type == self.pyside_option_name

    def is_internal_shiboken_generator_build_and_part_of_top_level_all(self):
        """
        Used to skip certain build rules and output, when we know that
        the CMake build of shiboken was already done as part of the
        top-level "all" build when shiboken2-module was built.
        """
        return self.is_internal_shiboken_generator_build() and self.is_top_level_build_all()

    def get_allowed_top_level_build_values(self):
        return [
            self._build_type_all,
            self.shiboken_module_option_name,
            self.shiboken_generator_option_name,
            self.pyside_option_name
        ]

    def get_allowed_internal_build_values(self):
        return [
            self.shiboken_module_option_name,
            self.shiboken_generator_option_name,
            self.pyside_option_name
        ]


config = Config()
