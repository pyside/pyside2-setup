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


wheel_module_exists = False

try:
    import os, sys

    from distutils import log
    from wheel import pep425tags
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
    from wheel.bdist_wheel import safer_name as _safer_name
    from wheel.pep425tags import get_abbr_impl, get_impl_ver, get_abi_tag
    from wheel.pep425tags import get_platform as wheel_get_platform
    from email.generator import Generator
    from wheel import __version__ as wheel_version

    from .options import *

    wheel_module_exists = True
except Exception as e:
    print('***** Exception while trying to prepare bdist_wheel override class: {}. Skipping wheel overriding.'.format(e))

def get_bdist_wheel_override(params):
    if wheel_module_exists:
        class PysideBuildWheelDecorated(PysideBuildWheel):
            def __init__(self, *args, **kwargs):
                self.params = params
                PysideBuildWheel.__init__(self, *args, **kwargs)
        return PysideBuildWheelDecorated
    else:
        return None

if wheel_module_exists:
    class PysideBuildWheel(_bdist_wheel):
        def __init__(self, *args, **kwargs):
            self.pyside_params = None
            _bdist_wheel.__init__(self, *args, **kwargs)

        @property
        def wheel_dist_name(self):
            # Slightly modified version of wheel's wheel_dist_name
            # method, to add the Qt version as well.
            # Example:
            #   PySide2-5.6-5.6.4-cp27-cp27m-macosx_10_10_intel.whl
            # The PySide2 version is "5.6".
            # The Qt version built against is "5.6.4".
            qt_version = self.params['qt_version']
            package_version = self.params['package_version']
            wheel_version = "{}-{}".format(package_version, qt_version)
            components = (_safer_name(self.distribution.get_name()),
                wheel_version)
            if self.build_number:
                components += (self.build_number,)
            return '-'.join(components)

        # Copy of get_tag from bdist_wheel.py, to allow setting a
        # multi-python impl tag, by removing an assert. Otherwise we
        # would have to rename wheels manually for limited api
        # packages. Also we set "none" abi tag on Windows, because
        # pip does not yet support "abi3" tag, leading to
        # installation failure when tried.
        def get_tag(self):
            # bdist sets self.plat_name if unset, we should only use
            # it for purepy wheels if the user supplied it.
            if self.plat_name_supplied:
                plat_name = self.plat_name
            elif self.root_is_pure:
                plat_name = 'any'
            else:
                plat_name = self.plat_name or wheel_get_platform()
                if plat_name in ('linux-x86_64', 'linux_x86_64') and sys.maxsize == 2147483647:
                    plat_name = 'linux_i686'

                # To allow uploading to pypi, we need the wheel name
                # to contain 'manylinux1'.
                # The wheel which will be uploaded to pypi will be
                # built on RHEL7, so it doesn't completely qualify for
                # manylinux1 support, but it's the minimum requirement
                # for building Qt. We only enable this for x64 limited
                # api builds (which are the only ones uploaded to
                # pypi).
                # TODO: Add actual distro detection, instead of
                # relying on limited_api option.
                if plat_name in ('linux-x86_64', 'linux_x86_64') and sys.maxsize > 2147483647 \
                             and self.py_limited_api:
                    plat_name = 'manylinux1_x86_64'
            plat_name = plat_name.replace('-', '_').replace('.', '_')

            if self.root_is_pure:
                if self.universal:
                    impl = 'py2.py3'
                else:
                    impl = self.python_tag
                tag = (impl, 'none', plat_name)
            else:
                impl_name = get_abbr_impl()
                impl_ver = get_impl_ver()
                impl = impl_name + impl_ver
                # We don't work on CPython 3.1, 3.0.
                if self.py_limited_api and (impl_name + impl_ver).startswith('cp3'):
                    impl = self.py_limited_api
                    if sys.platform == "win32":
                        abi_tag = 'none'
                    else:
                        abi_tag = 'abi3'
                else:
                    abi_tag = str(get_abi_tag()).lower()
                tag = (impl, abi_tag, plat_name)
                supported_tags = pep425tags.get_supported(
                    supplied_platform=plat_name if self.plat_name_supplied else None)
                # XXX switch to this alternate implementation for
                # non-pure:
                if not self.py_limited_api:
                    assert tag == supported_tags[0], "%s != %s" % (tag, supported_tags[0])
                    assert tag in supported_tags, \
                                  "would build wheel with unsupported tag {}".format(tag)
            return tag

        # Copy of get_tag from bdist_wheel.py, to write a triplet Tag
        # only once for the limited_api case.
        def write_wheelfile(self, wheelfile_base, generator='bdist_wheel (' + wheel_version + ')'):
            from email.message import Message
            msg = Message()
            msg['Wheel-Version'] = '1.0'  # of the spec
            msg['Generator'] = generator
            msg['Root-Is-Purelib'] = str(self.root_is_pure).lower()
            if self.build_number is not None:
                msg['Build'] = self.build_number

            # Doesn't work for bdist_wininst
            impl_tag, abi_tag, plat_tag = self.get_tag()
            limited_api_enabled = OPTION_LIMITED_API and sys.version_info[0] >= 3

            def writeTag(impl):
                for abi in abi_tag.split('.'):
                    for plat in plat_tag.split('.'):
                        msg['Tag'] = '-'.join((impl, abi, plat))
            if limited_api_enabled:
                writeTag(impl_tag)
            else:
                for impl in impl_tag.split('.'):
                    writeTag(impl)

            wheelfile_path = os.path.join(wheelfile_base, 'WHEEL')
            log.info('creating %s', wheelfile_path)
            with open(wheelfile_path, 'w') as f:
                Generator(f, maxheaderlen=0).flatten(msg)

        def finalize_options(self):
            if sys.platform == 'darwin':
                # Override the platform name to contain the correct
                # minimum deployment target.
                # This is used in the final wheel name.
                self.plat_name = self.params['macos_plat_name']

            # When limited API is requested, notify bdist_wheel to
            # create a properly named package.
            limited_api_enabled = OPTION_LIMITED_API and sys.version_info[0] >= 3
            if limited_api_enabled:
                self.py_limited_api = "cp35.cp36.cp37.cp38"

            _bdist_wheel.finalize_options(self)
