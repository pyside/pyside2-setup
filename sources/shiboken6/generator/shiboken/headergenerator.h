/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef HEADERGENERATOR_H
#define HEADERGENERATOR_H

#include "shibokengenerator.h"

#include <QtCore/QSet>

class AbstractMetaFunction;

/**
 *   The HeaderGenerator generate the declarations of C++ bindings classes.
 */
class HeaderGenerator : public ShibokenGenerator
{
public:
    OptionDescriptions options() const override { return OptionDescriptions(); }

    const char *name() const override { return "Header generator"; }

protected:
    QString fileNameSuffix() const override;
    QString fileNameForContext(const GeneratorContext &context) const override;
    void generateClass(TextStream &s, const GeneratorContext &classContext) override;
    bool finishGeneration() override;

private:
    void writeCopyCtor(TextStream &s, const AbstractMetaClass *metaClass) const;
    void writeProtectedFieldAccessors(TextStream &s, const AbstractMetaField &field) const;
    void writeFunction(TextStream &s, const AbstractMetaFunctionCPtr &func);
    void writeSbkTypeFunction(TextStream &s, const AbstractMetaEnum &cppEnum) const;
    void writeSbkTypeFunction(TextStream &s, const AbstractMetaClass *cppClass) const;
    void writeSbkTypeFunction(TextStream &s, const AbstractMetaType &metaType) const;
    void writeTypeIndexValueLine(TextStream &s, const TypeEntry *typeEntry) const;
    void writeTypeIndexValueLines(TextStream &s, const AbstractMetaClass *metaClass) const;
    void writeProtectedEnumSurrogate(TextStream &s, const AbstractMetaEnum &cppEnum) const;
    void writeInheritedOverloads(TextStream &s) const;

    QSet<AbstractMetaFunctionCPtr> m_inheritedOverloads;
};

#endif // HEADERGENERATOR_H

