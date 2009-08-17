/*
 * This file is part of the Boost Python Generator project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include <QtCore/QDebug>
#include <fileout.h>
#include "convertergenerator.h"

static Indentor INDENT;

ConverterGenerator::ConverterGenerator()
{
    // QPair
    m_conversions << qMakePair(QString("QPair<"), &m_qpairTypes);
    // QList
    m_conversions << qMakePair(QString("QList<"), &m_qlistTypes);
    // QVector
    m_conversions << qMakePair(QString("QVector<"), &m_qvectorTypes);
    // QMap
    m_conversions << qMakePair(QString("QMap<"), &m_qmapTypes);
    // QHash
    m_conversions << qMakePair(QString("QHash<"), &m_qhashTypes);
    // QMultiMap
    m_conversions << qMakePair(QString("QMultiMap<"), &m_qmultiMapTypes);

}

void ConverterGenerator::finishGeneration()
{
    if (!classes().size())
        return;

    QString fileOutPath;

    foreach (AbstractMetaClass *cls, classes()) {
        if (!shouldGenerate(cls))
            continue;

        if (fileOutPath.isNull()) {
            m_packageName = cls->package();
            fileOutPath = outputDirectory() + '/' + subDirectoryForClass(cls)
                          + "/converter_register_" + moduleName().toLower() + ".hpp";
        }

        foreach (AbstractMetaFunction* func, filterFunctions(cls))
            checkFunctionMetaTypes(func);
    }

    FileOut fileOut(fileOutPath);
    QTextStream& s = fileOut.stream;

    // write license comment
    s << licenseComment() << endl;

    s << "#ifndef CONVERTERREGISTER_" << moduleName().toUpper() << "_HPP\n";
    s << "#define CONVERTERREGISTER_" << moduleName().toUpper() << "_HPP\n\n";

    //Includes
    QStringList includes;
    foreach (AbstractMetaClass *cls, classes()) {
        if (cls->typeEntry()->include().isValid()) {
            QString include_file = cls->typeEntry()->include().toString();
            if (!includes.contains(include_file)) {
                s << include_file << endl;
                includes << include_file;
            }
        }

        if (cls->typeEntry()->generateCode()) {
            QList<Include> extra_includes = cls->typeEntry()->extraIncludes();
            foreach (Include include, extra_includes) {
                if (!includes.contains(include.toString())) {
                    s << include.toString() << endl;
                    includes << include.toString();
                }
            }
        }
    }

    s << "#include \"type_converter.hpp\"\n\n";

    s << "void register_type_converters_" << moduleName().toLower() << "()\n{\n";
    Indentation indent(INDENT);
    writeConverterRegistration(s, "register_qpair_converter", "QPair", m_qpairTypes);
    writeConverterRegistration(s, "register_container_converter", "QList", m_qlistTypes);
    writeConverterRegistration(s, "register_container_converter", "QVector", m_qvectorTypes);
    writeConverterRegistration(s, "register_dict_converter", "QMap", m_qmapTypes);
    writeConverterRegistration(s, "register_dict_converter", "QHash", m_qhashTypes);
    writeConverterRegistration(s, "register_multimap_converter", "QMultiMap", m_qmultiMapTypes);
    s << "}\n\n";
    s << "#endif\n\n";

    m_numGeneratedWritten = m_qpairTypes.size() + m_qlistTypes.size() +
                              m_qvectorTypes.size() + m_qmapTypes.size() +
                              m_qhashTypes.size();
}

void ConverterGenerator::writeConverterRegistration(QTextStream& out,
                                                    const QString& funcName,
                                                    const QString& type,
                                                    const QSet<QString>& params)
{
    foreach (QString param, params) {
        QString completeType(QMetaObject::normalizedType(
                (type + '<' + param + " >").toLatin1().data()));
        out << INDENT << "PySide::" << funcName;
        out << '<' << completeType << " >(\"";
        out << completeType << "\");" << endl;
    }
}

void ConverterGenerator::checkFunctionMetaTypes(AbstractMetaFunction* func)
{
    if (func->type())
        checkMetaType(functionReturnType(func));

    foreach (AbstractMetaArgument* arg, func->arguments()) {
        if (arg->type()) {
            checkMetaType(argumentString(func, arg,
                          (Generator::SkipName | Generator::SkipDefaultValues)));
        }
    }
}

// FIXME Use some AbstracyMetaAnything info instead of parse the cpp signature?
void ConverterGenerator::checkMetaType(const QString& cppSignature)
{
    QRegExp typeRegex("Q\\w+");

    foreach (Conversion conv, m_conversions) {
        int index = cppSignature.indexOf(conv.first);
        if (index >= 0) {
            QString templateArg = extractTemplateArgument(cppSignature.right(cppSignature.length() - index - conv.first.length()));
            conv.second->insert(templateArg);
            // detect types to generate includes
            int offset = 0;
            while ((offset = typeRegex.indexIn(templateArg, offset)) != -1) {
                const QString cap(typeRegex.cap(0));
                offset += cap.length();
            }
        }
    }
}

QString ConverterGenerator::extractTemplateArgument(const QString& templateParams)
{
    int stack = 0;
    for (int i = 0; i < templateParams.length(); ++i) {
        QChar c = templateParams[i];
        if (c == '<') {
            stack++;
        } else if (c == '>') {
            stack--;
            if (stack < 0)
                return templateParams.left(i).trimmed();
        }
    }
    Q_ASSERT(false);
    return QString();
}

