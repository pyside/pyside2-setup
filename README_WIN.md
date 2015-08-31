================
Notes On Windows
================

Quick and unsorted:

Building a debug PySide
-----------------------

Usually, you would use a pre-compiled, installed Windows 2.7/3.4 release for
building PySide. But if you need a debug build, which makes much more
sense during bug hunting, things are a little more difficult.

To do the minimum, I used a Python repository with a 3.4 checkout. You needto use
this version when you have VS2010. Python 3.5 makes a huge jump to VS2015, and VS2010
does not work with it because of new libraries.

When you have done a minimum build with VS2010 in debug 32 bit, then you need to create a 'Libs'
folder beneath 'Lib' and copy all .lib, .pyc, .dll and .pcb files into it.
Furthermore, you need to add the folders 'Include' and 'PC' to the INCLUDE variable.

You can now use this python for pyside-setup2's setup.py and provide the parameters:

```
	[ppath]python_d.exe setup.py develop --ignore-git --debug --qmake=[qpath]bin/qmake.exe
```
