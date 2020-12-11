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
#ifndef DOCGENERATOR_H
#define DOCGENERATOR_H

#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QScopedPointer>

#include "generator.h"
#include "documentation.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"
#include "qtxmltosphinxinterface.h"

class DocParser;

/**
*   The DocGenerator generates documentation from library being binded.
*/
class QtDocGenerator : public Generator, public QtXmlToSphinxDocGeneratorInterface
{
public:
    QtDocGenerator();
    ~QtDocGenerator();

    bool doSetup() override;

    const char* name() const override
    {
        return "QtDocGenerator";
    }

    OptionDescriptions options() const override;
    bool handleOption(const QString &key, const QString &value) override;

    // QtXmlToSphinxDocGeneratorInterface
    QString expandFunction(const QString &function) const override;
    QString expandClass(const QString &context,
                        const QString &name) const override;
    QString resolveContextForMethod(const QString &context,
                                    const QString &methodName) const override;
    const QLoggingCategory &loggingCategory() const override;

protected:
    bool shouldGenerate(const AbstractMetaClass *) const override;
    QString fileNameSuffix() const override;
    QString fileNameForContext(const GeneratorContext &context) const override;
    void generateClass(TextStream &ts, const GeneratorContext &classContext) override;
    bool finishGeneration() override;

private:
    void writeEnums(TextStream& s, const AbstractMetaClass* cppClass) const;

    void writeFields(TextStream &s, const AbstractMetaClass *cppClass) const;
    static QString functionSignature(const AbstractMetaClass* cppClass,
                                     const AbstractMetaFunctionCPtr &func);
    void writeFunction(TextStream& s, const AbstractMetaClass* cppClass,
                       const AbstractMetaFunctionCPtr &func, bool indexed = true);
    void writeFunctionParametersType(TextStream &s, const AbstractMetaClass *cppClass,
                                     const AbstractMetaFunctionCPtr &func) const;
    void writeFunctionList(TextStream& s, const AbstractMetaClass* cppClass);
    void writeFunctionBlock(TextStream& s, const QString& title, QStringList& functions);
    void writeParameterType(TextStream &s, const AbstractMetaClass *cppClass,
                            const AbstractMetaArgument &arg) const;

    void writeConstructors(TextStream &s, const AbstractMetaClass *cppClass) const;
    void writeFormattedText(TextStream &s, const Documentation &doc,
                            const AbstractMetaClass *metaclass = nullptr,
                            Documentation::Type docType = Documentation::Detailed) const;
    bool writeInjectDocumentation(TextStream& s, TypeSystem::DocModificationMode mode,
                                  const AbstractMetaClass* cppClass,
                                  const AbstractMetaFunctionCPtr &func);
    void writeDocSnips(TextStream &s, const CodeSnipList &codeSnips, TypeSystem::CodeSnipPosition position, TypeSystem::Language language);

    void writeModuleDocumentation();
    void writeAdditionalDocumentation() const;

    static QString parseArgDocStyle(const AbstractMetaClass *cppClass,
                                    const AbstractMetaFunctionCPtr &func);
    QString translateToPythonType(const AbstractMetaType &type, const AbstractMetaClass *cppClass) const;

    bool convertToRst(const QString &sourceFileName,
                      const QString &targetFileName,
                      const QString &context = QString(),
                      QString *errorMessage = nullptr) const;

    QString m_extraSectionDir;
    QStringList m_functionList;
    QMap<QString, QStringList> m_packages;
    QScopedPointer<DocParser> m_docParser;
    QtXmlToSphinxParameters m_parameters;
    QString m_additionalDocumentationList;
};

#endif // DOCGENERATOR_H
