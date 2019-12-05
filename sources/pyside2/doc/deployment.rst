|project| Deployment
====================

Deploying or freezing an application is a crucial part of many Python projects.
Most large projects are not based on a single Python file, so
the distribution of these applications becomes more difficult.

Here are a few distribution options that you could use:
 1. Sending a normal zip-file with the application's content.
 2. Building a proper `Python package (wheel) <https://packaging.python.org/>`_.
 3. Freezing the application into a single binary file or a directory.

If you choose the **third** option, consider using one of these tools:
 * `fbs`_
 * `PyInstaller`_
 * `cx_Freeze`_
 * `py2exe`_
 * `py2app`_

.. _fbs: https://build-system.fman.io/
.. _PyInstaller: https://www.pyinstaller.org/
.. _cx_Freeze: https://anthony-tuininga.github.io/cx_Freeze/
.. _py2exe: http://www.py2exe.org/
.. _py2app: https://py2app.readthedocs.io/en/latest/

|project| is a cross-platform framework,
so we would like to focus on solutions that work on the three
major platforms supported by Qt: Linux, macOS, and Windows.
The following table summarizes the platform support for those packaging
tools:

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
      </tbody>
    </table>

According to this table, only *fbs*, *cx_Freeze*, and *PyInstaller*
meets our cross-platform requirement.

As these are command-line tools, it could be hard to include
resources to your application, such as images, icons, and
meta-information. This means, you will need special hooks
or scripts to handle them before adding to the package.
In addition to this, these tools does not offer a mechanism
to update your application packages.

To create update packages, use the `PyUpdater <https://www.pyupdater.org/>`_,
which is built around PyInstaller.

The `fbs`_ tool offers a nice UI
that allows the user to install the application step-by-step.

Here you can find a set of tutorials on how to use the previously
described tools.

.. note::

   Deployment is supported only from Qt for Python 5.12.2 and later.

.. toctree::
    :name: mastertoc
    :maxdepth: 2

    deployment-fbs.rst
    deployment-pyinstaller.rst
    deployment-cxfreeze.rst
