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

#include "doxygenparser.h"
#include "abstractmetalang.h"
#include "reporthandler.h"

#include <QtXmlPatterns/QXmlQuery>

namespace
{

QString getSectionKindAttr(const AbstractMetaFunction* func)
{
    if (func->isSignal()) {
        return QLatin1String("signal");
    } else {
        QString kind = func->isPublic() ? QLatin1String("public") : QLatin1String("protected");
        if (func->isStatic())
            kind += QLatin1String("-static");
        else if (func->isSlot())
            kind += QLatin1String("-slot");
        return kind;
    }
}

}

Documentation DoxygenParser::retrieveModuleDocumentation()
{
        return retrieveModuleDocumentation(packageName());
}

void DoxygenParser::fillDocumentation(AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return;

    QString doxyFileSuffix;
    if (metaClass->enclosingClass()) {
        doxyFileSuffix += metaClass->enclosingClass()->name();
        doxyFileSuffix += "_1_1"; // FIXME: Check why _1_1!!
    }
    doxyFileSuffix += metaClass->name();
    doxyFileSuffix += ".xml";

    const char* prefixes[] = { "class", "struct", "namespace" };
    const int numPrefixes = sizeof(prefixes) / sizeof(const char*);
    bool isProperty = false;

    QString doxyFilePath;
    for (int i = 0; i < numPrefixes; ++i) {
        doxyFilePath = documentationDataDirectory() + "/" + prefixes[i] + doxyFileSuffix;
        if (QFile::exists(doxyFilePath))
            break;
        doxyFilePath.clear();
    }

    if (doxyFilePath.isEmpty()) {
        ReportHandler::warning("Can't find doxygen file for class "
                               + metaClass->name() + ", tried: "
                               + documentationDataDirectory() + "/{struct|class|namespace}"
                               + doxyFileSuffix);
        return;
    }
    QXmlQuery xquery;
    xquery.setFocus(QUrl(doxyFilePath));

    // Get class documentation
    QString classDoc = getDocumentation(xquery, "/doxygen/compounddef/detaileddescription",
                                        metaClass->typeEntry()->docModifications());
    if (classDoc.isEmpty()) {
        ReportHandler::warning("Can't find documentation for class \""
                               + metaClass->name() + "\".");
    }
    metaClass->setDocumentation(classDoc);

    //Functions Documentation
    AbstractMetaFunctionList funcs = metaClass->functionsInTargetLang();
    foreach (AbstractMetaFunction *func, funcs) {
        if (!func || func->isPrivate())
            continue;

        QString query = "/doxygen/compounddef/sectiondef";
        // properties
        if (func->isPropertyReader() || func->isPropertyWriter()
            || func->isPropertyResetter()) {
            query += "[@kind=\"property\"]/memberdef/name[text()=\""
                     + func->propertySpec()->name() + "\"]";
            isProperty = true;
        } else { // normal methods
            QString kind = getSectionKindAttr(func);
            query += "[@kind=\"" + kind + "-func\"]/memberdef/name[text()=\""
                     + func->originalName() + "\"]";

            if (func->arguments().isEmpty()) {
                QString args = func->isConstant() ? "() const " : "()";
                query += "/../argsstring[text()=\"" + args + "\"]";
            } else {
                int i = 1;
                foreach (AbstractMetaArgument* arg, func->arguments()) {
                    QString type;
                    if (!arg->type()->isPrimitive()) {
                        query += "/../param[" + QString::number(i) + "]/type/ref[text()=\""
                                 + arg->type()->name() + "\"]/../..";
                    } else {
                        query += "/../param[" + QString::number(i) + "]/type[text()=\""
                                 + arg->type()->name() + "\"]/..";
                    }
                    ++i;
                }
            }
        }
        if (!isProperty) {
            query += "/../detaileddescription";
        } else {
            query = "(" + query;
            query += "/../detaileddescription)[1]";
        }
        QString doc = getDocumentation(xquery, query, DocModificationList());
        func->setDocumentation(doc);
        isProperty = false;
    }

    //Fields
    AbstractMetaFieldList fields = metaClass->fields();
    foreach (AbstractMetaField *field, fields) {
        if (field->isPrivate())
            return;

        QString query = "/doxygen/compounddef/sectiondef/memberdef/name[text()=\""
                        + field->name() + "\"]/../detaileddescription";
        QString doc = getDocumentation(xquery, query, DocModificationList());
        field->setDocumentation(doc);
    }

    //Enums
    AbstractMetaEnumList enums = metaClass->enums();
    foreach (AbstractMetaEnum *meta_enum, enums) {
        QString query = "/doxygen/compounddef/sectiondef/memberdef[@kind=\"enum\"]/name[text()=\"" + meta_enum->name() + "\"]/..";
        QString doc = getDocumentation(xquery, query, DocModificationList());
        meta_enum->setDocumentation(doc);
    }

}

Documentation DoxygenParser::retrieveModuleDocumentation(const QString& name){

    QString sourceFile = documentationDataDirectory() + '/' + "indexpage.xml";

    if (!QFile::exists(sourceFile)) {
        ReportHandler::warning("Can't find doxygen XML file for module "
                               + name + ", tried: "
                                                 + sourceFile);
        return Documentation();
    }

    QXmlQuery xquery;
    xquery.setFocus(QUrl(sourceFile));

    // Module documentation
    QString query = "/doxygen/compounddef/detaileddescription";
    return Documentation(getDocumentation(xquery, query, DocModificationList()));
}

