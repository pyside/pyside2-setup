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

#include "qtdocparser.h"
#include "abstractmetalang.h"
#include "reporthandler.h"
#include "typesystem.h"

#include <QtXmlPatterns/QXmlQuery>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QUrl>

Documentation QtDocParser::retrieveModuleDocumentation()
{
    return retrieveModuleDocumentation(packageName());
}

void QtDocParser::fillDocumentation(AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return;

    QString scope = metaClass->name();
    const AbstractMetaClass* context = metaClass->enclosingClass();
    while(context) {
        if (context->enclosingClass() == 0)
            break;
        context = context->enclosingClass();
    }

    QString filename = metaClass->qualifiedCppName().toLower();
    filename.replace(QLatin1String("::"), QLatin1String("-"));
    QString sourceFile = documentationDataDirectory() + QLatin1Char('/')
                         + filename + QLatin1String(".xml");
    if (metaClass->enclosingClass())
        sourceFile.replace(QLatin1String("::"), QLatin1String("-"));

    if (!QFile::exists(sourceFile)) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find qdoc3 file for class " << metaClass->name() << ", tried: "
            << QDir::toNativeSeparators(sourceFile);
        return;
    }

    QXmlQuery xquery;
    xquery.setFocus(QUrl(sourceFile));

    QString className = metaClass->name();

    // Class/Namespace documentation
    QString type = metaClass->isNamespace() ? QLatin1String("namespace") : QLatin1String("class");
    QString query = QLatin1String("/WebXML/document/") + type + QLatin1String("[@name=\"")
                    + className + QLatin1String("\"]/description");

    DocModificationList signedModifs, classModifs;
    foreach (DocModification docModif, metaClass->typeEntry()->docModifications()) {
        if (docModif.signature().isEmpty())
            classModifs.append(docModif);
        else
            signedModifs.append(docModif);
    }

    Documentation doc(getDocumentation(xquery, query, classModifs));
    metaClass->setDocumentation(doc);


    //Functions Documentation
    AbstractMetaFunctionList funcs = metaClass->functionsInTargetLang();
    foreach (AbstractMetaFunction *func, funcs) {
        if (!func || func->isPrivate())
            continue;

        QString query = QLatin1String("/WebXML/document/") + type
                        + QLatin1String("[@name=\"") + className + QLatin1String("\"]");
        // properties
        if (func->isPropertyReader() || func->isPropertyWriter() || func->isPropertyResetter()) {
            query += QLatin1String("/property[@name=\"") + func->propertySpec()->name()
                     + QLatin1String("\"]");
        } else { // normal methods
            QString isConst = func->isConstant() ? QLatin1String("true") : QLatin1String("false");
            query += QLatin1String("/function[@name=\"") + func->originalName()
                     + QLatin1String("\" and count(parameter)=")
                     + QString::number(func->arguments().count())
                     + QLatin1String(" and @const=\"") + isConst + QLatin1String("\"]");

            int i = 1;
            foreach (AbstractMetaArgument* arg, func->arguments()) {
                QString type = arg->type()->name();

                if (arg->type()->isConstant())
                    type.prepend(QLatin1String("const "));

                if (arg->type()->referenceType() == LValueReference) {
                    type += QLatin1String(" &");
                } else if (arg->type()->referenceType() == RValueReference) {
                    type += QLatin1String(" &&");
                } else if (arg->type()->indirections()) {
                    type += QLatin1Char(' ');
                    for (int j = 0, max = arg->type()->indirections(); j < max; ++j)
                        type += QLatin1Char('*');
                }
                query += QLatin1String("/parameter[") + QString::number(i)
                         + QLatin1String("][@left=\"") + type + QLatin1String("\"]/..");
                ++i;
            }
        }
        query += QLatin1String("/description");
        DocModificationList funcModifs;
        foreach (DocModification funcModif, signedModifs) {
            if (funcModif.signature() == func->minimalSignature())
                funcModifs.append(funcModif);
        }
        doc.setValue(getDocumentation(xquery, query, funcModifs));
        func->setDocumentation(doc);
    }
#if 0
    // Fields
    AbstractMetaFieldList fields = metaClass->fields();
    foreach (AbstractMetaField *field, fields) {
        if (field->isPrivate())
            return;

        QString query = "/doxygen/compounddef/sectiondef/memberdef/name[text()=\"" + field->name() + "\"]/..";
        Documentation doc = getDocumentation(DocModificationList(), xquery, query);
        field->setDocumentation(doc);
    }
#endif
    // Enums
    AbstractMetaEnumList enums = metaClass->enums();
    foreach (AbstractMetaEnum *meta_enum, enums) {
        QString query = QLatin1String("/WebXML/document/") + type
                        + QLatin1String("[@name=\"")
                        + className + QLatin1String("\"]/enum[@name=\"")
                        + meta_enum->name() + QLatin1String("\"]/description");
        doc.setValue(getDocumentation(xquery, query, DocModificationList()));
        meta_enum->setDocumentation(doc);
    }
}

Documentation QtDocParser::retrieveModuleDocumentation(const QString& name)
{
    // TODO: This method of acquiring the module name supposes that the target language uses
    // dots as module separators in package names. Improve this.
    QString moduleName = name;
    moduleName.remove(0, name.lastIndexOf(QLatin1Char('.')) + 1);
    QString sourceFile = documentationDataDirectory() + QLatin1Char('/')
                         + moduleName.toLower() + QLatin1String(".xml");

    if (!QFile::exists(sourceFile)) {
        qCWarning(lcShiboken).noquote().nospace()
            << "Can't find qdoc3 file for module " <<  name << ", tried: "
            << QDir::toNativeSeparators(sourceFile);
        return Documentation();
    }

    QXmlQuery xquery;
    xquery.setFocus(QUrl(sourceFile));

    // Module documentation
    QString query = QLatin1String("/WebXML/document/page[@name=\"") + moduleName + QLatin1String("\"]/description");
    return Documentation(getDocumentation(xquery, query, DocModificationList()));
}
