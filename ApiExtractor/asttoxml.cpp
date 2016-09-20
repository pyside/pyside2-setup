/*
 * This file is part of the API Extractor project.
 *
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 *
 * Contact: PySide team <contact@pyside.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

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

    const NamespaceModelItemMap &namespaceMap = dom->namespaceMap();
    for (NamespaceModelItemMap::const_iterator it = namespaceMap.cbegin(), end = namespaceMap.cend(); it != end; ++it)
        writeOutNamespace(s, it.value());

    const ClassModelItemMap &typeMap = dom->classMap();
    for (ClassModelItemMap::const_iterator it = typeMap.cbegin(), end = typeMap.cend(); it != end; ++it)
        writeOutClass(s, it.value());

    s.writeEndElement();
}

void writeOutNamespace(QXmlStreamWriter &s, const NamespaceModelItem &item)
{
    s.writeStartElement(QLatin1String("namespace"));
    s.writeAttribute(QLatin1String("name"), item->name());

    const NamespaceModelItemMap &namespaceMap = item->namespaceMap();
    for (NamespaceModelItemMap::const_iterator it = namespaceMap.cbegin(), end = namespaceMap.cend(); it != end; ++it)
        writeOutNamespace(s, it.value());

    const ClassModelItemMap &typeMap = item->classMap();
    for (ClassModelItemMap::const_iterator it = typeMap.cbegin(), end = typeMap.cend(); it != end; ++it)
        writeOutClass(s, it.value());

    const EnumMap &enumMap = item->enumMap();
    for (EnumMap::const_iterator it = enumMap.cbegin(), end = enumMap.cend(); it != end; ++it)
        writeOutEnum(s, it.value());

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

    const EnumMap &enumMap = item->enumMap();
    for (EnumMap::const_iterator it = enumMap.cbegin(), end = enumMap.cend(); it != end; ++it)
        writeOutEnum(s, it.value());

    const FunctionModelItemMap &functionMap = item->functionMap();
    for (FunctionModelItemMap::const_iterator it = functionMap.cbegin(), end = functionMap.cend(); it != end; ++it)
        writeOutFunction(s, it.value());

    const ClassModelItemMap &typeMap = item->classMap();
    for (ClassModelItemMap::const_iterator it = typeMap.cbegin(), end = typeMap.cend(); it != end; ++it)
        writeOutClass(s, it.value());

    s.writeEndElement();
}

