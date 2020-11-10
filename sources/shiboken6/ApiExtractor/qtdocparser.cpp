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

#include "qtdocparser.h"
#include "abstractmetaenum.h"
#include "abstractmetafield.h"
#include "abstractmetafunction.h"
#include "abstractmetalang.h"
#include "modifications.h"
#include "messages.h"
#include "propertyspec.h"
#include "reporthandler.h"
#include "typesystem.h"
#include "xmlutils.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QXmlStreamAttributes>
#include <QtCore/QXmlStreamReader>
#include <QUrl>

Documentation QtDocParser::retrieveModuleDocumentation()
{
    return retrieveModuleDocumentation(packageName());
}

static void formatFunctionArgTypeQuery(QTextStream &str, const AbstractMetaArgument &arg)
{
    const AbstractMetaType &metaType = arg.type();
    if (metaType.isConstant())
        str << "const " ;
    switch (metaType.typeUsagePattern()) {
    case AbstractMetaType::FlagsPattern: {
        // Modify qualified name "QFlags<Qt::AlignmentFlag>" with name "Alignment"
        // to "Qt::Alignment" as seen by qdoc.
        const auto *flagsEntry = static_cast<const FlagsTypeEntry *>(metaType.typeEntry());
        QString name = flagsEntry->qualifiedCppName();
        if (name.endsWith(QLatin1Char('>')) && name.startsWith(QLatin1String("QFlags<"))) {
            const int lastColon = name.lastIndexOf(QLatin1Char(':'));
            if (lastColon != -1) {
                name.replace(lastColon + 1, name.size() - lastColon - 1, metaType.name());
                name.remove(0, 7);
            } else {
                name = metaType.name(); // QFlags<> of enum in global namespace
            }
        }
        str << name;
    }
        break;
    case AbstractMetaType::ContainerPattern: { // QVector<int>
        str << metaType.typeEntry()->qualifiedCppName() << '<';
        const auto instantiations = metaType.instantiations();
        for (int i = 0, size = instantiations.size(); i < size; ++i) {
            if (i)
                str << ", ";
            str << instantiations.at(i).typeEntry()->qualifiedCppName();
        }
        str << '>';
    }
        break;
    default: // Fully qualify enums (Qt::AlignmentFlag), nested classes, etc.
        str << metaType.typeEntry()->qualifiedCppName();
        break;
    }

    if (metaType.referenceType() == LValueReference)
        str << " &";
    else if (metaType.referenceType() == RValueReference)
        str << " &&";
    else if (metaType.indirections())
        str << ' ' << QByteArray(metaType.indirections(), '*');
}

enum FunctionMatchFlags
{
    MatchArgumentCount = 0x1,
    MatchArgumentType = 0x2,
    DescriptionOnly = 0x4
};

static QString functionXQuery(const QString &classQuery,
                              const AbstractMetaFunction *func,
                              unsigned matchFlags = MatchArgumentCount | MatchArgumentType
                                                    | DescriptionOnly)
{
    QString result;
    QTextStream str(&result);
    const AbstractMetaArgumentList &arguments = func->arguments();
    str << classQuery << "/function[@name=\"" << func->originalName()
        << "\" and @const=\"" << (func->isConstant() ? "true" : "false") << '"';
    if (matchFlags & MatchArgumentCount)
        str << " and count(parameter)=" << arguments.size();
    str << ']';
    if (!arguments.isEmpty() && (matchFlags & MatchArgumentType)) {
        for (int i = 0, size = arguments.size(); i < size; ++i) {
            str << "/parameter[" << (i + 1) << "][@type=\"";
            // Fixme: Use arguments.at(i)->type()->originalTypeDescription()
            //        instead to get unresolved typedefs?
            formatFunctionArgTypeQuery(str, arguments.at(i));
            str << "\"]/..";
        }
    }
    if (matchFlags & DescriptionOnly)
        str << "/description";
    return result;
}

static QStringList signaturesFromWebXml(QString w)
{
    QStringList result;
    if (w.isEmpty())
        return result;
    w.prepend(QLatin1String("<root>")); // Fake root element
    w.append(QLatin1String("</root>"));
    QXmlStreamReader reader(w);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement
            && reader.name() == QLatin1String("function")) {
            result.append(reader.attributes().value(QStringLiteral("signature")).toString());
        }
    }
    return result;
}

