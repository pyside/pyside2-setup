#!/usr/bin/env python
# Postinstall script for PySide
#
# TODO:
#   This file can be removed after OSX support
#   is implemented in pyside_build.update_rpath()

import os, sys, traceback, shutil, fnmatch, stat
from os.path import dirname, abspath
from subprocess import Popen, PIPE
import re


def filter_match(name, patterns):
    for pattern in patterns:
        if pattern is None:
            continue
        if fnmatch.fnmatch(name, pattern):
            return True
    return False


def back_tick(cmd, ret_err=False):
    """ Run command `cmd`, return stdout, or stdout, stderr if `ret_err`

    Roughly equivalent to ``check_output`` in Python 2.7

    Parameters
    ----------
    cmd : str
        command to execute
    ret_err : bool, optional
        If True, return stderr in addition to stdout.  If False, just return
        stdout

    Returns
    -------
    out : str or tuple
        If `ret_err` is False, return stripped string containing stdout from
        `cmd`.  If `ret_err` is True, return tuple of (stdout, stderr) where
        ``stdout`` is the stripped stdout, and ``stderr`` is the stripped
        stderr.

    Raises
    ------
    Raises RuntimeError if command returns non-zero exit code
    """
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE, shell=True)
    out, err = proc.communicate()
    if not isinstance(out, str):
        # python 3
        out = out.decode()
        err = err.decode()
    retcode = proc.returncode
    if retcode is None:
        proc.terminate()
        raise RuntimeError(cmd + ' process did not terminate')
    if retcode != 0:
        raise RuntimeError(cmd + ' process returned code %d\n*** %s' %
                           (retcode, err))
    out = out.strip()
    if not ret_err:
        return out
    return out, err.strip()


OSX_OUTNAME_RE = re.compile(r'\(compatibility version [\d.]+, current version '
                        '[\d.]+\)')

def osx_get_install_names(libpath):
    """ Get OSX library install names from library `libpath` using ``otool``

    Parameters
    ----------
    libpath : str
        path to library

    Returns
    -------
    install_names : list of str
        install names in library `libpath`
    """
    out = back_tick('otool -L ' + libpath)
    libs = [line for line in out.split('\n')][1:]
    return [OSX_OUTNAME_RE.sub('', lib).strip() for lib in libs]


OSX_RPATH_RE = re.compile(r"path (.+) \(offset \d+\)")

def osx_get_rpaths(libpath):
    """ Get rpaths from library `libpath` using ``otool``

    Parameters
    ----------
    libpath : str
        path to library

    Returns
    -------
    rpaths : list of str
        rpath values stored in ``libpath``

    Notes
    -----
    See ``man dyld`` for more information on rpaths in libraries
    """
    lines = back_tick('otool -l ' + libpath).split('\n')
    ctr = 0
    rpaths = []
    while ctr < len(lines):
        line = lines[ctr].strip()
        if line != 'cmd LC_RPATH':
            ctr += 1
            continue
        assert lines[ctr + 1].strip().startswith('cmdsize')
        rpath_line = lines[ctr + 2].strip()
        match = OSX_RPATH_RE.match(rpath_line)
        if match is None:
            raise RuntimeError('Unexpected path line: ' + rpath_line)
        rpaths.append(match.groups()[0])
        ctr += 3
    return rpaths


def osx_localize_libpaths(libpath, local_libs, enc_path=None):
    """ Set rpaths and install names to load local dynamic libs at run time

    Use ``install_name_tool`` to set relative install names in `libpath` (as
    named in `local_libs` to be relative to `enc_path`.  The default for
    `enc_path` is the directory containing `libpath`.

    Parameters
    ----------
    libpath : str
        path to library for which to set install names and rpaths
    local_libs : sequence of str
        library (install) names that should be considered relative paths
    enc_path : str, optional
        path that does or will contain the `libpath` library, and to which the
        `local_libs` are relative.  Defaults to current directory containing
        `libpath`.
    """
    if enc_path is None:
        enc_path = abspath(dirname(libpath))
    install_names = osx_get_install_names(libpath)
    need_rpath = False
    for install_name in install_names:
        if install_name[0] in '/@':
            continue
        back_tick('install_name_tool -change %s @rpath/%s %s' %
           (install_name, install_name, libpath))
        need_rpath = True
    if need_rpath and enc_path not in osx_get_rpaths(libpath):
        back_tick('install_name_tool -add_rpath %s %s' %
           (enc_path, libpath))


def post_install_osx():
    # Try to find PySide package
    try:
        import PySide
    except ImportError:
        print("The PySide package not found: %s" % traceback.print_exception(*sys.exc_info()))
        return
    pyside_path = os.path.abspath(os.path.dirname(PySide.__file__))
    print("PySide package found in %s..." % pyside_path)

    pyside_libs = [lib for lib in os.listdir(pyside_path) if filter_match(
                   lib, ["*.so", "*.dylib", "shiboken"])]

    # Update rpath in PySide libs
    for srcname in pyside_libs:
        srcpath = os.path.join(pyside_path, srcname)
        if os.path.isdir(srcpath):
            continue
        if not os.path.exists(srcpath):
            continue
        osx_localize_libpaths(srcpath, pyside_libs, pyside_path)
        print("Patched rpath in %s to %s." % (srcpath, pyside_path))

    # Check PySide installation status
    try:
        from PySide import QtCore
        print("PySide package successfully installed in %s..." % \
            os.path.abspath(os.path.dirname(QtCore.__file__)))
    except ImportError:
        print("The PySide package not installed: %s" % traceback.print_exception(*sys.exc_info()))


if __name__ == '__main__':
    if sys.platform == "darwin":
        post_install_osx()
