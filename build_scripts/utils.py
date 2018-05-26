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

import sys
import os
import stat
import re
import stat
import errno
import time
import shutil
import subprocess
import fnmatch
import glob
import itertools
import popenasync
import glob

# There is no urllib.request in Python2
try:
    import urllib.request as urllib
except ImportError:
    import urllib

from distutils import log
from distutils.errors import DistutilsOptionError
from distutils.errors import DistutilsSetupError
from distutils.spawn import spawn
from distutils.spawn import DistutilsExecError

try:
    WindowsError
except NameError:
    WindowsError = None


def has_option(name):
    try:
        sys.argv.remove("--{}".format(name))
        return True
    except ValueError:
        pass
    return False


def option_value(name):
    for index, option in enumerate(sys.argv):
        if option == '--' + name:
            if index+1 >= len(sys.argv):
                raise DistutilsOptionError("The option {} requires a "
                    "value".format(option))
            value = sys.argv[index+1]
            sys.argv[index:index+2] = []
            return value
        if option.startswith('--' + name + '='):
            value = option[len(name)+3:]
            sys.argv[index:index+1] = []
            return value
    env_val = os.getenv(name.upper().replace('-', '_'))
    return env_val


def filter_match(name, patterns):
    for pattern in patterns:
        if pattern is None:
            continue
        if fnmatch.fnmatch(name, pattern):
            return True
    return False


def update_env_path(newpaths):
    paths = os.environ['PATH'].lower().split(os.pathsep)
    for path in newpaths:
        if not path.lower() in paths:
            log.info("Inserting path '{}' to environment".format(path))
            paths.insert(0, path)
            os.environ['PATH'] = path + os.pathsep + os.environ['PATH']


def winsdk_setenv(platform_arch, build_type):
    from distutils.msvc9compiler import VERSION as MSVC_VERSION
    from distutils.msvc9compiler import Reg
    from distutils.msvc9compiler import HKEYS
    from distutils.msvc9compiler import WINSDK_BASE

    sdk_version_map = {
        "v6.0a": 9.0,
        "v6.1": 9.0,
        "v7.0": 9.0,
        "v7.0a": 10.0,
        "v7.1": 10.0
    }

    log.info("Searching Windows SDK with MSVC compiler version {}".format(
        MSVC_VERSION))
    setenv_paths = []
    for base in HKEYS:
        sdk_versions = Reg.read_keys(base, WINSDK_BASE)
        if sdk_versions:
            for sdk_version in sdk_versions:
                installationfolder = Reg.get_value(WINSDK_BASE + "\\" +
                    sdk_version, "installationfolder")
                productversion = Reg.get_value(WINSDK_BASE + "\\" +
                    sdk_version, "productversion")
                setenv_path = os.path.join(installationfolder, os.path.join(
                    'bin', 'SetEnv.cmd'))
                if not os.path.exists(setenv_path):
                    continue
                if not sdk_version in sdk_version_map:
                    continue
                if sdk_version_map[sdk_version] != MSVC_VERSION:
                    continue
                setenv_paths.append(setenv_path)
    if len(setenv_paths) == 0:
        raise DistutilsSetupError(
            "Failed to find the Windows SDK with MSVC compiler "
                "version {}".format(MSVC_VERSION))
    for setenv_path in setenv_paths:
        log.info("Found {}".format(setenv_path))

    # Get SDK env (use latest SDK version installed on system)
    setenv_path = setenv_paths[-1]
    log.info("Using {} ".format(setenv_path))
    build_arch = "/x86" if platform_arch.startswith("32") else "/x64"
    build_type = "/Debug" if build_type.lower() == "debug" else "/Release"
    setenv_cmd = [setenv_path, build_arch, build_type]
    setenv_env = get_environment_from_batch_command(setenv_cmd)
    setenv_env_paths = os.pathsep.join([setenv_env[k] for k in setenv_env if k.upper() == 'PATH']).split(os.pathsep)
    setenv_env_without_paths = dict([(k, setenv_env[k]) for k in setenv_env if k.upper() != 'PATH'])

    # Extend os.environ with SDK env
    log.info("Initializing Windows SDK env...")
    update_env_path(setenv_env_paths)
    for k in sorted(setenv_env_without_paths):
        v = setenv_env_without_paths[k]
        log.info("Inserting '{} = {}' to environment".format(k, v))
        os.environ[k] = v
    log.info("Done initializing Windows SDK env")