static QString msgArgumentCountMatch(const AbstractMetaFunction *func,
                                     const QStringList &matches)
{
    QString result;
    QTextStream str(&result);
    str << "\n  Note: Querying for the argument count=="
        << func->arguments().size() << " only yields " << matches.size()
        << " matches";
    if (!matches.isEmpty())
        str << ": \"" << matches.join(QLatin1String("\", \"")) << '"';
    return result;
}

QString QtDocParser::queryFunctionDocumentation(const QString &sourceFileName,
                                                const AbstractMetaClass* metaClass,
                                                const QString &classQuery,
                                                const  AbstractMetaFunction *func,
                                                const DocModificationList &signedModifs,
                                                const XQueryPtr &xquery,
                                                QString *errorMessage)
{
    DocModificationList funcModifs;
    for (const DocModification &funcModif : signedModifs) {
        if (funcModif.signature() == func->minimalSignature())
            funcModifs.append(funcModif);
    }

    // Properties
    if (func->isPropertyReader() || func->isPropertyWriter() || func->isPropertyResetter()) {
        const QString propertyQuery = classQuery + QLatin1String("/property[@name=\"")
            + func->propertySpec()->name() + QLatin1String("\"]/description");
        const QString properyDocumentation = getDocumentation(xquery, propertyQuery, funcModifs);
        if (properyDocumentation.isEmpty())
            *errorMessage = msgCannotFindDocumentation(sourceFileName, metaClass, func, propertyQuery);
        return properyDocumentation;
    }

    // Query with full match of argument types
    const QString fullQuery = functionXQuery(classQuery, func);
    const QString result = getDocumentation(xquery, fullQuery, funcModifs);
    if (!result.isEmpty())
        return result;
    *errorMessage = msgCannotFindDocumentation(sourceFileName, metaClass, func, fullQuery);
    if (func->arguments().isEmpty()) // No arguments, can't be helped
        return result;
    // Test whether some mismatch in argument types occurred by checking for
    // the argument count only. Include the outer <function> element.
    QString countOnlyQuery = functionXQuery(classQuery, func, MatchArgumentCount);
    QStringList signatures =
            signaturesFromWebXml(getDocumentation(xquery, countOnlyQuery, funcModifs));
    if (signatures.size() == 1) {
        // One match was found. Repeat the query restricted to the <description>
        // element and use the result with a warning.
        countOnlyQuery = functionXQuery(classQuery, func, MatchArgumentCount | DescriptionOnly);
        errorMessage->append(QLatin1String("\n  Falling back to \"") + signatures.constFirst()
                             + QLatin1String("\" obtained by matching the argument count only."));
        return getDocumentation(xquery, countOnlyQuery, funcModifs);
    }
    *errorMessage += msgArgumentCountMatch(func, signatures);
    return result;
}

