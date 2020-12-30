#!/usr/bin/env python

#############################################################################
##
## Copyright (C) 2016 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the test suite of Qt for Python.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import time,os

class TimeoutException(Exception):
   def __init__(self, msg):
      self.msg = msg

   def __str__(self):
      return repr(self.msg)

class ProcessTimer(object):
   '''Timeout function for controlling a subprocess.Popen instance.

   Naive implementation using busy loop, see later other means
   of doing this.
   '''

   def __init__(self, proc, timeout):
      self.proc = proc
      self.timeout = timeout

   def waitfor(self):
      time_passed = 0
      while(self.proc.poll() is None and time_passed < self.timeout):
         time_passed = time_passed + 1
         time.sleep(1)

      if time_passed >= self.timeout:
         raise TimeoutException("Timeout expired, possible deadlock")

if __name__ == "__main__":
   #simple example

   from subprocess import Popen

   proc = Popen(['sleep','10'])
   t = ProcessTimer(proc,5)
   try:
      t.waitfor()
   except TimeoutException:
      print("timeout - PID: %d" % (t.proc.pid))
      #TODO: detect SO and kill accordingly
      #Linux
      os.kill(t.proc.pid, 9)
      #Windows (not tested)
      #subprocess.Popen("taskkill /F /T /PID %i"%handle.pid , shell=True)
   print("exit code: %d" % (t.proc.poll()))

