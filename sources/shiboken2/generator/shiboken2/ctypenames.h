/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
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

#ifndef CTYPENAMES_H
#define CTYPENAMES_H

#include <QtCore/QString>

static inline QString boolT() { return QStringLiteral("bool"); }
static inline QString intT() { return QStringLiteral("int"); }
static inline QString unsignedT() { return QStringLiteral("unsigned"); }
static inline QString unsignedIntT() { return QStringLiteral("unsigned int"); }
static inline QString longT() { return QStringLiteral("long"); }
static inline QString unsignedLongT() { return QStringLiteral("unsigned long"); }
static inline QString shortT() { return QStringLiteral("short"); }
static inline QString unsignedShortT() { return QStringLiteral("unsigned short"); }
static inline QString unsignedCharT() { return QStringLiteral("unsigned char"); }
static inline QString longLongT() { return QStringLiteral("long long"); }
static inline QString unsignedLongLongT() { return QStringLiteral("unsigned long long"); }
static inline QString charT() { return QStringLiteral("char"); }
static inline QString floatT() { return QStringLiteral("float"); }
static inline QString doubleT() { return QStringLiteral("double"); }
static inline QString constCharPtrT() { return QStringLiteral("const char*"); }

static inline QString qByteArrayT() { return QStringLiteral("QByteArray"); }
static inline QString qMetaObjectT() { return QStringLiteral("QMetaObject"); }
static inline QString qObjectT() { return QStringLiteral("QObject"); }
static inline QString qStringT() { return QStringLiteral("QString"); }
static inline QString qVariantT() { return QStringLiteral("QVariant"); }

#endif // CTYPENAMES_H
