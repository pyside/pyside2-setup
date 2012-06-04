import subprocess
from distutils.spawn import find_executable
from popenasync import Popen

class QtInfo(object):
    def __init__(self, qmake_path=None):
        if qmake_path:
            self._qmake_path = qmake_path
        else:
            self._qmake_path = find_executable("qmake")

    def getQMakePath(self):
        return self._qmake_path

    def getVersion(self):
        return self.getProperty("QT_VERSION")

    def getBinsPath(self):
        return self.getProperty("QT_INSTALL_BINS")

    def getPluginsPath(self):
        return self.getProperty("QT_INSTALL_PLUGINS")

    def getImportsPath(self):
        return self.getProperty("QT_INSTALL_IMPORTS")

    def getTranslationsPath(self):
        return self.getProperty("QT_INSTALL_TRANSLATIONS")

    def getProperty(self, prop_name):
        cmd = [self._qmake_path, "-query", prop_name]
        proc = Popen(cmd,
            stdin = subprocess.PIPE,
            stdout = subprocess.PIPE, 
            stderr = subprocess.STDOUT,
            universal_newlines = 1,
            shell=False)
        prop = ''
        while proc.poll() is None:
            prop += proc.read_async(wait=0.1, e=0)
        proc.wait()
        if proc.returncode != 0:
            return None
        return prop.strip()

    version = property(getVersion)
    bins_dir = property(getBinsPath)
    plugins_dir = property(getPluginsPath)
    qmake_path = property(getQMakePath)
    imports_dir = property(getImportsPath)
    translations_dir = property(getTranslationsPath)
