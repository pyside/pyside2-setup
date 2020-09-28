|project| Deployment
====================


Deploying or freezing an application is an important part of a Python project,
this means to bundle all required resources so that the application finds everything it needs to
be able to run on a client's machine.
However, because most large projects aren't based on a single Python file, distributing these
applications can be a challenge.

Here are a few distribution options that you can use:
 1. Send a normal ZIP file with the application's content.
 2. Build a proper `Python package (wheel) <https://packaging.python.org/>`_.
 3. Freeze the application into a single binary file or directory.
 4. Provide native installer (msi, dmg)

If you choose Option 3, consider using one of these tools:
 * `fbs`_
 * `PyInstaller`_
 * `cx_Freeze`_
 * `py2exe`_
 * `py2app`_
 * `briefcase`_

.. _fbs: https://build-system.fman.io/
.. _PyInstaller: https://www.pyinstaller.org/
.. _cx_Freeze: https://anthony-tuininga.github.io/cx_Freeze/
.. _py2exe: http://www.py2exe.org/
.. _py2app: https://py2app.readthedocs.io/en/latest/
.. _briefcase: https://briefcase.readthedocs.io

Since |project| is a cross-platform framework, we focus on solutions for the three major
platforms that Qt supports: Windows, Linux, and macOS.

The following table summarizes the platform support for those packaging tools:

.. raw:: html

    <table class="docutils align-default">
      <thead>
        <tr>
          <th class="head">Name</th>
          <th class="head">License</th>
          <th class="head">Linux</th>
          <th class="head">macOS</th>
          <th class="head">Windows</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><p>fbs</p></td>
          <td><p>GPL</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
        </tr>
        <tr>
          <td><p>PyInstaller</p></td>
          <td><p>GPL</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
        </tr>
        <tr>
          <td><p>cx_Freeze</p></td>
          <td><p>MIT</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
        </tr>
        <tr>
          <td><p>py2exe</p></td>
          <td><p>MIT</p></td>
          <td><p style="color: red;">no</p></td>
          <td><p style="color: red;">no</p></td>
          <td><p style="color: green;">yes</p></td>
        </tr>
        <tr>
          <td><p>py2app</p></td>
          <td><p>MIT</p></td>
          <td><p style="color: red;">no</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: red;">no</p></td>
        </tr>
        <tr>
          <td><p>briefcase</p></td>
          <td><p>BSD3</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
          <td><p style="color: green;">yes</p></td>
        </tr>
      </tbody>
    </table>

Notice that only *fbs*, *cx_Freeze*, *briefcase*, and *PyInstaller* meet our cross-platform requirement.

Since these are command-line tools, you'll need special hooks or scripts to handle resources
such as images, icons, and meta-information, before adding them to your package. Additionally,
these tools don't offer a mechanism to update your application packages.

To create update packages, use the `PyUpdater <https://www.pyupdater.org/>`_, which is a tool
built around PyInstaller.

The `fbs`_ tool offers a nice UI for the user to install the
application step-by-step.

.. note::

   Deployment is supported only from Qt for Python 5.12.2 and later.

Here's a set of tutorials on how to use these tools:

.. toctree::
    :name: mastertoc
    :maxdepth: 2

    deployment-fbs.rst
    deployment-pyinstaller.rst
    deployment-cxfreeze.rst
    deployment-briefcase.rst