def find_vcdir(version):
    """
    This is the customized version of
    distutils.msvc9compiler.find_vcvarsall method
    """
    from distutils.msvc9compiler import VS_BASE
    from distutils.msvc9compiler import Reg
    from distutils import log
    vsbase = VS_BASE % version
    try:
        productdir = Reg.get_value(r"{}\Setup\VC".format(vsbase), "productdir")
    except KeyError:
        productdir = None

    # trying Express edition
    if productdir is None:
        try:
            from distutils.msvc9compiler import VSEXPRESS_BASE
        except ImportError:
            pass
        else:
            vsbase = VSEXPRESS_BASE % version
            try:
                productdir = Reg.get_value(r"{}\Setup\VC".format(vsbase),
                                           "productdir")
            except KeyError:
                productdir = None
                log.debug("Unable to find productdir in registry")

    if not productdir or not os.path.isdir(productdir):
        toolskey = "VS%0.f0COMNTOOLS" % version
        toolsdir = os.environ.get(toolskey, None)

        if toolsdir and os.path.isdir(toolsdir):
            productdir = os.path.join(toolsdir, os.pardir, os.pardir, "VC")
            productdir = os.path.abspath(productdir)
            if not os.path.isdir(productdir):
                log.debug("{} is not a valid directory".format(productdir))
                return None
        else:
            log.debug("Env var {} is not set or invalid".format(toolskey))
    if not productdir:
        log.debug("No productdir found")
        return None
    return productdir


def init_msvc_env(platform_arch, build_type):
    from distutils.msvc9compiler import VERSION as MSVC_VERSION

    log.info("Searching MSVC compiler version {}".format(MSVC_VERSION))
    vcdir_path = find_vcdir(MSVC_VERSION)
    if not vcdir_path:
        raise DistutilsSetupError(
            "Failed to find the MSVC compiler version {} on your "
            "system.".format(MSVC_VERSION))
    else:
        log.info("Found {}".format(vcdir_path))

    log.info("Searching MSVC compiler {} environment init script".format(
        MSVC_VERSION))
    if platform_arch.startswith("32"):
        vcvars_path = os.path.join(vcdir_path, "bin", "vcvars32.bat")
    else:
        vcvars_path = os.path.join(vcdir_path, "bin", "vcvars64.bat")
        if not os.path.exists(vcvars_path):
            vcvars_path = os.path.join(vcdir_path, "bin", "amd64",
                "vcvars64.bat")
            if not os.path.exists(vcvars_path):
                vcvars_path = os.path.join(vcdir_path, "bin", "amd64",
                    "vcvarsamd64.bat")

    if not os.path.exists(vcvars_path):
        # MSVC init script not found, try to find and init Windows SDK env
        log.error("Failed to find the MSVC compiler environment init script "
            "(vcvars.bat) on your system.")
        winsdk_setenv(platform_arch, build_type)
        return
    else:
        log.info("Found {}".format(vcvars_path))

    # Get MSVC env
    log.info("Using MSVC {} in {}".format(MSVC_VERSION, vcvars_path))
    msvc_arch = "x86" if platform_arch.startswith("32") else "amd64"
    log.info("Getting MSVC env for {} architecture".format(msvc_arch))
    vcvars_cmd = [vcvars_path, msvc_arch]
    msvc_env = get_environment_from_batch_command(vcvars_cmd)
    msvc_env_paths = os.pathsep.join([msvc_env[k] for k in msvc_env if k.upper() == 'PATH']).split(os.pathsep)
    msvc_env_without_paths = dict([(k, msvc_env[k]) for k in msvc_env if k.upper() != 'PATH'])

    # Extend os.environ with MSVC env
    log.info("Initializing MSVC env...")
    update_env_path(msvc_env_paths)
    for k in sorted(msvc_env_without_paths):
        v = msvc_env_without_paths[k]
        log.info("Inserting '{} = {}' to environment".format(k, v))
        os.environ[k] = v
    log.info("Done initializing MSVC env")


