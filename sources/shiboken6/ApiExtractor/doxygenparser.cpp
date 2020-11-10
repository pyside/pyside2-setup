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

#include "doxygenparser.h"
#include "abstractmetalang.h"
#include "abstractmetafield.h"
#include "abstractmetafunction.h"
#include "abstractmetaenum.h"
#include "messages.h"
#include "modifications.h"
#include "propertyspec.h"
#include "reporthandler.h"
#include "typesystem.h"
#include "xmlutils.h"

#include <QtCore/QFile>
#include <QtCore/QDir>

static QString getSectionKindAttr(const AbstractMetaFunction *func)
{
    if (func->isSignal())
        return QLatin1String("signal");
    QString kind = func->isPublic()
        ? QLatin1String("public") : QLatin1String("protected");
    if (func->isStatic())
        kind += QLatin1String("-static");
    else if (func->isSlot())
        kind += QLatin1String("-slot");
    return kind;
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
    bool isProperty = false;

    QString doxyFilePath;
    for (const char *prefix : prefixes) {
        doxyFilePath = documentationDataDirectory() + QLatin1Char('/')
                       + QLatin1String(prefix) + doxyFileSuffix;
        if (QFile::exists(doxyFilePath))
            break;
        doxyFilePath.clear();
    }

    if (doxyFilePath.isEmpty()) {
        qCWarning(lcShibokenDoc).noquote().nospace()
            << "Can't find doxygen file for class " << metaClass->name() << ", tried: "
            << QDir::toNativeSeparators(documentationDataDirectory())
            <<  "/{struct|class|namespace}"<< doxyFileSuffix;
        return;
    }

    QString errorMessage;
    XQueryPtr xquery = XQuery::create(doxyFilePath, &errorMessage);
    if (xquery.isNull()) {
        qCWarning(lcShibokenDoc, "%s", qPrintable(errorMessage));
        return;
    }

    static const QList<QPair<Documentation::Type, QString>> docTags = {
        { Documentation::Brief,  QLatin1String("briefdescription") },
        { Documentation::Detailed,  QLatin1String("detaileddescription") }
    };
    // Get class documentation
    Documentation classDoc;

    for (const auto &tag : docTags) {
        const QString classQuery = QLatin1String("/doxygen/compounddef/") + tag.second;
        QString doc = getDocumentation(xquery, classQuery,
                                            metaClass->typeEntry()->docModifications());
        if (doc.isEmpty())
            qCWarning(lcShibokenDoc, "%s",
                      qPrintable(msgCannotFindDocumentation(doxyFilePath, "class", metaClass->name(),
                                                            classQuery)));
        else
            classDoc.setValue(doc, tag.first);
    }
    metaClass->setDocumentation(classDoc);

    //Functions Documentation
    const AbstractMetaFunctionList &funcs = DocParser::documentableFunctions(metaClass);
    for (AbstractMetaFunction *func : funcs) {
        QString query = QLatin1String("/doxygen/compounddef/sectiondef");
        // properties
        if (func->isPropertyReader() || func->isPropertyWriter()
            || func->isPropertyResetter()) {
            const auto prop = metaClass->propertySpecs().at(func->propertySpecIndex());
            query += QLatin1String("[@kind=\"property\"]/memberdef/name[text()=\"")
                     + prop.name() + QLatin1String("\"]");
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
                const AbstractMetaArgumentList &arguments = func->arguments();
                for (const AbstractMetaArgument &arg : arguments) {
                    if (!arg.type().isPrimitive()) {
                        query += QLatin1String("/../param[") + QString::number(i)
                                 + QLatin1String("]/type/ref[text()=\"")
                                 + arg.type().cppSignature().toHtmlEscaped()
                                 + QLatin1String("\"]/../..");
                    } else {
                        query += QLatin1String("/../param[") + QString::number(i)
                                 + QLatin1String("]/type[text(), \"")
                                 + arg.type().cppSignature().toHtmlEscaped()
                                 + QLatin1String("\"]/..");
                    }
                    ++i;
                }
            }
        }
        Documentation funcDoc;
        for (const auto &tag : docTags) {
            QString funcQuery(query);
            if (!isProperty) {
                funcQuery += QLatin1String("/../") + tag.second;
            } else {
                funcQuery = QLatin1Char('(') + funcQuery;
                funcQuery += QLatin1String("/../%1)[1]").arg(tag.second);
            }

            QString doc = getDocumentation(xquery, funcQuery, DocModificationList());
            if (doc.isEmpty()) {
                qCWarning(lcShibokenDoc, "%s",
                          qPrintable(msgCannotFindDocumentation(doxyFilePath, metaClass, func,
                                                                funcQuery)));
            } else {
                funcDoc.setValue(doc, tag.first);
            }
        }
        func->setDocumentation(funcDoc);
        isProperty = false;
    }

    //Fields
    for (AbstractMetaField &field : metaClass->fields()) {
        if (field.isPrivate())
            return;

        Documentation fieldDoc;
        for (const auto &tag : docTags) {
            QString query = QLatin1String("/doxygen/compounddef/sectiondef/memberdef/name[text()=\"")
                            + field.name() + QLatin1String("\"]/../") + tag.second;
            QString doc = getDocumentation(xquery, query, DocModificationList());
            if (doc.isEmpty()) {
                qCWarning(lcShibokenDoc, "%s",
                          qPrintable(msgCannotFindDocumentation(doxyFilePath, metaClass, field,
                                                                query)));
            } else {
                fieldDoc.setValue(doc, tag.first);
            }
        }
        field.setDocumentation(fieldDoc);
    }

    //Enums
    for (AbstractMetaEnum &meta_enum : metaClass->enums()) {
        QString query = QLatin1String("/doxygen/compounddef/sectiondef/memberdef[@kind=\"enum\"]/name[text()=\"")
            + meta_enum.name() + QLatin1String("\"]/..");
        QString doc = getDocumentation(xquery, query, DocModificationList());
        if (doc.isEmpty()) {
            qCWarning(lcShibokenDoc, "%s",
                      qPrintable(msgCannotFindDocumentation(doxyFilePath, metaClass, meta_enum, query)));
        }
        meta_enum.setDocumentation(doc);
    }

}

Documentation DoxygenParser::retrieveModuleDocumentation(const QString& name){

    QString sourceFile = documentationDataDirectory() + QLatin1String("/indexpage.xml");

    if (!QFile::exists(sourceFile)) {
        qCWarning(lcShibokenDoc).noquote().nospace()
            << "Can't find doxygen XML file for module " << name << ", tried: "
            << QDir::toNativeSeparators(sourceFile);
        return Documentation();
    }

    QString errorMessage;
    XQueryPtr xquery = XQuery::create(sourceFile, &errorMessage);
    if (xquery.isNull()) {
        qCWarning(lcShibokenDoc, "%s", qPrintable(errorMessage));
        return {};
    }

    // Module documentation
    QString query = QLatin1String("/doxygen/compounddef/detaileddescription");
    return Documentation(getDocumentation(xquery, query, DocModificationList()));
}

