#############################################################################
##
## Copyright (C) 2019 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

import re
import sys
from textwrap import dedent
from argparse import ArgumentParser, Namespace, RawTextHelpFormatter
from subprocess import check_output, Popen, PIPE
from typing import Dict, List

content = """
Qt for Python @VERSION is a @TYPE release.

For more details, refer to the online documentation included in this
distribution. The documentation is also available online:

https://doc.qt.io/qtforpython/

Some of the changes listed in this file include issue tracking numbers
corresponding to tasks in the Qt Bug Tracker:

https://bugreports.qt.io/

Each of these identifiers can be entered in the bug tracker to obtain more
information about a particular change.

****************************************************************************
*                                  PySide2                                 *
****************************************************************************

@PYSIDE

****************************************************************************
*                                  Shiboken2                               *
****************************************************************************

@SHIBOKEN
"""


def parse_options() -> Namespace:
    tag_msg = ("Tags, branches, or SHA to compare\n"
               "e.g.: v5.12.1..5.12\n"
               "      v5.12.0..v5.12.1\n"
               "      cebc32a5..5.12")

    options = ArgumentParser(description="PySide2 changelog tool",
                             formatter_class=RawTextHelpFormatter)
    options.add_argument("-d",
                         "--directory",
                         type=str,
                         help="Repository directory, '.' is default.")
    options.add_argument("-v",
                         "--versions",
                         type=str,
                         help=tag_msg,
                         required=True)
    options.add_argument("-r",
                         "--release",
                         type=str,
                         help="Release version: e.g.: 5.12.4",
                         required=True)
    options.add_argument("-t",
                         "--type",
                         type=str,
                         help="Release type: bug-fix, minor, or major",
                         default="bug-fix")

    args = options.parse_args()
    if args.type not in ("bug-fix", "minor", "major"):
        print("Error:"
              "-y/--type needs to be: bug-fix (default), minor, or major")
        sys.exit(-1)

    return args


def check_tag(tag: str) -> bool:
    output = False

    if tag[0] == "v":
        # Git tag
        command = "git tag -l {}".format(tag)
        print("{}: {}".format(check_tag.__name__, command), file=sys.stderr)
        if check_output(command.split()):
            output = True
    elif re.match(r"^\d\.\d?", tag):
        # Git branch (origin)
        command = "git show-ref {}".format(tag)
        print("{}: {}".format(check_tag.__name__, command), file=sys.stderr)
        if check_output(command.split()):
            output = True
    else:
        # Git sha
        command = "git cat-file -t {}".format(tag)
        print("{}: {}".format(check_tag.__name__, command), file=sys.stderr)
        if check_output(command.split()):
            output = True

    return output


def get_commit_content(sha: str) -> str:
    command= "git log {} -n 1 --pretty=format:%s%n%n%b".format(sha)
    print("{}: {}".format(get_commit_content.__name__, command), file=sys.stderr)
    out, err = Popen(command, stdout=PIPE, shell=True).communicate()
    return out.decode("utf-8")


def git_command(versions: List[str], pattern: str):
    command = "git rev-list --reverse --grep '^{}'".format(pattern)
    command += " {}..{}".format(versions[0], versions[1])
    command += " | git cat-file --batch"
    command += " | grep -o -E \"^[0-9a-f]{40}\""
    print("{}: {}".format(git_command.__name__, command), file=sys.stderr)
    out_sha1, err = Popen(command, stdout=PIPE, shell=True).communicate()
    sha1_list = [s.decode("utf-8") for s in out_sha1.splitlines()]

    for sha in sha1_list:
        content = get_commit_content(sha).splitlines()
        # First line is title
        title = content[0]
        # Look for PYSIDE-XXXX
        task = None
        for line in content[::-1]:
            if line.startswith(pattern):
                task = line.replace("{}:".format(pattern), "").strip()
                break

        if not task:
            continue
        if "shiboken" in title:
            if sha not in shiboken2_commits:
                shiboken2_commits[sha] = {"title": title, "task": task}
        else:
            if sha not in pyside2_commits:
                pyside2_commits[sha] = {"title": title, "task": task}


def create_fixes_log(versions: List[str]) -> None:
    git_command(versions, "Fixes")


def create_task_log(versions: List[str]) -> None:
    git_command(versions, "Task-number")


def gen_list(d: Dict[str, Dict[str, str]]) -> str:
    if d:
        return "".join(" - [{}] {}\n".format(v["task"], v["title"])
                       for _, v in d.items())
    else:
        return " - No changes"

def sort_dict(d: Dict[str, Dict[str, str]]) -> Dict[str, Dict[str, str]]:
    return dict(sorted(d.items(),
        key=lambda kv: "{:5d}".format(
            int(kv[1]['task'].replace("PYSIDE-", "")))))

if __name__ == "__main__":

    args = parse_options()
    pyside2_commits: Dict[str, Dict[str, str]] = {}
    shiboken2_commits: Dict[str, Dict[str, str]] = {}

    # Getting commits information
    directory = args.directory if args.directory else "."
    versions = args.versions.split("..")
    if len(versions) == 2:
        if check_tag(versions[0]) and check_tag(versions[1]):
            create_fixes_log(versions)
            create_task_log(versions)

    # Sort commits
    pyside2_commits = sort_dict(pyside2_commits)
    shiboken2_commits = sort_dict(shiboken2_commits)

    # Generate message
    print(content
        .replace("@VERSION", args.release)
        .replace("@TYPE", args.type)
        .replace("@PYSIDE", gen_list(pyside2_commits))
        .replace("@SHIBOKEN", gen_list(shiboken2_commits)))