def copyfile(src, dst, force=True, vars=None, force_copy_symlink=False,
             make_writable_by_owner=False):
    if vars is not None:
        src = src.format(**vars)
        dst = dst.format(**vars)

    if not os.path.exists(src) and not force:
        log.info("**Skiping copy file {} to {}. "
            "Source does not exists.".format(src, dst))
        return

    if not os.path.islink(src) or force_copy_symlink:
        log.info("Copying file {} to {}.".format(src, dst))
        shutil.copy2(src, dst)
        if make_writable_by_owner:
            make_file_writable_by_owner(dst)

    else:
        link_target_path = os.path.realpath(src)
        if os.path.dirname(link_target_path) == os.path.dirname(src):
            link_target = os.path.basename(link_target_path)
            link_name = os.path.basename(src)
            current_directory = os.getcwd()
            try:
                target_dir = dst if os.path.isdir(dst) else os.path.dirname(dst)
                os.chdir(target_dir)
                if os.path.exists(link_name):
                    os.remove(link_name)
                log.info("Symlinking {} -> {} in {}.".format(link_name,
                    link_target, target_dir))
                os.symlink(link_target, link_name)
            except OSError:
                log.error("{} -> {}: Error creating symlink".format(link_name,
                    link_target))
            finally:
                os.chdir(current_directory)
        else:
            log.error("{} -> {}: Can only create symlinks within the same "
                "directory".format(src, link_target_path))

    return dst


def makefile(dst, content=None, vars=None):
    if vars is not None:
        if content is not None:
            content = content.format(**vars)
        dst = dst.format(**vars)

    log.info("Making file {}.".format(dst))

    dstdir = os.path.dirname(dst)
    if not os.path.exists(dstdir):
        os.makedirs(dstdir)

    f = open(dst, "wt")
    if content is not None:
        f.write(content)
    f.close()


def copydir(src, dst, filter=None, ignore=None, force=True, recursive=True,
    vars=None, dir_filter_function=None, file_filter_function=None,
    force_copy_symlinks=False):

    if vars is not None:
        src = src.format(**vars)
        dst = dst.format(**vars)
        if filter is not None:
            for i in range(len(filter)):
                filter[i] = filter[i].format(**vars)
        if ignore is not None:
            for i in range(len(ignore)):
                ignore[i] = ignore[i].format(**vars)

    if not os.path.exists(src) and not force:
        log.info("**Skiping copy tree {} to {}. Source does not exists. "
            "filter={}. ignore={}.".format(src, dst, filter, ignore))
        return []

    log.info("Copying tree {} to {}. filter={}. ignore={}.".format(src, dst,
        filter, ignore))

    names = os.listdir(src)

    results = []
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if os.path.isdir(srcname):
                if (dir_filter_function and
                        not dir_filter_function(name, src, srcname)):
                    continue
                if recursive:
                    results.extend(
                        copydir(srcname, dstname, filter, ignore, force,
                            recursive, vars, dir_filter_function,
                            file_filter_function, force_copy_symlinks))
            else:
                if ((file_filter_function is not None and
                        not file_filter_function(name, srcname)) or
                        (filter is not None and
                        not filter_match(name, filter)) or
                        (ignore is not None and filter_match(name, ignore))):
                    continue
                if not os.path.exists(dst):
                    os.makedirs(dst)
                results.append(copyfile(srcname, dstname, True, vars,
                    force_copy_symlinks))
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error as err:
            errors.extend(err.args[0])
        except EnvironmentError as why:
            errors.append((srcname, dstname, str(why)))
    try:
        if os.path.exists(dst):
            shutil.copystat(src, dst)
    except OSError as why:
        if WindowsError is not None and isinstance(why, WindowsError):
            # Copying file access times may fail on Windows
            pass
        else:
            errors.extend((src, dst, str(why)))
    if errors:
        raise EnvironmentError(errors)
    return results

def make_file_writable_by_owner(path):
    current_permissions = stat.S_IMODE(os.lstat(path).st_mode)
    os.chmod(path, current_permissions | stat.S_IWUSR)

def rmtree(dirname, ignore=False):
    def handle_remove_readonly(func, path, exc):
        excvalue = exc[1]
        if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
            os.chmod(path, stat.S_IRWXU| stat.S_IRWXG| stat.S_IRWXO) # 0777
            func(path)
        else:
            raise
    shutil.rmtree(dirname, ignore_errors=ignore, onerror=handle_remove_readonly)

