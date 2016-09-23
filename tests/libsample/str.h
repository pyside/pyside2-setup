/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of PySide2.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef STR_H
#define STR_H
#include <string>

#include "libsamplemacros.h"

class LIBSAMPLE_API Str
{
public:
    Str(const Str& s);
    Str(char c);
    Str(const char* cstr = "");
    ~Str();

    Str arg(const Str& s) const;

    Str& append(const Str& s);
    Str& prepend(const Str& s);

    const char* cstring() const;
    char get_char(int pos) const;
    bool set_char(int pos, char ch);

    int toInt(bool* ok = 0, int base = 10) const;

    void show() const;

    inline int size() const { return m_str.size(); }

    // nonsense operator just to test reverse operators
    Str operator+(int number) const;
    bool operator==(const Str& other) const;
    bool operator<(const Str& other) const;

private:
    void init(const char* cstr);
    std::string m_str;

    friend LIBSAMPLE_API Str operator+(int number, const Str& str);
    friend LIBSAMPLE_API unsigned int strHash(const Str& str);
};

LIBSAMPLE_API Str operator+(int number, const Str& str);
LIBSAMPLE_API unsigned int strHash(const Str& str);

typedef Str PStr;
LIBSAMPLE_API void changePStr(PStr* pstr, const char* suffix);
LIBSAMPLE_API void duplicatePStr(PStr* pstr = 0);

#endif // STR_H
