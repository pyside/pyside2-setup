/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt for Python project.
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

#ifndef TESTVIEW_H
#define TESTVIEW_H

#include <QObject>
#ifdef pysidetest_EXPORTS
#define PYSIDE_EXPORTS 1
#endif
#include "pysidemacros.h"

QT_BEGIN_NAMESPACE
class QWidget;
class QAbstractListModel;
class QAbstractItemDelegate;
QT_END_NAMESPACE

class PYSIDE_API TestView : public QObject
{
    Q_OBJECT
public:
    TestView(QAbstractListModel* model, QObject* parent = 0) : QObject(parent), m_model(model) {}
    QAbstractListModel* model() { return m_model; }
    QVariant getData();

    void setItemDelegate(QAbstractItemDelegate* delegate) { m_delegate = delegate; }
    QWidget* getEditorWidgetFromItemDelegate() const;

private:
    QAbstractListModel* m_model;
    QAbstractItemDelegate* m_delegate;
};

#endif // TESTVIEW_H

