# Postinstall script for PySide
#
# Generates the qt.conf file
#
# This file is based on pywin32_postinstall.py file from pywin32 project

import os, sys, traceback, shutil, fnmatch, stat

try:
    # When this script is run from inside the bdist_wininst installer,
    # file_created() and directory_created() are additional builtin
    # functions which write lines to Python23\pyside-install.log. This is
    # a list of actions for the uninstaller, the format is inspired by what
    # the Wise installer also creates.
    file_created
    is_bdist_wininst = True
except NameError:
    is_bdist_wininst = False # we know what it is not - but not what it is :)
    def file_created(file):
        pass

def install():
    if sys.platform == "win32":
        install_win32()
    else:
        install_linux()

def filter_match(name, patterns):
    for pattern in patterns:
        if pattern is None:
            continue
        if fnmatch.fnmatch(name, pattern):
            return True
    return False

def set_exec(name):
    mode = os.stat(name).st_mode
    new_mode = mode
    if new_mode & stat.S_IRUSR:
        new_mode = new_mode | stat.S_IXUSR
    if new_mode & stat.S_IRGRP:
        new_mode = new_mode | stat.S_IXGRP
    if new_mode & stat.S_IROTH:
        new_mode = new_mode | stat.S_IXOTH
    if (mode != new_mode):
        print("Setting exec for '%s' (mode %o => %o)" % (name, mode, new_mode))
        os.chmod(name, new_mode)

def install_linux():
    # Try to find PySide package
    try:
        import PySide
    except ImportError:
        print("The PySide package not found: %s" % traceback.print_exception(*sys.exc_info()))
        return
    pyside_path = os.path.abspath(os.path.dirname(PySide.__file__))
    print("PySide package found in %s..." % pyside_path)

    # Set exec mode on executables
    for elf in ["patchelf", "shiboken"]:
        elfpath = os.path.join(pyside_path, elf)
        set_exec(elfpath)

    # Update rpath in PySide libs
    from distutils.spawn import spawn
    patchelf_path = os.path.join(pyside_path, "patchelf")
    for srcname in os.listdir(pyside_path):
        if os.path.isdir(srcname):
            continue
        if not filter_match(srcname, ["Qt*.so", "phonon.so", "shiboken"]):
            continue
        srcpath = os.path.join(pyside_path, srcname)
        cmd = [
            patchelf_path,
            "--set-rpath",
            pyside_path,
            srcpath,
        ]
        spawn(cmd, search_path=0, verbose=1)
        print("Patched rpath in %s to %s." % (srcpath, pyside_path))

    # Check PySide installation status
    try:
        from PySide import QtCore
        print("PySide package successfully installed in %s..." % \
            os.path.abspath(os.path.dirname(QtCore.__file__)))
    except ImportError:
        print("The PySide package not installed: %s" % traceback.print_exception(*sys.exc_info()))

def install_win32():
    # Try to find PySide package
    try:
        from PySide import QtCore
    except ImportError:
        print("The PySide package not found: %s" % traceback.print_exception(*sys.exc_info()))
        return
    pyside_path = os.path.dirname(QtCore.__file__)
    pyside_path = pyside_path.replace("\\", "/")
    pyside_path = pyside_path.replace("lib/site-packages", "Lib/site-packages")
    print("PySide package found in %s..." % pyside_path)

    if is_bdist_wininst:
        # Run from inside the bdist_wininst installer
        import distutils.sysconfig
        exec_prefix = distutils.sysconfig.get_config_var("exec_prefix")
    else:
        # Run manually
        exec_prefix = os.path.dirname(sys.executable)

    # Generate qt.conf
    qtconf_path = os.path.join(exec_prefix, "qt.conf")
    print("Generating file %s..." % qtconf_path)
    f = open(qtconf_path, 'wt')
    file_created(qtconf_path)
    f.write("""[Paths]
Prefix = %(pyside_prefix)s
Binaries = %(pyside_prefix)s
Plugins = %(pyside_prefix)s/plugins
Translations = %(pyside_prefix)s/translations
""" % { "pyside_prefix": pyside_path })
    print("The PySide extensions were successfully installed.")

    # Install OpenSSL libs
    for dll in ["libeay32.dll", "ssleay32.dll"]:
        dest_path = os.path.join(exec_prefix, dll)
        src_path = os.path.join(pyside_path, dll)
        if not os.path.exists(dest_path) and os.path.exists(src_path):
            shutil.copy(src_path, dest_path)
            file_created(dest_path)
            print("Installed %s to %s." % (dll, exec_prefix))

def uninstall():
    print("The PySide extensions were successfully uninstalled.")

def usage():
    msg = \
"""%s: A post-install script for the PySide extensions.
    
This should be run automatically after installation, but if it fails you
can run it again with a '-install' parameter, to ensure the environment
is setup correctly.
"""
    print(msg.strip() % os.path.basename(sys.argv[0]))

# NOTE: If this script is run from inside the bdist_wininst created
# binary installer or uninstaller, the command line args are either
# '-install' or '-remove'.

# Important: From inside the binary installer this script MUST NOT
# call sys.exit() or raise SystemExit, otherwise not only this script
# but also the installer will terminate! (Is there a way to prevent
# this from the bdist_wininst C code?)

if __name__ == '__main__':
    if len(sys.argv)==1:
        usage()
        sys.exit(1)

    arg_index = 1
    while arg_index < len(sys.argv):
        arg = sys.argv[arg_index]
        if arg == "-install":
            install()
        elif arg == "-remove":
            # bdist_msi calls us before uninstall, so we can undo what we
            # previously did.  Sadly, bdist_wininst calls us *after*, so
            # we can't do much at all.
            if not is_bdist_wininst:
                uninstall()
        else:
            print("Unknown option: %s" % arg)
            usage()
            sys.exit(0)
        arg_index += 1
