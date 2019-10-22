#############################################################################
##
## Copyright (C) 2018 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of Qt for Python.
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

"""
scrape_testresults.py

Read the testresults website of COIN and find the pages that contain an
embedded exists_{platform}_{version}_ci.py .

The found pages will then be sorted by date/time and put into the registry.

This program utilizes the multiprocessing package for speedy access to
the web pages. The program works well in typically less than half an hour.

After the cache has been created, the runtime is substantially smaller.

"""

import sys
if sys.version_info[:2] < (3, 6):
    print("This program is written for Python 3.6 or higher.")
    sys.exit(1)

DEMO_URL = ("https://testresults.qt.io/coin/api/results/pyside/pyside-setup/"
            # The above URL part is fixed.
            "30c1193ec56a86b8d0920c325185b9870f96941e/"
            "MacOSMacOS_10_12x86_64MacOSMacOS_10_12x86_64Clangqtci-macos-"
                "10.12-x86_64-8-425364DebugAndRelease_Release/"
            "d80c5d4547ea2b3d74188bd458955aae39cb32b4/"
            "test_1535865484/"
            "log.txt.gz")

from bs4 import BeautifulSoup
from datetime import datetime
from multiprocessing import Pool
from textwrap import dedent
import requests
import os
import time
import re
import json
import argparse

my_name = __file__ if __file__.endswith(".py") else __file__[:-1]
test_path = os.path.join(os.path.dirname(__file__), "testresults", "embedded")
if not os.path.exists(test_path):
    os.makedirs(test_path)
cache_path = os.path.dirname(test_path)
target_path = os.path.dirname(__file__)
start_time = time.time()

def get_name(url):
    """
    Return the last piece of an url, including trailing slash.

    In effect, this undoes the accumulation of URL pieces.
    """
    name = url.rstrip("/").rsplit("/", 1)[-1]
    if url.endswith("/"):
        name += "/"
    return name

def rel_url(url):
    """
    throw the top URL away
    """
    return url[len(top_url):]

stop_all = False

def find_all_links(text, url, ignore=()):
    """
    Find all links in a page.

    Only simple links are allowed. That means safe characters and
    at most one "/" at the end for directories.
    """
    global stop_all
    soup = BeautifulSoup(text, "html.parser")
    lis = soup.find_all("a")
    names = list(row["href"] for row in lis)
    names = list(name for name in names if name not in ignore)
    for name in names:
        if not re.match(r"^[A-Za-z0-9_\-.]+/?$", name):
            print("Unexpected character in link:", name)
            # Not clear how to terminate the pool quick and clean.
            # We crash badly in handle_suburl_tup, ugly but works.
            stop_all = True
            return []
    urls = list(url + name for name in names)
    return urls

def read_url(url):
    # We intentionally let things fail, because we re-run things on failure.
    try:
        response = requests.get(url)
    except requests.exceptions.ContentDecodingError as e:
        # This is a permanent error which is in the data. We ignore that.
        print(os.getpid(), "Decoding Error:", e)
        print(os.getpid(), "Cannot fix this, ignored.")
        return None
    except requests.exceptions.RequestException as e:
        print("Read error:", e)
        raise
    else:
        return response

def get_timestamp(text):
    # agent:2018/06/29 15:02:15
    global stop_all
    prefix = "\nagent:"
    try:
        startpos = text.index(prefix)
    except ValueError:
        print("this is not the usual format of COIN log files")
        stop_all = True
        raise
    startpos += len(prefix)
    text = text[startpos : startpos + 80]
    ts = text[:19]
    ts = re.sub(r'[^0-9]', '_', ts)
    # check that it is a valid time stamp
    try:
        datetime.strptime(ts, "%Y_%m_%d_%H_%M_%S")
    except ValueError as e:
        print("Unexpected time stamp", e)
        stop_all = True
        raise
    return ts

def write_data(name, text):
    try:
        ts = get_timestamp(text)
    except ValueError:
        print()
        print(name)
        print()
        print(text)
        raise
    lines = text.split("\n")
    for idx, line in enumerate(lines):
        if "BEGIN_FILE" in line:
            start = idx + 1
            offset = line.index("BEGIN_FILE")
        if "END_FILE" in line:
            stop = idx
    lines = lines[start : stop]
    if offset:
        lines = list(line[offset:] for line in lines)
    # fix the lines - the original has no empty line after "# eof"
    while lines[-1] == "":
        lines.pop()
    text = "\n".join(lines) + "\n"
    modname = re.search(r"'(..*?)'", text).group(1)
    fn = os.path.join(test_path, f"{ts}-{name}-{modname}.py")
    if os.path.exists(fn):
        # do not change the file, we want to skip it
        return
    with open(fn, "w") as f:
        f.write(text)

def update_license(text):
    end_license = text.index("\n\n")
    with open(my_name) as fi:
        my_text = fi.read()
    my_end_license = my_text.index("\n\n")
    text = my_text[:my_end_license] + text[end_license:]
    return text

