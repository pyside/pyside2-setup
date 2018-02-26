import os, sys, re, subprocess
from distutils.spawn import find_executable

class QtInfo(object):
    def __init__(self, qmake_command=None):
        self.initialized = False

        if qmake_command:
            self._qmake_command = qmake_command
        else:
            self._qmake_command = [find_executable("qmake"),]

        # Dict to cache qmake values.
        self._query_dict = {}
        # Dict to cache mkspecs variables.
        self._mkspecs_dict = {}
        # Initialize the properties.
        self._initProperties()

    def getQMakeCommand(self):
        qmake_command_string = self._qmake_command[0]
        for entry in self._qmake_command[1:]:
            qmake_command_string += " {}".format(entry)
        return qmake_command_string

    def getVersion(self):
        return self.getProperty("QT_VERSION")

    def getBinsPath(self):
        return self.getProperty("QT_INSTALL_BINS")

    def getLibsPath(self):
        return self.getProperty("QT_INSTALL_LIBS")

    def getLibsExecsPath(self):
        return self.getProperty("QT_INSTALL_LIBEXECS")

    def getPluginsPath(self):
        return self.getProperty("QT_INSTALL_PLUGINS")

    def getPrefixPath(self):
        return self.getProperty("QT_INSTALL_PREFIX")

    def getImportsPath(self):
        return self.getProperty("QT_INSTALL_IMPORTS")

    def getTranslationsPath(self):
        return self.getProperty("QT_INSTALL_TRANSLATIONS")

    def getHeadersPath(self):
        return self.getProperty("QT_INSTALL_HEADERS")

    def getDocsPath(self):
        return self.getProperty("QT_INSTALL_DOCS")

    def getQmlPath(self):
        return self.getProperty("QT_INSTALL_QML")

    def getMacOSMinDeploymentTarget(self):
        """ Return value is a macOS version or None. """
        return self.getProperty("QMAKE_MACOSX_DEPLOYMENT_TARGET")

    def getBuildType(self):
        """ Return value is either debug, release, debug_release, or None. """
        return self.getProperty("BUILD_TYPE")

    def getSrcDir(self):
        """ Return path to Qt src dir or None.. """
        return self.getProperty("QT_INSTALL_PREFIX/src")

    def getProperty(self, prop_name):
        if prop_name not in self._query_dict:
            return None
        return self._query_dict[prop_name]

    def getProperties(self):
        return self._query_dict

    def getMkspecsVariables(self):
        return self._mkspecs_dict

    def _getQMakeOutput(self, args_list = []):
        cmd = self._qmake_command + args_list
        proc = subprocess.Popen(cmd, stdout = subprocess.PIPE, shell=False)
        output = proc.communicate()[0]
        proc.wait()
        if proc.returncode != 0:
            return None
        if sys.version_info >= (3,):
            output = str(output, 'ascii').strip()
        else:
            output = output.strip()
        return output

    def _parseQueryProperties(self, process_output):
        props = {}
        if not process_output:
            return props
        lines = [s.strip() for s in process_output.splitlines()]
        for line in lines:
            if line and ':' in line:
                key, value = line.split(':', 1)
                props[key] = value
        return props

    def _getQueryProperties(self):
        output = self._getQMakeOutput(['-query'])
        self._query_dict = self._parseQueryProperties(output)

    def _parseQtBuildType(self):
        key = 'QT_CONFIG'
        if key not in self._mkspecs_dict:
            return None

        qt_config = self._mkspecs_dict[key]
        if 'debug_and_release' in qt_config:
            return 'debug_and_release'

        split = qt_config.split(' ')
        if 'release' in split and 'debug' in split:
            return 'debug_and_release'

        if 'release' in split:
            return 'release'

        if 'debug' in split:
            return 'debug'

        return None

    def _getOtherProperties(self):
        # Get the src property separately, because it is not returned by qmake unless explicitly
        # specified.
        key = 'QT_INSTALL_PREFIX/src'
        result = self._getQMakeOutput(['-query', key])
        self._query_dict[key] = result

        # Get mkspecs variables and cache them.
        self._getQMakeMkspecsVariables()

        # Get macOS minimum deployment target.
        key = 'QMAKE_MACOSX_DEPLOYMENT_TARGET'
        if key in self._mkspecs_dict:
            self._query_dict[key] = self._mkspecs_dict[key]

        # Figure out how Qt was built: debug mode, release mode, or both.
        build_type = self._parseQtBuildType()
        if build_type:
            self._query_dict['BUILD_TYPE'] = build_type

    def _initProperties(self):
        self._getQueryProperties()
        self._getOtherProperties()

    def _getQMakeMkspecsVariables(self):
        # Create empty temporary qmake project file.
        temp_file_name = 'qmake_fake_empty_project.txt'
        open(temp_file_name, 'a').close()

        # Query qmake for all of its mkspecs variables.
        qmakeOutput = self._getQMakeOutput(['-E', temp_file_name])
        lines = [s.strip() for s in qmakeOutput.splitlines()]
        pattern = re.compile(r"^(.+?)=(.+?)$")
        for line in lines:
            found = pattern.search(line)
            if found:
                key = found.group(1).strip()
                value = found.group(2).strip()
                self._mkspecs_dict[key] = value

        # We need to clean up after qmake, which always creates a .qmake.stash file after a -E
        # invocation.
        qmake_stash_file = os.path.join(os.getcwd(), ".qmake.stash")
        if os.path.exists(qmake_stash_file):
            os.remove(qmake_stash_file)

        # Also clean up the temporary empty project file.
        if os.path.exists(temp_file_name):
            os.remove(temp_file_name)

    version = property(getVersion)
    bins_dir = property(getBinsPath)
    libs_dir = property(getLibsPath)
    lib_execs_dir = property(getLibsExecsPath)
    plugins_dir = property(getPluginsPath)
    prefix_dir = property(getPrefixPath)
    qmake_command = property(getQMakeCommand)
    imports_dir = property(getImportsPath)
    translations_dir = property(getTranslationsPath)
    headers_dir = property(getHeadersPath)
    docs_dir = property(getDocsPath)
    qml_dir = property(getQmlPath)
    macos_min_deployment_target = property(getMacOSMinDeploymentTarget)
    build_type = property(getBuildType)
    src_dir = property(getSrcDir)
