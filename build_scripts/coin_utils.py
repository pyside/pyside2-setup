#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
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

from .utils import download_and_extract_7z
import sys
import os

# This is a temp hack for 6.0.0 release
# The Qt6 repo is missing correct provisioning of libClang, so we need to
# install it by ourselves
def installLibClang(CI_HOST_OS):
    home = os.path.expanduser("~")
    file = "https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_100-based-mac.7z"
    target = os.path.join(home, "libclang-dynlibs-10.0")
    if sys.platform == "win32":
        file = "https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_100-based-windows-vs2019_64.7z"
    if CI_HOST_OS == "Linux":
        file = "https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_100-based-linux-Rhel7.6-gcc5.3-x86_64.7z"
    try:
        download_and_extract_7z(file, target)
    except RuntimeError as e:
        print("Exception occurred: {}".format(e))
        # It seems on MacOS we keep getting
        # [Errno socket error] [SSL: CERTIFICATE_VERIFY_FAILED] certificate verify failed
        # with https, re-try from other server with http
        file = file.replace("s://download","://master")
        print("One more try from url: {}".format(file))
        download_and_extract_7z(file, target)
    os.environ['LLVM_INSTALL_DIR'] = os.path.join(target, "libclang")
