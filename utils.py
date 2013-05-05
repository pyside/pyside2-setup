import sys
import os
import stat
import errno
import time
import shutil
import subprocess
import fnmatch
import itertools
import popenasync

from distutils.spawn import spawn
from distutils.spawn import DistutilsExecError

try:
    WindowsError
except NameError:
    WindowsError = None


def has_option(name):
    try:
        sys.argv.remove('--%s' % name)
        return True
    except ValueError:
        pass
    return False


def option_value(name):
    for index, option in enumerate(sys.argv):
        if option == '--' + name:
            if index+1 >= len(sys.argv):
                raise DistutilsOptionError(
                    'The option %s requires a value' % option)
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


def update_env_path(newpaths, logger):
    paths = os.environ['PATH'].lower().split(os.pathsep)
    for path in newpaths:
        if not path.lower() in paths:
            logger.info("Inserting path \"%s\" to environment" % path)
            paths.insert(0, path)
            os.environ['PATH'] = os.pathsep.join(paths)


def find_vcvarsall(version):
    from distutils import msvc9compiler
    vcvarsall_path = msvc9compiler.find_vcvarsall(version)
    return vcvarsall_path


def find_vcvarsall_paths(versions):
    vcvarsall_paths = []
    for version in versions:
        vcvarsall_path = find_vcvarsall(version)
        if vcvarsall_path:
            vcvarsall_paths.append([version, vcvarsall_path])
    return vcvarsall_paths


def init_msvc_env(default_msvc_version, platform_arch, logger):
    logger.info("Searching vcvarsall.bat")
    all_vcvarsall_paths = find_vcvarsall_paths([9.0, 10.0, 11.0])
    if len(all_vcvarsall_paths) == 0:
        raise DistutilsSetupError(
            "Failed to find the MSVC compiler on your system.")
    for v in all_vcvarsall_paths:
        logger.info("Found MSVC %s in %s" % (v[0], v[1]))
    
    if default_msvc_version:
        msvc_version = float(default_msvc_version)
        vcvarsall_path_tmp = [p for p in all_vcvarsall_paths if p[0] == msvc_version]
        vcvarsall_path = vcvarsall_path_tmp[0][1] if len(vcvarsall_path_tmp) > 0 else None
        if not vcvarsall_path:
            raise DistutilsSetupError(
                "Failed to find the vcvarsall.bat for MSVC version %s." % msvc_version)
    else:
        # By default use first MSVC compiler found in system
        msvc_version = all_vcvarsall_paths[0][0]
        vcvarsall_path = all_vcvarsall_paths[0][1]
    
    # Get MSVC env
    logger.info("Using MSVC %s in %s" % (msvc_version, vcvarsall_path))
    msvc_arch = "x86" if platform_arch.startswith("32") else "amd64"
    logger.info("Getting MSVC env for %s architecture" % msvc_arch)
    vcvarsall_cmd = [vcvarsall_path, msvc_arch]
    msvc_env = get_environment_from_batch_command(vcvarsall_cmd)
    msvc_env_paths = os.pathsep.join([msvc_env[k] for k in msvc_env if k.upper() == 'PATH']).split(os.pathsep)
    msvc_env_without_paths = dict([(k, msvc_env[k]) for k in msvc_env if k.upper() != 'PATH'])
    
    # Extend os.environ with MSVC env
    logger.info("Initializing MSVC env...")
    update_env_path(msvc_env_paths, logger)
    for k in sorted(msvc_env_without_paths):
        v = msvc_env_without_paths[k]
        logger.info("Inserting \"%s = %s\" to environment" % (k, v))
        os.environ[k] = v
    logger.info("Done initializing MSVC env")


def copyfile(src, dst, logger=None, force=True, vars=None, subst_content=False):
    if vars is not None:
        src = src.format(**vars)
        dst = dst.format(**vars)
    
    if not os.path.exists(src) and not force:
        if logger is not None:
            logger.info("**Skiping copy file %s to %s. Source does not exists." % (src, dst))
        return
    
    if logger is not None:
        logger.info("Copying file %s to %s." % (src, dst))
    
    if vars is None or not subst_content:
        shutil.copy2(src, dst)
        return
    
    print ("copyfile " + src)
    f = open(src, "rt")
    content =  f.read()
    f.close()
    content = content.format(**vars)
    f = open(dst, "wt")
    f.write(content)
    f.close()


def makefile(dst, content=None, logger=None, vars=None):
    if vars is not None:
        if content is not None:
            content = content.format(**vars)
        dst = dst.format(**vars)
    
    if logger is not None:
        logger.info("Making file %s." % (dst))
    
    dstdir = os.path.dirname(dst)
    if not os.path.exists(dstdir):
        os.makedirs(dstdir)
    
    f = open(dst, "wt")
    if content is not None:
        f.write(content)
    f.close()


def copydir(src, dst, logger=None, filter=None, ignore=None, force=True,
    recursive=True, vars=None, subst_files_content=False):
    
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
        if logger is not None:
            logger.info("**Skiping copy tree %s to %s. Source does not exists. filter=%s. ignore=%s." % \
                (src, dst, filter, ignore))
        return
    
    if logger is not None:
        logger.info("Copying tree %s to %s. filter=%s. ignore=%s." % \
            (src, dst, filter, ignore))
    
    names = os.listdir(src)
    
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if os.path.isdir(srcname):
                if recursive:
                    copydir(srcname, dstname, logger, filter, ignore, force, recursive,
                        vars, subst_files_content)
            else:
                if (filter is not None and not filter_match(name, filter)) or \
                    (ignore is not None and filter_match(name, ignore)):
                    continue
                if not os.path.exists(dst):
                    os.makedirs(dst)
                copyfile(srcname, dstname, logger, True, vars, subst_files_content)
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error as err:
            errors.extend(err.args[0])
        except EnvironmentError as why:
            errors.append((srcname, dstname, str(why)))
    try:
        shutil.copystat(src, dst)
    except OSError as why:
        if WindowsError is not None and isinstance(why, WindowsError):
            # Copying file access times may fail on Windows
            pass
        else:
            errors.extend((src, dst, str(why)))
    if errors:
        raise EnvironmentError(errors)


def rmtree(dirname):
    def handleRemoveReadonly(func, path, exc):
        excvalue = exc[1]
        if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
            os.chmod(path, stat.S_IRWXU| stat.S_IRWXG| stat.S_IRWXO) # 0777
            func(path)
        else:
            raise
    shutil.rmtree(dirname, ignore_errors=False, onerror=handleRemoveReadonly)


def run_process(args, logger=None, initial_env=None):
    def log(buffer, checkNewLine=False):
        endsWithNewLine = False
        if buffer.endswith('\n'):
            endsWithNewLine = True
        if checkNewLine and buffer.find('\n') == -1:
            return buffer
        lines = buffer.splitlines()
        buffer = ''
        if checkNewLine and not endsWithNewLine:
            buffer = lines[-1]
            lines = lines[:-1]
        for line in lines:
            if not logger is None:
                logger.info(line.rstrip('\r'))
            else:
                print(line.rstrip('\r'))
        return buffer
    
    log("Running process: {0}".format(" ".join([(" " in x and '"{0}"'.format(x) or x) for x in args])))
    
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
        log_buffer = log(proc.read_async(wait=0.1, e=0))
    if log_buffer:
        log(log_buffer)
    
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
                print("Unexpected result: %s" % ob)
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
    cmd = 'cmd.exe /s /c "{env_cmd} && echo "{tag}" && set"'.format(**vars())
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