def run_process_output(args, initial_env=None):
    if initial_env is None:
        initial_env = os.environ
    std_out = subprocess.Popen(args, env = initial_env, universal_newlines = 1,
                              stdout=subprocess.PIPE).stdout
    result = []
    for raw_line in std_out.readlines():
        line = raw_line if sys.version_info >= (3,) else raw_line.decode('utf-8')
        result.append(line.rstrip())
    return result

def run_process(args, initial_env=None):
    def _log(buffer, check_new_line=False):
        ends_with_new_line = False
        if buffer.endswith('\n'):
            ends_with_new_line = True
        if check_new_line and buffer.find('\n') == -1:
            return buffer
        lines = buffer.splitlines()
        buffer = ''
        if check_new_line and not ends_with_new_line:
            buffer = lines[-1]
            lines = lines[:-1]
        for line in lines:
            log.info(line.rstrip('\r'))
        return buffer
    _log("Running process in {0}: {1}".format(os.getcwd(),
        " ".join([(" " in x and '"{0}"'.format(x) or x) for x in args])))

    if sys.platform != "win32":
        try:
            spawn(args)
            return 0
        except DistutilsExecError:
            return -1

    shell = False
    if sys.platform == "win32":
        shell = True

    if initial_env is None:
        initial_env = os.environ

    proc = popenasync.Popen(args,
        stdin = subprocess.PIPE,
        stdout = subprocess.PIPE,
        stderr = subprocess.STDOUT,
        universal_newlines = 1,
        shell = shell,
        env = initial_env)

    log_buffer = None;
    while proc.poll() is None:
        log_buffer = _log(proc.read_async(wait=0.1, e=0))
    if log_buffer:
        _log(log_buffer)

    proc.wait()
    return proc.returncode


def get_environment_from_batch_command(env_cmd, initial=None):
    """
    Take a command (either a single command or list of arguments)
    and return the environment created after running that command.
    Note that if the command must be a batch file or .cmd file, or the
    changes to the environment will not be captured.

    If initial is supplied, it is used as the initial environment passed
    to the child process.
    """

    def validate_pair(ob):
        try:
            if not (len(ob) == 2):
                print("Unexpected result: {}".format(ob))
                raise ValueError
        except:
            return False
        return True

    def consume(iter):
        try:
            while True: next(iter)
        except StopIteration:
            pass

    if not isinstance(env_cmd, (list, tuple)):
        env_cmd = [env_cmd]
    # construct the command that will alter the environment
    env_cmd = subprocess.list2cmdline(env_cmd)
    # create a tag so we can tell in the output when the proc is done
    tag = 'Done running command'
    # construct a cmd.exe command to do accomplish this
    cmd = 'cmd.exe /E:ON /V:ON /s /c "{} && echo "{}" && set"'.format(env_cmd,
        tag)
    # launch the process
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, env=initial)
    # parse the output sent to stdout
    lines = proc.stdout
    if sys.version_info[0] > 2:
        # make sure the lines are strings
        make_str = lambda s: s.decode()
        lines = map(make_str, lines)
    # consume whatever output occurs until the tag is reached
    consume(itertools.takewhile(lambda l: tag not in l, lines))
    # define a way to handle each KEY=VALUE line
    handle_line = lambda l: l.rstrip().split('=',1)
    # parse key/values into pairs
    pairs = map(handle_line, lines)
    # make sure the pairs are valid
    valid_pairs = filter(validate_pair, pairs)
    # construct a dictionary of the pairs
    result = dict(valid_pairs)
    # let the process finish
    proc.communicate()
    return result


def regenerate_qt_resources(src, pyside_rcc_path, pyside_rcc_options):
    names = os.listdir(src)
    for name in names:
        srcname = os.path.join(src, name)
        if os.path.isdir(srcname):
            regenerate_qt_resources(srcname,
                                    pyside_rcc_path,
                                    pyside_rcc_options)
        elif srcname.endswith('.qrc'):
            # Replace last occurence of '.qrc' in srcname
            srcname_split = srcname.rsplit('.qrc', 1)
            dstname = '_rc.py'.join(srcname_split)
            if os.path.exists(dstname):
                log.info('Regenerating {} from {}'.format(dstname,
                    os.path.basename(srcname)))
                run_process([pyside_rcc_path,
                             pyside_rcc_options,
                             srcname, '-o', dstname])


