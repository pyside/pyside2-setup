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

#include "asttoxml.h"
#include "parser/control.h"
#include "parser/parser.h"
#include "parser/binder.h"


#include <QtCore/QXmlStreamWriter>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>

typedef QHash<QString, EnumModelItem> EnumMap;
typedef QHash<QString, FunctionModelItem> FunctionModelItemMap;
typedef QHash<QString, ClassModelItem> ClassModelItemMap;
typedef QHash<QString, NamespaceModelItem> NamespaceModelItemMap;

void astToXML(QString name)
{
    QFile file(name);

    if (!file.open(QFile::ReadOnly))
        return;

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    QByteArray contents = stream.readAll().toUtf8();
    file.close();

    Control control;
    Parser p(&control);
    pool __pool;

    TranslationUnitAST *ast = p.parse(contents, contents.size(), &__pool);

    CodeModel model;
    Binder binder(&model, p.location());
    FileModelItem dom = binder.run(ast);

    QFile outputFile;
    if (!outputFile.open(stdout, QIODevice::WriteOnly))
        return;

    QXmlStreamWriter s(&outputFile);
    s.setAutoFormatting(true);

    s.writeStartElement(QLatin1String("code"));

    const NamespaceList &namespaces = dom->namespaces();
    foreach (const NamespaceModelItem &n, namespaces)
        writeOutNamespace(s, n);

    const ClassModelItemMap &typeMap = dom->classMap();
    for (ClassModelItemMap::const_iterator it = typeMap.cbegin(), end = typeMap.cend(); it != end; ++it)
        writeOutClass(s, it.value());

    s.writeEndElement();
}

void writeOutNamespace(QXmlStreamWriter &s, const NamespaceModelItem &item)
{
    s.writeStartElement(QLatin1String("namespace"));
    s.writeAttribute(QLatin1String("name"), item->name());

    const NamespaceList &namespaces = item->namespaces();
    foreach (const NamespaceModelItem &n, namespaces)
        writeOutNamespace(s, n);

    const ClassModelItemMap &typeMap = item->classMap();
    for (ClassModelItemMap::const_iterator it = typeMap.cbegin(), end = typeMap.cend(); it != end; ++it)
        writeOutClass(s, it.value());

    const EnumList &enums = item->enums();
    foreach (const EnumModelItem &e, enums)
        writeOutEnum(s, e);

    s.writeEndElement();
}

void writeOutEnum(QXmlStreamWriter &s, const EnumModelItem &item)
{
    QString qualifiedName = item->qualifiedName().join(QLatin1String("::"));
    s.writeStartElement(QLatin1String("enum"));
    s.writeAttribute(QLatin1String("name"), qualifiedName);

    EnumeratorList enumList = item->enumerators();
    for (int i = 0; i < enumList.size() ; i++) {
        s.writeStartElement(QLatin1String("enumerator"));
        if (!enumList[i]->value().isEmpty())
            s.writeAttribute(QLatin1String("value"), enumList[i]->value());
        s.writeCharacters(enumList[i]->name());

        s.writeEndElement();
    }
    s.writeEndElement();
}

void writeOutFunction(QXmlStreamWriter &s, const FunctionModelItem &item)
{
    QString qualifiedName = item->qualifiedName().join(QLatin1String("::"));
    s.writeStartElement(QLatin1String("function"));
    s.writeAttribute(QLatin1String("name"), qualifiedName);

    ArgumentList arguments = item->arguments();
    for (int i = 0; i < arguments.size() ; i++) {
        s.writeStartElement(QLatin1String("argument"));
        s.writeAttribute(QLatin1String("type"), arguments[i]->type().qualifiedName().join(QLatin1String("::")));
        s.writeEndElement();
    }
    s.writeEndElement();
}

void writeOutClass(QXmlStreamWriter &s, const ClassModelItem &item)
{
    QString qualifiedName = item->qualifiedName().join(QLatin1String("::"));
    s.writeStartElement(QLatin1String("class"));
    s.writeAttribute(QLatin1String("name"), qualifiedName);

    const EnumList &enums = item->enums();
    foreach (const EnumModelItem &e, enums)
        writeOutEnum(s, e);

    const FunctionModelItemMap &functionMap = item->functionMap();
    for (FunctionModelItemMap::const_iterator it = functionMap.cbegin(), end = functionMap.cend(); it != end; ++it)
        writeOutFunction(s, it.value());

    const ClassModelItemMap &typeMap = item->classMap();
    for (ClassModelItemMap::const_iterator it = typeMap.cbegin(), end = typeMap.cend(); it != end; ++it)
        writeOutClass(s, it.value());

    s.writeEndElement();
}

