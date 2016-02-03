import sys
import subprocess
from distutils.spawn import find_executable

class QtInfo(object):
    def __init__(self, qmake_command=None):
        if qmake_command:
            self._qmake_command = qmake_command
        else:
            self._qmake_command = [find_executable("qmake"),]

    def getQMakePath(self):
        qmake_path = self._qmake_path[0] # Guessing the first of the commands is qmake
        if not os.path.exists(qmake_path): # Making it here a bit more failsave.
            qmake_path = find_executable(qmake_path) # In case someone just passed the name of the executable I'll search for it now.
            if qmake_path == None: # If we couldn't resolv the location of the executable
                raise ValueError("Could not find the location of %s. Please pass the absolute path to qmake or please pass the correct name of the executable." %(qmake_path))
        return qmake_path

    def getVersion(self):
        return self.getProperty("QT_VERSION")

    def getBinsPath(self):
        return self.getProperty("QT_INSTALL_BINS")

    def getLibsPath(self):
        return self.getProperty("QT_INSTALL_LIBS")

    def getPluginsPath(self):
        return self.getProperty("QT_INSTALL_PLUGINS")

    def getImportsPath(self):
        return self.getProperty("QT_INSTALL_IMPORTS")

    def getTranslationsPath(self):
        return self.getProperty("QT_INSTALL_TRANSLATIONS")

    def getHeadersPath(self):
        return self.getProperty("QT_INSTALL_HEADERS")

    def getDocsPath(self):
        return self.getProperty("QT_INSTALL_DOCS")

    def getProperty(self, prop_name):
        cmd = self._qmake_command + ["-query", prop_name]
        proc = subprocess.Popen(cmd, stdout = subprocess.PIPE, shell=False)
        prop = proc.communicate()[0]
        proc.wait()
        if proc.returncode != 0:
            return None
        if sys.version_info >= (3,):
            return str(prop, 'ascii').strip()
        return prop.strip()

    version = property(getVersion)
    bins_dir = property(getBinsPath)
    libs_dir = property(getLibsPath)
    plugins_dir = property(getPluginsPath)
    qmake_command = property(getQMakePath)
    imports_dir = property(getImportsPath)
    translations_dir = property(getTranslationsPath)
    headers_dir = property(getHeadersPath)
    docs_dir = property(getDocsPath)