def back_tick(cmd, ret_err=False):
    """
    Run command `cmd`, return stdout, or stdout, stderr,
    return_code if `ret_err` is True.

    Roughly equivalent to ``check_output`` in Python 2.7

    Parameters
    ----------
    cmd : str
        command to execute
    ret_err : bool, optional
        If True, return stderr and return_code in addition to stdout.
        If False, just return stdout

    Returns
    -------
    out : str or tuple
        If `ret_err` is False, return stripped string containing stdout from
        `cmd`.
        If `ret_err` is True, return tuple of (stdout, stderr, return_code)
        where ``stdout`` is the stripped stdout, and ``stderr`` is the stripped
        stderr, and ``return_code`` is the process exit code.

    Raises
    ------
    Raises RuntimeError if command returns non-zero exit code when ret_err
    isn't set.
    """
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, shell=True)
    out, err = proc.communicate()
    if not isinstance(out, str):
        # python 3
        out = out.decode()
        err = err.decode()
    retcode = proc.returncode
    if retcode is None and not ret_err:
        proc.terminate()
        raise RuntimeError(cmd + ' process did not terminate')
    if retcode != 0 and not ret_err:
        raise RuntimeError("{} process returned code {}\n*** {}".format(
            (cmd, retcode, err)))
    out = out.strip()
    if not ret_err:
        return out
    return out, err.strip(), retcode


MACOS_OUTNAME_RE = re.compile(r'\(compatibility version [\d.]+, current version '
                        '[\d.]+\)')

def macos_get_install_names(libpath):
    """
    Get macOS library install names from library `libpath` using ``otool``

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
    return [MACOS_OUTNAME_RE.sub('', lib).strip() for lib in libs]


MACOS_RPATH_RE = re.compile(r"path (.+) \(offset \d+\)")

def macos_get_rpaths(libpath):
    """ Get rpath load commands from library `libpath` using ``otool``

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
        match = MACOS_RPATH_RE.match(rpath_line)
        if match is None:
            raise RuntimeError('Unexpected path line: ' + rpath_line)
        rpaths.append(match.groups()[0])
        ctr += 3
    return rpaths


def macos_fix_rpaths_for_library(library_path, qt_lib_dir):
    """ Adds required rpath load commands to given library.

    This is a necessary post-installation step, to allow loading PySide
    modules without setting DYLD_LIBRARY_PATH or DYLD_FRAMEWORK_PATH.
    The CMake rpath commands which are added at build time are used only
    for testing (make check), and they are stripped once the equivalent
    of make install is executed (except for shiboken, which currently
    uses CMAKE_INSTALL_RPATH_USE_LINK_PATH, which might be necessary to
    remove in the future).

    Parameters
    ----------
    library_path : str
        path to library for which to set rpaths.
    qt_lib_dir : str
        rpath to installed Qt lib directory.
    """

    install_names = macos_get_install_names(library_path)
    existing_rpath_commands = macos_get_rpaths(library_path)

    needs_loader_path = False
    for install_name in install_names:
        # Absolute path, skip it.
        if install_name[0] == '/':
            continue

        # If there are dynamic library install names that contain
        # @rpath tokens, we will provide an rpath load command with the
        # value of "@loader_path". This will allow loading dependent
        # libraries from within the same directory as 'library_path'.
        if install_name[0] == '@':
            needs_loader_path = True
            break

    if needs_loader_path and "@loader_path" not in existing_rpath_commands:
        back_tick('install_name_tool -add_rpath {rpath} {library_path}'.format(
                  rpath="@loader_path", library_path=library_path))

    # If the library depends on a Qt library, add an rpath load comment
    # pointing to the Qt lib directory.
    macos_add_qt_rpath(library_path, qt_lib_dir, existing_rpath_commands,
        install_names)

def macos_add_qt_rpath(library_path, qt_lib_dir,
                     existing_rpath_commands = [], library_dependencies = []):
    """
    Adds an rpath load command to the Qt lib directory if necessary

    Checks if library pointed to by 'library_path' has Qt dependencies,
    and adds an rpath load command that points to the Qt lib directory
    (qt_lib_dir).
    """
    if not existing_rpath_commands:
        existing_rpath_commands = macos_get_rpaths(library_path)

    # Return early if qt rpath is already present.
    if qt_lib_dir in existing_rpath_commands:
        return

    # Check if any library dependencies are Qt libraries (hacky).
    if not library_dependencies:
        library_dependencies = macos_get_install_names(library_path)

    needs_qt_rpath = False
    for library in library_dependencies:
        if 'Qt' in library:
            needs_qt_rpath = True
            break

    if needs_qt_rpath:
        back_tick('install_name_tool -add_rpath {rpath} {library_path}'.format(
                  rpath=qt_lib_dir, library_path=library_path))

