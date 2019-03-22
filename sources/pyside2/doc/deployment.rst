==========
Deployment
==========

Deploying or freezing an application is a crucial part of many Python projects.
Most large projects are not based on a single Python file, so
the distribution of these applications becomes more difficult.

The options for a project are:
 1. Sending a normal zip-file with the application's content.
 2. Building a proper `Python package (wheel) <https://packaging.python.org/>`_.
 3. Freezing the application in a single binary file, or into a directory.

For the **third** option, there are many available tools:
 * `PyInstaller <https://www.pyinstaller.org/>`_,
 * `cx_Freeze <https://anthony-tuininga.github.io/cx_Freeze/>`_,
 * `py2exe <http://www.py2exe.org/>`_,
 * `py2app <https://py2app.readthedocs.io/en/latest/>`_,

Since |project| is a cross-platform framework,
we would like to focus on solutions that at least work on
the three major platform supported by Qt: Linux, macOS, and Windows.

The following table summarizes the above mentioned tools support:

===========   =======   =====   =====   =======
Name          License   Linux   macOS   Windows
===========   =======   =====   =====   =======
py2exe        MIT       no      no      yes
py2app        MIT       no      yes     no
cx_Freeze     MIT       yes     yes     yes
PyInstaller   GPL       yes     yes     yes
===========   =======   =====   =====   =======

From the table we can see that only *cx_Freeze* and *PyInstaller*
meet our requirements.

All tools are command-line based, and it could become
a hard task to include more resources to your application, such as
images, icons, and meta-information, because you will need to create
special hooks or separate scripts to handle them.
Additionally, since this only
allows you to freeze your current application, you don't have
any mechanism to update your application.

To cover the update part, there is a tool built around PyInstaller
called `PyUpdater <https://www.pyupdater.org/>`_ which enables
a simple mechanism to ship applications updates.

On top of all these features, including also a nice interface
that allows the user to install the application step by step,
or even better, provide templates to create new projects to easily
freeze-them-up is something really beneficial for both developers
and end-users.
This is where `fbs <https://build-system.fman.io>`_ enters the
game, being based on PyInstaller, but including all the nice features
we previously mentioned.

Here you can find a set of tutorials on how to use the previously
described tools.

.. note:: Deployment is possible only in Qt for Python 5.12.2

.. toctree::
    :name: mastertoc
    :maxdepth: 2

    deployment-pyinstaller.rst
    deployment-cxfreeze.rst
    deployment-fbs.rst
