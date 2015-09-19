#PySide2

This is the main repository for PySide2, the latest and greatest version of the Python bindings of Qt. 
PySide 2 supports Qt4 and Qt5. For building, please read about [gettting the dependencies](https://github.com/PySide/pyside-setup2/wiki/Dependencies). Then download the sources by running `git clone --recursive https://github.com/PySide/pyside-setup2.git`.

###Building

####Windows
On Windows, once you have gotten the dependencies and the source, `cd pyside-setup2.git` to enter the directory and then:
```
python setup.py install --qmake=\path\to\bin\qmake --cmake=/path/to/bin/cmake --openssl=/path/to/openssl/bin
```