# Find an executable specified by a glob pattern ('foo*') in the OS path
def find_glob_in_path(pattern):
    result = []
    if sys.platform == 'win32':
        pattern += '.exe'

    for path in os.environ.get('PATH', '').split(os.pathsep):
        for match in glob.glob(os.path.join(path, pattern)):
            result.append(match)
    return result

# Locate the most recent version of llvm_config in the path.
def find_llvm_config():
    version_re = re.compile('(\d+)\.(\d+)\.(\d+)')
    result = None
    last_version_string = '000000'
    for llvm_config in find_glob_in_path('llvm-config*'):
        try:
            output = run_process_output([llvm_config, '--version'])
            if output:
                match = version_re.match(output[0])
                if match:
                    version_string = '%02d%02d%02d' % (int(match.group(1)),
                                    int(match.group(2)), int(match.group(3)))
                    if (version_string > last_version_string):
                        result = llvm_config
                        last_version_string = version_string
        except OSError:
            pass
    return result

# Add Clang to path for Windows for the shiboken ApiExtractor tests.
# Revisit once Clang is bundled with Qt.
def detect_clang():
    source = 'LLVM_INSTALL_DIR'
    clang_dir = os.environ.get(source, None)
    if not clang_dir:
        source = 'CLANG_INSTALL_DIR'
        clang_dir = os.environ.get(source, None)
        if not clang_dir:
            source = find_llvm_config()
            try:
                if source is not None:
                    output = run_process_output([source, '--prefix'])
                    if output:
                        clang_dir = output[0]
            except OSError:
                pass
    if clang_dir:
        arch = '64' if sys.maxsize > 2**31-1 else '32'
        clang_dir = clang_dir.replace('_ARCH_', arch)
    return (clang_dir, source)

def download_and_extract_7z(fileurl, target):
    """ Downloads 7z file from fileurl and extract to target  """
    print("Downloading fileUrl {} ".format(fileurl))
    info = ""
    try:
        localfile, info = urllib.urlretrieve(fileurl)
    except:
        print("Error downloading {} : {}".format(fileurl, info))
        raise RuntimeError(' Error downloading {}'.format(fileurl))

    try:
        outputDir = "-o" + target
        print("calling 7z x {} {}".format(localfile, outputDir))
        subprocess.call(["7z", "x", "-y", localfile, outputDir])
    except:
        raise RuntimeError(' Error extracting {}'.format(localfile))

def split_and_strip(input):
    lines = [s.strip() for s in input.splitlines()]
    return lines

def ldd_get_dependencies(executable_path):
    """
    Returns a dictionary of dependencies that `executable_path`
    depends on.

    The keys are library names and the values are the library paths.

    """
    output = ldd(executable_path)
    lines = split_and_strip(output)
    pattern = re.compile(r"\s*(.*?)\s+=>\s+(.*?)\s+\(.*\)")
    dependencies = {}
    for line in lines:
        match = pattern.search(line)
        if match:
            dependencies[match.group(1)] = match.group(2)
    return dependencies

def ldd_get_paths_for_dependencies(dependencies_regex, executable_path = None,
    dependencies = None):
    """
    Returns file paths to shared library dependencies that match given
    `dependencies_regex` against given `executable_path`.

    The function retrieves the list of shared library dependencies using
    ld.so for the given `executable_path` in order to search for
    libraries that match the `dependencies_regex`, and then returns a
    list of absolute paths of the matching libraries.

    If no matching library is found in the list of dependencies,
    an empty list is returned.
    """

    if not dependencies and not executable_path:
        return None

    if not dependencies:
        dependencies = ldd_get_dependencies(executable_path)

    pattern = re.compile(dependencies_regex)

    paths = []
    for key in dependencies:
        match = pattern.search(key)
        if match:
            paths.append(dependencies[key])

    return paths

