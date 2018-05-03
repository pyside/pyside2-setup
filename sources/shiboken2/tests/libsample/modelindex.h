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

#ifndef MODELINDEX_H
#define MODELINDEX_H

#include "libsamplemacros.h"

class ModelIndex
{
public:
    ModelIndex() : m_value(0) {}
    ModelIndex(const ModelIndex& other) { m_value = other.m_value; }
    inline void setValue(int value) { m_value = value; }
    inline int value() const { return m_value; }
    static int getValue(const ModelIndex& index) { return index.value(); }
private:
    int m_value;
};

class ReferentModelIndex
{
public:
    ReferentModelIndex() {}
    ReferentModelIndex(const ModelIndex& index) : m_index(index) {}
    ReferentModelIndex(const ReferentModelIndex& other) { m_index = other.m_index; }
    inline void setValue(int value) { m_index.setValue(value); }
    inline int value() const { return m_index.value(); }
    operator const ModelIndex&() const { return m_index; }
private:
    ModelIndex m_index;
};

class PersistentModelIndex
{
public:
    PersistentModelIndex() {}
    PersistentModelIndex(const ModelIndex& index) : m_index(index) {}
    PersistentModelIndex(const PersistentModelIndex& other) { m_index = other.m_index; }
    inline void setValue(int value) { m_index.setValue(value); }
    inline int value() const { return m_index.value(); }
    operator ModelIndex() const { return m_index; }
private:
    ModelIndex m_index;
};

#endif