def eval_data(force=False):
    """
    Read all found files, sort them and keep the latest version.
    """
    files = []
    for entry in os.scandir(test_path):
        if "exists_" in entry.name and entry.name.endswith(".py"):
            if force or os.path.getmtime(entry.path) >= start_time:
                # this file is newly created
                files.append(entry.path)
    files.sort()
    # read the files and update in chronological order
    results = {}
    for fn in files:
        with open(fn) as f:
            text = f.read()
        modname = re.search("'(..*?)'", text).group(1)
        results[modname] = text
    for fn in results:
        name = os.path.join(target_path, fn + ".py")
        with open(name, "w") as f:
            f.write(update_license(results[fn]))
        print("+++ generated:", name)
    return len(results)

def handle_suburl(idx, n, url, level):
    if level == 1:
        print(os.getpid(), "Reading", idx+1, "of", n, rel_url(url))
    response = read_url(url)
    urls = find_all_links(response.text, url)
    for sub_url in urls:
        name = get_name(sub_url)
        if name.endswith("/"):
            if name.startswith("build_"):
                continue
            if name == "tasks/":
                continue
            handle_suburl(0, 0, sub_url, level + 1)
        else:
            if name.startswith("log.txt"):
                test_name = sub_url.split("/")[-2]
                print(os.getpid(), test_name)
                response = read_url(sub_url)
                if response and "BEGIN_FILE" in response.text:
                    print(os.getpid(), test_name, "FOUND!")
                    write_data(test_name, response.text)
                else:
                    print(os.getpid(), test_name)


def handle_suburl_tup(idx_n_url_level):
    if stop_all:
        return # bad solution, but it stops fast
    idx, n, url, level = idx_n_url_level
    try:
        ret = handle_suburl(idx, n, url, level)
        return url, None
    except requests.exceptions.RequestException as e:
        return url, e

def handle_batch(urls, level):
    n = len(urls)
    args = ((idx, n, url, level) for (idx, url) in enumerate(urls))
    with Pool(10) as p:
        records = list(p.imap_unordered(handle_suburl_tup, args))
    # re-read the failed ones
    runs = [n]
    for idx in range(10):
        urls = list(x[0] for x in records if x[-1])
        if not urls:
            break
        print("Pausing 5 seconds")
        time.sleep(5)
        n = len(urls)
        runs.append(n)
        args = ((idx, n, url, level) for (idx, url) in enumerate(urls))
        with Pool(10) as p:
            records = list(p.imap_unordered(handle_suburl_tup, args))
    # Return success when the remaining URLs are empty.
    print("Runs:", ", ".join(map(str, runs)))
    return not urls

def handle_topurl(url):
    """
    Find all links to directories.

    We maintain a cache of these links. The cache is only updated
    when all URLs have been successfully processed.
    """
    try:
        response = requests.get(url)
    except requests.exceptions.RequestException as e:
        print("Skipped", e)
        return
    global top_url
    top_url = url
    urls = find_all_links(response.text, url, ignore=("tasks/",))
    work_urls = set(urls)
    cache_file = os.path.join(cache_path, "known_urls.json")
    if os.path.exists(cache_file):
        with open(cache_file, 'r') as fp:
            known_urls = json.load(fp)
            work_urls -= set(known_urls)
    level = 1
    for sub_url in work_urls:
        name = get_name(sub_url)
        if name.endswith("/"):
            if name.startswith("build_"):
                continue
            work_urls.add(sub_url)
    success = handle_batch(work_urls, 1)
    if success:
        with open(cache_file, 'w') as fp:
            json.dump(urls, fp, sort_keys=True, indent=4)
    return success

def get_test_results(starturl):
    ok = handle_topurl(starturl)
    stop_time = time.time()
    runtime = stop_time - start_time
    hours, remainder = divmod(runtime, 3600)
    minutes, seconds = divmod(remainder, 60)

    runtime_formatted = '%d:%02d:%06.3f' % (hours, minutes, seconds)
    print(f"Run time: {runtime_formatted}s")
    if ok:
        found = eval_data()
        print(f"Successful scan, {found} new files.")
        if found:
            print("Please check if a git push is necessary.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        usage=dedent(f"""\
            {os.path.basename(my_name)} [-h] scan

            Scan the COIN testresults website for embedded exists_{{platf}}_{{version}}_ci.py files.

            Warning: On the first call, this script may take almost 30 minutes to run.
            Subsequent calls are *much* faster due to caching.

            {os.path.basename(my_name)} [-h] eval

            Enforces evaluation when a scan did not complete yet.

            For more information, see the file
                sources/shiboken2/libshiboken/signature_doc.rst
            """))
    subparsers = parser.add_subparsers(dest="command", metavar="", title="required argument")
    # create the parser for the "scan" command
    parser_scan = subparsers.add_parser("scan", help="run the scan")
    parser_eval = subparsers.add_parser("eval", help="force evaluation")
    args = parser.parse_args()
    if not args.command:
        parser.print_usage()
        exit(1)
    if args.command == "scan":
        # Using this from the intranet require an internal URL
        get_test_results("https://testresults.qt.io/coin/api/results/pyside/pyside-setup/")
    elif args.command == "eval":
        eval_data(force=True)