def ldd(executable_path):
    """
    Returns ld.so output of shared library dependencies for given
    `executable_path`.

    This is a partial port of /usr/bin/ldd from bash to Python.
    The dependency list is retrieved by setting the
    LD_TRACE_LOADED_OBJECTS=1 environment variable, and executing the
    given path via the dynamic loader ld.so.

    Only works on Linux. The port is required to make this work on
    systems that might not have ldd.
    This is because ldd (on Ubuntu) is shipped in the libc-bin package
    that, which might have a
    minuscule percentage of not being installed.

    Parameters
    ----------
    executable_path : str
        path to executable or shared library.

    Returns
    -------
    output : str
        the raw output retrieved from the dynamic linker.
    """

    chosen_rtld = None
    # List of ld's considered by ldd on Ubuntu (here's hoping it's the
    # same on all distros).
    rtld_list = ["/lib/ld-linux.so.2", "/lib64/ld-linux-x86-64.so.2",
        "/libx32/ld-linux-x32.so.2"]

    # Choose appropriate runtime dynamic linker.
    for rtld in rtld_list:
        if os.path.isfile(rtld) and os.access(rtld, os.X_OK):
            (_, _, code) = back_tick(rtld, True)
            # Code 127 is returned by ld.so when called without any
            # arguments (some kind of sanity check I guess).
            if code == 127:
                (_, _, code) = back_tick("{} --verify {}".format(rtld,
                    executable_path), True)
                # Codes 0 and 2 mean given executable_path can be
                # understood by ld.so.
                if code in [0, 2]:
                    chosen_rtld = rtld
                    break

    if not chosen_rtld:
        raise RuntimeError("Could not find appropriate ld.so to query "
            "for dependencies.")

    # Query for shared library dependencies.
    rtld_env = "LD_TRACE_LOADED_OBJECTS=1"
    rtld_cmd = "{} {} {}".format(rtld_env, chosen_rtld, executable_path)
    (out, _, return_code) = back_tick(rtld_cmd, True)
    if return_code == 0:
        return out
    else:
        raise RuntimeError("ld.so failed to query for dependent shared "
            "libraries of {} ".format(executable_path))

def find_files_using_glob(path, pattern):
    """ Returns list of files that matched glob `pattern` in `path`. """
    final_pattern = os.path.join(path, pattern)
    maybe_files = glob.glob(final_pattern)
    return maybe_files

def find_qt_core_library_glob(lib_dir):
    """ Returns path to the QtCore library found in `lib_dir`. """
    maybe_file = find_files_using_glob(lib_dir, "libQt5Core.so.?")
    if len(maybe_file) == 1:
        return maybe_file[0]
    return None

# @TODO: Possibly fix ICU library copying on macOS and Windows.
# This would require to implement the equivalent of the custom written
# ldd for the specified platforms.
# This has less priority because ICU libs are not used in the default
# Qt configuration build.
def copy_icu_libs(destination_lib_dir):
    """
    Copy ICU libraries that QtCore depends on,
    to given `destination_lib_dir`.
    """
    qt_core_library_path = find_qt_core_library_glob(destination_lib_dir)

    if not qt_core_library_path or not os.path.exists(qt_core_library_path):
        raise RuntimeError('QtCore library does not exist at path: {}. '
            'Failed to copy ICU libraries.'.format(qt_core_library_path))

    dependencies = ldd_get_dependencies(qt_core_library_path)

    icu_regex = r"^libicu.+"
    icu_compiled_pattern = re.compile(icu_regex)
    icu_required = False
    for dependency in dependencies:
        match = icu_compiled_pattern.search(dependency)
        if match:
            icu_required = True
            break

    if icu_required:
        paths = ldd_get_paths_for_dependencies(icu_regex,
            dependencies=dependencies)
        if not paths:
            raise RuntimeError("Failed to find the necessary ICU libraries "
                "required by QtCore.")
        log.info('Copying the detected ICU libraries required by QtCore.')

        if not os.path.exists(destination_lib_dir):
            os.makedirs(destination_lib_dir)

        for path in paths:
            basename = os.path.basename(path)
            destination = os.path.join(destination_lib_dir, basename)
            copyfile(path, destination, force_copy_symlink=True)
            # Patch the ICU libraries to contain the $ORIGIN rpath
            # value, so that only the local package libraries are used.
            linux_set_rpaths(destination, '$ORIGIN')

        # Patch the QtCore library to find the copied over ICU libraries
        # (if necessary).
        log.info("Checking if QtCore library needs a new rpath to make it "
            "work with ICU libs.")
        rpaths = linux_get_rpaths(qt_core_library_path)
        if not rpaths or not rpaths_has_origin(rpaths):
            log.info('Patching QtCore library to contain $ORIGIN rpath.')
            rpaths.insert(0, '$ORIGIN')
            new_rpaths_string = ":".join(rpaths)
            linux_set_rpaths(qt_core_library_path, new_rpaths_string)

