/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of PySide2.
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

#ifndef SHIBOKENNORMALIZE_P_H
#define SHIBOKENNORMALIZE_P_H

#include <QMetaObject>
#include <QByteArray>


#if (QT_VERSION < QT_VERSION_CHECK(4, 7, 0))
    QByteArray QMetaObject_normalizedTypeQt47(const char *type);
    QByteArray QMetaObject_normalizedSignatureQt47(const char *type);

    #define SBK_NORMALIZED_TYPE(x) QMetaObject_normalizedTypeQt47(x)
    #define SBK_NORMALIZED_SIGNATURE(x) QMetaObject_normalizedSignatureQt47(x)
#else
    #define SBK_NORMALIZED_TYPE(x) QMetaObject::normalizedType(x)
    #define SBK_NORMALIZED_SIGNATURE(x) QMetaObject::normalizedSignature(x)
#endif

#endif //SHIBOKENNORMALIZE_P_H
