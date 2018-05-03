/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt for Python.
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

#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include "str.h"
#include "libsamplemacros.h"
#include <vector>

class LIBSAMPLE_API ByteArray
{
public:
    ByteArray();
    ByteArray(char data);
    ByteArray(const char* data);
    ByteArray(const char* data, int len);
    ByteArray(const ByteArray& other);

    int size() const;
    char at(int i) const;
    char operator[](int i) const;

    const char* data() const;

    ByteArray& append(char c);
    ByteArray& append(const char* data);
    ByteArray& append(const char* data, int len);
    ByteArray& append(const ByteArray& other);

    bool operator==(const ByteArray& other) const;
    bool operator!=(const ByteArray& other) const;

    ByteArray& operator+=(char c);
    ByteArray& operator+=(const char* data);
    ByteArray& operator+=(const ByteArray& other);

    static unsigned int hash(const ByteArray& byteArray);
private:
    std::vector<char> m_data;
    friend LIBSAMPLE_API bool operator==(const ByteArray& ba1, const char* ba2);
    friend LIBSAMPLE_API bool operator==(const char* ba1, const ByteArray& ba2);
    friend LIBSAMPLE_API bool operator!=(const ByteArray& ba1, const char* ba2);
    friend LIBSAMPLE_API bool operator!=(const char* ba1, const ByteArray& ba2);

    friend LIBSAMPLE_API ByteArray operator+(const ByteArray& ba1, const ByteArray& ba2);
    friend LIBSAMPLE_API ByteArray operator+(const ByteArray& ba1, const char* ba2);
    friend LIBSAMPLE_API ByteArray operator+(const char* ba1, const ByteArray& ba2);
    friend LIBSAMPLE_API ByteArray operator+(const ByteArray& ba1, char ba2);
    friend LIBSAMPLE_API ByteArray operator+(char ba1, const ByteArray& ba2);
};

LIBSAMPLE_API bool operator==(const ByteArray& ba1, const char* ba2);
LIBSAMPLE_API bool operator==(const char* ba1, const ByteArray& ba2);
LIBSAMPLE_API bool operator!=(const ByteArray& ba1, const char* ba2);
LIBSAMPLE_API bool operator!=(const char* ba1, const ByteArray& ba2);

LIBSAMPLE_API ByteArray operator+(const ByteArray& ba1, const ByteArray& ba2);
LIBSAMPLE_API ByteArray operator+(const ByteArray& ba1, const char* ba2);
LIBSAMPLE_API ByteArray operator+(const char* ba1, const ByteArray& ba2);
LIBSAMPLE_API ByteArray operator+(const ByteArray& ba1, char ba2);
LIBSAMPLE_API ByteArray operator+(char ba1, const ByteArray& ba2);

#endif // BYTEARRAY_H
