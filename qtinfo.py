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
        return self._qmake_path

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
