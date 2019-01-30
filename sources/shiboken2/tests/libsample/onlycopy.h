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

#ifndef ONLYCOPYCLASS_H
#define ONLYCOPYCLASS_H

#include "libsamplemacros.h"
#include <list>

// These classes simulate a situation found in QWebEngineHistoryItem.

class OnlyCopyPrivate;

class LIBSAMPLE_API OnlyCopy
{
public:
    OnlyCopy(const OnlyCopy& other);
    OnlyCopy& operator=(const OnlyCopy& other);
    ~OnlyCopy();

    int value() const;
    static int getValue(OnlyCopy onlyCopy) { return onlyCopy.value(); }
    static int getValueFromReference(const OnlyCopy& onlyCopy) { return onlyCopy.value(); }
private:
    OnlyCopyPrivate *d;
    explicit OnlyCopy(int value);
    explicit OnlyCopy(OnlyCopyPrivate *d); // rejected due to unknown OnlyCopyPrivate
    friend class FriendOfOnlyCopy;
};

class LIBSAMPLE_API FriendOfOnlyCopy
{
public:
    static OnlyCopy createOnlyCopy(int value);
    static std::list<OnlyCopy> createListOfOnlyCopy(int quantity);
};

#endif