def linux_set_rpaths(executable_path, rpath_string):
    """ Patches the `executable_path` with a new rpath string. """

    if not hasattr(linux_set_rpaths, "patchelf_path"):
        script_dir = os.getcwd()
        patchelf_path = os.path.join(script_dir, "patchelf")
        setattr(linux_set_rpaths, "patchelf_path", patchelf_path)

    cmd = [linux_set_rpaths.patchelf_path, '--set-rpath',
        rpath_string, executable_path]

    if run_process(cmd) != 0:
        raise RuntimeError("Error patching rpath in {}".format(
            executable_path))

def linux_get_rpaths(executable_path):
    """
    Returns a list of run path values embedded in the executable or just
    an empty list.
    """

    cmd = "readelf -d {}".format(executable_path)
    (out, err, code) = back_tick(cmd, True)
    if code != 0:
        raise RuntimeError("Running `readelf -d {}` failed with error "
            "output:\n {}. ".format(executable_path, err))
    lines = split_and_strip(out)
    pattern = re.compile(r"^.+?\(RUNPATH\).+?\[(.+?)\]$")

    rpath_line = None
    for line in lines:
        match = pattern.search(line)
        if match:
            rpath_line = match.group(1)
            break

    rpaths = []

    if rpath_line:
        rpaths = rpath_line.split(':')

    return rpaths

def rpaths_has_origin(rpaths):
    """
    Return True if the specified list of rpaths has an "$ORIGIN" value
    (aka current dir).
    """
    if not rpaths:
        return False

    pattern = re.compile(r"^\$ORIGIN(/)?$")
    for rpath in rpaths:
        match = pattern.search(rpath)
        if match:
            return True
    return False

def memoize(function):
    """
    Decorator to wrap a function with a memoizing callable.
    It returns cached values when the wrapped function is called with
    the same arguments.
    """
    memo = {}
    def wrapper(*args):
        if args in memo:
            return memo[args]
        else:
            rv = function(*args)
            memo[args] = rv
            return rv
    return wrapper

def get_python_dict(python_script_path):
    try:
        with open(python_script_path) as f:
            python_dict = {}
            code = compile(f.read(), python_script_path, 'exec')
            exec(code, {}, python_dict)
            return python_dict
    except IOError as e:
        print("get_python_dict: Couldn't get dict from python "
            "file: {}.".format(python_script_path))
        raise

def install_pip_dependencies(env_pip, packages):
    for p in packages:
        run_instruction([env_pip, "install", p], "Failed to install " + p)

def get_qtci_virtualEnv(python_ver, host, hostArch, targetArch):
    _pExe = "python"
    _env = "env" + str(python_ver)
    env_python = _env + "/bin/python"
    env_pip = _env + "/bin/pip"

    if host == "Windows":
        print("New virtualenv to build " + targetArch + " in " + hostArch + " host.")
        _pExe = "python.exe"
        # With windows we are creating building 32-bit target in 64-bit host
        if hostArch == "X86_64" and targetArch == "X86":
            if python_ver == "3":
                _pExe = os.path.join(os.getenv("PYTHON3_32_PATH"), "python.exe")
            else:
                _pExe = os.path.join(os.getenv("PYTHON2_32_PATH"), "python.exe")
        else:
            if python_ver == "3":
                _pExe = os.path.join(os.getenv("PYTHON3_PATH"), "python.exe")
        env_python = _env + "\\Scripts\\python.exe"
        env_pip = _env + "\\Scripts\\pip.exe"
    else:
        if python_ver == "3":
            _pExe = "python3"
    return(_pExe, _env, env_pip, env_python)

def run_instruction(instruction, error):
    print("Running Coin instruction: " + ' '.join(str(e) for e in instruction))
    result = subprocess.call(instruction)
    if result != 0:
        print("ERROR : " + error)
        exit(result)
