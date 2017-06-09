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
#ifndef DOCGENERATOR_H
#define DOCGENERATOR_H

#include <QtCore/QStack>
#include <QtCore/QHash>
#include <QtCore/QTextStream>
#include <QXmlStreamReader>
#include "generator.h"
#include "docparser.h"
#include "typesystem_enums.h"
#include "typesystem_typedefs.h"

class QtDocParser;
class AbstractMetaFunction;
class AbstractMetaClass;
QT_BEGIN_NAMESPACE
class QXmlStreamReader;
QT_END_NAMESPACE
class QtDocGenerator;

class QtXmlToSphinx
{
public:
    struct TableCell
    {
        short rowSpan;
        short colSpan;
        QString data;

        TableCell(const QString& text = QString()) : rowSpan(0), colSpan(0), data(text) {}
        TableCell(const char* text) : rowSpan(0), colSpan(0), data(QLatin1String(text)) {}
    };

    typedef QList<TableCell> TableRow;
    class Table : public QList<TableRow>
    {
        public:
            Table() : m_hasHeader(false), m_normalized(false)
            {
            }

            void enableHeader(bool enable)
            {
                m_hasHeader = enable;
            }

            bool hasHeader() const
            {
                return m_hasHeader;
            }

            void normalize();

            bool isNormalized() const
            {
                return m_normalized;
            }

            void clear() {
                m_normalized = false;
                QList<TableRow>::clear();
            }

        private:
            bool m_hasHeader;
            bool m_normalized;
    };

    QtXmlToSphinx(QtDocGenerator* generator, const QString& doc, const QString& context = QString());

    QString result() const
    {
        return m_result;
    }

private:
    QString resolveContextForMethod(const QString& methodName);
    QString expandFunction(const QString& function);
    QString transform(const QString& doc);

    void handleHeadingTag(QXmlStreamReader& reader);
    void handleParaTag(QXmlStreamReader& reader);
    void handleItalicTag(QXmlStreamReader& reader);
    void handleBoldTag(QXmlStreamReader& reader);
    void handleArgumentTag(QXmlStreamReader& reader);
    void handleSeeAlsoTag(QXmlStreamReader& reader);
    void handleSnippetTag(QXmlStreamReader& reader);
    void handleDotsTag(QXmlStreamReader& reader);
    void handleLinkTag(QXmlStreamReader& reader);
    void handleImageTag(QXmlStreamReader& reader);
    void handleListTag(QXmlStreamReader& reader);
    void handleTermTag(QXmlStreamReader& reader);
    void handleSuperScriptTag(QXmlStreamReader& reader);
    void handleQuoteFileTag(QXmlStreamReader& reader);

    // table tagsvoid QtXmlToSphinx::handleValueTag(QXmlStreamReader& reader)

    void handleTableTag(QXmlStreamReader& reader);
    void handleRowTag(QXmlStreamReader& reader);
    void handleItemTag(QXmlStreamReader& reader);
    void handleRawTag(QXmlStreamReader& reader);
    void handleCodeTag(QXmlStreamReader& reader);

    void handleIgnoredTag(QXmlStreamReader& reader);
    void handleUnknownTag(QXmlStreamReader& reader);
    void handleUselessTag(QXmlStreamReader& reader);
    void handleAnchorTag(QXmlStreamReader& reader);

    typedef void (QtXmlToSphinx::*TagHandler)(QXmlStreamReader&);
    QHash<QString, TagHandler> m_handlerMap;
    QStack<TagHandler> m_handlers;
    QTextStream m_output;
    QString m_result;

    QStack<QString*> m_buffers;


    Table m_currentTable;
    bool m_tableHasHeader;
    QString m_context;
    QtDocGenerator* m_generator;
    bool m_insideBold;
    bool m_insideItalic;
    QString m_lastTagName;
    QString m_opened_anchor;

    QString readFromLocations(const QStringList& locations, const QString& path, const QString& identifier);
    QString readFromLocation(const QString& location, const QString& identifier, bool* ok = 0);
    void pushOutputBuffer();
    QString popOutputBuffer();
    void writeTable(Table& table);
};

inline QTextStream& operator<<(QTextStream& s, const QtXmlToSphinx& xmlToSphinx)
{
    return s << xmlToSphinx.result();
}

QTextStream& operator<<(QTextStream& s, const QtXmlToSphinx::Table &table);

/**
*   The DocGenerator generates documentation from library being binded.
*/
class QtDocGenerator : public Generator
{
public:
    QtDocGenerator();
    ~QtDocGenerator();

    QString libSourceDir() const
    {
        return m_libSourceDir;
    }

    bool doSetup(const QMap<QString, QString>& args);

    const char* name() const
    {
        return "QtDocGenerator";
    }

    OptionDescriptions options() const;

    QStringList codeSnippetDirs() const
    {
        return m_codeSnippetDirs;
    }

protected:
    QString fileNamePrefix() const;
    QString fileNameForContext(GeneratorContext &context) const;
    void generateClass(QTextStream &s, GeneratorContext &classContext);
    bool finishGeneration();

    void writeFunctionArguments(QTextStream&, const AbstractMetaFunction*, Options) const {}
    void writeArgumentNames(QTextStream&, const AbstractMetaFunction*, Options) const {}

private:
    void writeEnums(QTextStream& s, const AbstractMetaClass* cppClass);

    void writeFields(QTextStream &s, const AbstractMetaClass *cppClass);
    void writeArguments(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaFunction *func);
    void writeFunctionSignature(QTextStream& s, const AbstractMetaClass* cppClass, const AbstractMetaFunction* func);
    void writeFunction(QTextStream& s, bool writeDoc, const AbstractMetaClass* cppClass, const AbstractMetaFunction* func);
    void writeFunctionParametersType(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaFunction* func);
    void writeFunctionList(QTextStream& s, const AbstractMetaClass* cppClass);
    void writeFunctionBlock(QTextStream& s, const QString& title, QStringList& functions);
    void writeParamerteType(QTextStream &s, const AbstractMetaClass *cppClass, const AbstractMetaArgument *arg);

    void writeConstructors(QTextStream &s, const AbstractMetaClass *cppClass);
    void writeFormatedText(QTextStream& s, const Documentation& doc, const AbstractMetaClass* metaclass = 0);
    bool writeInjectDocumentation(QTextStream& s, TypeSystem::DocModificationMode mode, const AbstractMetaClass* cppClass, const AbstractMetaFunction* func);
    void writeDocSnips(QTextStream &s, const CodeSnipList &codeSnips, TypeSystem::CodeSnipPosition position, TypeSystem::Language language);


    QString parseArgDocStyle(const AbstractMetaClass *cppClass, const AbstractMetaFunction *func);
    QString translateToPythonType(const AbstractMetaType *type, const AbstractMetaClass *cppClass);

    QString m_docDataDir;
    QString m_libSourceDir;
    QStringList m_codeSnippetDirs;
    QString m_extraSectionDir;
    QStringList m_functionList;
    QMap<QString, QStringList> m_packages;
    DocParser* m_docParser;
};

#endif // DOCGENERATOR_H