void QtDocParser::fillDocumentation(AbstractMetaClass* metaClass)
{
    if (!metaClass)
        return;

    const AbstractMetaClass* context = metaClass->enclosingClass();
    while(context) {
        if (context->enclosingClass() == nullptr)
            break;
        context = context->enclosingClass();
    }

    QString sourceFileRoot = documentationDataDirectory() + QLatin1Char('/')
        + metaClass->qualifiedCppName().toLower();
    sourceFileRoot.replace(QLatin1String("::"), QLatin1String("-"));

    QFileInfo sourceFile(sourceFileRoot + QStringLiteral(".webxml"));
    if (!sourceFile.exists())
        sourceFile.setFile(sourceFileRoot + QStringLiteral(".xml"));
   if (!sourceFile.exists()) {
        qCWarning(lcShibokenDoc).noquote().nospace()
            << "Can't find qdoc file for class " << metaClass->name() << ", tried: "
            << QDir::toNativeSeparators(sourceFile.absoluteFilePath());
        return;
    }

    const QString sourceFileName = sourceFile.absoluteFilePath();
    QString errorMessage;
    XQueryPtr xquery = XQuery::create(sourceFileName, &errorMessage);
    if (xquery.isNull()) {
        qCWarning(lcShibokenDoc, "%s", qPrintable(errorMessage));
        return;
    }

    QString className = metaClass->name();

    // Class/Namespace documentation
    const QString classQuery = QLatin1String("/WebXML/document/")
         + (metaClass->isNamespace() ? QLatin1String("namespace") : QLatin1String("class"))
         + QLatin1String("[@name=\"") + className + QLatin1String("\"]");
    QString query = classQuery + QLatin1String("/description");

    DocModificationList signedModifs, classModifs;
    const DocModificationList &mods = metaClass->typeEntry()->docModifications();
    for (const DocModification &docModif : mods) {
        if (docModif.signature().isEmpty())
            classModifs.append(docModif);
        else
            signedModifs.append(docModif);
    }

    Documentation doc(getDocumentation(xquery, query, classModifs));
    if (doc.isEmpty())
         qCWarning(lcShibokenDoc, "%s", qPrintable(msgCannotFindDocumentation(sourceFileName, "class", className, query)));
    metaClass->setDocumentation(doc);

    //Functions Documentation
    const AbstractMetaFunctionList &funcs = DocParser::documentableFunctions(metaClass);
    for (AbstractMetaFunction *func : funcs) {
        const QString documentation =
            queryFunctionDocumentation(sourceFileName, metaClass, classQuery,
                                       func, signedModifs, xquery, &errorMessage);
        if (!errorMessage.isEmpty())
            qCWarning(lcShibokenDoc, "%s", qPrintable(errorMessage));
        func->setDocumentation(Documentation(documentation));
    }
#if 0
    // Fields
    const AbstractMetaFieldList &fields = metaClass->fields();
    for (AbstractMetaField *field : fields) {
        if (field->isPrivate())
            return;

        QString query = "/doxygen/compounddef/sectiondef/memberdef/name[text()=\"" + field->name() + "\"]/..";
        Documentation doc = getDocumentation(DocModificationList(), xquery, query);
        field->setDocumentation(doc);
    }
#endif
    // Enums
    for (AbstractMetaEnum &meta_enum : metaClass->enums()) {
        query.clear();
        QTextStream(&query) << classQuery << "/enum[@name=\""
            << meta_enum.name() << "\"]/description";
        doc.setValue(getDocumentation(xquery, query, DocModificationList()));
        if (doc.isEmpty()) {
            qCWarning(lcShibokenDoc, "%s",
                      qPrintable(msgCannotFindDocumentation(sourceFileName, metaClass, meta_enum, query)));
        }
        meta_enum.setDocumentation(doc);
    }
}

static QString qmlReferenceLink(const QFileInfo &qmlModuleFi)
{
    QString result;
    QTextStream(&result) << "<para>The module also provides <link"
        << " type=\"page\""
        << " page=\"http://doc.qt.io/qt-5/" << qmlModuleFi.baseName() << ".html\""
        << ">QML types</link>.</para>";
    return result;
}

Documentation QtDocParser::retrieveModuleDocumentation(const QString& name)
{
    // TODO: This method of acquiring the module name supposes that the target language uses
    // dots as module separators in package names. Improve this.
    QString moduleName = name;
    moduleName.remove(0, name.lastIndexOf(QLatin1Char('.')) + 1);
    const QString prefix = documentationDataDirectory() + QLatin1Char('/')
        + moduleName.toLower();
    QString sourceFile = prefix + QLatin1String(".xml");

    if (!QFile::exists(sourceFile))
        sourceFile = prefix + QLatin1String("-module.webxml");
    if (!QFile::exists(sourceFile)) {
        qCWarning(lcShibokenDoc).noquote().nospace()
            << "Can't find qdoc file for module " <<  name << ", tried: "
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
    QString query = QLatin1String("/WebXML/document/module[@name=\"")
        + moduleName + QLatin1String("\"]/description");
    Documentation doc = getDocumentation(xquery, query, DocModificationList());
    if (doc.isEmpty()) {
        qCWarning(lcShibokenDoc, "%s", qPrintable(msgCannotFindDocumentation(sourceFile, "module", name, query)));
        return doc;
    }

    // If a QML module info file exists, insert a link to the Qt docs.
    const QFileInfo qmlModuleFi(prefix + QLatin1String("-qmlmodule.webxml"));
    if (qmlModuleFi.isFile()) {
        QString docString = doc.value();
        const int pos = docString.lastIndexOf(QLatin1String("</description>"));
        if (pos != -1) {
            docString.insert(pos, qmlReferenceLink(qmlModuleFi));
            doc.setValue(docString);
        }
    }

    return doc;
}
