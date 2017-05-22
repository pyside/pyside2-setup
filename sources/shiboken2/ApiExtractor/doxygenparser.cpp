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
#include "typesystem.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QFile>
#include <QtCore/QDir>

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
        doxyFileSuffix += QLatin1String("_1_1"); // FIXME: Check why _1_1!!
    }
    doxyFileSuffix += metaClass->name();
    doxyFileSuffix += QLatin1String(".xml");

    const char* prefixes[] = { "class", "struct", "namespace" };
    const int numPrefixes = sizeof(prefixes) / sizeof(const char*);
    bool isProperty = false;

    QString doxyFilePath;
    for (int i = 0; i < numPrefixes; ++i) {
        doxyFilePath = documentationDataDirectory() + QLatin1Char('/')
                       + QLatin1String(prefixes[i]) + doxyFileSuffix;
        if (QFile::exists(doxyFilePath))
            break;
        doxyFilePath.clear();
    }

    if (doxyFilePath.isEmpty()) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find doxygen file for class " << metaClass->name() << ", tried: "
            << QDir::toNativeSeparators(documentationDataDirectory())
            <<  "/{struct|class|namespace}"<< doxyFileSuffix;
        return;
    }
    QXmlQuery xquery;
    xquery.setFocus(QUrl(doxyFilePath));

    // Get class documentation
    QString classDoc = getDocumentation(xquery, QLatin1String("/doxygen/compounddef/detaileddescription"),
                                        metaClass->typeEntry()->docModifications());
    if (classDoc.isEmpty()) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find documentation for class \"" << metaClass->name() << "\".";
    }
    metaClass->setDocumentation(classDoc);

    //Functions Documentation
    AbstractMetaFunctionList funcs = metaClass->functionsInTargetLang();
    foreach (AbstractMetaFunction *func, funcs) {
        if (!func || func->isPrivate())
            continue;

        QString query = QLatin1String("/doxygen/compounddef/sectiondef");
        // properties
        if (func->isPropertyReader() || func->isPropertyWriter()
            || func->isPropertyResetter()) {
            query += QLatin1String("[@kind=\"property\"]/memberdef/name[text()=\"")
                     + func->propertySpec()->name() + QLatin1String("\"]");
            isProperty = true;
        } else { // normal methods
            QString kind = getSectionKindAttr(func);
            query += QLatin1String("[@kind=\"") + kind
                     + QLatin1String("-func\"]/memberdef/name[text()=\"")
                     + func->originalName() + QLatin1String("\"]");

            if (func->arguments().isEmpty()) {
                QString args = func->isConstant() ? QLatin1String("() const ") : QLatin1String("()");
                query += QLatin1String("/../argsstring[text()=\"") + args + QLatin1String("\"]");
            } else {
                int i = 1;
                foreach (AbstractMetaArgument* arg, func->arguments()) {
                    QString type;
                    if (!arg->type()->isPrimitive()) {
                        query += QLatin1String("/../param[") + QString::number(i)
                                 + QLatin1String("]/type/ref[text()=\"")
                                 + arg->type()->name() + QLatin1String("\"]/../..");
                    } else {
                        query += QLatin1String("/../param[") + QString::number(i)
                                 + QLatin1String("]/type[text()=\"")
                                 + arg->type()->name() + QLatin1String("\"]/..");
                    }
                    ++i;
                }
            }
        }
        if (!isProperty) {
            query += QLatin1String("/../detaileddescription");
        } else {
            query = QLatin1Char('(') + query;
            query += QLatin1String("/../detaileddescription)[1]");
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

        QString query = QLatin1String("/doxygen/compounddef/sectiondef/memberdef/name[text()=\"")
                        + field->name() + QLatin1String("\"]/../detaileddescription");
        QString doc = getDocumentation(xquery, query, DocModificationList());
        field->setDocumentation(doc);
    }

    //Enums
    AbstractMetaEnumList enums = metaClass->enums();
    foreach (AbstractMetaEnum *meta_enum, enums) {
        QString query = QLatin1String("/doxygen/compounddef/sectiondef/memberdef[@kind=\"enum\"]/name[text()=\"")
            + meta_enum->name() + QLatin1String("\"]/..");
        QString doc = getDocumentation(xquery, query, DocModificationList());
        meta_enum->setDocumentation(doc);
    }

}

Documentation DoxygenParser::retrieveModuleDocumentation(const QString& name){

    QString sourceFile = documentationDataDirectory() + QLatin1String("/indexpage.xml");

    if (!QFile::exists(sourceFile)) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find doxygen XML file for module " << name << ", tried: "
            << QDir::toNativeSeparators(sourceFile);
        return Documentation();
    }

    QXmlQuery xquery;
    xquery.setFocus(QUrl(sourceFile));

    // Module documentation
    QString query = QLatin1String("/doxygen/compounddef/detaileddescription");
    return Documentation(getDocumentation(xquery, query, DocModificationList()));
}

