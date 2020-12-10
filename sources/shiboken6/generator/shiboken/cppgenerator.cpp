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

#include <memory>

#include "cppgenerator.h"
#include "ctypenames.h"
#include "pytypenames.h"
#include "fileout.h"
#include "overloaddata.h"
#include <abstractmetaenum.h>
#include <abstractmetafield.h>
#include <abstractmetafunction.h>
#include <abstractmetalang.h>
#include <messages.h>
#include <modifications.h>
#include <propertyspec.h>
#include <reporthandler.h>
#include <sourcelocation.h>
#include <textstream.h>
#include <typedatabase.h>
#include <parser/enumvalue.h>

#include <QtCore/QDir>
#include <QtCore/QMetaObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QMetaType>

#include <algorithm>
#include <cstring>

static const char CPP_ARG0[] = "cppArg0";

static inline QString reprFunction() { return QStringLiteral("__repr__"); }

QString CppGenerator::m_currentErrorCode(QLatin1String("{}"));

static const char typeNameFunc[] = R"CPP(
template <class T>
static const char *typeNameOf(const T &t)
{
    const char *typeName =  typeid(t).name();
    auto size = std::strlen(typeName);
#if defined(Q_CC_MSVC) // MSVC: "class QPaintDevice * __ptr64"
    if (auto lastStar = strchr(typeName, '*')) {
        // MSVC: "class QPaintDevice * __ptr64"
        while (*--lastStar == ' ') {
        }
        size = lastStar - typeName + 1;
    }
#else // g++, Clang: "QPaintDevice *" -> "P12QPaintDevice"
    if (size > 2 && typeName[0] == 'P' && std::isdigit(typeName[1])) {
        ++typeName;
        --size;
    }
#endif
    char *result = new char[size + 1];
    result[size] = '\0';
    memcpy(result, typeName, size);
    return result;
}
)CPP";

// utility functions
inline AbstractMetaType getTypeWithoutContainer(const AbstractMetaType &arg)
{
    if (arg.typeEntry()->isContainer()) {
        // only support containers with 1 type
        if (arg.instantiations().size() == 1)
            return arg.instantiations().constFirst();
    }
    return arg;
}

// A helper for writing C++ return statements for either void ("return;")
// or some return value ("return value;")
class returnStatement
{
public:
    explicit returnStatement(QString s) : m_returnValue(std::move(s)) {}

    friend TextStream &operator<<(TextStream &s, const returnStatement &r);

private:
    const QString m_returnValue;
};

TextStream &operator<<(TextStream &s, const returnStatement &r)
{
    s << "return";
    if (!r.m_returnValue.isEmpty())
        s << ' ' << r.m_returnValue;
    s << ';';
    return s;
}

// Protocol function name / function parameters / return type
struct ProtocolEntry
{
    QString name;
    QString arguments;
    QString returnType;
};

using ProtocolEntries = QList<ProtocolEntry>;

static bool contains(const ProtocolEntries &l, const QString &needle)
{
    for (const auto &m : l) {
        if (m.name == needle)
            return true;
    }
    return false;
}

// Maps special function names to function parameters and return types
// used by CPython API in the mapping protocol.
const ProtocolEntries &mappingProtocols()
{
    static const ProtocolEntries result = {
        {QLatin1String("__mlen__"),
         QLatin1String("PyObject *self"),
         QLatin1String("Py_ssize_t")},
        {QLatin1String("__mgetitem__"),
         QLatin1String("PyObject *self, PyObject *_key"),
         QLatin1String("PyObject*")},
        {QLatin1String("__msetitem__"),
         QLatin1String("PyObject *self, PyObject *_key, PyObject *_value"),
         intT()}};
    return result;
}

// Maps special function names to function parameters and return types
// used by CPython API in the sequence protocol.
const ProtocolEntries &sequenceProtocols()
{
    static const ProtocolEntries result = {
        {QLatin1String("__len__"),
         QLatin1String("PyObject *self"),
         QLatin1String("Py_ssize_t")},
        {QLatin1String("__getitem__"),
         QLatin1String("PyObject *self, Py_ssize_t _i"),
         QLatin1String("PyObject*")},
        {QLatin1String("__setitem__"),
         QLatin1String("PyObject *self, Py_ssize_t _i, PyObject *_value"),
         intT()},
        {QLatin1String("__getslice__"),
         QLatin1String("PyObject *self, Py_ssize_t _i1, Py_ssize_t _i2"),
         QLatin1String("PyObject*")},
        {QLatin1String("__setslice__"),
         QLatin1String("PyObject *self, Py_ssize_t _i1, Py_ssize_t _i2, PyObject *_value"),
         intT()},
        {QLatin1String("__contains__"),
         QLatin1String("PyObject *self, PyObject *_value"),
         intT()},
        {QLatin1String("__concat__"),
         QLatin1String("PyObject *self, PyObject *_other"),
         QLatin1String("PyObject*")}
    };
    return result;
}

CppGenerator::CppGenerator() = default;

QString CppGenerator::fileNameSuffix() const
{
    return QLatin1String("_wrapper.cpp");
}

QString CppGenerator::fileNameForContext(const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    if (!context.forSmartPointer()) {
        QString fileNameBase = metaClass->qualifiedCppName().toLower();
        fileNameBase.replace(QLatin1String("::"), QLatin1String("_"));
        return fileNameBase + fileNameSuffix();
    }
    const AbstractMetaType &smartPointerType = context.preciseType();
    QString fileNameBase = getFileNameBaseForSmartPointer(smartPointerType, metaClass);
    return fileNameBase + fileNameSuffix();
}

QList<AbstractMetaFunctionCList>
    CppGenerator::filterGroupedOperatorFunctions(const AbstractMetaClass *metaClass,
                                                 OperatorQueryOptions query)
{
    // ( func_name, num_args ) => func_list
    QMap<QPair<QString, int>, AbstractMetaFunctionCList> results;
    const auto &funcs = metaClass->operatorOverloads(query);
    for (const auto &func : funcs) {
        if (func->isModifiedRemoved()
            || func->usesRValueReferences()
            || func->name() == QLatin1String("operator[]")
            || func->name() == QLatin1String("operator->")
            || func->name() == QLatin1String("operator!")) {
            continue;
        }
        int args;
        if (func->isComparisonOperator()) {
            args = -1;
        } else {
            args = func->arguments().size();
        }
        QPair<QString, int > op(func->name(), args);
        results[op].append(func);
    }
    QList<AbstractMetaFunctionCList> result;
    result.reserve(results.size());
    for (auto it = results.cbegin(), end = results.cend(); it != end; ++it)
        result.append(it.value());
    return result;
}

AbstractMetaFunctionCPtr CppGenerator::boolCast(const AbstractMetaClass *metaClass) const
{
    if (!useIsNullAsNbNonZero())
        return {};
    // TODO: This could be configurable someday
    const auto func = metaClass->findFunction(QLatin1String("isNull"));
    if (func.isNull() || func->isVoid() || !func->type().typeEntry()->isPrimitive()
        || !func->isPublic()) {
        return {};
    }
    auto pte = static_cast<const PrimitiveTypeEntry *>(func->type().typeEntry());
    while (pte->referencedTypeEntry())
        pte = pte->referencedTypeEntry();
    return func->isConstant() && pte->name() == QLatin1String("bool")
            && func->arguments().isEmpty() ? func : AbstractMetaFunctionCPtr{};
}

std::optional<AbstractMetaType>
    CppGenerator::findSmartPointerInstantiation(const TypeEntry *entry) const
{
    for (const auto &i : instantiatedSmartPointers()) {
        if (i.instantiations().at(0).typeEntry() == entry)
            return i;
    }
    return {};
}

void CppGenerator::clearTpFuncs()
{
    m_tpFuncs = {
        {QLatin1String("__str__"), {}}, {QLatin1String("__str__"), {}},
        {reprFunction(), {}}, {QLatin1String("__iter__"), {}},
        {QLatin1String("__next__"), {}}
    };
}

// Prevent ELF symbol qt_version_tag from being generated into the source
static const char includeQDebug[] =
"#ifndef QT_NO_VERSION_TAGGING\n"
"#  define QT_NO_VERSION_TAGGING\n"
"#endif\n"
"#include <QDebug>\n";

static QString chopType(QString s)
{
    if (s.endsWith(QLatin1String("_Type")))
        s.chop(5);
    else if (s.endsWith(QLatin1String("_TypeF()")))
        s.chop(8);
    return s;
}

static bool isStdSetterName(QString setterName, QString propertyName)
{
   return setterName.size() == propertyName.size() + 3
          && setterName.startsWith(QLatin1String("set"))
          && setterName.endsWith(QStringView{propertyName}.right(propertyName.size() - 1))
          && setterName.at(3) == propertyName.at(0).toUpper();
}

static QString buildPropertyString(const QPropertySpec &spec)
{
    QString text;
    text += QLatin1Char('"');
    text += spec.name();
    text += QLatin1Char(':');

    if (spec.read() != spec.name())
        text += spec.read();

    if (!spec.write().isEmpty()) {
        text += QLatin1Char(':');
        if (!isStdSetterName(spec.write(), spec.name()))
            text += spec.write();
    }

    text += QLatin1Char('"');
    return text;
}

static void writePyGetSetDefEntry(TextStream &s, const QString &name,
                                  const QString &getFunc, const QString &setFunc)
{
    s << "{const_cast<char *>(\"" << name << "\"), " << getFunc << ", "
        << (setFunc.isEmpty() ? QLatin1String(NULL_PTR) : setFunc) << "},\n";
}

/*!
    Function used to write the class generated binding code on the buffer
    \param s the output buffer
    \param metaClass the pointer to metaclass information
*/
void CppGenerator::generateClass(TextStream &s, const GeneratorContext &classContext)
{
    s.setLanguage(TextStream::Language::Cpp);
    const AbstractMetaClass *metaClass = classContext.metaClass();

    // write license comment
    s << licenseComment() << '\n';

    if (!avoidProtectedHack() && !metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
        s << "//workaround to access protected functions\n";
        s << "#define protected public\n\n";
    }

    // headers
    s << "// default includes\n";
    s << "#include <shiboken.h>\n";
    if (usePySideExtensions()) {
        s << includeQDebug;
        s << "#include <pysidesignal.h>\n"
            << "#include <pysideproperty.h>\n"
            << "#include <pyside.h>\n"
            << "#include <pysideqenum.h>\n"
            << "#include <feature_select.h>\n"
            << "#include <qapp_macro.h>\n\n"
            << "QT_WARNING_DISABLE_DEPRECATED\n\n";
     }

    s << "#include <typeinfo>\n";
    if (usePySideExtensions() && metaClass->isQObject()) {
        s << "#include <signalmanager.h>\n";
        s << "#include <pysidemetafunction.h>\n";
    }

    // The multiple inheritance initialization function
    // needs the 'set' class from C++ STL.
    if (getMultipleInheritingClass(metaClass) != nullptr)
        s << "#include <algorithm>\n#include <set>\n";
    if (metaClass->generateExceptionHandling())
        s << "#include <exception>\n";
    s << "#include <iterator>\n"; // For containers

    if (wrapperDiagnostics())
        s << "#include <helper.h>\n#include <iostream>\n";

    s << "\n// module include\n" << "#include \"" << getModuleHeaderFileName() << "\"\n";

    QString headerfile = fileNameForContext(classContext);
    headerfile.replace(QLatin1String(".cpp"), QLatin1String(".h"));
    s << "\n// main header\n" << "#include \"" << headerfile << "\"\n";

    s  << '\n' << "// inner classes\n";
    const AbstractMetaClassList &innerClasses = metaClass->innerClasses();
    for (AbstractMetaClass *innerClass : innerClasses) {
        GeneratorContext innerClassContext = contextForClass(innerClass);
        if (shouldGenerate(innerClass) && !innerClass->typeEntry()->isSmartPointer()) {
            QString headerfile = fileNameForContext(innerClassContext);
            headerfile.replace(QLatin1String(".cpp"), QLatin1String(".h"));
            s << "#include \"" << headerfile << "\"\n";
        }
    }

    AbstractMetaEnumList classEnums = metaClass->enums();
    metaClass->getEnumsFromInvisibleNamespacesToBeGenerated(&classEnums);

    //Extra includes
    QList<Include> includes;
    if (!classContext.useWrapper())
        includes += metaClass->typeEntry()->extraIncludes();
    for (const AbstractMetaEnum &cppEnum : qAsConst(classEnums))
        includes.append(cppEnum.typeEntry()->extraIncludes());
    if (!includes.isEmpty()) {
        s << "\n// Extra includes\n";
        std::sort(includes.begin(), includes.end());
        for (const Include &inc : qAsConst(includes))
            s << inc.toString() << '\n';
        s << '\n';
    }

    s << "\n#include <cctype>\n#include <cstring>\n";

    if (metaClass->typeEntry()->typeFlags() & ComplexTypeEntry::Deprecated)
        s << "#Deprecated\n";

    // Use class base namespace
    {
        const AbstractMetaClass *context = metaClass->enclosingClass();
        while (context) {
            if (context->isNamespace() && !context->enclosingClass()
                && static_cast<const NamespaceTypeEntry *>(context->typeEntry())->generateUsing()) {
                s << "\nusing namespace " << context->qualifiedCppName() << ";\n";
                break;
            }
            context = context->enclosingClass();
        }
    }

    s  << "\n\n" << typeNameFunc << '\n';

    // Create string literal for smart pointer getter method.
    if (classContext.forSmartPointer()) {
        const auto *typeEntry =
                static_cast<const SmartPointerTypeEntry *>(classContext.preciseType()
                                                           .typeEntry());
        QString rawGetter = typeEntry->getter();
        s << "static const char * " << SMART_POINTER_GETTER << " = \"" << rawGetter << "\";";
    }

    // class inject-code native/beginning
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeClassCodeSnips(s, metaClass->typeEntry()->codeSnips(),
                            TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode,
                            classContext);
        s << '\n';
    }

    // python conversion rules
    if (metaClass->typeEntry()->hasTargetConversionRule()) {
        s << "// Python Conversion\n";
        s << metaClass->typeEntry()->conversionRule() << '\n';
    }

    if (classContext.useWrapper()) {
        s << "// Native ---------------------------------------------------------\n\n";

        if (avoidProtectedHack() && usePySideExtensions()) {
            s << "void " << classContext.wrapperName() << "::pysideInitQtMetaTypes()\n{\n";
            Indentation indent(s);
            writeInitQtMetaTypeFunctionBody(s, classContext);
            s << "}\n\n";
        }

        const auto &funcs = filterFunctions(metaClass);
        int maxOverrides = 0;
        writeCacheResetNative(s, classContext);
        for (const auto &func : funcs) {
            const bool notAbstract = !func->isAbstract();
            if ((func->isPrivate() && notAbstract && !func->isVisibilityModifiedToPrivate())
                || (func->isModifiedRemoved() && notAbstract))
                continue;
            if (func->functionType() == AbstractMetaFunction::ConstructorFunction && !func->isUserAdded())
                writeConstructorNative(s, classContext, func);
            else if (shouldWriteVirtualMethodNative(func))
                writeVirtualMethodNative(s, func, maxOverrides++);
        }

        if (!avoidProtectedHack() || !metaClass->hasPrivateDestructor()) {
            if (usePySideExtensions() && metaClass->isQObject())
                writeMetaObjectMethod(s, classContext);
            writeDestructorNative(s, classContext);
        }
    }

    StringStream smd(TextStream::Language::Cpp);
    StringStream md(TextStream::Language::Cpp);
    StringStream signatureStream(TextStream::Language::Cpp);

    s << "\n// Target ---------------------------------------------------------\n\n"
        << "extern \"C\" {\n";
    const auto &functionGroups = getFunctionGroups(metaClass);
    for (auto it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
        AbstractMetaFunctionCList overloads;
        QSet<QString> seenSignatures;
        bool staticEncountered = false;
        for (const auto &func : it.value()) {
            if (!func->isAssignmentOperator()
                && !func->usesRValueReferences()
                && !func->isConversionOperator()
                && !func->isModifiedRemoved()
                && (!func->isPrivate() || func->functionType() == AbstractMetaFunction::EmptyFunction)
                && func->ownerClass() == func->implementingClass()
                && (func->name() != QLatin1String("qt_metacall"))) {
                // PYSIDE-331: Inheritance works correctly when there are disjoint functions.
                // But when a function is both in a class and inherited in a subclass,
                // then we need to search through all subclasses and collect the new signatures.
                overloads << getFunctionAndInheritedOverloads(func, &seenSignatures);
                if (func->isStatic())
                    staticEncountered = true;
            }
        }
        // PYSIDE-886: If the method does not have any static overloads declared
        // in the class in question, remove all inherited static methods as setting
        // METH_STATIC in that case can cause crashes for the instance methods.
        // Manifested as crash when calling QPlainTextEdit::find() (clash with
        // static QWidget::find(WId)).
        if (!staticEncountered) {
            for (int i = overloads.size() - 1; i >= 0; --i) {
                if (overloads.at(i)->isStatic())
                    overloads.removeAt(i);
            }
        }

        if (overloads.isEmpty())
            continue;

        const auto rfunc = overloads.constFirst();
        if (contains(sequenceProtocols(), rfunc->name())
            || contains(mappingProtocols(), rfunc->name())) {
            continue;
        }

        if (rfunc->isConstructor()) {
            // @TODO: Implement constructor support for smart pointers, so that they can be
            // instantiated in python code.
            if (classContext.forSmartPointer())
                continue;
            writeConstructorWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
        }
        // call operators
        else if (rfunc->name() == QLatin1String("operator()")) {
            writeMethodWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
        }
        else if (!rfunc->isOperatorOverload()) {

            if (classContext.forSmartPointer()) {
                const auto *smartPointerTypeEntry =
                        static_cast<const SmartPointerTypeEntry *>(
                            classContext.preciseType().typeEntry());

                if (smartPointerTypeEntry->getter() == rfunc->name()) {
                    // Replace the return type of the raw pointer getter method with the actual
                    // return type.
                    QString innerTypeName =
                            classContext.preciseType().getSmartPointerInnerType().cppSignature();
                    QString pointerToInnerTypeName = innerTypeName + QLatin1Char('*');
                    // @TODO: This possibly leaks, but there are a bunch of other places where this
                    // is done, so this will be fixed in bulk with all the other cases, because the
                    // ownership of the pointers is not clear at the moment.
                    auto pointerToInnerType =
                            buildAbstractMetaTypeFromString(pointerToInnerTypeName);
                    Q_ASSERT(pointerToInnerType.has_value());
                    auto mutableRfunc = overloads.constFirst();
                    qSharedPointerConstCast<AbstractMetaFunction>(mutableRfunc)->setType(pointerToInnerType.value());
                } else if (smartPointerTypeEntry->refCountMethodName().isEmpty()
                           || smartPointerTypeEntry->refCountMethodName() != rfunc->name()) {
                    // Skip all public methods of the smart pointer except for the raw getter and
                    // the ref count method.
                    continue;
                }
            }

            writeMethodWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
            // For a mixture of static and member function overloads,
            // a separate PyMethodDef entry is written which is referenced
            // in the PyMethodDef list and later in getattro() for handling
            // the non-static case.
            if (OverloadData::hasStaticAndInstanceFunctions(overloads)) {
                QString methDefName = cpythonMethodDefinitionName(rfunc);
                smd << "static PyMethodDef " << methDefName << " = " << indent;
                writeMethodDefinitionEntries(smd, overloads, 1);
                smd << outdent << ";\n\n";
            }
            writeMethodDefinition(md, overloads);
        }
    }
    const QString methodsDefinitions = md.toString();
    const QString singleMethodDefinitions = smd.toString();

    const QString className = chopType(cpythonTypeName(metaClass));

    if (metaClass->typeEntry()->isValue() || metaClass->typeEntry()->isSmartPointer()) {
        writeCopyFunction(s, classContext);
        signatureStream << fullPythonClassName(metaClass) << ".__copy__()\n";
    }

    // Write single method definitions
    s << singleMethodDefinitions;

    if (usePySideExtensions()) {
        // PYSIDE-1019: Write a compressed list of all properties `name:getter[:setter]`.
        //              Default values are suppressed.
        QStringList sorter;
        for (const auto &spec : metaClass->propertySpecs()) {
            if (!spec.generateGetSetDef())
                sorter.append(buildPropertyString(spec));
        }
        sorter.sort();

        s << '\n';
        s << "static const char *" << className << "_PropertyStrings[] = {\n" << indent;
        for (const auto &entry : qAsConst(sorter))
            s << entry << ",\n";
        s << NULL_PTR << " // Sentinel\n"
            << outdent << "};\n\n";
    }

    // Write methods definition
    s << "static PyMethodDef " << className << "_methods[] = {\n" << indent
        << methodsDefinitions << '\n';
    if (metaClass->typeEntry()->isValue() || metaClass->typeEntry()->isSmartPointer()) {
        s << "{\"__copy__\", reinterpret_cast<PyCFunction>(" << className << "___copy__)"
            << ", METH_NOARGS},\n";
    }
    s << '{' << NULL_PTR << ", " << NULL_PTR << "} // Sentinel\n" << outdent
        << "};\n\n";

    // Write tp_s/getattro function
    const AttroCheck attroCheck = checkAttroFunctionNeeds(metaClass);
    if (attroCheck.testFlag(AttroCheckFlag::GetattroSmartPointer)) {
        writeSmartPointerGetattroFunction(s, classContext);
        writeSmartPointerSetattroFunction(s, classContext);
    } else {
        if ((attroCheck & AttroCheckFlag::GetattroMask) != 0)
            writeGetattroFunction(s, attroCheck, classContext);
        if ((attroCheck & AttroCheckFlag::SetattroMask) != 0)
            writeSetattroFunction(s, attroCheck, classContext);
    }

    const auto f = boolCast(metaClass);
    if (!f.isNull()) {
        ErrorCode errorCode(-1);
        s << "static int " << cpythonBaseName(metaClass) << "___nb_bool(PyObject *self)\n"
            << "{\n" << indent;
        writeCppSelfDefinition(s, classContext);
        if (f->allowThread()) {
            s << "int result;\n"
                << BEGIN_ALLOW_THREADS << '\n'
                << "result = !" << CPP_SELF_VAR << "->isNull();\n"
                << END_ALLOW_THREADS << '\n'
                << "return result;\n";
        } else {
            s << "return !" << CPP_SELF_VAR << "->isNull();\n";
        }
        s << outdent << "}\n\n";
    }

    if (supportsNumberProtocol(metaClass) && !metaClass->typeEntry()->isSmartPointer()) {
        const QList<AbstractMetaFunctionCList> opOverloads = filterGroupedOperatorFunctions(
                metaClass,
                OperatorQueryOption::ArithmeticOp
                | OperatorQueryOption::LogicalOp
                | OperatorQueryOption::BitwiseOp);

        for (const AbstractMetaFunctionCList &allOverloads : opOverloads) {
            AbstractMetaFunctionCList overloads;
            for (const auto &func : allOverloads) {
                if (!func->isModifiedRemoved()
                    && !func->isPrivate()
                    && (func->ownerClass() == func->implementingClass() || func->isAbstract()))
                    overloads.append(func);
            }

            if (overloads.isEmpty())
                continue;

            writeMethodWrapper(s, overloads, classContext);
            writeSignatureInfo(signatureStream, overloads);
        }
    }

    const QString signaturesString = signatureStream.toString();

    if (supportsSequenceProtocol(metaClass)) {
        writeSequenceMethods(s, metaClass, classContext);
    }

    if (supportsMappingProtocol(metaClass)) {
        writeMappingMethods(s, metaClass, classContext);
    }

    if (!metaClass->isNamespace() && metaClass->hasComparisonOperatorOverload()) {
        s << "// Rich comparison\n";
        writeRichCompareFunction(s, classContext);
    }

    if (shouldGenerateGetSetList(metaClass) && !classContext.forSmartPointer()) {
        const AbstractMetaFieldList &fields = metaClass->fields();
        for (const AbstractMetaField &metaField : fields) {
            if (metaField.canGenerateGetter())
                writeGetterFunction(s, metaField, classContext);
            if (metaField.canGenerateSetter())
                writeSetterFunction(s, metaField, classContext);
            s << '\n';
        }

        for (const QPropertySpec &property : metaClass->propertySpecs()) {
            if (property.generateGetSetDef() || !usePySideExtensions()) {
                writeGetterFunction(s, property, classContext);
                if (property.hasWrite())
                    writeSetterFunction(s, property, classContext);
            }
        }

        s << "// Getters and Setters for " << metaClass->name() << '\n';
        s << "static PyGetSetDef " << cpythonGettersSettersDefinitionName(metaClass)
            << "[] = {\n" << indent;
        for (const AbstractMetaField &metaField : fields) {
            const bool canGenerateGetter = metaField.canGenerateGetter();
            const bool canGenerateSetter = metaField.canGenerateSetter();
            if (canGenerateGetter || canGenerateSetter) {
                const QString getter = canGenerateGetter
                    ? cpythonGetterFunctionName(metaField) : QString();
                const QString setter = canGenerateSetter
                    ? cpythonSetterFunctionName(metaField) : QString();
                const auto names = metaField.definitionNames();
                for (const auto &name : names)
                    writePyGetSetDefEntry(s, name, getter, setter);
            }
        }

        for (const QPropertySpec &property : metaClass->propertySpecs()) {
            if (property.generateGetSetDef() || !usePySideExtensions()) {
                const QString setter = property.hasWrite()
                    ? cpythonSetterFunctionName(property, metaClass) : QString();
                writePyGetSetDefEntry(s, property.name(),
                                      cpythonGetterFunctionName(property, metaClass), setter);
            }
        }
        s << '{' << NULL_PTR << "} // Sentinel\n"
            << outdent << "};\n\n";
    }

    s << "} // extern \"C\"\n\n";

    if (!metaClass->typeEntry()->hashFunction().isEmpty())
        writeHashFunction(s, classContext);

    // Write tp_traverse and tp_clear functions.
    writeTpTraverseFunction(s, metaClass);
    writeTpClearFunction(s, metaClass);

    writeClassDefinition(s, metaClass, classContext);
    s << '\n';

    if (metaClass->isPolymorphic() && metaClass->baseClass())
        writeTypeDiscoveryFunction(s, metaClass);

    writeFlagsNumberMethodsDefinitions(s, classEnums);
    s << '\n';

    writeConverterFunctions(s, metaClass, classContext);
    writeClassRegister(s, metaClass, classContext, signatureStream);

    // class inject-code native/end
    if (!metaClass->typeEntry()->codeSnips().isEmpty()) {
        writeClassCodeSnips(s, metaClass->typeEntry()->codeSnips(),
                            TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode,
                            classContext);
        s << '\n';
    }
}

void CppGenerator::writeCacheResetNative(TextStream &s, const GeneratorContext &classContext) const
{
    s << "void " << classContext.wrapperName()
        << "::resetPyMethodCache()\n{\n" << indent
        << "std::fill_n(m_PyMethodCache, sizeof(m_PyMethodCache) / sizeof(m_PyMethodCache[0]), false);\n"
        << outdent << "}\n\n";
}

void CppGenerator::writeConstructorNative(TextStream &s, const GeneratorContext &classContext,
                                          const AbstractMetaFunctionCPtr &func) const
{
    const QString qualifiedName = classContext.wrapperName() + QLatin1String("::");
    s << functionSignature(func, qualifiedName, QString(),
                           OriginalTypeDescription | SkipDefaultValues);
    s << " : ";
    writeFunctionCall(s, func);
    s << "\n{\n" << indent;
    if (wrapperDiagnostics())
        s << R"(std::cerr << __FUNCTION__ << ' ' << this << '\n';)" << '\n';
    const AbstractMetaArgument *lastArg = func->arguments().isEmpty() ? nullptr : &func->arguments().constLast();
    s << "resetPyMethodCache();\n";
    writeCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode, func, lastArg);
    s << "// ... middle\n";
    writeCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode, func, lastArg);
    s << outdent << "}\n\n";
}

void CppGenerator::writeDestructorNative(TextStream &s,
                                         const GeneratorContext &classContext) const
{
    s << classContext.wrapperName() << "::~"
        << classContext.wrapperName() << "()\n{\n" << indent;
    if (wrapperDiagnostics())
        s << R"(std::cerr << __FUNCTION__ << ' ' << this << '\n';)" << '\n';
    // kill pyobject
    s << R"(SbkObject *wrapper = Shiboken::BindingManager::instance().retrieveWrapper(this);
Shiboken::Object::destroy(wrapper, this);
)" << outdent << "}\n";
}

static bool allArgumentsRemoved(const AbstractMetaFunctionCPtr& func)
{
    if (func->arguments().isEmpty())
        return false;
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument &arg : arguments) {
        if (!func->argumentRemoved(arg.argumentIndex() + 1))
            return false;
    }
    return true;
}

QString CppGenerator::getVirtualFunctionReturnTypeName(const AbstractMetaFunctionCPtr &func) const
{
    if (func->type().isVoid())
        return QLatin1String("\"\"");

    if (!func->typeReplaced(0).isEmpty())
        return QLatin1Char('"') + func->typeReplaced(0) + QLatin1Char('"');

    // SbkType would return null when the type is a container.
    auto typeEntry = func->type().typeEntry();
    if (typeEntry->isContainer()) {
        return QLatin1Char('"')
               + reinterpret_cast<const ContainerTypeEntry *>(typeEntry)->typeName()
               + QLatin1Char('"');
    }
    if (typeEntry->isSmartPointer())
        return QLatin1Char('"') + typeEntry->qualifiedCppName() + QLatin1Char('"');

    if (avoidProtectedHack()) {
        auto metaEnum = findAbstractMetaEnum(func->type());
        if (metaEnum.has_value() && metaEnum->isProtected()) {
            return QLatin1Char('"') + protectedEnumSurrogateName(metaEnum.value())
                    + QLatin1Char('"');
        }
    }

    if (func->type().isPrimitive())
        return QLatin1Char('"') + func->type().name() + QLatin1Char('"');

    return QLatin1String("reinterpret_cast<PyTypeObject *>(Shiboken::SbkType< ")
        + typeEntry->qualifiedCppName() + QLatin1String(" >())->tp_name");
}

// When writing an overridden method of a wrapper class, write the part
// calling the C++ function in case no overload in Python exists.
void CppGenerator::writeVirtualMethodCppCall(TextStream &s,
                                             const AbstractMetaFunctionCPtr &func,
                                             const QString &funcName,
                                             const CodeSnipList &snips,
                                             const AbstractMetaArgument *lastArg,
                                             const TypeEntry *retType,
                                             const QString &returnStatement) const
{
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning,
                       TypeSystem::ShellCode, func, lastArg);
    }

    if (func->isAbstract()) {
        s << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '"
            << func->ownerClass()->name() << '.' << funcName
            << "()' not implemented.\");\n"
            << returnStatement << '\n';
        return;
    }

    if (retType)
        s << "return ";
    s << "this->::" << func->implementingClass()->qualifiedCppName() << "::";
    writeFunctionCall(s, func, Generator::VirtualCall);
    s << ";\n";
    if (retType)
        return;
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd,
                       TypeSystem::ShellCode, func, lastArg);
    }
    s << "return;\n";
}

// Determine the return statement (void or a result value).
QString CppGenerator::virtualMethodReturn(TextStream &s,
                                          const AbstractMetaFunctionCPtr &func,
                                          const FunctionModificationList &functionModifications) const
{
    if (func->isVoid())
        return QLatin1String("return;");
    const AbstractMetaType &returnType = func->type();
    for (const FunctionModification &mod : functionModifications) {
        for (const ArgumentModification &argMod : mod.argument_mods()) {
            if (argMod.index() == 0 && !argMod.replacedDefaultExpression().isEmpty()) {
                static const QRegularExpression regex(QStringLiteral("%(\\d+)"));
                Q_ASSERT(regex.isValid());
                QString expr = argMod.replacedDefaultExpression();
                for (int offset = 0; ; ) {
                    const QRegularExpressionMatch match = regex.match(expr, offset);
                    if (!match.hasMatch())
                        break;
                    const int argId = match.capturedView(1).toInt() - 1;
                    if (argId < 0 || argId > func->arguments().count()) {
                        qCWarning(lcShiboken, "The expression used in return value contains an invalid index.");
                        break;
                    }
                    expr.replace(match.captured(0), func->arguments().at(argId).name());
                    offset = match.capturedStart(1);
                }
                DefaultValue defaultReturnExpr(DefaultValue::Custom, expr);
                return QLatin1String("return ") + defaultReturnExpr.returnValue()
                       + QLatin1Char(';');
            }
        }
    }
    QString errorMessage;
    const auto defaultReturnExpr = minimalConstructor(returnType, &errorMessage);
    if (!defaultReturnExpr.has_value()) {
        QString errorMsg = QLatin1String(__FUNCTION__) + QLatin1String(": ");
        if (const AbstractMetaClass *c = func->implementingClass())
            errorMsg += c->qualifiedCppName() + QLatin1String("::");
        errorMsg += func->signature();
        errorMsg = msgCouldNotFindMinimalConstructor(errorMsg,
                                                     func->type().cppSignature(),
                                                     errorMessage);
        qCWarning(lcShiboken).noquote().nospace() << errorMsg;
        s  << "\n#error " << errorMsg << '\n';
    }
    if (returnType.referenceType() == LValueReference) {
        s << "static " << returnType.typeEntry()->qualifiedCppName()
            << " result;\n";
        return QLatin1String("return result;");
    }
    return QLatin1String("return ") + defaultReturnExpr->returnValue()
           + QLatin1Char(';');
}

void CppGenerator::writeVirtualMethodNative(TextStream &s,
                                            const AbstractMetaFunctionCPtr &func,
                                            int cacheIndex) const
{
    //skip metaObject function, this will be written manually ahead
    if (usePySideExtensions() && func->ownerClass() && func->ownerClass()->isQObject() &&
        ((func->name() == QLatin1String("metaObject")) || (func->name() == QLatin1String("qt_metacall"))))
        return;

    const TypeEntry *retType = func->type().typeEntry();
    const QString funcName = func->isOperatorOverload()
        ? pythonOperatorFunctionName(func) : func->definitionNames().constFirst();

    QString prefix = wrapperName(func->ownerClass()) + QLatin1String("::");
    s << functionSignature(func, prefix, QString(), Generator::SkipDefaultValues|Generator::OriginalTypeDescription)
      << "\n{\n" << indent;

    const FunctionModificationList &functionModifications = func->modifications();

    const QString returnStatement = virtualMethodReturn(s, func, functionModifications);

    if (func->isAbstract() && func->isModifiedRemoved()) {
        qCWarning(lcShiboken, "%s", qPrintable(msgPureVirtualFunctionRemoved(func.data())));
        s << returnStatement << '\n' << outdent << "}\n\n";
        return;
    }

    const CodeSnipList snips = func->hasInjectedCode()
        ? func->injectedCodeSnips() : CodeSnipList();
    const AbstractMetaArgument *lastArg = func->arguments().isEmpty()
        ?  nullptr : &func->arguments().constLast();

    //Write declaration/native injected code
    if (!snips.isEmpty()) {
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionDeclaration,
                       TypeSystem::ShellCode, func, lastArg);
    }

    if (wrapperDiagnostics()) {
        s << "std::cerr << ";
#ifndef Q_CC_MSVC // g++ outputs __FUNCTION__ unqualified
        s << '"' << prefix << R"(" << )";
#endif
        s  << R"(__FUNCTION__ << ' ' << this << " m_PyMethodCache[" << )"
           << cacheIndex << R"( << "]=" << m_PyMethodCache[)" << cacheIndex
           << R"(] << '\n';)" << '\n';
    }
    // PYSIDE-803: Build a boolean cache for unused overrides.
    const bool multi_line = func->isVoid() || !snips.isEmpty() || func->isAbstract();
    s << "if (m_PyMethodCache[" << cacheIndex << "])" << (multi_line ? " {\n" : "\n");
    {
        Indentation indentation(s);
        writeVirtualMethodCppCall(s, func, funcName, snips, lastArg, retType,
                                  returnStatement);
    }
    if (multi_line)
        s << "}\n";

    s << "Shiboken::GilState gil;\n";

    // Get out of virtual method call if someone already threw an error.
    s << "if (PyErr_Occurred())\n" << indent
        << returnStatement << '\n' << outdent;

    //PYSIDE-1019: Add info about properties.
    int propFlag = 0;
    if (func->isPropertyReader())
        propFlag |= 1;
    if (func->isPropertyWriter())
        propFlag |= 2;
    if (propFlag && func->isStatic())
        propFlag |= 4;
    QString propStr;
    if (propFlag)
        propStr = QString::number(propFlag) + QLatin1Char(':');

    s << "static PyObject *nameCache[2] = {};\n";
    if (propFlag)
        s << "// This method belongs to a property.\n";
    s << "static const char *funcName = \"" << propStr << funcName << "\";\n"
        << "Shiboken::AutoDecRef " << PYTHON_OVERRIDE_VAR
        << "(Shiboken::BindingManager::instance().getOverride(this, nameCache, funcName));\n"
        << "if (" << PYTHON_OVERRIDE_VAR << ".isNull()) {\n"
        << indent << "gil.release();\n";
    if (useOverrideCaching(func->ownerClass()))
        s << "m_PyMethodCache[" << cacheIndex << "] = true;\n";
    writeVirtualMethodCppCall(s, func, funcName, snips, lastArg, retType,
                              returnStatement);
    s << outdent << "}\n\n"; //WS

    writeConversionRule(s, func, TypeSystem::TargetLangCode);

    s << "Shiboken::AutoDecRef " << PYTHON_ARGS << "(";

    if (func->arguments().isEmpty() || allArgumentsRemoved(func)) {
        s << "PyTuple_New(0));\n";
    } else {
        QStringList argConversions;
        const AbstractMetaArgumentList &arguments = func->arguments();
        for (const AbstractMetaArgument &arg : arguments) {
            if (func->argumentRemoved(arg.argumentIndex() + 1))
                continue;

            QString argConv;
            const auto &argType = arg.type();
            auto argTypeEntry = static_cast<const PrimitiveTypeEntry *>(argType.typeEntry());
            bool convert = argTypeEntry->isObject()
                            || argTypeEntry->isValue()
                            || argType.isValuePointer()
                            || argType.isNativePointer()
                            || argTypeEntry->isFlags()
                            || argTypeEntry->isEnum()
                            || argTypeEntry->isContainer()
                            || argType.referenceType() == LValueReference;

            if (!convert && argTypeEntry->isPrimitive()) {
                if (argTypeEntry->basicReferencedTypeEntry())
                    argTypeEntry = argTypeEntry->basicReferencedTypeEntry();
                convert = !formatUnits().contains(argTypeEntry->name());
            }

            StringStream ac(TextStream::Language::Cpp);
            if (!func->conversionRule(TypeSystem::TargetLangCode, arg.argumentIndex() + 1).isEmpty()) {
                // Has conversion rule.
                ac << arg.name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX);
            } else {
                QString argName = arg.name();
                if (convert)
                    writeToPythonConversion(ac, arg.type(), func->ownerClass(), argName);
                else
                    ac << argName;
            }

            argConversions << ac.toString();
        }

        s << "Py_BuildValue(\"(" << getFormatUnitString(func, false) << ")\",\n"
            << argConversions.join(QLatin1String(",\n")) << "\n));\n";
    }

    bool invalidateReturn = false;
    QSet<int> invalidateArgs;
    for (const FunctionModification &funcMod : functionModifications) {
        for (const ArgumentModification &argMod : funcMod.argument_mods()) {
            const int index = argMod.index();
            if (argMod.resetAfterUse() && !invalidateArgs.contains(index)) {
                invalidateArgs.insert(index);
                s << "bool invalidateArg" << index
                    << " = PyTuple_GET_ITEM(" << PYTHON_ARGS << ", "
                    << index - 1 << ")->ob_refcnt == 1;\n";
            } else if (index == 0 &&
                       argMod.ownerships().value(TypeSystem::TargetLangCode) == TypeSystem::CppOwnership) {
                invalidateReturn = true;
            }
        }
    }
    s << '\n';

    if (!snips.isEmpty()) {
        if (func->injectedCodeUsesPySelf())
            s << "PyObject *pySelf = BindingManager::instance().retrieveWrapper(this);\n";

        const AbstractMetaArgument *lastArg = func->arguments().isEmpty() ? nullptr : &func->arguments().constLast();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode, func, lastArg);
    }

    if (!func->injectedCodeCallsPythonOverride()) {
        s << "Shiboken::AutoDecRef " << PYTHON_RETURN_VAR << "(PyObject_Call("
            << PYTHON_OVERRIDE_VAR << ", " << PYTHON_ARGS << ", nullptr));\n"
            << "// An error happened in python code!\n"
            << "if (" << PYTHON_RETURN_VAR << ".isNull()) {\n" << indent
            << "PyErr_Print();\n" << returnStatement << '\n' << outdent
            << "}\n";

        if (!func->isVoid()) {
            if (invalidateReturn)
                s << "bool invalidateArg0 = " << PYTHON_RETURN_VAR << "->ob_refcnt == 1;\n";

            if (func->typeReplaced(0) != QLatin1String("PyObject")) {

                s << "// Check return type\n";
                if (func->typeReplaced(0).isEmpty()) {
                    s << "PythonToCppFunc " << PYTHON_TO_CPP_VAR << " = "
                        << cpythonIsConvertibleFunction(func->type())
                        << PYTHON_RETURN_VAR << ");\n"
                        << "if (!" << PYTHON_TO_CPP_VAR << ") {\n";
                    {
                        Indentation indent(s);
                        s << "Shiboken::warning(PyExc_RuntimeWarning, 2, "\
                             "\"Invalid return value in function %s, expected %s, got %s.\", \""
                            << func->ownerClass()->name() << '.' << funcName << "\", "
                            << getVirtualFunctionReturnTypeName(func)
                            << ", Py_TYPE(" << PYTHON_RETURN_VAR << ")->tp_name);\n"
                            << returnStatement << '\n';
                    }
                    s << "}\n";

                } else {

                    s << "// Check return type\n"
                        << "bool typeIsValid = ";
                    writeTypeCheck(s, func->type(), QLatin1String(PYTHON_RETURN_VAR),
                                   isNumber(func->type().typeEntry()), func->typeReplaced(0));
                    s << ";\n";
                    s << "if (!typeIsValid";
                    if (func->type().isPointerToWrapperType())
                        s << " && " << PYTHON_RETURN_VAR << " != Py_None";
                    s << ") {\n";
                    {
                        Indentation indent(s);
                        s << "Shiboken::warning(PyExc_RuntimeWarning, 2, "\
                             "\"Invalid return value in function %s, expected %s, got %s.\", \""
                            << func->ownerClass()->name() << '.' << funcName << "\", "
                            << getVirtualFunctionReturnTypeName(func)
                            << ", Py_TYPE(" << PYTHON_RETURN_VAR << ")->tp_name);\n"
                            << returnStatement << '\n';
                    }
                    s << "}\n";

                }
            }

            if (!func->conversionRule(TypeSystem::NativeCode, 0).isEmpty()) {
                // Has conversion rule.
                writeConversionRule(s, func, TypeSystem::NativeCode, QLatin1String(CPP_RETURN_VAR));
            } else if (!func->injectedCodeHasReturnValueAttribution(TypeSystem::NativeCode)) {
                writePythonToCppTypeConversion(s, func->type(), QLatin1String(PYTHON_RETURN_VAR),
                                               QLatin1String(CPP_RETURN_VAR), func->implementingClass());
            }
        }
    }

    if (invalidateReturn) {
        s << "if (invalidateArg0)\n" << indent
            << "Shiboken::Object::releaseOwnership(" << PYTHON_RETURN_VAR
            << ".object());\n" << outdent;
    }
    for (int argIndex : qAsConst(invalidateArgs)) {
        s << "if (invalidateArg" << argIndex << ")\n" << indent
            << "Shiboken::Object::invalidate(PyTuple_GET_ITEM(" << PYTHON_ARGS
            << ", " << (argIndex - 1) << "));\n" << outdent;
    }


    for (const FunctionModification &funcMod : functionModifications) {
        for (const ArgumentModification &argMod : funcMod.argument_mods()) {
            if (argMod.ownerships().contains(TypeSystem::NativeCode)
                && argMod.index() == 0
                && argMod.ownerships().value(TypeSystem::NativeCode) == TypeSystem::CppOwnership) {
                s << "if (Shiboken::Object::checkType(" << PYTHON_RETURN_VAR << "))\n";
                Indentation indent(s);
                s << "Shiboken::Object::releaseOwnership(" << PYTHON_RETURN_VAR << ");\n";
            }
        }
    }

    if (func->hasInjectedCode()) {
        s << '\n';
        const AbstractMetaArgument *lastArg = func->arguments().isEmpty() ? nullptr : &func->arguments().constLast();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode, func, lastArg);
    }

    if (!func->isVoid()) {
        s << "return ";
        if (avoidProtectedHack() && retType->isEnum()) {
            auto metaEnum = findAbstractMetaEnum(retType);
            bool isProtectedEnum = metaEnum.has_value() && metaEnum->isProtected();
            if (isProtectedEnum) {
                QString typeCast;
                if (metaEnum->enclosingClass())
                    typeCast += QLatin1String("::") + metaEnum->enclosingClass()->qualifiedCppName();
                typeCast += QLatin1String("::") + metaEnum->name();
                s << '(' << typeCast << ')';
            }
        }
        if (func->type().referenceType() == LValueReference && !func->type().isPointer())
            s << " *";
        s << CPP_RETURN_VAR << ";\n";
    }

    s << outdent << "}\n\n";
}

void CppGenerator::writeMetaObjectMethod(TextStream &s,
                                         const GeneratorContext &classContext) const
{

    const QString wrapperClassName = classContext.wrapperName();
    const QString qualifiedCppName = classContext.metaClass()->qualifiedCppName();
    s << "const QMetaObject *" << wrapperClassName << "::metaObject() const\n{\n";
    s << indent << "if (QObject::d_ptr->metaObject)\n"
        << indent << "return QObject::d_ptr->dynamicMetaObject();\n" << outdent
        << "SbkObject *pySelf = Shiboken::BindingManager::instance().retrieveWrapper(this);\n"
        << "if (pySelf == nullptr)\n"
        << indent << "return " << qualifiedCppName << "::metaObject();\n" << outdent
        << "return PySide::SignalManager::retrieveMetaObject(reinterpret_cast<PyObject *>(pySelf));\n"
        << outdent << "}\n\n";

    // qt_metacall function
    s << "int " << wrapperClassName << "::qt_metacall(QMetaObject::Call call, int id, void **args)\n";
    s << "{\n" << indent;

    const auto list = classContext.metaClass()->queryFunctionsByName(QLatin1String("qt_metacall"));

    CodeSnipList snips;
    if (list.size() == 1) {
        const auto func = list.constFirst();
        snips = func->injectedCodeSnips();
        if (func->isUserAdded()) {
            CodeSnipList snips = func->injectedCodeSnips();
            writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionAny, TypeSystem::NativeCode, func);
        }
    }

    s << "int result = " << qualifiedCppName << "::qt_metacall(call, id, args);\n"
        << "return result < 0 ? result : PySide::SignalManager::qt_metacall(this, call, id, args);\n"
        << outdent << "}\n\n";

    // qt_metacast function
    writeMetaCast(s, classContext);
}

void CppGenerator::writeMetaCast(TextStream &s,
                                 const GeneratorContext &classContext) const
{
    const QString wrapperClassName = classContext.wrapperName();
    const QString qualifiedCppName = classContext.metaClass()->qualifiedCppName();
    s << "void *" << wrapperClassName << "::qt_metacast(const char *_clname)\n{\n"
        << indent << "if (!_clname)\n" << indent << "return {};\n" << outdent
        << "SbkObject *pySelf = Shiboken::BindingManager::instance().retrieveWrapper(this);\n"
        << "if (pySelf && PySide::inherits(Py_TYPE(pySelf), _clname))\n"
        << indent << "return static_cast<void *>(const_cast< "
        << wrapperClassName << " *>(this));\n" << outdent
        << "return " << qualifiedCppName << "::qt_metacast(_clname);\n"
        << outdent << "}\n\n";
}

void CppGenerator::writeEnumConverterFunctions(TextStream &s, const AbstractMetaEnum &metaEnum) const
{
    if (metaEnum.isPrivate() || metaEnum.isAnonymous())
        return;
    writeEnumConverterFunctions(s, metaEnum.typeEntry());
}

void CppGenerator::writeEnumConverterFunctions(TextStream &s, const TypeEntry *enumType) const
{
    if (!enumType)
        return;
    QString typeName = fixedCppTypeName(enumType);
    QString enumPythonType = cpythonTypeNameExt(enumType);
    QString cppTypeName = getFullTypeName(enumType).trimmed();
    if (avoidProtectedHack()) {
        auto metaEnum = findAbstractMetaEnum(enumType);
        if (metaEnum.has_value() && metaEnum->isProtected())
            cppTypeName = protectedEnumSurrogateName(metaEnum.value());
    }
    StringStream c(TextStream::Language::Cpp);
    c << "*reinterpret_cast<" << cppTypeName << " *>(cppOut) =\n"
        << "    ";
    if (enumType->isFlags())
        c << cppTypeName << "(QFlag(int(PySide::QFlags::getValue(reinterpret_cast<PySideQFlagsObject *>(pyIn)))))";
    else
        c << "static_cast<" << cppTypeName << ">(Shiboken::Enum::getValue(pyIn))";
    c << ";\n";
    writePythonToCppFunction(s, c.toString(), typeName, typeName);

    QString pyTypeCheck = QStringLiteral("PyObject_TypeCheck(pyIn, %1)").arg(enumPythonType);
    writeIsPythonConvertibleToCppFunction(s, typeName, typeName, pyTypeCheck);

    c.clear();

    c << "const int castCppIn = int(*reinterpret_cast<const "
        << cppTypeName << " *>(cppIn));\n" << "return ";
    if (enumType->isFlags()) {
        c << "reinterpret_cast<PyObject *>(PySide::QFlags::newObject(castCppIn, "
            << enumPythonType << "))";
    } else {
        c << "Shiboken::Enum::newItem(" << enumPythonType << ", castCppIn)";
    }
    c << ";\n";
    writeCppToPythonFunction(s, c.toString(), typeName, typeName);
    s << '\n';

    if (enumType->isFlags())
        return;

    auto flags = reinterpret_cast<const EnumTypeEntry *>(enumType)->flags();
    if (!flags)
        return;

    // QFlags part.

    writeEnumConverterFunctions(s, flags);

    c.clear();
    cppTypeName = getFullTypeName(flags).trimmed();
    c << "*reinterpret_cast<" << cppTypeName << " *>(cppOut) =\n"
      << "    " << cppTypeName
      << "(QFlag(int(Shiboken::Enum::getValue(pyIn))));\n";

    QString flagsTypeName = fixedCppTypeName(flags);
    writePythonToCppFunction(s, c.toString(), typeName, flagsTypeName);
    writeIsPythonConvertibleToCppFunction(s, typeName, flagsTypeName, pyTypeCheck);

    c.clear();
    c << "Shiboken::AutoDecRef pyLong(PyNumber_Long(pyIn));\n"
        << "*reinterpret_cast<" << cppTypeName << " *>(cppOut) =\n"
        << "    " << cppTypeName
        << "(QFlag(int(PyLong_AsLong(pyLong.object()))));\n";
    // PYSIDE-898: Include an additional condition to detect if the type of the
    // enum corresponds to the object that is being evaluated.
    // Using only `PyNumber_Check(...)` is too permissive,
    // then we would have been unable to detect the difference between
    // a PolarOrientation and Qt::AlignmentFlag, which was the main
    // issue of the bug.
    const QString numberCondition = QStringLiteral("PyNumber_Check(pyIn) && ") + pyTypeCheck;
    writePythonToCppFunction(s, c.toString(), QLatin1String("number"), flagsTypeName);
    writeIsPythonConvertibleToCppFunction(s, QLatin1String("number"), flagsTypeName, numberCondition);
}

void CppGenerator::writeConverterFunctions(TextStream &s, const AbstractMetaClass *metaClass,
                                           const GeneratorContext &classContext) const
{
    s << "// Type conversion functions.\n\n";

    AbstractMetaEnumList classEnums = metaClass->enums();
    metaClass->getEnumsFromInvisibleNamespacesToBeGenerated(&classEnums);
    if (!classEnums.isEmpty())
        s << "// Python to C++ enum conversion.\n";
    for (const AbstractMetaEnum &metaEnum : qAsConst(classEnums))
        writeEnumConverterFunctions(s, metaEnum);

    if (metaClass->isNamespace())
        return;

    QString typeName;
    if (!classContext.forSmartPointer())
        typeName = getFullTypeName(metaClass);
    else
        typeName = getFullTypeName(classContext.preciseType());

    QString cpythonType = cpythonTypeName(metaClass);

    // Returns the C++ pointer of the Python wrapper.
    s << "// Python to C++ pointer conversion - returns the C++ object of the Python wrapper (keeps object identity).\n";

    QString sourceTypeName = metaClass->name();
    QString targetTypeName = metaClass->name() + QLatin1String("_PTR");
    StringStream c(TextStream::Language::Cpp);
    c << "Shiboken::Conversions::pythonToCppPointer(" << cpythonType << ", pyIn, cppOut);";
    writePythonToCppFunction(s, c.toString(), sourceTypeName, targetTypeName);

    // "Is convertible" function for the Python object to C++ pointer conversion.
    const QString pyTypeCheck = QLatin1String("PyObject_TypeCheck(pyIn, reinterpret_cast<PyTypeObject *>(")
        + cpythonType + QLatin1String("))");
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, pyTypeCheck, QString(), true);
    s << '\n';

    // C++ pointer to a Python wrapper, keeping identity.
    s << "// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity).\n";
    c.clear();
    if (usePySideExtensions() && metaClass->isQObject()) {
        c << "return PySide::getWrapperForQObject(reinterpret_cast<"
            << typeName << " *>(const_cast<void *>(cppIn)), " << cpythonType << ");\n";
    } else {
        c << "auto pyOut = reinterpret_cast<PyObject *>(Shiboken::BindingManager::instance().retrieveWrapper(cppIn));\n"
            << "if (pyOut) {\n";
        {
            Indentation indent(c);
            c << "Py_INCREF(pyOut);\nreturn pyOut;\n";
        }
        c << "}\n"
            << "bool changedTypeName = false;\n"
            << "auto tCppIn = reinterpret_cast<const " << typeName << R"( *>(cppIn);
const char *typeName = typeid(*tCppIn).name();
auto sbkType = Shiboken::ObjectType::typeForTypeName(typeName);
if (sbkType && Shiboken::ObjectType::hasSpecialCastFunction(sbkType)) {
    typeName = typeNameOf(tCppIn);
    changedTypeName = true;
}
)"
            << "PyObject *result = Shiboken::Object::newObject(" << cpythonType
            << R"(, const_cast<void *>(cppIn), false, /* exactType */ changedTypeName, typeName);
if (changedTypeName)
    delete [] typeName;
return result;)";
    }
    std::swap(targetTypeName, sourceTypeName);
    writeCppToPythonFunction(s, c.toString(), sourceTypeName, targetTypeName);

    // The conversions for an Object Type end here.
    if (!metaClass->typeEntry()->isValue() && !metaClass->typeEntry()->isSmartPointer()) {
        s << '\n';
        return;
    }

    // Always copies C++ value (not pointer, and not reference) to a new Python wrapper.
    s  << '\n' << "// C++ to Python copy conversion.\n";
    if (!classContext.forSmartPointer())
        targetTypeName = metaClass->name();
    else
        targetTypeName = classContext.preciseType().name();

    sourceTypeName = targetTypeName + QLatin1String("_COPY");

    c.clear();

    QString computedWrapperName;
    if (!classContext.forSmartPointer()) {
        computedWrapperName = classContext.useWrapper()
            ? classContext.wrapperName() : metaClass->qualifiedCppName();
    } else {
        computedWrapperName = classContext.smartPointerWrapperName();
    }

    c << "return Shiboken::Object::newObject(" << cpythonType
        << ", new ::" << computedWrapperName << "(*reinterpret_cast<const "
        << typeName << " *>(cppIn)), true, true);";
    writeCppToPythonFunction(s, c.toString(), sourceTypeName, targetTypeName);
    s << '\n';

    // Python to C++ copy conversion.
    s << "// Python to C++ copy conversion.\n";
    if (!classContext.forSmartPointer())
        sourceTypeName = metaClass->name();
    else
        sourceTypeName = classContext.preciseType().name();

    targetTypeName = sourceTypeName + QStringLiteral("_COPY");
    c.clear();

    QString pyInVariable = QLatin1String("pyIn");
    QString wrappedCPtrExpression;
    if (!classContext.forSmartPointer())
        wrappedCPtrExpression = cpythonWrapperCPtr(metaClass->typeEntry(), pyInVariable);
    else
        wrappedCPtrExpression = cpythonWrapperCPtr(classContext.preciseType(), pyInVariable);

    c << "*reinterpret_cast<" << typeName << " *>(cppOut) = *"
        << wrappedCPtrExpression << ';';
    writePythonToCppFunction(s, c.toString(), sourceTypeName, targetTypeName);

    // "Is convertible" function for the Python object to C++ value copy conversion.
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, pyTypeCheck);
    s << '\n';

    // User provided implicit conversions.
    CustomConversion *customConversion = metaClass->typeEntry()->customConversion();

    // Implicit conversions.
    AbstractMetaFunctionCList implicitConvs;
    if (!customConversion || !customConversion->replaceOriginalTargetToNativeConversions()) {
        const auto &allImplicitConvs = implicitConversions(metaClass->typeEntry());
        for (const auto &func : allImplicitConvs) {
            if (!func->isUserAdded())
                implicitConvs << func;
        }
    }

    if (!implicitConvs.isEmpty())
        s << "// Implicit conversions.\n";

    AbstractMetaType targetType = buildAbstractMetaTypeFromAbstractMetaClass(metaClass);
    for (const auto &conv : qAsConst(implicitConvs)) {
        if (conv->isModifiedRemoved())
            continue;

        QString typeCheck;
        QString toCppConv;
        QString toCppPreConv;
        if (conv->isConversionOperator()) {
            const AbstractMetaClass *sourceClass = conv->ownerClass();
            typeCheck = QStringLiteral("PyObject_TypeCheck(pyIn, %1)").arg(cpythonTypeNameExt(sourceClass->typeEntry()));
            toCppConv = QLatin1Char('*') + cpythonWrapperCPtr(sourceClass->typeEntry(), QLatin1String("pyIn"));
        } else {
            // Constructor that does implicit conversion.
            if (!conv->typeReplaced(1).isEmpty() || conv->isModifiedToArray(1))
                continue;
            const AbstractMetaType sourceType = conv->arguments().constFirst().type();
            typeCheck = cpythonCheckFunction(sourceType);
            bool isUserPrimitiveWithoutTargetLangName = sourceType.isUserPrimitive()
                                                        && sourceType.typeEntry()->targetLangApiName() == sourceType.typeEntry()->name();
            if (!sourceType.isWrapperType()
                && !isUserPrimitiveWithoutTargetLangName
                && !sourceType.typeEntry()->isEnum()
                && !sourceType.typeEntry()->isFlags()
                && !sourceType.typeEntry()->isContainer()) {
                typeCheck += QLatin1Char('(');
            }
            if (sourceType.isWrapperType()) {
                typeCheck += QLatin1String("pyIn)");
                toCppConv = (sourceType.referenceType() == LValueReference
                             || !sourceType.isPointerToWrapperType())
                    ? QLatin1String(" *") : QString();
                toCppConv += cpythonWrapperCPtr(sourceType.typeEntry(), QLatin1String("pyIn"));
            } else if (typeCheck.contains(QLatin1String("%in"))) {
                typeCheck.replace(QLatin1String("%in"), QLatin1String("pyIn"));
                typeCheck.append(QLatin1Char(')'));
            } else {
                typeCheck += QLatin1String("pyIn)");
            }

            if (sourceType.isUserPrimitive()
                || sourceType.isExtendedCppPrimitive()
                || sourceType.typeEntry()->isContainer()
                || sourceType.typeEntry()->isEnum()
                || sourceType.typeEntry()->isFlags()) {
                StringStream pc(TextStream::Language::Cpp);
                pc << getFullTypeNameWithoutModifiers(sourceType) << " cppIn";
                writeMinimalConstructorExpression(pc, sourceType);
                pc << ";\n";
                writeToCppConversion(pc, sourceType, nullptr, QLatin1String("pyIn"), QLatin1String("cppIn"));
                pc << ';';
                toCppPreConv = pc.toString();
                toCppConv.append(QLatin1String("cppIn"));
            } else if (!sourceType.isWrapperType()) {
                StringStream tcc(TextStream::Language::Cpp);
                writeToCppConversion(tcc, sourceType, metaClass, QLatin1String("pyIn"), QLatin1String("/*BOZO-1061*/"));
                toCppConv = tcc.toString();
            }
        }
        const AbstractMetaType sourceType = conv->isConversionOperator()
                                            ? buildAbstractMetaTypeFromAbstractMetaClass(conv->ownerClass())
                                            : conv->arguments().constFirst().type();
        writePythonToCppConversionFunctions(s, sourceType, targetType, typeCheck, toCppConv, toCppPreConv);
    }

    writeCustomConverterFunctions(s, customConversion);
}

void CppGenerator::writeCustomConverterFunctions(TextStream &s,
                                                 const CustomConversion *customConversion) const
{
    if (!customConversion)
        return;
    const CustomConversion::TargetToNativeConversions &toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty())
        return;
    s << "// Python to C++ conversions for type '" << customConversion->ownerType()->qualifiedCppName() << "'.\n";
    for (CustomConversion::TargetToNativeConversion *toNative : toCppConversions)
        writePythonToCppConversionFunctions(s, toNative, customConversion->ownerType());
    s << '\n';
}

void CppGenerator::writeConverterRegister(TextStream &s, const AbstractMetaClass *metaClass,
                                          const GeneratorContext &classContext) const
{
    if (metaClass->isNamespace())
        return;
    s << "// Register Converter\n"
        << "SbkConverter *converter = Shiboken::Conversions::createConverter("
        << cpythonTypeName(metaClass) << ',' << '\n';
    {
        Indentation indent(s);
        QString sourceTypeName = metaClass->name();
        QString targetTypeName = sourceTypeName + QLatin1String("_PTR");
        s << pythonToCppFunctionName(sourceTypeName, targetTypeName) << ',' << '\n'
            << convertibleToCppFunctionName(sourceTypeName, targetTypeName) << ',' << '\n';
        std::swap(targetTypeName, sourceTypeName);
        s << cppToPythonFunctionName(sourceTypeName, targetTypeName);
        if (metaClass->typeEntry()->isValue() || metaClass->typeEntry()->isSmartPointer()) {
            s << ',' << '\n';
            sourceTypeName = metaClass->name() + QLatin1String("_COPY");
            s << cppToPythonFunctionName(sourceTypeName, targetTypeName);
        }
    }
    s << ");\n";

    s << '\n';

    auto writeConversions = [&s](const QString &signature)
    {
        s << "Shiboken::Conversions::registerConverterName(converter, \"" << signature << "\");\n"
            << "Shiboken::Conversions::registerConverterName(converter, \"" << signature << "*\");\n"
            << "Shiboken::Conversions::registerConverterName(converter, \"" << signature << "&\");\n";
    };

    auto writeConversionsForType = [writeConversions](const QString &fullTypeName)
    {
        QStringList lst = fullTypeName.split(QLatin1String("::"),
                                               Qt::SkipEmptyParts);
        while (!lst.isEmpty()) {
            QString signature = lst.join(QLatin1String("::"));
            writeConversions(signature);
            lst.removeFirst();
        }
    };


    if (!classContext.forSmartPointer()) {
        writeConversionsForType(metaClass->qualifiedCppName());
    } else {
        const QString &smartPointerType = classContext.preciseType().instantiations().at(0).cppSignature();
        const QString &smartPointerName = classContext.preciseType().typeEntry()->name();

        QStringList lst = smartPointerType.split(QLatin1String("::"),
                                               Qt::SkipEmptyParts);
        while (!lst.isEmpty()) {
            QString signature = lst.join(QLatin1String("::"));
            writeConversions(QStringLiteral("%1<%2 >").arg(smartPointerName, signature));
            lst.removeFirst();
        }

        writeConversionsForType(smartPointerType);
    }

    s << "Shiboken::Conversions::registerConverterName(converter, typeid(::";
    QString qualifiedCppNameInvocation;
    if (!classContext.forSmartPointer())
        qualifiedCppNameInvocation = metaClass->qualifiedCppName();
    else
        qualifiedCppNameInvocation = classContext.preciseType().cppSignature();

    s << qualifiedCppNameInvocation << ").name());\n";

    if (classContext.useWrapper()) {
        s << "Shiboken::Conversions::registerConverterName(converter, typeid(::"
            << classContext.wrapperName() << ").name());\n";
    }

    s << '\n';

    if (!metaClass->typeEntry()->isValue() && !metaClass->typeEntry()->isSmartPointer())
        return;

    // Python to C++ copy (value, not pointer neither reference) conversion.
    s << "// Add Python to C++ copy (value, not pointer neither reference) conversion to type converter.\n";
    QString sourceTypeName = metaClass->name();
    QString targetTypeName = sourceTypeName + QLatin1String("_COPY");
    QString toCpp = pythonToCppFunctionName(sourceTypeName, targetTypeName);
    QString isConv = convertibleToCppFunctionName(sourceTypeName, targetTypeName);
    writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);

    // User provided implicit conversions.
    CustomConversion *customConversion = metaClass->typeEntry()->customConversion();

    // Add implicit conversions.
    AbstractMetaFunctionCList implicitConvs;
    if (!customConversion || !customConversion->replaceOriginalTargetToNativeConversions()) {
        const auto &allImplicitConvs = implicitConversions(metaClass->typeEntry());
        for (const auto &func : allImplicitConvs) {
            if (!func->isUserAdded())
                implicitConvs << func;
        }
    }

    if (!implicitConvs.isEmpty())
        s << "// Add implicit conversions to type converter.\n";

    AbstractMetaType targetType = buildAbstractMetaTypeFromAbstractMetaClass(metaClass);
    for (const auto &conv : qAsConst(implicitConvs)) {
        if (conv->isModifiedRemoved())
            continue;
        AbstractMetaType sourceType;
        if (conv->isConversionOperator()) {
            sourceType = buildAbstractMetaTypeFromAbstractMetaClass(conv->ownerClass());
        } else {
            // Constructor that does implicit conversion.
            if (!conv->typeReplaced(1).isEmpty() || conv->isModifiedToArray(1))
                continue;
            sourceType = conv->arguments().constFirst().type();
        }
        QString toCpp = pythonToCppFunctionName(sourceType, targetType);
        QString isConv = convertibleToCppFunctionName(sourceType, targetType);
        writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);
    }

    writeCustomConverterRegister(s, customConversion, QLatin1String("converter"));
}

void CppGenerator::writeCustomConverterRegister(TextStream &s, const CustomConversion *customConversion,
                                                const QString &converterVar) const
{
    if (!customConversion)
        return;
    const CustomConversion::TargetToNativeConversions &toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty())
        return;
    s << "// Add user defined implicit conversions to type converter.\n";
    for (CustomConversion::TargetToNativeConversion *toNative : toCppConversions) {
        QString toCpp = pythonToCppFunctionName(toNative, customConversion->ownerType());
        QString isConv = convertibleToCppFunctionName(toNative, customConversion->ownerType());
        writeAddPythonToCppConversion(s, converterVar, toCpp, isConv);
    }
}

void CppGenerator::writeContainerConverterFunctions(TextStream &s,
                                                    const AbstractMetaType &containerType) const
{
    writeCppToPythonFunction(s, containerType);
    writePythonToCppConversionFunctions(s, containerType);
}

void CppGenerator::writeSmartPointerConverterFunctions(TextStream &s,
                                                       const AbstractMetaType &smartPointerType) const
{
    const AbstractMetaClass *targetClass = AbstractMetaClass::findClass(classes(), smartPointerType.instantiations().at(0).typeEntry());

    if (targetClass) {
        const auto *smartPointerTypeEntry =
                static_cast<const SmartPointerTypeEntry *>(
                    smartPointerType.typeEntry());

        // TODO: Missing conversion to smart pointer pointer type:

        s << "// Register smartpointer conversion for all derived classes\n";
        const auto classes = targetClass->typeSystemBaseClasses();
        for (auto k : classes) {
            if (smartPointerTypeEntry->matchesInstantiation(k->typeEntry())) {
                if (auto smartTargetType = findSmartPointerInstantiation(k->typeEntry())) {
                    s << "// SmartPointer derived class: "
                        << smartTargetType->cppSignature() << "\n";
                    writePythonToCppConversionFunctions(s, smartPointerType, smartTargetType.value(), {}, {}, {});
                }
            }
        }
    }
}

void CppGenerator::writeMethodWrapperPreamble(TextStream &s, OverloadData &overloadData,
                                              const GeneratorContext &context) const
{
    const auto rfunc = overloadData.referenceFunction();
    const AbstractMetaClass *ownerClass = rfunc->targetLangOwner();
    Q_ASSERT(ownerClass == context.metaClass());
    int minArgs = overloadData.minArgs();
    int maxArgs = overloadData.maxArgs();
    bool initPythonArguments;

    // If method is a constructor...
    if (rfunc->isConstructor()) {
        // Check if the right constructor was called.
        if (!ownerClass->hasPrivateDestructor()) {
            s << "if (Shiboken::Object::isUserType(self) && !Shiboken::ObjectType::canCallConstructor(self->ob_type, Shiboken::SbkType< ::";
            QString qualifiedCppName;
            if (!context.forSmartPointer())
                qualifiedCppName = ownerClass->qualifiedCppName();
            else
                qualifiedCppName = context.preciseType().cppSignature();

            s << qualifiedCppName << " >()))\n";
            Indentation indent(s);
            s << returnStatement(m_currentErrorCode)  << '\n' << '\n';
        }
        // Declare pointer for the underlying C++ object.
        s << "::";
        if (!context.forSmartPointer()) {
            s << (context.useWrapper() ? context.wrapperName() : ownerClass->qualifiedCppName());
        } else {
            s << context.smartPointerWrapperName();
        }
        s << " *cptr{};\n";

        initPythonArguments = maxArgs > 0;

    } else {
        if (rfunc->implementingClass() &&
            (!rfunc->implementingClass()->isNamespace() && overloadData.hasInstanceFunction())) {
            writeCppSelfDefinition(s, rfunc, context, overloadData.hasStaticFunction());
        }
        if (!rfunc->isInplaceOperator() && overloadData.hasNonVoidReturnType())
            s << "PyObject *" << PYTHON_RETURN_VAR << "{};\n";

        initPythonArguments = minArgs != maxArgs || maxArgs > 1;
    }

    s << R"(PyObject *errInfo{};
SBK_UNUSED(errInfo)
static const char *fullName = ")" << fullPythonFunctionName(rfunc, true)
         << "\";\nSBK_UNUSED(fullName)\n";
    if (maxArgs > 0) {
        s << "int overloadId = -1;\n"
            << "PythonToCppFunc " << PYTHON_TO_CPP_VAR;
        if (pythonFunctionWrapperUsesListOfArguments(overloadData)) {
            s << "[] = { " << NULL_PTR;
            for (int i = 1; i < maxArgs; ++i)
                s << ", " << NULL_PTR;
            s << " };\n";
        } else {
            s << "{};\n";
        }
        writeUnusedVariableCast(s, QLatin1String(PYTHON_TO_CPP_VAR));
    }

    if (initPythonArguments) {
        s << "const Py_ssize_t numArgs = ";
        if (minArgs == 0 && maxArgs == 1 && !rfunc->isConstructor() && !pythonFunctionWrapperUsesListOfArguments(overloadData))
            s << "(" << PYTHON_ARG << " == 0 ? 0 : 1);\n";
        else
            writeArgumentsInitializer(s, overloadData);
    }
}

void CppGenerator::writeConstructorWrapper(TextStream &s, const AbstractMetaFunctionCList &overloads,
                                           const GeneratorContext &classContext) const
{
    ErrorCode errorCode(-1);
    OverloadData overloadData(overloads, this);

    const auto rfunc = overloadData.referenceFunction();
    const AbstractMetaClass *metaClass = rfunc->ownerClass();

    s << "static int\n";
    s << cpythonFunctionName(rfunc)
        << "(PyObject *self, PyObject *args, PyObject *kwds)\n{\n" << indent;

    if (usePySideExtensions() && metaClass->isQObject())
        s << "const QMetaObject *metaObject;\n";

    s << "SbkObject *sbkSelf = reinterpret_cast<SbkObject *>(self);\n";

    if (metaClass->isAbstract() || metaClass->baseClassNames().size() > 1) {
        s << "SbkObjectType *type = reinterpret_cast<SbkObjectType *>(self->ob_type);\n"
            << "SbkObjectType *myType = reinterpret_cast<SbkObjectType *>("
            << cpythonTypeNameExt(metaClass->typeEntry()) << ");\n";
    }

    if (metaClass->isAbstract()) {
        s << "if (type == myType) {\n" << indent
            << "PyErr_SetString(PyExc_NotImplementedError,\n" << indent
            << "\"'" << metaClass->qualifiedCppName()
            << "' represents a C++ abstract class and cannot be instantiated\");\n" << outdent
            << returnStatement(m_currentErrorCode) << '\n' << outdent
            << "}\n\n";
    }

    if (metaClass->baseClassNames().size() > 1) {
        if (!metaClass->isAbstract())
            s << "if (type != myType)\n" << indent;
        s << "Shiboken::ObjectType::copyMultipleInheritance(type, myType);\n";
        if (!metaClass->isAbstract())
            s << outdent << '\n';
    }

    writeMethodWrapperPreamble(s, overloadData, classContext);

    s << '\n';

    if (overloadData.maxArgs() > 0)
        writeOverloadedFunctionDecisor(s, overloadData);

    writeFunctionCalls(s, overloadData, classContext);
    s << '\n';

    s << "if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::"
        << metaClass->qualifiedCppName() << " >(), cptr)) {\n";
    {
        Indentation indent(s);
        s << "delete cptr;\n";
        if (overloadData.maxArgs() > 0)
            s << "Py_XDECREF(errInfo);\n";
        s << returnStatement(m_currentErrorCode) << '\n';
    }
    s << "}\n";
    if (overloadData.maxArgs() > 0)
        s << "if (!cptr) goto " << cpythonFunctionName(rfunc) << "_TypeError;\n\n";

    s << "Shiboken::Object::setValidCpp(sbkSelf, true);\n";
    // If the created C++ object has a C++ wrapper the ownership is assigned to Python
    // (first "1") and the flag indicating that the Python wrapper holds an C++ wrapper
    // is marked as true (the second "1"). Otherwise the default values apply:
    // Python owns it and C++ wrapper is false.
    if (shouldGenerateCppWrapper(overloads.constFirst()->ownerClass()))
        s << "Shiboken::Object::setHasCppWrapper(sbkSelf, true);\n";
    // Need to check if a wrapper for same pointer is already registered
    // Caused by bug PYSIDE-217, where deleted objects' wrappers are not released
    s << "if (Shiboken::BindingManager::instance().hasWrapper(cptr)) {\n";
    {
        Indentation indent(s);
        s << "Shiboken::BindingManager::instance().releaseWrapper("
             "Shiboken::BindingManager::instance().retrieveWrapper(cptr));\n";
    }
    s << "}\nShiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);\n";

    // Create metaObject and register signal/slot
    bool errHandlerNeeded = overloadData.maxArgs() > 0;
    if (metaClass->isQObject() && usePySideExtensions()) {
        errHandlerNeeded = true;
        s << "\n// QObject setup\n"
            << "PySide::Signal::updateSourceObject(self);\n"
            << "metaObject = cptr->metaObject(); // <- init python qt properties\n"
            << "if (errInfo && PyDict_Check(errInfo)) {\n" << indent
            << "if (!PySide::fillQtProperties(self, metaObject, errInfo))\n" << indent
            << "goto " << cpythonFunctionName(rfunc) << "_TypeError;\n" << outdent
            << "Py_DECREF(errInfo);\n" << outdent
            << "};\n";
    }

    // Constructor code injections, position=end
    bool hasCodeInjectionsAtEnd = false;
    for (const auto &func : overloads) {
        const CodeSnipList &injectedCodeSnips = func->injectedCodeSnips();
        for (const CodeSnip &cs : injectedCodeSnips) {
            if (cs.position == TypeSystem::CodeSnipPositionEnd) {
                hasCodeInjectionsAtEnd = true;
                break;
            }
        }
    }
    if (hasCodeInjectionsAtEnd) {
        // FIXME: C++ arguments are not available in code injection on constructor when position = end.
        s <<"switch (overloadId) {\n";
        for (const auto &func : overloads) {
            Indentation indent(s);
            const CodeSnipList &injectedCodeSnips = func->injectedCodeSnips();
            for (const CodeSnip &cs : injectedCodeSnips) {
                if (cs.position == TypeSystem::CodeSnipPositionEnd) {
                    s << "case " << metaClass->functions().indexOf(func) << ':' << '\n'
                        << "{\n";
                    {
                        Indentation indent(s);
                        writeCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode, func);
                    }
                    s << "}\n";
                    break;
                }
            }
        }
        s << "}\n";
    }

    s << "\n\nreturn 1;\n";
    if (errHandlerNeeded)
        writeErrorSection(s, overloadData);
    s<< outdent << "}\n\n";
}

void CppGenerator::writeMethodWrapper(TextStream &s, const AbstractMetaFunctionCList &overloads,
                                      const GeneratorContext &classContext) const
{
    OverloadData overloadData(overloads, this);
    const auto rfunc = overloadData.referenceFunction();

    int maxArgs = overloadData.maxArgs();

    s << "static PyObject *";
    s << cpythonFunctionName(rfunc) << "(PyObject *self";
    if (maxArgs > 0) {
        s << ", PyObject *" << (pythonFunctionWrapperUsesListOfArguments(overloadData) ? "args" : PYTHON_ARG);
        if (overloadData.hasArgumentWithDefaultValue() || rfunc->isCallOperator())
            s << ", PyObject *kwds";
    }
    s << ")\n{\n" << indent;

    writeMethodWrapperPreamble(s, overloadData, classContext);

    s << '\n';

    /*
     * This code is intended for shift operations only:
     * Make sure reverse <</>> operators defined in other classes (specially from other modules)
     * are called. A proper and generic solution would require an reengineering in the operator
     * system like the extended converters.
     *
     * Solves #119 - QDataStream <</>> operators not working for QPixmap
     * http://bugs.openbossa.org/show_bug.cgi?id=119
     */
    bool hasReturnValue = overloadData.hasNonVoidReturnType();
    bool callExtendedReverseOperator = hasReturnValue
                                       && !rfunc->isInplaceOperator()
                                       && !rfunc->isCallOperator()
                                       && rfunc->isOperatorOverload();

    QScopedPointer<Indentation> reverseIndent;

    if (callExtendedReverseOperator) {
        QString revOpName = ShibokenGenerator::pythonOperatorFunctionName(rfunc).insert(2, QLatin1Char('r'));
        // For custom classes, operations like __radd__ and __rmul__
        // will enter an infinite loop.
        if (rfunc->isBinaryOperator() && revOpName.contains(QLatin1String("shift"))) {
            s << "Shiboken::AutoDecRef attrName(Py_BuildValue(\"s\", \"" << revOpName << "\"));\n";
            s << "if (!isReverse\n";
            {
                Indentation indent(s);
                s << "&& Shiboken::Object::checkType(" << PYTHON_ARG << ")\n"
                    << "&& !PyObject_TypeCheck(" << PYTHON_ARG << ", self->ob_type)\n"
                    << "&& PyObject_HasAttr(" << PYTHON_ARG << ", attrName)) {\n";

                // This PyObject_CallMethod call will emit lots of warnings like
                // "deprecated conversion from string constant to char *" during compilation
                // due to the method name argument being declared as "char *" instead of "const char *"
                // issue 6952 http://bugs.python.org/issue6952
                s << "PyObject *revOpMethod = PyObject_GetAttr(" << PYTHON_ARG << ", attrName);\n";
                s << "if (revOpMethod && PyCallable_Check(revOpMethod)) {\n";
                {
                    Indentation indent(s);
                    s << PYTHON_RETURN_VAR << " = PyObject_CallFunction(revOpMethod, const_cast<char *>(\"O\"), self);\n"
                        << "if (PyErr_Occurred() && (PyErr_ExceptionMatches(PyExc_NotImplementedError)"
                        << " || PyErr_ExceptionMatches(PyExc_AttributeError))) {\n";
                    {
                        Indentation indent(s);
                        s << "PyErr_Clear();\n"
                            << "Py_XDECREF(" << PYTHON_RETURN_VAR << ");\n"
                            << PYTHON_RETURN_VAR << " = " << NULL_PTR << ";\n";
                    }
                    s << "}\n";
                }
                s << "}\n"
                    << "Py_XDECREF(revOpMethod);\n\n";
            } //
            s << "}\n\n"
                << "// Do not enter here if other object has implemented a reverse operator.\n"
                << "if (!" << PYTHON_RETURN_VAR << ") {\n";
            reverseIndent.reset(new Indentation(s));
        } // binary shift operator
    }

    if (maxArgs > 0)
        writeOverloadedFunctionDecisor(s, overloadData);

    writeFunctionCalls(s, overloadData, classContext);

    if (!reverseIndent.isNull()) { // binary shift operator
        reverseIndent.reset();
        s  << '\n' << "} // End of \"if (!" << PYTHON_RETURN_VAR << ")\"\n";
    }

    s << '\n';

    writeFunctionReturnErrorCheckSection(s, hasReturnValue && !rfunc->isInplaceOperator());

    if (hasReturnValue) {
        if (rfunc->isInplaceOperator()) {
            s << "Py_INCREF(self);\nreturn self;\n";
        } else {
            s << "return " << PYTHON_RETURN_VAR << ";\n";
        }
    } else {
        s << "Py_RETURN_NONE;\n";
    }

    if (maxArgs > 0)
        writeErrorSection(s, overloadData);

    s<< outdent << "}\n\n";
}

void CppGenerator::writeArgumentsInitializer(TextStream &s, OverloadData &overloadData) const
{
    const auto rfunc = overloadData.referenceFunction();
    s << "PyTuple_GET_SIZE(args);\n";
    writeUnusedVariableCast(s, QLatin1String("numArgs"));

    int minArgs = overloadData.minArgs();
    int maxArgs = overloadData.maxArgs();

    s << "PyObject *";
    s << PYTHON_ARGS << "[] = {"
        << QString(maxArgs, QLatin1Char('0')).split(QLatin1String(""), Qt::SkipEmptyParts).join(QLatin1String(", "))
        << "};\n\n";

    if (overloadData.hasVarargs()) {
        maxArgs--;
        if (minArgs > maxArgs)
            minArgs = maxArgs;

        s << "PyObject *nonvarargs = PyTuple_GetSlice(args, 0, " << maxArgs << ");\n"
            << "Shiboken::AutoDecRef auto_nonvarargs(nonvarargs);\n"
            << PYTHON_ARGS << '[' << maxArgs << "] = PyTuple_GetSlice(args, "
            << maxArgs << ", numArgs);\n"
            << "Shiboken::AutoDecRef auto_varargs(" << PYTHON_ARGS << "["
            << maxArgs << "]);\n\n";
    }

    bool usesNamedArguments = overloadData.hasArgumentWithDefaultValue();

    s << "// invalid argument lengths\n";
    bool ownerClassIsQObject = rfunc->ownerClass() && rfunc->ownerClass()->isQObject() && rfunc->isConstructor();
    if (usesNamedArguments) {
        if (!ownerClassIsQObject) {
            s << "if (numArgs > " << maxArgs << ") {\n";
            {
                Indentation indent(s);
                s << "static PyObject *const too_many = "
                     "Shiboken::String::createStaticString(\">\");\n"
                    << "errInfo = too_many;\n"
                    << "Py_INCREF(errInfo);\n"
                    << "goto " << cpythonFunctionName(rfunc) << "_TypeError;\n";
            }
            s << '}';
        }
        if (minArgs > 0) {
            if (!ownerClassIsQObject)
                s << " else ";
            s << "if (numArgs < " << minArgs << ") {\n";
            {
                Indentation indent(s);
                s << "static PyObject *const too_few = "
                     "Shiboken::String::createStaticString(\"<\");\n"
                    << "errInfo = too_few;\n"
                    << "Py_INCREF(errInfo);\n"
                    << "goto " << cpythonFunctionName(rfunc) << "_TypeError;\n";
            }
            s << '}';
        }
    }
    const QList<int> invalidArgsLength = overloadData.invalidArgumentLengths();
    if (!invalidArgsLength.isEmpty()) {
        QStringList invArgsLen;
        for (int i : qAsConst(invalidArgsLength))
            invArgsLen << QStringLiteral("numArgs == %1").arg(i);
        if (usesNamedArguments && (!ownerClassIsQObject || minArgs > 0))
            s << " else ";
        s << "if (" << invArgsLen.join(QLatin1String(" || ")) << ")\n";
        Indentation indent(s);
        s << "goto " << cpythonFunctionName(rfunc) << "_TypeError;";
    }
    s  << "\n\n";

    QString funcName;
    if (rfunc->isOperatorOverload())
        funcName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
    else
        funcName = rfunc->name();

    QString argsVar = overloadData.hasVarargs() ?  QLatin1String("nonvarargs") : QLatin1String("args");
    s << "if (!";
    if (usesNamedArguments) {
        s << "PyArg_ParseTuple(" << argsVar << ", \"|" << QByteArray(maxArgs, 'O')
            << ':' << funcName << '"';
    } else {
        s << "PyArg_UnpackTuple(" << argsVar << ", \"" << funcName << "\", "
            << minArgs << ", " << maxArgs;
    }
    for (int i = 0; i < maxArgs; i++)
        s << ", &(" << PYTHON_ARGS << '[' << i << "])";
    s << "))\n";
    {
        Indentation indent(s);
        s << returnStatement(m_currentErrorCode) << '\n';
    }
    s << '\n';
}

void CppGenerator::writeCppSelfConversion(TextStream &s, const GeneratorContext &context,
                                          const QString &className, bool useWrapperClass) const
{
    static const QString pythonSelfVar = QLatin1String("self");
    if (useWrapperClass)
        s << "static_cast<" << className << " *>(";
    if (!context.forSmartPointer())
       s << cpythonWrapperCPtr(context.metaClass(), pythonSelfVar);
    else
       s << cpythonWrapperCPtr(context.preciseType(), pythonSelfVar);
    if (useWrapperClass)
        s << ')';
}

void CppGenerator::writeCppSelfDefinition(TextStream &s,
                                          const GeneratorContext &context,
                                          bool hasStaticOverload,
                                          bool cppSelfAsReference) const
{
    Q_ASSERT(!(cppSelfAsReference && hasStaticOverload));

    const AbstractMetaClass *metaClass = context.metaClass();
    bool useWrapperClass = avoidProtectedHack() && metaClass->hasProtectedMembers()
        && !metaClass->attributes().testFlag(AbstractMetaAttributes::FinalCppClass);
    Q_ASSERT(!useWrapperClass || context.useWrapper());
    QString className;
    if (!context.forSmartPointer()) {
        className = useWrapperClass
            ? context.wrapperName()
            : (QLatin1String("::") + metaClass->qualifiedCppName());
    } else {
        className = context.smartPointerWrapperName();
    }

    writeInvalidPyObjectCheck(s, QLatin1String("self"));

    if (cppSelfAsReference) {
         s << "auto &" <<  CPP_SELF_VAR << " = *";
         writeCppSelfConversion(s, context, className, useWrapperClass);
         s << ";\n";
         return;
    }

    if (!hasStaticOverload) {
        s << "auto " <<  CPP_SELF_VAR << " = ";
        writeCppSelfConversion(s, context, className, useWrapperClass);
        s << ";\n";
        writeUnusedVariableCast(s, QLatin1String(CPP_SELF_VAR));
        return;
    }

    s << className << " *" << CPP_SELF_VAR << " = nullptr;\n";
    writeUnusedVariableCast(s, QLatin1String(CPP_SELF_VAR));

    // Checks if the underlying C++ object is valid.
    s << "if (self)\n";
    {
        Indentation indent(s);
        s << CPP_SELF_VAR << " = ";
        writeCppSelfConversion(s, context, className, useWrapperClass);
        s << ";\n";
    }
}

void CppGenerator::writeCppSelfDefinition(TextStream &s,
                                          const AbstractMetaFunctionCPtr &func,
                                          const GeneratorContext &context,
                                          bool hasStaticOverload) const
{
    if (!func->ownerClass() || func->isConstructor())
        return;

    if (func->isOperatorOverload() && func->isBinaryOperator()) {
        QString checkFunc = cpythonCheckFunction(func->ownerClass()->typeEntry());
        s << "bool isReverse = " << checkFunc << PYTHON_ARG << ")\n";
        {
            Indentation indent1(s, 4);
            s << "&& !" << checkFunc << "self);\n";
        }
        s << "if (isReverse)\n";
        Indentation indent(s);
        s << "std::swap(self, " << PYTHON_ARG << ");\n";
    }

    writeCppSelfDefinition(s, context, hasStaticOverload);
}

void CppGenerator::writeErrorSection(TextStream &s, OverloadData &overloadData) const
{
    const auto rfunc = overloadData.referenceFunction();
    s  << '\n' << cpythonFunctionName(rfunc) << "_TypeError:\n";
    Indentation indentation(s);
    QString funcName = fullPythonFunctionName(rfunc, true);

    QString argsVar = pythonFunctionWrapperUsesListOfArguments(overloadData)
        ? QLatin1String("args") : QLatin1String(PYTHON_ARG);
    s << "Shiboken::setErrorAboutWrongArguments(" << argsVar
                << ", fullName, errInfo);\n"
        << "Py_XDECREF(errInfo);\n"
        << "return " << m_currentErrorCode << ";\n";
}

void CppGenerator::writeFunctionReturnErrorCheckSection(TextStream &s, bool hasReturnValue)
{
    s << "if (PyErr_Occurred()";
    if (hasReturnValue)
        s << " || !" << PYTHON_RETURN_VAR;
    s << ") {\n";
    {
        Indentation indent(s);
        if (hasReturnValue)
            s << "Py_XDECREF(" << PYTHON_RETURN_VAR << ");\n";
        s << returnStatement(m_currentErrorCode) << '\n';
    }
    s << "}\n";
}

void CppGenerator::writeInvalidPyObjectCheck(TextStream &s, const QString &pyObj)
{
    s << "if (!Shiboken::Object::isValid(" << pyObj << "))\n";
    Indentation indent(s);
    s << returnStatement(m_currentErrorCode) << '\n';
}

static QString pythonToCppConverterForArgumentName(const QString &argumentName)
{
    static const QRegularExpression pyArgsRegex(QLatin1String(PYTHON_ARGS)
                                                + QLatin1String(R"((\[\d+[-]?\d*\]))"));
    Q_ASSERT(pyArgsRegex.isValid());
    const QRegularExpressionMatch match = pyArgsRegex.match(argumentName);
    QString result = QLatin1String(PYTHON_TO_CPP_VAR);
    if (match.hasMatch())
        result += match.captured(1);
    return result;
}

void CppGenerator::writeTypeCheck(TextStream &s, AbstractMetaType argType,
                                  const QString &argumentName, bool isNumber,
                                  const QString &customType, bool rejectNull) const
{
    QString customCheck;
    if (!customType.isEmpty()) {
        AbstractMetaType metaType;
        // PYSIDE-795: Note: XML-Overrides are handled in this shibokengenerator function!
        // This enables iterables for QMatrix4x4 for instance.
        auto customCheckResult = guessCPythonCheckFunction(customType);
        customCheck = customCheckResult.checkFunction;
        if (customCheckResult.type.has_value())
            argType = customCheckResult.type.value();
    }

    // TODO-CONVERTER: merge this with the code below.
    QString typeCheck;
    if (customCheck.isEmpty())
        typeCheck = cpythonIsConvertibleFunction(argType, argType.isEnum() ? false : isNumber);
    else
        typeCheck = customCheck;
    typeCheck.append(QString::fromLatin1("(%1)").arg(argumentName));

    // TODO-CONVERTER -----------------------------------------------------------------------
    if (customCheck.isEmpty() && !argType.typeEntry()->isCustom()) {
        typeCheck = QString::fromLatin1("(%1 = %2))").arg(pythonToCppConverterForArgumentName(argumentName), typeCheck);
        if (!isNumber && argType.typeEntry()->isCppPrimitive())
            typeCheck.prepend(QString::fromLatin1("%1(%2) && ").arg(cpythonCheckFunction(argType), argumentName));
    }
    // TODO-CONVERTER -----------------------------------------------------------------------

    if (rejectNull)
        typeCheck = QString::fromLatin1("(%1 != Py_None && %2)").arg(argumentName, typeCheck);

    s << typeCheck;
}

static void checkTypeViability(const AbstractMetaFunctionCPtr &func,
                               const AbstractMetaType &type, int argIdx)
{
    if (type.isVoid()
        || !type.typeEntry()->isPrimitive()
        || type.indirections() == 0
        || (type.indirections() == 1 && type.typeUsagePattern() == AbstractMetaType::NativePointerAsArrayPattern)
        || type.isCString()
        || func->argumentRemoved(argIdx)
        || !func->typeReplaced(argIdx).isEmpty()
        || !func->conversionRule(TypeSystem::All, argIdx).isEmpty()
        || func->hasInjectedCode())
        return;
    QString message;
    QTextStream str(&message);
    str << func->sourceLocation()
        << "There's no user provided way (conversion rule, argument"
           " removal, custom code, etc) to handle the primitive ";
    if (argIdx == 0)
        str << "return type '" << type.cppSignature() << '\'';
    else
        str << "type '" << type.cppSignature() << "' of argument " << argIdx;
    str << " in function '";
    if (func->ownerClass())
        str << func->ownerClass()->qualifiedCppName() << "::";
    str << func->signature() << "'.";
    qCWarning(lcShiboken).noquote().nospace() << message;
}

static void checkTypeViability(const AbstractMetaFunctionCPtr &func)
{
    if (func->isUserAdded())
        return;
    checkTypeViability(func, func->type(), 0);
    for (int i = 0; i < func->arguments().count(); ++i)
        checkTypeViability(func, func->arguments().at(i).type(), i + 1);
}

void CppGenerator::writeTypeCheck(TextStream &s, const OverloadData *overloadData,
                                  QString argumentName) const
{
    QSet<const TypeEntry *> numericTypes;
    const OverloadDataList &overloads = overloadData->previousOverloadData()->nextOverloadData();
    for (OverloadData *od : overloads) {
        for (const auto &func : od->overloads()) {
            checkTypeViability(func);
            const AbstractMetaType &argType = od->argument(func)->type();
            if (!argType.isPrimitive())
                continue;
            if (ShibokenGenerator::isNumber(argType.typeEntry()))
                numericTypes << argType.typeEntry();
        }
    }

    // This condition trusts that the OverloadData object will arrange for
    // PyInt type to come after the more precise numeric types (e.g. float and bool)
    AbstractMetaType argType = overloadData->argType();
    if (auto viewOn = argType.viewOn())
        argType = *viewOn;
    bool numberType = numericTypes.count() == 1 || ShibokenGenerator::isPyInt(argType);
    QString customType = (overloadData->hasArgumentTypeReplace() ? overloadData->argumentTypeReplaced() : QString());
    bool rejectNull = shouldRejectNullPointerArgument(overloadData->referenceFunction(), overloadData->argPos());
    writeTypeCheck(s, argType, argumentName, numberType, customType, rejectNull);
}

void CppGenerator::writeArgumentConversion(TextStream &s,
                                           const AbstractMetaType &argType,
                                           const QString &argName, const QString &pyArgName,
                                           const AbstractMetaClass *context,
                                           const QString &defaultValue,
                                           bool castArgumentAsUnused) const
{
    if (argType.typeEntry()->isCustom() || argType.typeEntry()->isVarargs())
        return;
    if (argType.isWrapperType())
        writeInvalidPyObjectCheck(s, pyArgName);
    writePythonToCppTypeConversion(s, argType, pyArgName, argName, context, defaultValue);
    if (castArgumentAsUnused)
        writeUnusedVariableCast(s, argName);
}

static const QStringList &knownPythonTypes()
{
    static const QStringList result = {
        pyBoolT(), pyIntT(), pyFloatT(), pyLongT(),
        QLatin1String("PyObject"), QLatin1String("PyString"),
        QLatin1String("PyBuffer"), QLatin1String("PySequence"),
        QLatin1String("PyTuple"), QLatin1String("PyList"),
        QLatin1String("PyDict"), QLatin1String("PyObject*"),
        QLatin1String("PyObject *"), QLatin1String("PyTupleObject*")};
    return result;
}

std::optional<AbstractMetaType>
    CppGenerator::getArgumentType(const AbstractMetaFunctionCPtr &func, int argPos) const
{
    if (argPos < 0 || argPos > func->arguments().size()) {
        qCWarning(lcShiboken).noquote().nospace()
            << QStringLiteral("Argument index for function '%1' out of range.").arg(func->signature());
        return {};
    }

    QString typeReplaced = func->typeReplaced(argPos);
    if (typeReplaced.isEmpty()) {
        if (argPos == 0)
            return func->type();
        auto argType = func->arguments().at(argPos - 1).type();
        return argType.viewOn() ? *argType.viewOn() : argType;
    }

    auto argType = buildAbstractMetaTypeFromString(typeReplaced);
    if (!argType.has_value() && !knownPythonTypes().contains(typeReplaced)) {
        qCWarning(lcShiboken, "%s",
                  qPrintable(msgUnknownTypeInArgumentTypeReplacement(typeReplaced, func.data())));
    }
    return argType;
}

static inline QString arrayHandleType(const AbstractMetaTypeList &nestedArrayTypes)
{
    switch (nestedArrayTypes.size()) {
    case 1:
        return QStringLiteral("Shiboken::Conversions::ArrayHandle<")
            + nestedArrayTypes.constLast().minimalSignature()
            + QLatin1Char('>');
    case 2:
        return QStringLiteral("Shiboken::Conversions::Array2Handle<")
            + nestedArrayTypes.constLast().minimalSignature()
            + QStringLiteral(", ")
            + QString::number(nestedArrayTypes.constFirst().arrayElementCount())
            + QLatin1Char('>');
    }
    return QString();
}

void CppGenerator::writePythonToCppTypeConversion(TextStream &s,
                                                  const AbstractMetaType &type,
                                                  const QString &pyIn,
                                                  const QString &cppOut,
                                                  const AbstractMetaClass * /* context */,
                                                  const QString &defaultValue) const
{
    const TypeEntry *typeEntry = type.typeEntry();
    if (typeEntry->isCustom() || typeEntry->isVarargs())
        return;

    QString cppOutAux = cppOut + QLatin1String("_local");

    const bool isEnum = typeEntry->isEnum();
    const bool isFlags = typeEntry->isFlags();
    bool treatAsPointer = isValueTypeWithCopyConstructorOnly(type);
    bool isPointerOrObjectType = (type.isObjectType() || type.isPointer())
        && !type.isUserPrimitive() && !type.isExtendedCppPrimitive()
        && !isEnum && !isFlags;
    const bool isNotContainerEnumOrFlags = !typeEntry->isContainer()
                                           && !isEnum && !isFlags;
    bool mayHaveImplicitConversion = type.referenceType() == LValueReference
                                     && !type.isUserPrimitive()
                                     && !type.isExtendedCppPrimitive()
                                     && isNotContainerEnumOrFlags
                                     && !(treatAsPointer || isPointerOrObjectType);

    const AbstractMetaTypeList &nestedArrayTypes = type.nestedArrayTypes();
    const bool isCppPrimitiveArray = !nestedArrayTypes.isEmpty()
        && nestedArrayTypes.constLast().isCppPrimitive();
    QString typeName = isCppPrimitiveArray
        ? arrayHandleType(nestedArrayTypes)
        : getFullTypeNameWithoutModifiers(type);

    bool isProtectedEnum = false;

    if (mayHaveImplicitConversion) {
        s << typeName << ' ' << cppOutAux;
        writeMinimalConstructorExpression(s, type, defaultValue);
        s << ";\n";
    } else if (avoidProtectedHack() && isEnum) {
        auto metaEnum = findAbstractMetaEnum(type);
        if (metaEnum.has_value() && metaEnum->isProtected()) {
            typeName = QLatin1String("long");
            isProtectedEnum = true;
        }
    }

    s << typeName;
    if (isCppPrimitiveArray) {
        s << ' ' << cppOut;
    } else if (treatAsPointer || isPointerOrObjectType) {
        s << " *" << cppOut;
        if (!defaultValue.isEmpty()) {
            const bool needsConstCast = !isNullPtr(defaultValue)
                && type.indirections() == 1 && type.isConstant()
                && type.referenceType() == NoReference;
            s << " = ";
            if (needsConstCast)
                s << "const_cast<" << typeName << " *>(";
            s << defaultValue;
            if (needsConstCast)
                s << ')';
        }
    } else if (type.referenceType() == LValueReference && !typeEntry->isPrimitive() && isNotContainerEnumOrFlags) {
        s << " *" << cppOut << " = &" << cppOutAux;
    } else {
        s << ' ' << cppOut;
        if (isProtectedEnum && avoidProtectedHack()) {
            s << " = ";
            if (defaultValue.isEmpty())
                s << "0";
            else
                s << "(long)" << defaultValue;
        } else if (type.isUserPrimitive() || isEnum || isFlags) {
            writeMinimalConstructorExpression(s, typeEntry, defaultValue);
        } else if (!type.isContainer() && !type.isSmartPointer()) {
            writeMinimalConstructorExpression(s, type, defaultValue);
        }
    }
    s << ";\n";

    QString pythonToCppFunc = pythonToCppConverterForArgumentName(pyIn);

    if (!defaultValue.isEmpty())
        s << "if (" << pythonToCppFunc << ") ";

    QString pythonToCppCall = QString::fromLatin1("%1(%2, &%3)").arg(pythonToCppFunc, pyIn, cppOut);
    if (!mayHaveImplicitConversion) {
        s << pythonToCppCall << ";\n";
        return;
    }

    if (!defaultValue.isEmpty())
        s << "{\n";

    s << "if (Shiboken::Conversions::isImplicitConversion(reinterpret_cast<SbkObjectType *>("
        << cpythonTypeNameExt(type) << "), " << pythonToCppFunc << "))\n";
    {
        Indentation indent(s);
        s << pythonToCppFunc << '(' << pyIn << ", &" << cppOutAux << ");\n";
    }
    s << "else\n";
    {
        Indentation indent(s);
        s << pythonToCppCall << ";\n";
    }

    if (!defaultValue.isEmpty())
        s << '}';
    s << '\n';
}

static void addConversionRuleCodeSnippet(CodeSnipList &snippetList, QString &rule,
                                         TypeSystem::Language /* conversionLanguage */,
                                         TypeSystem::Language snippetLanguage,
                                         const QString &outputName = QString(),
                                         const QString &inputName = QString())
{
    if (rule.isEmpty())
        return;
    if (snippetLanguage == TypeSystem::TargetLangCode) {
        rule.replace(QLatin1String("%in"), inputName);
        rule.replace(QLatin1String("%out"), outputName + QLatin1String("_out"));
    } else {
        rule.replace(QLatin1String("%out"), outputName);
    }
    CodeSnip snip(snippetLanguage);
    snip.position = (snippetLanguage == TypeSystem::NativeCode) ? TypeSystem::CodeSnipPositionAny : TypeSystem::CodeSnipPositionBeginning;
    snip.addCode(rule);
    snippetList << snip;
}

void CppGenerator::writeConversionRule(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                       TypeSystem::Language language) const
{

    CodeSnipList snippets;
    const AbstractMetaArgumentList &arguments = func->arguments();
    for (const AbstractMetaArgument &arg : arguments) {
        QString rule = func->conversionRule(language, arg.argumentIndex() + 1);
        addConversionRuleCodeSnippet(snippets, rule, language, TypeSystem::TargetLangCode,
                                     arg.name(), arg.name());
    }
    writeCodeSnips(s, snippets, TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode, func);
}

void CppGenerator::writeConversionRule(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                       TypeSystem::Language language, const QString &outputVar) const
{
    CodeSnipList snippets;
    QString rule = func->conversionRule(language, 0);
    addConversionRuleCodeSnippet(snippets, rule, language, language, outputVar);
    writeCodeSnips(s, snippets, TypeSystem::CodeSnipPositionAny, language, func);
}

void CppGenerator::writeNoneReturn(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                   bool thereIsReturnValue)
{
    if (thereIsReturnValue && (func->isVoid() || func->argumentRemoved(0))
        && !func->injectedCodeHasReturnValueAttribution()) {
        s << PYTHON_RETURN_VAR << " = Py_None;\n"
            << "Py_INCREF(Py_None);\n";
    }
}

void CppGenerator::writeOverloadedFunctionDecisor(TextStream &s, const OverloadData &overloadData) const
{
    s << "// Overloaded function decisor\n";
    const auto rfunc = overloadData.referenceFunction();
    const AbstractMetaFunctionCList &functionOverloads = overloadData.overloadsWithoutRepetition();
    for (int i = 0; i < functionOverloads.count(); i++) {
        const auto func = functionOverloads.at(i);
        s << "// " << i << ": ";
        if (func->isStatic())
            s << "static ";
        if (const auto *decl = func->declaringClass())
            s << decl->name() << "::";
        s << func->minimalSignature() << '\n';
    }
    writeOverloadedFunctionDecisorEngine(s, &overloadData);
    s << '\n';

    // Ensure that the direct overload that called this reverse
    // is called.
    if (rfunc->isOperatorOverload() && !rfunc->isCallOperator()) {
        s << "if (isReverse && overloadId == -1) {\n";
        {
            Indentation indent(s);
            s << "PyErr_SetString(PyExc_NotImplementedError, \"reverse operator not implemented.\");\n"
                << "return {};\n";
        }
        s << "}\n\n";
    }

    s << "// Function signature not found.\n"
        << "if (overloadId == -1) goto "
        << cpythonFunctionName(overloadData.referenceFunction()) << "_TypeError;\n\n";
}

void CppGenerator::writeOverloadedFunctionDecisorEngine(TextStream &s,
                                                        const OverloadData *parentOverloadData) const
{
    bool hasDefaultCall = parentOverloadData->nextArgumentHasDefaultValue();
    auto referenceFunction = parentOverloadData->referenceFunction();

    // If the next argument has not an argument with a default value, it is still possible
    // that one of the overloads for the current overload data has its final occurrence here.
    // If found, the final occurrence of a method is attributed to the referenceFunction
    // variable to be used further on this method on the conditional that identifies default
    // method calls.
    if (!hasDefaultCall) {
        for (const auto &func : parentOverloadData->overloads()) {
            if (parentOverloadData->isFinalOccurrence(func)) {
                referenceFunction = func;
                hasDefaultCall = true;
                break;
            }
        }
    }

    int maxArgs = parentOverloadData->maxArgs();
    // Python constructors always receive multiple arguments.
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(*parentOverloadData);

    // Functions without arguments are identified right away.
    if (maxArgs == 0) {
        s << "overloadId = " << parentOverloadData->headOverloadData()->overloads().indexOf(referenceFunction)
            << "; // " << referenceFunction->minimalSignature() << '\n';
        return;

    }
    // To decide if a method call is possible at this point the current overload
    // data object cannot be the head, since it is just an entry point, or a root,
    // for the tree of arguments and it does not represent a valid method call.
    if (!parentOverloadData->isHeadOverloadData()) {
        bool isLastArgument = parentOverloadData->nextOverloadData().isEmpty();
        bool signatureFound = parentOverloadData->overloads().size() == 1;

        // The current overload data describes the last argument of a signature,
        // so the method can be identified right now.
        if (isLastArgument || (signatureFound && !hasDefaultCall)) {
            const auto func = parentOverloadData->referenceFunction();
            s << "overloadId = " << parentOverloadData->headOverloadData()->overloads().indexOf(func)
                << "; // " << func->minimalSignature() << '\n';
            return;
        }
    }

    bool isFirst = true;

    // If the next argument has a default value the decisor can perform a method call;
    // it just need to check if the number of arguments received from Python are equal
    // to the number of parameters preceding the argument with the default value.
    const OverloadDataList &overloads = parentOverloadData->nextOverloadData();
    if (hasDefaultCall) {
        isFirst = false;
        int numArgs = parentOverloadData->argPos() + 1;
        s << "if (numArgs == " << numArgs << ") {\n";
        {
            Indentation indent(s);
            auto func = referenceFunction;
            for (OverloadData *overloadData : overloads) {
                const auto defValFunc = overloadData->getFunctionWithDefaultValue();
                if (!defValFunc.isNull()) {
                    func = defValFunc;
                    break;
                }
            }
            s << "overloadId = " << parentOverloadData->headOverloadData()->overloads().indexOf(func)
                << "; // " << func->minimalSignature() << '\n';
        }
        s << '}';
    }

    for (OverloadData *overloadData : overloads) {
        bool signatureFound = overloadData->overloads().size() == 1
                                && !overloadData->getFunctionWithDefaultValue()
                                && !overloadData->findNextArgWithDefault();

        const auto refFunc = overloadData->referenceFunction();

        QStringList typeChecks;

        QString pyArgName = (usePyArgs && maxArgs > 1)
            ? pythonArgsAt(overloadData->argPos())
            : QLatin1String(PYTHON_ARG);
        OverloadData *od = overloadData;
        int startArg = od->argPos();
        int sequenceArgCount = 0;
        while (od && !od->argType().isVarargs()) {
            bool typeReplacedByPyObject = od->argumentTypeReplaced() == QLatin1String("PyObject");
            if (!typeReplacedByPyObject) {
                if (usePyArgs)
                    pyArgName = pythonArgsAt(od->argPos());
                StringStream tck(TextStream::Language::Cpp);
                auto func = od->referenceFunction();

                if (func->isConstructor() && func->arguments().count() == 1) {
                    const AbstractMetaClass *ownerClass = func->ownerClass();
                    const ComplexTypeEntry *baseContainerType = ownerClass->typeEntry()->baseContainerType();
                    if (baseContainerType && baseContainerType == func->arguments().constFirst().type().typeEntry()
                        && ownerClass->isCopyable()) {
                        tck << '!' << cpythonCheckFunction(ownerClass->typeEntry()) << pyArgName << ")\n";
                        Indentation indent(s);
                        tck << "&& ";
                    }
                }
                writeTypeCheck(tck, od, pyArgName);
                typeChecks << tck.toString();
            }

            sequenceArgCount++;

            if (od->nextOverloadData().isEmpty()
                || od->nextArgumentHasDefaultValue()
                || od->nextOverloadData().size() != 1
                || od->overloads().size() != od->nextOverloadData().constFirst()->overloads().size()) {
                overloadData = od;
                od = nullptr;
            } else {
                od = od->nextOverloadData().constFirst();
            }
        }

        if (usePyArgs && signatureFound) {
            AbstractMetaArgumentList args = refFunc->arguments();
            const bool isVarargs = args.size() > 1 && args.constLast().type().isVarargs();
            int numArgs = args.size() - OverloadData::numberOfRemovedArguments(refFunc);
            if (isVarargs)
                --numArgs;
            typeChecks.prepend(QString::fromLatin1("numArgs %1 %2").arg(isVarargs ? QLatin1String(">=") : QLatin1String("==")).arg(numArgs));
        } else if (sequenceArgCount > 1) {
            typeChecks.prepend(QString::fromLatin1("numArgs >= %1").arg(startArg + sequenceArgCount));
        } else if (refFunc->isOperatorOverload() && !refFunc->isCallOperator()) {
            QString check;
            if (!refFunc->isReverseOperator())
                check.append(QLatin1Char('!'));
            check.append(QLatin1String("isReverse"));
            typeChecks.prepend(check);
        }

        if (isFirst) {
            isFirst = false;
        } else {
            s << " else ";
        }
        s << "if (";
        if (typeChecks.isEmpty()) {
            s << "true";
        } else {
            Indentation indent(s);
            s << typeChecks.join(QLatin1String("\n&& "));
        }
        s << ") {\n";
        {
            Indentation indent(s);
            writeOverloadedFunctionDecisorEngine(s, overloadData);
        }
        s << "}";
    }
    s << '\n';
}

void CppGenerator::writeFunctionCalls(TextStream &s, const OverloadData &overloadData,
                                      const GeneratorContext &context) const
{
    const AbstractMetaFunctionCList &overloads = overloadData.overloadsWithoutRepetition();
    s << "// Call function/method\n"
        << (overloads.count() > 1 ? "switch (overloadId) " : "") << "{\n";
    {
        Indentation indent(s);
        if (overloads.count() == 1) {
            writeSingleFunctionCall(s, overloadData, overloads.constFirst(), context);
        } else {
            for (int i = 0; i < overloads.count(); i++) {
                const auto func = overloads.at(i);
                s << "case " << i << ": // " << func->signature() << "\n{\n";
                {
                    Indentation indent(s);
                    writeSingleFunctionCall(s, overloadData, func, context);
                    if (func->attributes().testFlag(AbstractMetaAttributes::Deprecated)) {
                        s << "PyErr_WarnEx(PyExc_DeprecationWarning, \"";
                        if (auto cls = context.metaClass())
                            s << cls->name() << '.';
                        s << func->signature() << " is deprecated\", 1);\n";
                    }
                    s << "break;\n";
                }
                s << "}\n";
            }
        }
    }
    s << "}\n";
}

void CppGenerator::writeSingleFunctionCall(TextStream &s,
                                           const OverloadData &overloadData,
                                           const AbstractMetaFunctionCPtr &func,
                                           const GeneratorContext &context) const
{
    if (func->isDeprecated()) {
        s << "Shiboken::warning(PyExc_DeprecationWarning, 1, \"Function: '"
                    << func->signature().replace(QLatin1String("::"), QLatin1String("."))
                    << "' is marked as deprecated, please check the documentation for more information.\");\n";
    }

    if (func->functionType() == AbstractMetaFunction::EmptyFunction) {
        s << "PyErr_Format(PyExc_TypeError, \"%s is a private method.\", \""
          << func->signature().replace(QLatin1String("::"), QLatin1String("."))
          << "\");\n"
          << returnStatement(m_currentErrorCode) << '\n';
        return;
    }

    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(overloadData);

    // Handle named arguments.
    writeNamedArgumentResolution(s, func, usePyArgs, overloadData);

    bool injectCodeCallsFunc = injectedCodeCallsCppFunction(context, func);
    bool mayHaveUnunsedArguments = !func->isUserAdded() && func->hasInjectedCode() && injectCodeCallsFunc;
    int removedArgs = 0;
    for (int argIdx = 0; argIdx < func->arguments().count(); ++argIdx) {
        bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode, argIdx + 1).isEmpty();
        const AbstractMetaArgument &arg = func->arguments().at(argIdx);
        if (func->argumentRemoved(argIdx + 1)) {
            if (!arg.defaultValueExpression().isEmpty()) {
                const QString cppArgRemoved = QLatin1String(CPP_ARG_REMOVED)
                    + QString::number(argIdx);
                s << getFullTypeName(arg.type()) << ' ' << cppArgRemoved;
                s << " = " << guessScopeForDefaultValue(func, arg) << ";\n";
                writeUnusedVariableCast(s, cppArgRemoved);
            } else if (!injectCodeCallsFunc && !func->isUserAdded() && !hasConversionRule) {
                // When an argument is removed from a method signature and no other means of calling
                // the method are provided (as with code injection) the generator must abort.
                qFatal("No way to call '%s::%s' with the modifications described in the type system.",
                       qPrintable(func->ownerClass()->name()), qPrintable(func->signature()));
            }
            removedArgs++;
            continue;
        }
        if (hasConversionRule)
            continue;
        auto argType = getArgumentType(func, argIdx + 1);
        if (!argType.has_value() || (mayHaveUnunsedArguments && !func->injectedCodeUsesArgument(argIdx)))
            continue;
        int argPos = argIdx - removedArgs;
        QString argName = QLatin1String(CPP_ARG) + QString::number(argPos);
        QString pyArgName = usePyArgs ? pythonArgsAt(argPos) : QLatin1String(PYTHON_ARG);
        QString defaultValue = guessScopeForDefaultValue(func, arg);
        writeArgumentConversion(s, argType.value(), argName, pyArgName,
                                func->implementingClass(), defaultValue,
                                func->isUserAdded());
    }

    s << '\n';

    int numRemovedArgs = OverloadData::numberOfRemovedArguments(func);

    s << "if (!PyErr_Occurred()) {\n" << indent;
    writeMethodCall(s, func, context, func->arguments().size() - numRemovedArgs);
    if (!func->isConstructor())
        writeNoneReturn(s, func, overloadData.hasNonVoidReturnType());
    s << outdent << "}\n";
}

QString CppGenerator::cppToPythonFunctionName(const QString &sourceTypeName, QString targetTypeName)
{
    if (targetTypeName.isEmpty())
        targetTypeName = sourceTypeName;
    return sourceTypeName + QLatin1String("_CppToPython_") + targetTypeName;
}

QString CppGenerator::pythonToCppFunctionName(const QString &sourceTypeName, const QString &targetTypeName)
{
    return sourceTypeName + QLatin1String("_PythonToCpp_") + targetTypeName;
}
QString CppGenerator::pythonToCppFunctionName(const AbstractMetaType &sourceType, const AbstractMetaType &targetType)
{
    return pythonToCppFunctionName(fixedCppTypeName(sourceType), fixedCppTypeName(targetType));
}
QString CppGenerator::pythonToCppFunctionName(const CustomConversion::TargetToNativeConversion *toNative,
                                              const TypeEntry *targetType)
{
    return pythonToCppFunctionName(fixedCppTypeName(toNative), fixedCppTypeName(targetType));
}

QString CppGenerator::convertibleToCppFunctionName(const QString &sourceTypeName, const QString &targetTypeName)
{
    return QLatin1String("is_") + sourceTypeName + QLatin1String("_PythonToCpp_")
            + targetTypeName + QLatin1String("_Convertible");
}
QString CppGenerator::convertibleToCppFunctionName(const AbstractMetaType &sourceType, const AbstractMetaType &targetType)
{
    return convertibleToCppFunctionName(fixedCppTypeName(sourceType), fixedCppTypeName(targetType));
}
QString CppGenerator::convertibleToCppFunctionName(const CustomConversion::TargetToNativeConversion *toNative,
                                                   const TypeEntry *targetType)
{
    return convertibleToCppFunctionName(fixedCppTypeName(toNative), fixedCppTypeName(targetType));
}

void CppGenerator::writeCppToPythonFunction(TextStream &s, const QString &code, const QString &sourceTypeName,
                                            QString targetTypeName) const
{

    QString prettyCode = code;
    processCodeSnip(prettyCode);

    s << "static PyObject *" << cppToPythonFunctionName(sourceTypeName, targetTypeName)
        << "(const void *cppIn) {\n" << indent << prettyCode
        << ensureEndl << outdent << "}\n";
}

static void replaceCppToPythonVariables(QString &code, const QString &typeName)
{
    const QString line = QLatin1String("auto &cppInRef = *reinterpret_cast<")
        + typeName + QLatin1String(" *>(const_cast<void *>(cppIn));");
    CodeSnipAbstract::prependCode(&code, line);
    code.replace(QLatin1String("%INTYPE"), typeName);
    code.replace(QLatin1String("%OUTTYPE"), QLatin1String("PyObject *"));
    code.replace(QLatin1String("%in"), QLatin1String("cppInRef"));
    code.replace(QLatin1String("%out"), QLatin1String("pyOut"));
}

void CppGenerator::writeCppToPythonFunction(TextStream &s, const CustomConversion *customConversion) const
{
    QString code = customConversion->nativeToTargetConversion();
    replaceCppToPythonVariables(code, getFullTypeName(customConversion->ownerType()));
    writeCppToPythonFunction(s, code, fixedCppTypeName(customConversion->ownerType()));
}
void CppGenerator::writeCppToPythonFunction(TextStream &s, const AbstractMetaType &containerType) const
{
    const CustomConversion *customConversion = containerType.typeEntry()->customConversion();
    if (!customConversion) {
        qFatal("Can't write the C++ to Python conversion function for container type '%s' - "\
               "no conversion rule was defined for it in the type system.",
               qPrintable(containerType.typeEntry()->qualifiedCppName()));
    }
    if (!containerType.typeEntry()->isContainer()) {
        writeCppToPythonFunction(s, customConversion);
        return;
    }
    QString code = customConversion->nativeToTargetConversion();
    for (int i = 0; i < containerType.instantiations().count(); ++i) {
        const AbstractMetaType &type = containerType.instantiations().at(i);
        QString typeName = getFullTypeName(type);
        if (type.isConstant())
            typeName = QLatin1String("const ") + typeName;
        code.replace(QString::fromLatin1("%INTYPE_%1").arg(i), typeName);
    }
    replaceCppToPythonVariables(code, getFullTypeNameWithoutModifiers(containerType));
    processCodeSnip(code);
    writeCppToPythonFunction(s, code, fixedCppTypeName(containerType));
}

void CppGenerator::writePythonToCppFunction(TextStream &s, const QString &code, const QString &sourceTypeName,
                                            const QString &targetTypeName) const
{
    QString prettyCode = code;
    processCodeSnip(prettyCode);
    s << "static void " << pythonToCppFunctionName(sourceTypeName, targetTypeName)
        << "(PyObject *pyIn, void *cppOut) {\n" << indent << prettyCode
        << ensureEndl << outdent << "}\n";
}

void CppGenerator::writeIsPythonConvertibleToCppFunction(TextStream &s,
                                                         const QString &sourceTypeName,
                                                         const QString &targetTypeName,
                                                         const QString &condition,
                                                         QString pythonToCppFuncName,
                                                         bool acceptNoneAsCppNull) const
{
    if (pythonToCppFuncName.isEmpty())
        pythonToCppFuncName = pythonToCppFunctionName(sourceTypeName, targetTypeName);

    s << "static PythonToCppFunc " << convertibleToCppFunctionName(sourceTypeName, targetTypeName);
    s << "(PyObject *pyIn) {\n" << indent;
    if (acceptNoneAsCppNull) {
        s << "if (pyIn == Py_None)\n";
        Indentation indent(s);
        s << "return Shiboken::Conversions::nonePythonToCppNullPtr;\n";
    }
    s << "if (" << condition << ")\n";
    {
        Indentation indent(s);
        s << "return " << pythonToCppFuncName << ";\n";
    }
    s << "return {};\n" << outdent << "}\n";
}

void CppGenerator::writePythonToCppConversionFunctions(TextStream &s,
                                                       const AbstractMetaType &sourceType,
                                                       const AbstractMetaType &targetType,
                                                       QString typeCheck,
                                                       QString conversion,
                                                       const QString &preConversion) const
{
    QString sourcePyType = cpythonTypeNameExt(sourceType);

    // Python to C++ conversion function.
    StringStream c(TextStream::Language::Cpp);
    if (conversion.isEmpty())
        conversion = QLatin1Char('*') + cpythonWrapperCPtr(sourceType, QLatin1String("pyIn"));
    if (!preConversion.isEmpty())
        c << preConversion << '\n';
    const QString fullTypeName = targetType.isSmartPointer()
        ? targetType.cppSignature()
        : getFullTypeName(targetType.typeEntry());
    c << "*reinterpret_cast<" << fullTypeName << " *>(cppOut) = "
        << fullTypeName << '(' << conversion << ");";
    QString sourceTypeName = fixedCppTypeName(sourceType);
    QString targetTypeName = fixedCppTypeName(targetType);
    writePythonToCppFunction(s, c.toString(), sourceTypeName, targetTypeName);

    // Python to C++ convertible check function.
    if (typeCheck.isEmpty())
        typeCheck = QString::fromLatin1("PyObject_TypeCheck(pyIn, %1)").arg(sourcePyType);
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, typeCheck);
    s << '\n';
}

void CppGenerator::writePythonToCppConversionFunctions(TextStream &s,
                                                      const CustomConversion::TargetToNativeConversion *toNative,
                                                      const TypeEntry *targetType) const
{
    // Python to C++ conversion function.
    QString code = toNative->conversion();
    QString inType;
    if (toNative->sourceType())
        inType = cpythonTypeNameExt(toNative->sourceType());
    else
        inType = QString::fromLatin1("(%1_TypeF())").arg(toNative->sourceTypeName());
    code.replace(QLatin1String("%INTYPE"), inType);
    code.replace(QLatin1String("%OUTTYPE"), targetType->qualifiedCppName());
    code.replace(QLatin1String("%in"), QLatin1String("pyIn"));
    code.replace(QLatin1String("%out"),
                 QLatin1String("*reinterpret_cast<") + getFullTypeName(targetType) + QLatin1String(" *>(cppOut)"));

    QString sourceTypeName = fixedCppTypeName(toNative);
    QString targetTypeName = fixedCppTypeName(targetType);
    writePythonToCppFunction(s, code, sourceTypeName, targetTypeName);

    // Python to C++ convertible check function.
    QString typeCheck = toNative->sourceTypeCheck();
    if (typeCheck.isEmpty()) {
        QString pyTypeName = toNative->sourceTypeName();
        if (pyTypeName == QLatin1String("Py_None") || pyTypeName == QLatin1String("PyNone"))
            typeCheck = QLatin1String("%in == Py_None");
        else if (pyTypeName == QLatin1String("SbkEnumType"))
            typeCheck = QLatin1String("Shiboken::isShibokenEnum(%in)");
        else if (pyTypeName == QLatin1String("SbkObject"))
            typeCheck = QLatin1String("Shiboken::Object::checkType(%in)");
        else if (pyTypeName == QLatin1String("PyTypeObject"))
            typeCheck = QLatin1String("PyType_Check(%in)");
        else if (pyTypeName == QLatin1String("PyObject"))
            typeCheck = QLatin1String("PyObject_TypeCheck(%in, &PyBaseObject_Type)");
        // PYSIDE-795: We abuse PySequence for iterables
        else if (pyTypeName == QLatin1String("PySequence"))
            typeCheck = QLatin1String("Shiboken::String::checkIterable(%in)");
        else if (pyTypeName.startsWith(QLatin1String("Py")))
            typeCheck = pyTypeName + QLatin1String("_Check(%in)");
    }
    if (typeCheck.isEmpty()) {
        if (!toNative->sourceType() || toNative->sourceType()->isPrimitive()) {
            qFatal("User added implicit conversion for C++ type '%s' must provide either an input "\
                   "type check function or a non primitive type entry.",
                   qPrintable(targetType->qualifiedCppName()));

        }
        typeCheck = QString::fromLatin1("PyObject_TypeCheck(%in, %1)").arg(cpythonTypeNameExt(toNative->sourceType()));
    }
    typeCheck.replace(QLatin1String("%in"), QLatin1String("pyIn"));
    processCodeSnip(typeCheck);
    writeIsPythonConvertibleToCppFunction(s, sourceTypeName, targetTypeName, typeCheck);
}

void CppGenerator::writePythonToCppConversionFunctions(TextStream &s, const AbstractMetaType &containerType) const
{
    const CustomConversion *customConversion = containerType.typeEntry()->customConversion();
    if (!customConversion) {
        //qFatal
        return;
    }
    const CustomConversion::TargetToNativeConversions &toCppConversions = customConversion->targetToNativeConversions();
    if (toCppConversions.isEmpty()) {
        //qFatal
        return;
    }
    // Python to C++ conversion function.
    QString cppTypeName = getFullTypeNameWithoutModifiers(containerType);
    QString code = toCppConversions.constFirst()->conversion();
    const QString line = QLatin1String("auto &cppOutRef = *reinterpret_cast<")
        + cppTypeName + QLatin1String(" *>(cppOut);");
    CodeSnipAbstract::prependCode(&code, line);
    for (int i = 0; i < containerType.instantiations().count(); ++i) {
        const AbstractMetaType &type = containerType.instantiations().at(i);
        QString typeName = getFullTypeName(type);
        if (type.isValue() && isValueTypeWithCopyConstructorOnly(type)) {
            for (int pos = 0; ; ) {
                const QRegularExpressionMatch match = convertToCppRegEx().match(code, pos);
                if (!match.hasMatch())
                    break;
                pos = match.capturedEnd();
                const QString varName = match.captured(1);
                QString rightCode = code.mid(pos);
                rightCode.replace(varName, QLatin1Char('*') + varName);
                code.replace(pos, code.size() - pos, rightCode);
            }
            typeName.append(QLatin1String(" *"));
        }
        code.replace(QString::fromLatin1("%OUTTYPE_%1").arg(i), typeName);
    }
    code.replace(QLatin1String("%OUTTYPE"), cppTypeName);
    code.replace(QLatin1String("%in"), QLatin1String("pyIn"));
    code.replace(QLatin1String("%out"), QLatin1String("cppOutRef"));
    QString typeName = fixedCppTypeName(containerType);
    writePythonToCppFunction(s, code, typeName, typeName);

    // Python to C++ convertible check function.
    QString typeCheck = cpythonCheckFunction(containerType);
    if (typeCheck.isEmpty())
        typeCheck = QLatin1String("false");
    else
        typeCheck = typeCheck + QLatin1String("pyIn)");
    writeIsPythonConvertibleToCppFunction(s, typeName, typeName, typeCheck);
    s << '\n';
}

void CppGenerator::writeAddPythonToCppConversion(TextStream &s, const QString &converterVar,
                                                 const QString &pythonToCppFunc,
                                                 const QString &isConvertibleFunc) const
{
    s << "Shiboken::Conversions::addPythonToCppValueConversion(" << converterVar << ',' << '\n';
    {
        Indentation indent(s);
        s << pythonToCppFunc << ',' << '\n' << isConvertibleFunc;
    }
    s << ");\n";
}

void CppGenerator::writeNamedArgumentResolution(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                                bool usePyArgs, const OverloadData &overloadData) const
{
    const AbstractMetaArgumentList &args = OverloadData::getArgumentsWithDefaultValues(func);
    if (args.isEmpty()) {
        if (overloadData.hasArgumentWithDefaultValue()) {
            s << "if (kwds) {\n";
            {
                Indentation indent(s);
                s << "errInfo = kwds;\n"
                    << "Py_INCREF(errInfo);\n"
                    << "goto " << cpythonFunctionName(func) << "_TypeError;\n";
            }
            s << "}\n";
        }
        return;
    }

    s << "if (kwds) {\n";
    {
        Indentation indent(s);
        s << "PyObject *value{};\n"
            << "PyObject *kwds_dup = PyDict_Copy(kwds);\n";
        for (const AbstractMetaArgument &arg : args) {
            const int pyArgIndex = arg.argumentIndex()
                - OverloadData::numberOfRemovedArguments(func, arg.argumentIndex());
            QString pyArgName = usePyArgs ? pythonArgsAt(pyArgIndex) : QLatin1String(PYTHON_ARG);
            QString pyKeyName = QLatin1String("key_") + arg.name();
            s << "static PyObject *const " << pyKeyName
                << " = Shiboken::String::createStaticString(\"" << arg.name() << "\");\n"
                << "if (PyDict_Contains(kwds, " << pyKeyName << ")) {\n";
            {
                Indentation indent(s);
                s << "value = PyDict_GetItem(kwds, " << pyKeyName << ");\n"
                    << "if (value && " << pyArgName << ") {\n";
                {
                    Indentation indent(s);
                    s << "errInfo = " << pyKeyName << ";\n"
                        << "Py_INCREF(errInfo);\n"
                        << "goto " << cpythonFunctionName(func) << "_TypeError;\n";
                }
                s << "}\nif (value) {\n";
                {
                    Indentation indent(s);
                    s << pyArgName << " = value;\nif (!";
                    writeTypeCheck(s, arg.type(), pyArgName, isNumber(arg.type().typeEntry()),
                                   func->typeReplaced(arg.argumentIndex() + 1));
                    s << ")\n";
                    {
                        Indentation indent(s);
                        s << "goto " << cpythonFunctionName(func) << "_TypeError;\n";
                    }
                }
                s << "}\nPyDict_DelItem(kwds_dup, " << pyKeyName << ");\n";
            }
            s << "}\n";
        }
        // PYSIDE-1305: Handle keyword args correctly.
        // Normal functions handle their parameters immediately.
        // For constructors that are QObject, we need to delay that
        // until extra keyword signals and properties are handled.
        s << "if (PyDict_Size(kwds_dup) > 0) {\n";
        {
            Indentation indent(s);
            s << "errInfo = kwds_dup;\n";
            if (!(func->isConstructor() && func->ownerClass()->isQObject()))
                s << "goto " << cpythonFunctionName(func) << "_TypeError;\n";
            else
                s << "// fall through to handle extra keyword signals and properties\n";
        }
        s << "} else {\n";
        {
            Indentation indent(s);
            s << "Py_DECREF(kwds_dup);\n";
        }
        s << "}\n";
    }
    s << "}\n";
}

QString CppGenerator::argumentNameFromIndex(const AbstractMetaFunctionCPtr &func, int argIndex,
                                            const AbstractMetaClass **wrappedClass,
                                            QString *errorMessage) const
{
    if (errorMessage != nullptr)
        errorMessage->clear();
    *wrappedClass = nullptr;
    QString pyArgName;
    if (argIndex == -1) {
        pyArgName = QLatin1String("self");
        *wrappedClass = func->implementingClass();
    } else if (argIndex == 0) {
        const auto funcType = func->type();
        AbstractMetaType returnType = getTypeWithoutContainer(funcType);
        if (!returnType.isVoid()) {
            pyArgName = QLatin1String(PYTHON_RETURN_VAR);
            *wrappedClass = AbstractMetaClass::findClass(classes(), returnType.typeEntry());
            if (*wrappedClass == nullptr && errorMessage != nullptr)
                *errorMessage = msgClassNotFound(returnType.typeEntry());
        } else {
            if (errorMessage != nullptr) {
                QTextStream str(errorMessage);
                str << "Invalid Argument index (0, return value) on function modification: "
                    <<  funcType.name() << ' ';
                if (const AbstractMetaClass *declaringClass = func->declaringClass())
                   str << declaringClass->name() << "::";
                 str << func->name() << "()";
            }
        }
    } else {
        int realIndex = argIndex - 1 - OverloadData::numberOfRemovedArguments(func, argIndex - 1);
        AbstractMetaType argType = getTypeWithoutContainer(func->arguments().at(realIndex).type());
        *wrappedClass = AbstractMetaClass::findClass(classes(), argType.typeEntry());
        if (*wrappedClass == nullptr && errorMessage != nullptr)
            *errorMessage = msgClassNotFound(argType.typeEntry());
        if (argIndex == 1
            && !func->isConstructor()
            && OverloadData::isSingleArgument(getFunctionGroups(func->implementingClass())[func->name()]))
            pyArgName = QLatin1String(PYTHON_ARG);
        else
            pyArgName = pythonArgsAt(argIndex - 1);
    }
    return pyArgName;
}

const char defaultExceptionHandling[] = R"(} catch (const std::exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
} catch (...) {
    PyErr_SetString(PyExc_RuntimeError, "An unknown exception was caught");
}
)";

void CppGenerator::writeMethodCall(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                   const GeneratorContext &context, int maxArgs) const
{
    s << "// " << func->minimalSignature() << (func->isReverseOperator() ? " [reverse operator]": "") << '\n';
    if (func->isConstructor()) {
        const CodeSnipList &snips = func->injectedCodeSnips();
        for (const CodeSnip &cs : snips) {
            if (cs.position == TypeSystem::CodeSnipPositionEnd) {
                auto klass = func->ownerClass();
                s << "overloadId = "
                  << klass->functions().indexOf(func)
                  << ";\n";
                break;
            }
        }
    }

    if (func->isAbstract()) {
        s << "if (Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject *>(self))) {\n";
        {
            Indentation indent(s);
            s << "PyErr_SetString(PyExc_NotImplementedError, \"pure virtual method '";
            s << func->ownerClass()->name() << '.' << func->name() << "()' not implemented.\");\n";
            s << returnStatement(m_currentErrorCode) << '\n';
        }
        s << "}\n";
    }

    // Used to provide contextual information to custom code writer function.
    const AbstractMetaArgument *lastArg = nullptr;

    CodeSnipList snips;
    if (func->hasInjectedCode()) {
        snips = func->injectedCodeSnips();

        // Find the last argument available in the method call to provide
        // the injected code writer with information to avoid invalid replacements
        // on the %# variable.
        if (maxArgs > 0 && maxArgs < func->arguments().size() - OverloadData::numberOfRemovedArguments(func)) {
            int removedArgs = 0;
            for (int i = 0; i < maxArgs + removedArgs; i++) {
                lastArg = &func->arguments().at(i);
                if (func->argumentRemoved(i + 1))
                    removedArgs++;
            }
        } else if (maxArgs != 0 && !func->arguments().isEmpty()) {
            lastArg = &func->arguments().constLast();
        }

        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode, func, lastArg);
    }

    writeConversionRule(s, func, TypeSystem::NativeCode);

    if (!func->isUserAdded()) {
        QStringList userArgs;
        if (func->functionType() != AbstractMetaFunction::CopyConstructorFunction) {
            int removedArgs = 0;
            for (int i = 0; i < maxArgs + removedArgs; i++) {
                const AbstractMetaArgument &arg = func->arguments().at(i);
                bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode,
                                                               arg.argumentIndex() + 1).isEmpty();
                if (func->argumentRemoved(i + 1)) {
                    // If some argument with default value is removed from a
                    // method signature, the said value must be explicitly
                    // added to the method call.
                    removedArgs++;

                    // If have conversion rules I will use this for removed args
                    if (hasConversionRule)
                        userArgs << arg.name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX);
                    else if (!arg.defaultValueExpression().isEmpty())
                        userArgs.append(QLatin1String(CPP_ARG_REMOVED) + QString::number(i));
                } else {
                    int idx = arg.argumentIndex() - removedArgs;
                    bool deRef = isValueTypeWithCopyConstructorOnly(arg.type())
                                 || arg.type().isObjectTypeUsedAsValueType()
                                 || (arg.type().referenceType() == LValueReference
                                     && arg.type().isWrapperType() && !arg.type().isPointer());
                    if (hasConversionRule) {
                        userArgs.append(arg.name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX));
                    } else {
                        QString argName;
                        if (deRef)
                            argName += QLatin1Char('*');
                        argName += QLatin1String(CPP_ARG) + QString::number(idx);
                        userArgs.append(argName);
                    }
                }
            }

            // If any argument's default value was modified the method must be called
            // with this new value whenever the user doesn't pass an explicit value to it.
            // Also, any unmodified default value coming after the last user specified
            // argument and before the modified argument must be explicitly stated.
            QStringList otherArgs;
            bool otherArgsModified = false;
            bool argsClear = true;
            for (int i = func->arguments().size() - 1; i >= maxArgs + removedArgs; i--) {
                const AbstractMetaArgument &arg = func->arguments().at(i);
                const bool defValModified = arg.hasModifiedDefaultValueExpression();
                bool hasConversionRule = !func->conversionRule(TypeSystem::NativeCode,
                                                               arg.argumentIndex() + 1).isEmpty();
                if (argsClear && !defValModified && !hasConversionRule)
                    continue;
                argsClear = false;
                otherArgsModified |= defValModified || hasConversionRule || func->argumentRemoved(i + 1);
                if (hasConversionRule)
                    otherArgs.prepend(arg.name() + QLatin1String(CONV_RULE_OUT_VAR_SUFFIX));
                else
                    otherArgs.prepend(QLatin1String(CPP_ARG_REMOVED) + QString::number(i));
            }
            if (otherArgsModified)
                userArgs << otherArgs;
        }

        bool isCtor = false;
        StringStream mc(TextStream::Language::Cpp);

        StringStream uva(TextStream::Language::Cpp);
        if (func->isOperatorOverload() && !func->isCallOperator()) {
            QString firstArg(QLatin1Char('('));
            if (!func->isPointerOperator()) // no de-reference operator
                firstArg += QLatin1Char('*');
            firstArg += QLatin1String(CPP_SELF_VAR);
            firstArg += QLatin1Char(')');
            QString secondArg = QLatin1String(CPP_ARG0);
            if (!func->isUnaryOperator()
                && func->arguments().constFirst().type().shouldDereferencePointer()) {
                secondArg.prepend(QLatin1String("(*"));
                secondArg.append(QLatin1Char(')'));
            }

            if (func->isUnaryOperator())
                std::swap(firstArg, secondArg);

            QString op = func->originalName();
            op.remove(0, int(std::strlen("operator")));

            if (func->isBinaryOperator()) {
                if (func->isReverseOperator())
                    std::swap(firstArg, secondArg);

                if (((op == QLatin1String("++")) || (op == QLatin1String("--"))) && !func->isReverseOperator())  {
                    s  << "\nfor (int i = 0; i < " << secondArg
                        << "; ++i, " << firstArg << op << ");\n";
                    mc << firstArg;
                } else {
                    mc << firstArg << ' ' << op << ' ' << secondArg;
                }
            } else {
                mc << op << ' ' << secondArg;
            }
        } else if (!injectedCodeCallsCppFunction(context, func)) {
            if (func->isConstructor()) {
                isCtor = true;
                const auto owner = func->ownerClass();
                Q_ASSERT(owner == context.metaClass());
                QString className = context.useWrapper()
                    ? context.wrapperName() : owner->qualifiedCppName();

                if (func->functionType() == AbstractMetaFunction::CopyConstructorFunction && maxArgs == 1) {
                    mc << "new ::" << className << "(*" << CPP_ARG0 << ')';
                } else {
                    QString ctorCall = className + QLatin1Char('(') + userArgs.join(QLatin1String(", ")) + QLatin1Char(')');
                    if (usePySideExtensions() && owner->isQObject()) {
                        s << "void *addr = PySide::nextQObjectMemoryAddr();\n";
                        uva << "if (addr) {\n";
                        {
                            Indentation indent(uva);

                            uva << "cptr = new (addr) ::" << ctorCall << ";\n"
                                << "PySide::setNextQObjectMemoryAddr(0);"
                                << '\n';
                        }
                        uva << "} else {\n";
                        {
                            Indentation indent(uva);
                            uva << "cptr = new ::" << ctorCall << ";\n";
                        }
                        uva << "}\n";
                    } else {
                        mc << "new ::" << ctorCall;
                    }
                }
            } else {
                QString methodCallClassName;
                if (context.forSmartPointer())
                    methodCallClassName = context.preciseType().cppSignature();
                else if (func->ownerClass())
                    methodCallClassName = func->ownerClass()->qualifiedCppName();

                if (func->ownerClass()) {
                    if (!avoidProtectedHack() || !func->isProtected()) {
                        if (func->isStatic()) {
                            mc << "::" << methodCallClassName << "::";
                        } else {
                            const QString selfVarCast = func->ownerClass() == func->implementingClass()
                                ? QLatin1String(CPP_SELF_VAR)
                                : QLatin1String("reinterpret_cast<") + methodCallClassName
                                  + QLatin1String(" *>(") + QLatin1String(CPP_SELF_VAR) + QLatin1Char(')');
                            if (func->isConstant()) {
                                if (avoidProtectedHack()) {
                                    auto ownerClass = func->ownerClass();
                                    mc << "const_cast<const ::";
                                    if (ownerClass->hasProtectedMembers()
                                        && !ownerClass->attributes().testFlag(AbstractMetaAttributes::FinalCppClass)) {
                                        // PYSIDE-500: Need a special wrapper cast when inherited
                                        const QString selfWrapCast = ownerClass == func->implementingClass()
                                            ? QLatin1String(CPP_SELF_VAR)
                                            : QLatin1String("reinterpret_cast<") + wrapperName(ownerClass)
                                              + QLatin1String(" *>(") + QLatin1String(CPP_SELF_VAR) + QLatin1Char(')');
                                        mc << wrapperName(ownerClass);
                                        mc << " *>(" << selfWrapCast << ")->";
                                    }
                                    else {
                                        mc << methodCallClassName;
                                        mc << " *>(" << selfVarCast << ")->";
                                    }
                                } else {
                                    mc << "const_cast<const ::" << methodCallClassName;
                                    mc <<  " *>(" << selfVarCast << ")->";
                                }
                            } else {
                                mc << selfVarCast << "->";
                            }
                        }

                        if (!func->isAbstract() && func->isVirtual())
                            mc << "::%CLASS_NAME::";

                        mc << func->originalName();
                    } else {
                        if (!func->isStatic()) {
                            const auto *owner = func->ownerClass();
                            const bool directInheritance = context.metaClass() == owner;
                            mc << (directInheritance ? "static_cast" : "reinterpret_cast")
                                << "<::" << wrapperName(owner) << " *>(" << CPP_SELF_VAR << ")->";
                        }

                        if (!func->isAbstract())
                            mc << (func->isProtected() ? wrapperName(func->ownerClass()) :
                                                         QLatin1String("::")
                                                         + methodCallClassName) << "::";
                        mc << func->originalName() << "_protected";
                    }
                } else {
                    mc << func->originalName();
                }
                mc << '(' << userArgs.join(QLatin1String(", ")) << ')';
                if (!func->isAbstract() && func->isVirtual()) {
                    if (!avoidProtectedHack() || !func->isProtected()) {
                        QString virtualCall = mc;
                        QString normalCall = virtualCall;
                        virtualCall.replace(QLatin1String("%CLASS_NAME"),
                                            methodCallClassName);
                        normalCall.remove(QLatin1String("::%CLASS_NAME::"));
                        mc.clear();
                        mc << "Shiboken::Object::hasCppWrapper(reinterpret_cast<SbkObject *>(self))\n"
                            << "    ? " << virtualCall << '\n'
                            << "    : " <<  normalCall;
                    }
                }
            }
        }

        if (!injectedCodeCallsCppFunction(context, func)) {
            const bool allowThread = func->allowThread();
            const bool generateExceptionHandling = func->generateExceptionHandling();
            if (generateExceptionHandling) {
                s << "try {\n" << indent;
                if (allowThread) {
                    s << "Shiboken::ThreadStateSaver threadSaver;\n"
                        << "threadSaver.save();\n";
                }
            } else if (allowThread) {
                s << BEGIN_ALLOW_THREADS << '\n';
            }
            if (isCtor) {
                if (uva.size() > 0)
                    s << uva.toString() << '\n';
                else
                    s << "cptr = " << mc.toString() << ";\n";
            } else if (!func->isVoid() && !func->isInplaceOperator()) {
                bool writeReturnType = true;
                if (avoidProtectedHack()) {
                    auto metaEnum = findAbstractMetaEnum(func->type());
                    if (metaEnum.has_value()) {
                        QString enumName;
                        if (metaEnum->isProtected())
                            enumName = protectedEnumSurrogateName(metaEnum.value());
                        else
                            enumName = func->type().cppSignature();
                        const QString methodCall = enumName + QLatin1Char('(')
                                                   + mc.toString() + QLatin1Char(')');
                        mc.clear();
                        mc << methodCall;
                        s << enumName;
                        writeReturnType = false;
                    }
                }
                QString methodCall = mc.toString();
                if (writeReturnType) {
                    s << func->type().cppSignature();
                    if (func->type().isObjectTypeUsedAsValueType()) {
                        s << '*';
                        methodCall = QLatin1String("new ")
                                     + func->type().typeEntry()->qualifiedCppName()
                                     + QLatin1Char('(') + mc.toString() + QLatin1Char(')');
                    }
                }
                s << " " << CPP_RETURN_VAR << " = " << methodCall << ";\n";
            } else {
                s << mc.toString() << ";\n";
            }

            if (allowThread) {
                s << (generateExceptionHandling
                                ? "threadSaver.restore();" : END_ALLOW_THREADS) << '\n';
            }

            // Convert result
            if (!func->conversionRule(TypeSystem::TargetLangCode, 0).isEmpty()) {
                writeConversionRule(s, func, TypeSystem::TargetLangCode, QLatin1String(PYTHON_RETURN_VAR));
            } else if (!isCtor && !func->isInplaceOperator() && !func->isVoid()
                && !func->injectedCodeHasReturnValueAttribution(TypeSystem::TargetLangCode)) {
                s << PYTHON_RETURN_VAR << " = ";
                if (func->type().isObjectTypeUsedAsValueType()) {
                    s << "Shiboken::Object::newObject(reinterpret_cast<SbkObjectType *>("
                        << cpythonTypeNameExt(func->type().typeEntry())
                        << "), " << CPP_RETURN_VAR << ", true, true)";
                } else {
                    writeToPythonConversion(s, func->type(), func->ownerClass(), QLatin1String(CPP_RETURN_VAR));
                }
                s << ";\n";
            }

            if (generateExceptionHandling) { // "catch" code
                s << outdent << defaultExceptionHandling;
            }
        }
    }

    if (func->hasInjectedCode() && !func->isConstructor())
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode, func, lastArg);

    bool hasReturnPolicy = false;

    // Ownership transference between C++ and Python.
    QList<ArgumentModification> ownership_mods;
    // Python object reference management.
    QList<ArgumentModification> refcount_mods;
    for (const auto &func_mod : func->modifications()) {
        for (const ArgumentModification &arg_mod : func_mod.argument_mods()) {
            if (arg_mod.ownerships().contains(TypeSystem::TargetLangCode))
                ownership_mods.append(arg_mod);
            else if (!arg_mod.referenceCounts().isEmpty())
                refcount_mods.append(arg_mod);
        }
    }

    // If there's already a setParent(return, me), don't use the return heuristic!
    if (func->argumentOwner(func->ownerClass(), -1).index == 0)
        hasReturnPolicy = true;

    if (!ownership_mods.isEmpty()) {
        s  << '\n' << "// Ownership transferences.\n";
        for (const ArgumentModification &arg_mod : qAsConst(ownership_mods)) {
            const AbstractMetaClass *wrappedClass = nullptr;
            QString errorMessage;
            QString pyArgName = argumentNameFromIndex(func, arg_mod.index(), &wrappedClass, &errorMessage);
            if (!wrappedClass) {
                QString message;
                QTextStream str(&message);
                str << "Invalid ownership modification for argument " << arg_mod.index()
                    << " (" << pyArgName << ") of ";
                if (const AbstractMetaClass *declaringClass = func->declaringClass())
                    str << declaringClass->name() << "::";
                str << func->name() << "(): " << errorMessage;
                qCWarning(lcShiboken, "%s", qPrintable(message));
                s << "#error " << message << '\n';
                break;
            }

            if (arg_mod.index() == 0 || arg_mod.owner().index == 0)
                hasReturnPolicy = true;

            // The default ownership does nothing. This is useful to avoid automatic heuristically
            // based generation of code defining parenting.
            const auto ownership =
                arg_mod.ownerships().value(TypeSystem::TargetLangCode, TypeSystem::DefaultOwnership);
            if (ownership == TypeSystem::DefaultOwnership)
                continue;

            s << "Shiboken::Object::";
            if (ownership == TypeSystem::TargetLangOwnership) {
                s << "getOwnership(" << pyArgName << ");";
            } else if (wrappedClass->hasVirtualDestructor()) {
                if (arg_mod.index() == 0)
                    s << "releaseOwnership(" << PYTHON_RETURN_VAR << ");";
                else
                    s << "releaseOwnership(" << pyArgName << ");";
            } else {
                s << "invalidate(" << pyArgName << ");";
            }
            s << '\n';
        }

    } else if (!refcount_mods.isEmpty()) {
        for (const ArgumentModification &arg_mod : qAsConst(refcount_mods)) {
            ReferenceCount refCount = arg_mod.referenceCounts().constFirst();
            if (refCount.action != ReferenceCount::Set
                && refCount.action != ReferenceCount::Remove
                && refCount.action != ReferenceCount::Add) {
                qCWarning(lcShiboken) << "\"set\", \"add\" and \"remove\" are the only values supported by Shiboken for action attribute of reference-count tag.";
                continue;
            }
            const AbstractMetaClass *wrappedClass = nullptr;

            QString pyArgName;
            if (refCount.action == ReferenceCount::Remove) {
                pyArgName = QLatin1String("Py_None");
            } else {
                QString errorMessage;
                pyArgName = argumentNameFromIndex(func, arg_mod.index(), &wrappedClass, &errorMessage);
                if (pyArgName.isEmpty()) {
                    QString message;
                    QTextStream str(&message);
                    str << "Invalid reference count modification for argument "
                        << arg_mod.index() << " of ";
                    if (const AbstractMetaClass *declaringClass = func->declaringClass())
                        str << declaringClass->name() << "::";
                    str << func->name() << "(): " << errorMessage;
                    qCWarning(lcShiboken, "%s", qPrintable(message));
                    s << "#error " << message << "\n\n";
                    break;
                }
            }

            if (refCount.action == ReferenceCount::Add || refCount.action == ReferenceCount::Set)
                s << "Shiboken::Object::keepReference(";
            else
                s << "Shiboken::Object::removeReference(";

            s << "reinterpret_cast<SbkObject *>(self), \"";
            QString varName = arg_mod.referenceCounts().constFirst().varName;
            if (varName.isEmpty())
                varName = func->minimalSignature() + QString::number(arg_mod.index());

            s << varName << "\", " << pyArgName
              << (refCount.action == ReferenceCount::Add ? ", true" : "")
              << ");\n";

            if (arg_mod.index() == 0)
                hasReturnPolicy = true;
        }
    }
    writeParentChildManagement(s, func, !hasReturnPolicy);
}

QStringList CppGenerator::getAncestorMultipleInheritance(const AbstractMetaClass *metaClass)
{
    QStringList result;
    const AbstractMetaClassList &baseClases = metaClass->typeSystemBaseClasses();
    if (!baseClases.isEmpty()) {
        for (const AbstractMetaClass *baseClass : baseClases) {
            QString offset;
            QTextStream(&offset) << "reinterpret_cast<uintptr_t>(static_cast<const "
                << baseClass->qualifiedCppName() << " *>(class_ptr)) - base";
            result.append(offset);
            offset.clear();
            QTextStream(&offset) << "reinterpret_cast<uintptr_t>(static_cast<const "
                << baseClass->qualifiedCppName() << " *>(static_cast<const "
                << metaClass->qualifiedCppName()
                << " *>(static_cast<const void *>(class_ptr)))) - base";
            result.append(offset);
        }

        for (const AbstractMetaClass *baseClass : baseClases)
            result.append(getAncestorMultipleInheritance(baseClass));
    }
    return result;
}

void CppGenerator::writeMultipleInheritanceInitializerFunction(TextStream &s, const AbstractMetaClass *metaClass)
{
    QString className = metaClass->qualifiedCppName();
    const QStringList ancestors = getAncestorMultipleInheritance(metaClass);
    s << "static int mi_offsets[] = { ";
    for (int i = 0; i < ancestors.size(); i++)
        s << "-1, ";
    s << "-1 };\n"
        << "int *\n"
        << multipleInheritanceInitializerFunctionName(metaClass) << "(const void *cptr)\n"
        << "{\n" << indent
        << "if (mi_offsets[0] == -1) {\n";
    {
        Indentation indent(s);
        s << "std::set<int> offsets;\n"
            << "const auto *class_ptr = reinterpret_cast<const " << className << " *>(cptr);\n"
            << "const auto base = reinterpret_cast<uintptr_t>(class_ptr);\n";

        for (const QString &ancestor : ancestors)
            s << "offsets.insert(int(" << ancestor << "));\n";

        s << "\noffsets.erase(0);\n\n"
            << "std::copy(offsets.cbegin(), offsets.cend(), mi_offsets);\n";
    }
    s << "}\nreturn mi_offsets;\n" << outdent << "}\n";
}

void CppGenerator::writeSpecialCastFunction(TextStream &s, const AbstractMetaClass *metaClass)
{
    QString className = metaClass->qualifiedCppName();
    s << "static void * " << cpythonSpecialCastFunctionName(metaClass)
        << "(void *obj, SbkObjectType *desiredType)\n{\n" << indent
        << "auto me = reinterpret_cast< ::" << className << " *>(obj);\n";
    bool firstClass = true;
    const AbstractMetaClassList &allAncestors = metaClass->allTypeSystemAncestors();
    for (const AbstractMetaClass *baseClass : allAncestors) {
        if (!firstClass)
            s << "else ";
        s << "if (desiredType == reinterpret_cast<SbkObjectType *>("
             << cpythonTypeNameExt(baseClass->typeEntry()) << "))\n";
        Indentation indent(s);
        s << "return static_cast< ::" << baseClass->qualifiedCppName() << " *>(me);\n";
        firstClass = false;
    }
    s << "return me;\n" << outdent << "}\n\n";
}

void CppGenerator::writePrimitiveConverterInitialization(TextStream &s, const CustomConversion *customConversion) const
{
    const TypeEntry *type = customConversion->ownerType();
    QString converter = converterObject(type);
    s << "// Register converter for type '" << type->qualifiedTargetLangName() << "'.\n"
        << converter << " = Shiboken::Conversions::createConverter(";
    if (type->targetLangApiName() == type->name())
        s << '0';
    else if (type->targetLangApiName() == QLatin1String("PyObject"))
        s << "&PyBaseObject_Type";
    else
        s << '&' << type->targetLangApiName() << "_Type";
    QString typeName = fixedCppTypeName(type);
    s << ", " << cppToPythonFunctionName(typeName, typeName) << ");\n"
        << "Shiboken::Conversions::registerConverterName(" << converter << ", \""
        << type->qualifiedCppName() << "\");\n";
    writeCustomConverterRegister(s, customConversion, converter);
}

void CppGenerator::writeEnumConverterInitialization(TextStream &s, const AbstractMetaEnum &metaEnum) const
{
    if (metaEnum.isPrivate() || metaEnum.isAnonymous())
        return;
    writeEnumConverterInitialization(s, metaEnum.typeEntry());
}

void CppGenerator::writeEnumConverterInitialization(TextStream &s, const TypeEntry *enumType) const
{
    if (!enumType)
        return;
    QString enumFlagName = enumType->isFlags() ? QLatin1String("flag") : QLatin1String("enum");
    QString enumPythonType = cpythonTypeNameExt(enumType);

    const FlagsTypeEntry *flags = nullptr;
    if (enumType->isFlags())
        flags = static_cast<const FlagsTypeEntry *>(enumType);

    s << "// Register converter for " << enumFlagName << " '" << enumType->qualifiedCppName()
        << "'.\n{\n";
    {
        Indentation indent(s);
        QString typeName = fixedCppTypeName(enumType);
        s << "SbkConverter *converter = Shiboken::Conversions::createConverter("
            << enumPythonType << ',' << '\n';
        {
            Indentation indent(s);
            s << cppToPythonFunctionName(typeName, typeName) << ");\n";
        }

        if (flags) {
            QString enumTypeName = fixedCppTypeName(flags->originator());
            QString toCpp = pythonToCppFunctionName(enumTypeName, typeName);
            QString isConv = convertibleToCppFunctionName(enumTypeName, typeName);
            writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);
        }

        QString toCpp = pythonToCppFunctionName(typeName, typeName);
        QString isConv = convertibleToCppFunctionName(typeName, typeName);
        writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);

        if (flags) {
            QString toCpp = pythonToCppFunctionName(QLatin1String("number"), typeName);
            QString isConv = convertibleToCppFunctionName(QLatin1String("number"), typeName);
            writeAddPythonToCppConversion(s, QLatin1String("converter"), toCpp, isConv);
        }

        s << "Shiboken::Enum::setTypeConverter(" << enumPythonType << ", converter);\n";

        QString signature = enumType->qualifiedCppName();
        // Replace "QFlags<Class::Option>" by "Class::Options"
        if (flags && signature.startsWith(QLatin1String("QFlags<")) && signature.endsWith(QLatin1Char('>'))) {
            signature.chop(1);
            signature.remove(0, 7);
            const int lastQualifierPos = signature.lastIndexOf(QLatin1String("::"));
            if (lastQualifierPos != -1) {
                signature.replace(lastQualifierPos + 2, signature.size() - lastQualifierPos - 2,
                                  flags->flagsName());
            } else {
                signature = flags->flagsName();
            }
        }

        while (true) {
            s << "Shiboken::Conversions::registerConverterName(converter, \""
                << signature << "\");\n";
            const int qualifierPos = signature.indexOf(QLatin1String("::"));
            if (qualifierPos != -1)
                signature.remove(0, qualifierPos + 2);
            else
                break;
        }
    }
    s << "}\n";

    if (!flags)
        writeEnumConverterInitialization(s, static_cast<const EnumTypeEntry *>(enumType)->flags());
}

void CppGenerator::writeContainerConverterInitialization(TextStream &s, const AbstractMetaType &type) const
{
    QByteArray cppSignature = QMetaObject::normalizedSignature(type.cppSignature().toUtf8());
    s << "// Register converter for type '" << cppSignature << "'.\n";
    QString converter = converterObject(type);
    s << converter << " = Shiboken::Conversions::createConverter(";
    if (type.typeEntry()->targetLangApiName() == QLatin1String("PyObject")) {
        s << "&PyBaseObject_Type";
    } else {
        QString baseName = cpythonBaseName(type.typeEntry());
        if (baseName == QLatin1String("PySequence"))
            baseName = QLatin1String("PyList");
        s << '&' << baseName << "_Type";
    }
    QString typeName = fixedCppTypeName(type);
    s << ", " << cppToPythonFunctionName(typeName, typeName) << ");\n";
    QString toCpp = pythonToCppFunctionName(typeName, typeName);
    QString isConv = convertibleToCppFunctionName(typeName, typeName);
    s << "Shiboken::Conversions::registerConverterName(" << converter << ", \"" << cppSignature << "\");\n";
    if (usePySideExtensions() && cppSignature.startsWith("const ") && cppSignature.endsWith("&")) {
        cppSignature.chop(1);
        cppSignature.remove(0, sizeof("const ") / sizeof(char) - 1);
        s << "Shiboken::Conversions::registerConverterName(" << converter << ", \"" << cppSignature << "\");\n";
    }
    writeAddPythonToCppConversion(s, converterObject(type), toCpp, isConv);
}

void CppGenerator::writeSmartPointerConverterInitialization(TextStream &s, const AbstractMetaType &type) const
{
    const QByteArray cppSignature = type.cppSignature().toUtf8();
    auto writeConversionRegister = [this, &s](const AbstractMetaType &sourceType, const QString &targetTypeName, const QString &targetConverter)
    {
        const QString sourceTypeName = fixedCppTypeName(sourceType);
        const QString toCpp = pythonToCppFunctionName(sourceTypeName, targetTypeName);
        const QString isConv = convertibleToCppFunctionName(sourceTypeName, targetTypeName);

        writeAddPythonToCppConversion(s, targetConverter, toCpp, isConv);
    };

    auto klass = AbstractMetaClass::findClass(classes(), type.instantiations().at(0).typeEntry());
    if (!klass)
        return;

    const auto classes = klass->typeSystemBaseClasses();
    if (classes.isEmpty())
        return;

    s << "// Register SmartPointer converter for type '" << cppSignature << "'." << '\n'
       << "///////////////////////////////////////////////////////////////////////////////////////\n\n";

    for (auto k : classes) {
        auto smartTargetType = findSmartPointerInstantiation(k->typeEntry());
        if (smartTargetType.has_value()) {
            s << "// Convert to SmartPointer derived class: ["
                << smartTargetType->cppSignature() << "]\n";
            const QString converter =
                QLatin1String("Shiboken::Conversions::getConverter(\"%1\")").arg(smartTargetType->cppSignature());
            writeConversionRegister(type, fixedCppTypeName(smartTargetType.value()), converter);
        } else {
            s << "// Class not found:" << type.instantiations().at(0).cppSignature();
        }
    }

    s << "///////////////////////////////////////////////////////////////////////////////////////" << '\n' << '\n';
}

void CppGenerator::writeExtendedConverterInitialization(TextStream &s, const TypeEntry *externalType,
                                                        const AbstractMetaClassCList &conversions) const
{
    s << "// Extended implicit conversions for " << externalType->qualifiedTargetLangName()
      << ".\n";
    for (const AbstractMetaClass *sourceClass : conversions) {
        const QString converterVar = QLatin1String("reinterpret_cast<SbkObjectType *>(")
            + cppApiVariableName(externalType->targetLangPackage()) + QLatin1Char('[')
            + getTypeIndexVariableName(externalType) + QLatin1String("])");
        QString sourceTypeName = fixedCppTypeName(sourceClass->typeEntry());
        QString targetTypeName = fixedCppTypeName(externalType);
        QString toCpp = pythonToCppFunctionName(sourceTypeName, targetTypeName);
        QString isConv = convertibleToCppFunctionName(sourceTypeName, targetTypeName);
        writeAddPythonToCppConversion(s, converterVar, toCpp, isConv);
    }
}

QString CppGenerator::multipleInheritanceInitializerFunctionName(const AbstractMetaClass *metaClass)
{
    return cpythonBaseName(metaClass->typeEntry()) + QLatin1String("_mi_init");
}

bool CppGenerator::supportsMappingProtocol(const AbstractMetaClass *metaClass) const
{
    for (const auto &m : mappingProtocols()) {
        if (metaClass->hasFunction(m.name))
            return true;
    }

    return false;
}

bool CppGenerator::supportsNumberProtocol(const AbstractMetaClass *metaClass) const
{
    return metaClass->hasArithmeticOperatorOverload()
            || metaClass->hasLogicalOperatorOverload()
            || metaClass->hasBitwiseOperatorOverload()
            || hasBoolCast(metaClass);
}

bool CppGenerator::supportsSequenceProtocol(const AbstractMetaClass *metaClass) const
{
    for (const auto &seq : sequenceProtocols()) {
        if (metaClass->hasFunction(seq.name))
            return true;
    }

    const ComplexTypeEntry *baseType = metaClass->typeEntry()->baseContainerType();
    return baseType && baseType->isContainer();
}

bool CppGenerator::shouldGenerateGetSetList(const AbstractMetaClass *metaClass) const
{
    for (const AbstractMetaField &f : metaClass->fields()) {
        if (!f.isStatic())
            return true;
    }
    // Generate all user-added properties unless Pyside extensions are used,
    // in which only the explicitly specified ones are generated (rest is handled
    // in libpyside).
    return usePySideExtensions()
        ? std::any_of(metaClass->propertySpecs().cbegin(), metaClass->propertySpecs().cend(),
                      [] (const QPropertySpec &s) { return s.generateGetSetDef(); })
        : !metaClass->propertySpecs().isEmpty();
    return false;
}

struct pyTypeSlotEntry
{
    explicit pyTypeSlotEntry(const char *name, const QString &function) :
        m_name(name), m_function(function) {}

    const char *m_name;
    const QString &m_function;
};

TextStream &operator<<(TextStream &str, const pyTypeSlotEntry &e)
{
    str << '{' << e.m_name << ',';
    const int padding = qMax(0, 18 - int(strlen(e.m_name)));
    for (int p = 0; p < padding; ++p)
        str << ' ';
    if (e.m_function.isEmpty())
        str << NULL_PTR;
    else
        str << "reinterpret_cast<void *>(" << e.m_function << ')';
    str << "},\n";
    return str;
}

void CppGenerator::writeClassDefinition(TextStream &s,
                                        const AbstractMetaClass *metaClass,
                                        const GeneratorContext &classContext)
{
    QString tp_flags;
    QString tp_init;
    QString tp_new;
    QString tp_dealloc;
    QString tp_hash;
    QString tp_call;
    QString cppClassName = metaClass->qualifiedCppName();
    const QString className = chopType(cpythonTypeName(metaClass));
    QString baseClassName;
    AbstractMetaFunctionCList ctors;
    const auto &allCtors = metaClass->queryFunctions(FunctionQueryOption::Constructors);
    for (const auto &f : allCtors) {
        if (!f->isPrivate() && !f->isModifiedRemoved() && !classContext.forSmartPointer())
            ctors.append(f);
    }

    if (!metaClass->baseClass())
        baseClassName = QLatin1String("reinterpret_cast<PyTypeObject *>(SbkObject_TypeF())");

    bool onlyPrivCtor = !metaClass->hasNonPrivateConstructor();

    const AbstractMetaClass *qCoreApp = AbstractMetaClass::findClass(classes(), QLatin1String("QCoreApplication"));
    const bool isQApp = qCoreApp != Q_NULLPTR && metaClass->inheritsFrom(qCoreApp);

    tp_flags = QLatin1String("Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES");
    if (metaClass->isNamespace() || metaClass->hasPrivateDestructor()) {
        tp_dealloc = metaClass->hasPrivateDestructor() ?
                     QLatin1String("SbkDeallocWrapperWithPrivateDtor") :
                     QLatin1String("Sbk_object_dealloc /* PYSIDE-832: Prevent replacement of \"0\" with subtype_dealloc. */");
        tp_init.clear();
    } else {
        QString deallocClassName = classContext.useWrapper()
            ? classContext.wrapperName() : cppClassName;
        if (isQApp)
            tp_dealloc = QLatin1String("&SbkDeallocQAppWrapper");
        else
            tp_dealloc = QLatin1String("&SbkDeallocWrapper");
        if (!onlyPrivCtor && !ctors.isEmpty())
            tp_init = cpythonFunctionName(ctors.constFirst());
    }

    const AttroCheck attroCheck = checkAttroFunctionNeeds(metaClass);
    const QString tp_getattro = (attroCheck & AttroCheckFlag::GetattroMask) != 0
        ? cpythonGetattroFunctionName(metaClass) : QString();
    const QString tp_setattro = (attroCheck & AttroCheckFlag::SetattroMask) != 0
        ? cpythonSetattroFunctionName(metaClass) : QString();

    if (metaClass->hasPrivateDestructor() || onlyPrivCtor) {
        // tp_flags = QLatin1String("Py_TPFLAGS_DEFAULT|Py_TPFLAGS_CHECKTYPES");
        // This is not generally possible, because PySide does not care about
        // privacy the same way. This worked before the heap types were used,
        // because inheritance is not really checked for static types.
        // Instead, we check this at runtime, see SbkObjectTypeTpNew.
        if (metaClass->fullName().startsWith(QLatin1String("PySide6.Qt"))) {
            // PYSIDE-595: No idea how to do non-inheritance correctly.
            // Since that is only relevant in shiboken, I used a shortcut for
            // PySide.
            tp_new = QLatin1String("SbkObjectTpNew");
        }
        else {
            tp_new = QLatin1String("SbkDummyNew /* PYSIDE-595: Prevent replacement "
                                   "of \"0\" with base->tp_new. */");
        }
        tp_flags.append(QLatin1String("|Py_TPFLAGS_HAVE_GC"));
    }
    else if (isQApp) {
        tp_new = QLatin1String("SbkQAppTpNew"); // PYSIDE-571: need singleton app
    }
    else {
        tp_new = QLatin1String("SbkObjectTpNew");
        tp_flags.append(QLatin1String("|Py_TPFLAGS_HAVE_GC"));
    }

    QString tp_richcompare;
    if (!metaClass->isNamespace() && metaClass->hasComparisonOperatorOverload())
        tp_richcompare = cpythonBaseName(metaClass) + QLatin1String("_richcompare");

    QString tp_getset;
    if (shouldGenerateGetSetList(metaClass) && !classContext.forSmartPointer())
        tp_getset = cpythonGettersSettersDefinitionName(metaClass);

    // search for special functions
    clearTpFuncs();
    for (const auto &func : metaClass->functions()) {
        if (m_tpFuncs.contains(func->name()))
            m_tpFuncs[func->name()] = cpythonFunctionName(func);
    }
    if (m_tpFuncs.value(reprFunction()).isEmpty()
        && metaClass->hasToStringCapability()) {
        m_tpFuncs[reprFunction()] = writeReprFunction(s,
                classContext,
                metaClass->toStringCapabilityIndirections());
    }

    // class or some ancestor has multiple inheritance
    const AbstractMetaClass *miClass = getMultipleInheritingClass(metaClass);
    if (miClass) {
        if (metaClass == miClass)
            writeMultipleInheritanceInitializerFunction(s, metaClass);
        writeSpecialCastFunction(s, metaClass);
        s << '\n';
    }

    s << "// Class Definition -----------------------------------------------\n"
         "extern \"C\" {\n";

    if (!metaClass->typeEntry()->hashFunction().isEmpty())
        tp_hash = QLatin1Char('&') + cpythonBaseName(metaClass) + QLatin1String("_HashFunc");

    const auto callOp = metaClass->findFunction(QLatin1String("operator()"));
    if (!callOp.isNull() && !callOp->isModifiedRemoved())
        tp_call = QLatin1Char('&') + cpythonFunctionName(callOp);

    QString computedClassTargetFullName;
    if (!classContext.forSmartPointer())
        computedClassTargetFullName = getClassTargetFullName(metaClass);
    else
        computedClassTargetFullName = getClassTargetFullName(classContext.preciseType());

    const QString typePtr = QLatin1String("_") + className
        + QLatin1String("_Type");
    s << "static SbkObjectType *" << typePtr << " = nullptr;\n"
        << "static SbkObjectType *" << className << "_TypeF(void)\n"
        << "{\n" << indent << "return " << typePtr << ";\n" << outdent
        << "}\n\nstatic PyType_Slot " << className << "_slots[] = {\n" << indent
        << "{Py_tp_base,        nullptr}, // inserted by introduceWrapperType\n"
        << pyTypeSlotEntry("Py_tp_dealloc", tp_dealloc)
        << pyTypeSlotEntry("Py_tp_repr", m_tpFuncs.value(reprFunction()))
        << pyTypeSlotEntry("Py_tp_hash", tp_hash)
        << pyTypeSlotEntry("Py_tp_call", tp_call)
        << pyTypeSlotEntry("Py_tp_str", m_tpFuncs.value(QLatin1String("__str__")))
        << pyTypeSlotEntry("Py_tp_getattro", tp_getattro)
        << pyTypeSlotEntry("Py_tp_setattro", tp_setattro)
        << pyTypeSlotEntry("Py_tp_traverse", className + QLatin1String("_traverse"))
        << pyTypeSlotEntry("Py_tp_clear", className + QLatin1String("_clear"))
        << pyTypeSlotEntry("Py_tp_richcompare", tp_richcompare)
        << pyTypeSlotEntry("Py_tp_iter", m_tpFuncs.value(QLatin1String("__iter__")))
        << pyTypeSlotEntry("Py_tp_iternext", m_tpFuncs.value(QLatin1String("__next__")))
        << pyTypeSlotEntry("Py_tp_methods", className + QLatin1String("_methods"))
        << pyTypeSlotEntry("Py_tp_getset", tp_getset)
        << pyTypeSlotEntry("Py_tp_init", tp_init)
        << pyTypeSlotEntry("Py_tp_new", tp_new);
    if (supportsSequenceProtocol(metaClass)) {
        s << "// type supports sequence protocol\n";
        writeTypeAsSequenceDefinition(s, metaClass);
    }
    if (supportsMappingProtocol(metaClass)) {
        s << "// type supports mapping protocol\n";
        writeTypeAsMappingDefinition(s, metaClass);
    }
    if (supportsNumberProtocol(metaClass)) {
        // This one must come last. See the function itself.
        s << "// type supports number protocol\n";
        writeTypeAsNumberDefinition(s, metaClass);
    }
    s << "{0, " << NULL_PTR << "}\n" << outdent << "};\n";

    int packageLevel = packageName().count(QLatin1Char('.')) + 1;
    s << "static PyType_Spec " << className << "_spec = {\n" << indent
        << '"' << packageLevel << ':' << computedClassTargetFullName << "\",\n"
        << "sizeof(SbkObject),\n0,\n" << tp_flags << ",\n"
        << className << "_slots\n" << outdent
        << "};\n\n} //extern \"C\"\n";
}

void CppGenerator::writeMappingMethods(TextStream &s,
                                       const AbstractMetaClass *metaClass,
                                       const GeneratorContext &context) const
{
    for (const auto & m : mappingProtocols()) {
        const auto func = metaClass->findFunction(m.name);
        if (func.isNull())
            continue;
        QString funcName = cpythonFunctionName(func);
        CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
        s << m.returnType << ' ' << funcName << '(' << m.arguments << ")\n{\n";
        writeInvalidPyObjectCheck(s, QLatin1String("self"));

        writeCppSelfDefinition(s, func, context);

        const AbstractMetaArgument *lastArg = func->arguments().isEmpty()
            ? nullptr : &func->arguments().constLast();
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode, func, lastArg);
        s<< "}\n\n";
    }
}

void CppGenerator::writeSequenceMethods(TextStream &s,
                                        const AbstractMetaClass *metaClass,
                                        const GeneratorContext &context) const
{
    bool injectedCode = false;

    for (const auto &seq : sequenceProtocols()) {
        const auto func = metaClass->findFunction(seq.name);
        if (func.isNull())
            continue;
        injectedCode = true;
        QString funcName = cpythonFunctionName(func);

        CodeSnipList snips = func->injectedCodeSnips(TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode);
        s << seq.returnType << ' ' << funcName << '(' << seq.arguments << ")\n{\n" << indent;
        writeInvalidPyObjectCheck(s, QLatin1String("self"));

        writeCppSelfDefinition(s, func, context);

        const AbstractMetaArgument *lastArg = func->arguments().isEmpty() ? nullptr : &func->arguments().constLast();
        writeCodeSnips(s, snips,TypeSystem::CodeSnipPositionAny, TypeSystem::TargetLangCode, func, lastArg);
        s<< outdent << "}\n\n";
    }

    if (!injectedCode)
        writeDefaultSequenceMethods(s, context);
}

// Sequence protocol structure member names
static const QHash<QString, QString> &sqFuncs()
{
    static const QHash<QString, QString> result = {
        {QLatin1String("__concat__"), QLatin1String("sq_concat")},
        {QLatin1String("__contains__"), QLatin1String("sq_contains")},
        {QLatin1String("__getitem__"), QLatin1String("sq_item")},
        {QLatin1String("__getslice__"), QLatin1String("sq_slice")},
        {QLatin1String("__len__"), QLatin1String("sq_length")},
        {QLatin1String("__setitem__"), QLatin1String("sq_ass_item")},
        {QLatin1String("__setslice__"), QLatin1String("sq_ass_slice")}
    };
    return result;
}

void CppGenerator::writeTypeAsSequenceDefinition(TextStream &s, const AbstractMetaClass *metaClass) const
{
    bool hasFunctions = false;
    QMap<QString, QString> funcs;
    for (const auto &seq : sequenceProtocols()) {
        const auto func = metaClass->findFunction(seq.name);
        if (!func.isNull()) {
            funcs.insert(seq.name, QLatin1Char('&') + cpythonFunctionName(func));
            hasFunctions = true;
        }
    }

    QString baseName = cpythonBaseName(metaClass);

    //use default implementation
    if (!hasFunctions) {
        funcs[QLatin1String("__len__")] = baseName + QLatin1String("__len__");
        funcs[QLatin1String("__getitem__")] = baseName + QLatin1String("__getitem__");
        funcs[QLatin1String("__setitem__")] = baseName + QLatin1String("__setitem__");
    }

    for (auto it = sqFuncs().cbegin(), end = sqFuncs().cend(); it != end; ++it) {
        const QString &sqName = it.key();
        auto fit = funcs.constFind(sqName);
        if (fit != funcs.constEnd()) {
            s <<  "{Py_" << it.value() << ", reinterpret_cast<void *>("
               << fit.value() << ")},\n";
        }
    }
}

void CppGenerator::writeTypeAsMappingDefinition(TextStream &s, const AbstractMetaClass *metaClass) const
{
    // Sequence protocol structure members names
    static const QHash<QString, QString> mpFuncs{
        {QLatin1String("__mlen__"), QLatin1String("mp_length")},
        {QLatin1String("__mgetitem__"), QLatin1String("mp_subscript")},
        {QLatin1String("__msetitem__"), QLatin1String("mp_ass_subscript")},
    };
    QMap<QString, QString> funcs;
    for (const auto &m : mappingProtocols()) {
        const auto func = metaClass->findFunction(m.name);
        if (!func.isNull()) {
            const QString entry = QLatin1String("reinterpret_cast<void *>(&")
                                  + cpythonFunctionName(func) + QLatin1Char(')');
            funcs.insert(m.name, entry);
        } else {
            funcs.insert(m.name, QLatin1String(NULL_PTR));
        }
    }

    for (auto it = mpFuncs.cbegin(), end = mpFuncs.cend(); it != end; ++it) {
        const auto fit = funcs.constFind(it.key());
        if (fit != funcs.constEnd())
            s <<  "{Py_" << it.value() << ", " << fit.value() <<  "},\n";
    }
}

// Number protocol structure members names
static const QHash<QString, QString> &nbFuncs()
{
    static const QHash<QString, QString> result = {
        {QLatin1String("__add__"), QLatin1String("nb_add")},
        {QLatin1String("__sub__"), QLatin1String("nb_subtract")},
        {QLatin1String("__mul__"), QLatin1String("nb_multiply")},
        {QLatin1String("__div__"), QLatin1String("nb_divide")},
        {QLatin1String("__mod__"), QLatin1String("nb_remainder")},
        {QLatin1String("__neg__"), QLatin1String("nb_negative")},
        {QLatin1String("__pos__"), QLatin1String("nb_positive")},
        {QLatin1String("__invert__"), QLatin1String("nb_invert")},
        {QLatin1String("__lshift__"), QLatin1String("nb_lshift")},
        {QLatin1String("__rshift__"), QLatin1String("nb_rshift")},
        {QLatin1String("__and__"), QLatin1String("nb_and")},
        {QLatin1String("__xor__"), QLatin1String("nb_xor")},
        {QLatin1String("__or__"), QLatin1String("nb_or")},
        {QLatin1String("__iadd__"), QLatin1String("nb_inplace_add")},
        {QLatin1String("__isub__"), QLatin1String("nb_inplace_subtract")},
        {QLatin1String("__imul__"), QLatin1String("nb_inplace_multiply")},
        {QLatin1String("__idiv__"), QLatin1String("nb_inplace_divide")},
        {QLatin1String("__imod__"), QLatin1String("nb_inplace_remainder")},
        {QLatin1String("__ilshift__"), QLatin1String("nb_inplace_lshift")},
        {QLatin1String("__irshift__"), QLatin1String("nb_inplace_rshift")},
        {QLatin1String("__iand__"), QLatin1String("nb_inplace_and")},
        {QLatin1String("__ixor__"), QLatin1String("nb_inplace_xor")},
        {QLatin1String("__ior__"), QLatin1String("nb_inplace_or")},
        {boolT(), QLatin1String("nb_nonzero")}
    };
    return result;
}

void CppGenerator::writeTypeAsNumberDefinition(TextStream &s, const AbstractMetaClass *metaClass) const
{
    QMap<QString, QString> nb;

    const QList<AbstractMetaFunctionCList> opOverloads =
            filterGroupedOperatorFunctions(metaClass,
                                           OperatorQueryOption::ArithmeticOp
                                           | OperatorQueryOption::LogicalOp
                                           | OperatorQueryOption::BitwiseOp);

    for (const AbstractMetaFunctionCList &opOverload : opOverloads) {
        const auto rfunc = opOverload.at(0);
        QString opName = ShibokenGenerator::pythonOperatorFunctionName(rfunc);
        nb[opName] = cpythonFunctionName(rfunc);
    }

    QString baseName = cpythonBaseName(metaClass);

    if (hasBoolCast(metaClass))
        nb.insert(boolT(), baseName + QLatin1String("___nb_bool"));

    for (auto it = nbFuncs().cbegin(), end = nbFuncs().cend(); it != end; ++it) {
        const QString &nbName = it.key();
        if (nbName == QLatin1String("__div__") || nbName == QLatin1String("__idiv__"))
            continue; // excludeFromPy3K
        const auto nbIt = nb.constFind(nbName);
        if (nbIt != nb.constEnd()) {
            const QString fixednbName = nbName == boolT()
                ? QLatin1String("nb_bool") : it.value();
            s <<  "{Py_" << fixednbName << ", reinterpret_cast<void *>("
               << nbIt.value() << ")},\n";
        }
    }

    auto nbIt = nb.constFind(QLatin1String("__div__"));
    if (nbIt != nb.constEnd())
        s << "{Py_nb_true_divide, reinterpret_cast<void *>(" << nbIt.value() << ")},\n";

    nbIt = nb.constFind(QLatin1String("__idiv__"));
    if (nbIt != nb.constEnd()) {
        s << "// This function is unused in Python 3. We reference it here.\n"
            << "{0, reinterpret_cast<void *>(" << nbIt.value() << ")},\n"
            << "// This list is ending at the first 0 entry.\n"
            << "// Therefore, we need to put the unused functions at the very end.\n";
    }
}

void CppGenerator::writeTpTraverseFunction(TextStream &s, const AbstractMetaClass *metaClass) const
{
    QString baseName = cpythonBaseName(metaClass);
    s << "static int " << baseName
        << "_traverse(PyObject *self, visitproc visit, void *arg)\n{\n" << indent
        << "return reinterpret_cast<PyTypeObject *>(SbkObject_TypeF())->tp_traverse(self, visit, arg);\n"
        << outdent << "}\n";
}

void CppGenerator::writeTpClearFunction(TextStream &s, const AbstractMetaClass *metaClass) const
{
    QString baseName = cpythonBaseName(metaClass);
    s << "static int " << baseName << "_clear(PyObject *self)\n{\n" << indent
       << "return reinterpret_cast<PyTypeObject *>(SbkObject_TypeF())->tp_clear(self);\n"
       << outdent << "}\n";
}

void CppGenerator::writeCopyFunction(TextStream &s, const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    const QString className = chopType(cpythonTypeName(metaClass));
    s << "static PyObject *" << className << "___copy__(PyObject *self)\n"
        << "{\n" << indent;
    writeCppSelfDefinition(s, context, false, true);
    QString conversionCode;
    if (!context.forSmartPointer())
        conversionCode = cpythonToPythonConversionFunction(metaClass);
    else
        conversionCode = cpythonToPythonConversionFunction(context.preciseType());

    s << "PyObject *" << PYTHON_RETURN_VAR << " = " << conversionCode
        << CPP_SELF_VAR << ");\n";
    writeFunctionReturnErrorCheckSection(s);
    s << "return " << PYTHON_RETURN_VAR << ";\n" << outdent
        << "}\n\n";
}

static inline void writeGetterFunctionStart(TextStream &s, const QString &funcName)
{
    s << "static PyObject *" << funcName << "(PyObject *self, void *)\n"
       << "{\n" << indent;
}

void CppGenerator::writeGetterFunction(TextStream &s,
                                       const AbstractMetaField &metaField,
                                       const GeneratorContext &context) const
{
    ErrorCode errorCode(QString::fromLatin1(NULL_PTR));
    writeGetterFunctionStart(s, cpythonGetterFunctionName(metaField));

    writeCppSelfDefinition(s, context);

    AbstractMetaType fieldType = metaField.type();
    // Force use of pointer to return internal variable memory
    bool newWrapperSameObject = !fieldType.isConstant() && fieldType.isWrapperType()
            && !fieldType.isPointer();

    QString cppField;
    if (avoidProtectedHack() && metaField.isProtected()) {
        QTextStream(&cppField) << "static_cast<"
            << context.wrapperName() << " *>("
            << CPP_SELF_VAR << ")->" << protectedFieldGetterName(metaField) << "()";
    } else {
        cppField = QLatin1String(CPP_SELF_VAR) + QLatin1String("->")
                   + metaField.originalName();
        if (newWrapperSameObject) {
            cppField.prepend(QLatin1String("&("));
            cppField.append(QLatin1Char(')'));
        }
    }
    if (fieldType.isCppIntegralPrimitive() || fieldType.isEnum()) {
        s << getFullTypeNameWithoutModifiers(fieldType) << " cppOut_local = " << cppField << ";\n";
        cppField = QLatin1String("cppOut_local");
    } else if (avoidProtectedHack() && metaField.isProtected()) {
        s << getFullTypeNameWithoutModifiers(fieldType);
        if (fieldType.isContainer() || fieldType.isFlags() || fieldType.isSmartPointer()) {
            s << " &";
            cppField.prepend(QLatin1Char('*'));
        } else if ((!fieldType.isConstant() && !fieldType.isEnum() && !fieldType.isPrimitive()) || fieldType.indirections() == 1) {
            s << " *";
        }
        s << " fieldValue = " << cppField << ";\n";
        cppField = QLatin1String("fieldValue");
    }

    s << "PyObject *pyOut = {};\n";
    if (newWrapperSameObject) {
        // Special case colocated field with same address (first field in a struct)
        s << "if (reinterpret_cast<void *>("
                    << cppField
                    << ") == reinterpret_cast<void *>("
                    << CPP_SELF_VAR << ")) {\n";
        {
            Indentation indent(s);
            s << "pyOut = reinterpret_cast<PyObject *>(Shiboken::Object::findColocatedChild("
                        << "reinterpret_cast<SbkObject *>(self), reinterpret_cast<SbkObjectType *>("
                        << cpythonTypeNameExt(fieldType)
                        << ")));\n";
            s << "if (pyOut) {\n";
            {
                Indentation indent(s);
                s << "Py_IncRef(pyOut);\n"
                   << "return pyOut;\n";
            }
            s << "}\n";
        }
        // Check if field wrapper has already been created.
        s << "} else if (Shiboken::BindingManager::instance().hasWrapper(" << cppField << ")) {" << "\n";
        {
            Indentation indent(s);
            s << "pyOut = reinterpret_cast<PyObject *>(Shiboken::BindingManager::instance().retrieveWrapper("
                << cppField << "));" << "\n"
                << "Py_IncRef(pyOut);" << "\n"
                << "return pyOut;" << "\n";
        }
        s << "}\n";
        // Create and register new wrapper
        s << "pyOut = "
            << "Shiboken::Object::newObject(reinterpret_cast<SbkObjectType *>(" << cpythonTypeNameExt(fieldType)
            << "), " << cppField << ", false, true);\n"
            << "Shiboken::Object::setParent(self, pyOut)";
    } else {
        s << "pyOut = ";
        writeToPythonConversion(s, fieldType, metaField.enclosingClass(), cppField);
    }
    s << ";\nreturn pyOut;\n" << outdent << "}\n";
}

// Write a getter for QPropertySpec
void CppGenerator::writeGetterFunction(TextStream &s, const QPropertySpec &property,
                                       const GeneratorContext &context) const
{
    ErrorCode errorCode(0);
    writeGetterFunctionStart(s, cpythonGetterFunctionName(property, context.metaClass()));
    writeCppSelfDefinition(s, context);
    const QString value = QStringLiteral("value");
    s << "auto " << value << " = " << CPP_SELF_VAR << "->" << property.read() << "();\n"
        << "auto pyResult = ";
    writeToPythonConversion(s, property.type(), context.metaClass(), value);
    s << ";\nif (PyErr_Occurred() || !pyResult) {\n";
    {
        Indentation indent(s);
        s << "Py_XDECREF(pyResult);\nreturn {};\n";
    }
    s << "}\nreturn pyResult;\n" << outdent << "}\n\n";
}

// Write setter function preamble (type checks on "pyIn")
void CppGenerator::writeSetterFunctionPreamble(TextStream &s, const QString &name,
                                               const QString &funcName,
                                               const AbstractMetaType &type,
                                               const GeneratorContext &context) const
{
    s << "static int " << funcName << "(PyObject *self, PyObject *pyIn, void *)\n"
        << "{\n" << indent;

    writeCppSelfDefinition(s, context);

    s << "if (pyIn == " << NULL_PTR << ") {\n" << indent
        << "PyErr_SetString(PyExc_TypeError, \"'"
        << name << "' may not be deleted\");\n"
        << "return -1;\n"
        << outdent << "}\n";

    s << "PythonToCppFunc " << PYTHON_TO_CPP_VAR << "{nullptr};\n"
        << "if (!";
    writeTypeCheck(s, type, QLatin1String("pyIn"), isNumber(type.typeEntry()));
    s << ") {\n" << indent
        << "PyErr_SetString(PyExc_TypeError, \"wrong type attributed to '"
        << name << "', '" << type.name() << "' or convertible type expected\");\n"
        << "return -1;\n"
        << outdent << "}\n\n";
}

void CppGenerator::writeSetterFunction(TextStream &s,
                                       const AbstractMetaField &metaField,
                                       const GeneratorContext &context) const
{
    ErrorCode errorCode(0);

    const AbstractMetaType &fieldType = metaField.type();
    writeSetterFunctionPreamble(s, metaField.name(), cpythonSetterFunctionName(metaField),
                                fieldType, context);


    const QString cppField = QLatin1String(CPP_SELF_VAR) + QLatin1String("->")
                             + metaField.originalName();
    if (avoidProtectedHack() && metaField.isProtected()) {
        s << getFullTypeNameWithoutModifiers(fieldType);
        if (fieldType.indirections() == 1)
            s << " *";
        s << " cppOut;\n"
            << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut);\n"
            << "static_cast<" << context.wrapperName()
            << " *>(" << CPP_SELF_VAR << ")->" << protectedFieldSetterName(metaField)
            << "(cppOut)";
    } else if (fieldType.isCppIntegralPrimitive() || fieldType.typeEntry()->isEnum()
               || fieldType.typeEntry()->isFlags()) {
        s << getFullTypeNameWithoutModifiers(fieldType) << " cppOut_local = "
            << cppField << ";\n"
            << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut_local);\n"
            << cppField << " = cppOut_local";
    } else {
        if (fieldType.isPointerToConst())
            s << "const ";
        s << getFullTypeNameWithoutModifiers(fieldType)
            << QString::fromLatin1(" *").repeated(fieldType.indirections()) << "& cppOut_ptr = "
            << cppField << ";\n"
            << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut_ptr)";
    }
    s << ";\n\n";

    if (fieldType.isPointerToWrapperType()) {
        s << "Shiboken::Object::keepReference(reinterpret_cast<SbkObject *>(self), \""
            << metaField.name() << "\", pyIn);\n";
    }

    s << "return 0;\n" << outdent << "}\n";
}

// Write a setter for QPropertySpec
void CppGenerator::writeSetterFunction(TextStream &s, const QPropertySpec &property,
                                       const GeneratorContext &context) const
{
    ErrorCode errorCode(0);
    writeSetterFunctionPreamble(s, property.name(),
                                cpythonSetterFunctionName(property, context.metaClass()),
                                property.type(), context);

    s << "auto cppOut = " << CPP_SELF_VAR << "->" << property.read() << "();\n"
        << PYTHON_TO_CPP_VAR << "(pyIn, &cppOut);\n"
        << "if (PyErr_Occurred())\n";
    {
        Indentation indent(s);
        s << "return -1;\n";
    }
    s << CPP_SELF_VAR << "->" << property.write() << "(cppOut);\n"
        << "return 0;\n" << outdent << "}\n\n";
}

void CppGenerator::writeRichCompareFunction(TextStream &s,
                                            const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    QString baseName = cpythonBaseName(metaClass);
    s << "static PyObject * ";
    s << baseName << "_richcompare(PyObject *self, PyObject *" << PYTHON_ARG
        << ", int op)\n{\n" << indent;
    writeCppSelfDefinition(s, context, false, true);
    writeUnusedVariableCast(s, QLatin1String(CPP_SELF_VAR));
    s << "PyObject *" << PYTHON_RETURN_VAR << "{};\n"
        << "PythonToCppFunc " << PYTHON_TO_CPP_VAR << ";\n";
    writeUnusedVariableCast(s, QLatin1String(PYTHON_TO_CPP_VAR));
    s << '\n';

    s << "switch (op) {\n";
    {
        Indentation indent(s);
        const QList<AbstractMetaFunctionCList> &groupedFuncs =
            filterGroupedOperatorFunctions(metaClass, OperatorQueryOption::ComparisonOp);
        for (const AbstractMetaFunctionCList &overloads : groupedFuncs) {
            const auto rfunc = overloads[0];

            QString operatorId = ShibokenGenerator::pythonRichCompareOperatorId(rfunc);
            s << "case " << operatorId << ':' << '\n';

            Indentation indent(s);

            QString op = rfunc->originalName();
            op = op.right(op.size() - QLatin1String("operator").size());

            int alternativeNumericTypes = 0;
            for (const auto &func : overloads) {
                if (!func->isStatic() &&
                    ShibokenGenerator::isNumber(func->arguments().at(0).type().typeEntry()))
                    alternativeNumericTypes++;
            }

            bool first = true;
            OverloadData overloadData(overloads, this);
            const OverloadDataList &nextOverloads = overloadData.nextOverloadData();
            for (OverloadData *od : nextOverloads) {
                const auto func = od->referenceFunction();
                if (func->isStatic())
                    continue;
                auto argTypeO = getArgumentType(func, 1);
                if (!argTypeO.has_value())
                    continue;
                auto argType = argTypeO.value();
                if (!first) {
                    s << " else ";
                } else {
                    first = false;
                }
                s << "if (";
                writeTypeCheck(s, argType, QLatin1String(PYTHON_ARG), alternativeNumericTypes == 1 || isPyInt(argType));
                s << ") {\n";
                {
                    Indentation indent(s);
                    s << "// " << func->signature() << '\n';
                    writeArgumentConversion(s, argType, QLatin1String(CPP_ARG0),
                                            QLatin1String(PYTHON_ARG), metaClass,
                                            QString(), func->isUserAdded());

                    // If the function is user added, use the inject code
                    bool generateOperatorCode = true;
                    if (func->isUserAdded()) {
                        CodeSnipList snips = func->injectedCodeSnips();
                        if (!snips.isEmpty()) {
                            writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionAny,
                                           TypeSystem::TargetLangCode, func,
                                           &func->arguments().constLast());
                            generateOperatorCode = false;
                        }
                    }
                    if (generateOperatorCode) {
                        if (!func->isVoid())
                            s << func->type().cppSignature() << " " << CPP_RETURN_VAR << " = ";
                        // expression
                        if (func->isPointerOperator())
                            s << '&';
                        s << CPP_SELF_VAR << ' ' << op << '(';
                        if (argType.shouldDereferencePointer())
                            s << '*';
                        s << CPP_ARG0 << ");\n"
                            << PYTHON_RETURN_VAR << " = ";
                        if (!func->isVoid())
                            writeToPythonConversion(s, func->type(), metaClass, QLatin1String(CPP_RETURN_VAR));
                        else
                            s << "Py_None;\n" << "Py_INCREF(Py_None)";
                        s << ";\n";
                    }
                }
                s << '}';
            }

            s << " else {\n";
            if (operatorId == QLatin1String("Py_EQ") || operatorId == QLatin1String("Py_NE")) {
                Indentation indent(s);
                s << PYTHON_RETURN_VAR << " = "
                    << (operatorId == QLatin1String("Py_EQ") ? "Py_False" : "Py_True") << ";\n"
                    << "Py_INCREF(" << PYTHON_RETURN_VAR << ");\n";
            } else {
                Indentation indent(s);
                s << "goto " << baseName << "_RichComparison_TypeError;\n";
            }
            s << "}\n\n";

            s << "break;\n";
        }
        s << "default:\n";
        {
            Indentation indent(s);
            s << "// PYSIDE-74: By default, we redirect to object's tp_richcompare (which is `==`, `!=`).\n"
                << "return FallbackRichCompare(self, " << PYTHON_ARG << ", op);\n"
                << "goto " << baseName << "_RichComparison_TypeError;\n";
        }
    }
    s << "}\n\n";

    s << "if (" << PYTHON_RETURN_VAR << " && !PyErr_Occurred())\n";
    {
        Indentation indent(s);
        s << "return " << PYTHON_RETURN_VAR << ";\n";
    }
    s << baseName << "_RichComparison_TypeError:\n"
        << "PyErr_SetString(PyExc_NotImplementedError, \"operator not implemented.\");\n"
        << returnStatement(m_currentErrorCode)  << '\n' << '\n'
        << outdent << "}\n\n";
}

QString CppGenerator::methodDefinitionParameters(const OverloadData &overloadData) const
{
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(overloadData);
    const auto func = overloadData.referenceFunction();
    int min = overloadData.minArgs();
    int max = overloadData.maxArgs();

    QString result;
    QTextStream s(&result);
    s << "reinterpret_cast<PyCFunction>("
        << cpythonFunctionName(func) << "), ";
    if ((min == max) && (max < 2) && !usePyArgs) {
        if (max == 0)
            s << "METH_NOARGS";
        else
            s << "METH_O";
    } else {
        s << "METH_VARARGS";
        if (overloadData.hasArgumentWithDefaultValue())
            s << "|METH_KEYWORDS";
    }
    // METH_STATIC causes a crash when used for global functions (also from
    // invisible namespaces).
    auto ownerClass = func->ownerClass();
    if (ownerClass
        && !invisibleTopNamespaces().contains(const_cast<AbstractMetaClass *>(ownerClass))
        && overloadData.hasStaticFunction()) {
        s << "|METH_STATIC";
    }
    return result;
}

void CppGenerator::writeMethodDefinitionEntries(TextStream &s,
                                                const AbstractMetaFunctionCList &overloads,
                                                qsizetype maxEntries) const
{
    Q_ASSERT(!overloads.isEmpty());
    OverloadData overloadData(overloads, this);
    const QStringList names = overloadData.referenceFunction()->definitionNames();
    const QString parameters = methodDefinitionParameters(overloadData);
    const qsizetype count = maxEntries > 0
        ? qMin(names.size(), maxEntries) : names.size();
    for (qsizetype i = 0; i < count; ++i) {
        if (i)
            s << ",\n";
         s << "{\"" << names.at(i) << "\", " << parameters << '}';
    }
}

void CppGenerator::writeMethodDefinition(TextStream &s, const AbstractMetaFunctionCList &overloads) const
{
    Q_ASSERT(!overloads.isEmpty());
    const auto func = overloads.constFirst();
    if (m_tpFuncs.contains(func->name()))
        return;

    if (OverloadData::hasStaticAndInstanceFunctions(overloads)) {
        s << cpythonMethodDefinitionName(func);
    } else {
        writeMethodDefinitionEntries(s, overloads);
    }
    s << ',' << '\n';
}

void CppGenerator::writeSignatureInfo(TextStream &s, const AbstractMetaFunctionCList &overloads) const
{
    OverloadData overloadData(overloads, this);
    const auto rfunc = overloadData.referenceFunction();
    QString funcName = fullPythonFunctionName(rfunc, false);

    int idx = overloads.length() - 1;
    bool multiple = idx > 0;

    for (const auto &f : overloads) {
        QStringList args;
        // PYSIDE-1328: `self`-ness cannot be computed in Python because there are mixed cases.
        // Toplevel functions like `PySide6.QtCore.QEnum` are always self-less.
        if (!(f->isStatic()) && f->ownerClass())
            args << QLatin1String("self");
        const AbstractMetaArgumentList &arguments = f->arguments();
        for (const AbstractMetaArgument &arg : arguments)  {
            auto metaType = arg.type();
            if (auto viewOn = metaType.viewOn())
                metaType = *viewOn;
            QString strArg = metaType.pythonSignature();
            if (!arg.defaultValueExpression().isEmpty()) {
                strArg += QLatin1Char('=');
                QString e = arg.defaultValueExpression();
                e.replace(QLatin1String("::"), QLatin1String("."));
                strArg += e;
            }
            args << arg.name() + QLatin1Char(':') + strArg;
        }
        // mark the multiple signatures as such, to make it easier to generate different code
        if (multiple)
            s << idx-- << ':';
        s << funcName << '(' << args.join(QLatin1Char(',')) << ')';
        if (!f->isVoid())
            s << "->" << f->type().pythonSignature();
        s << '\n';
    }
}

void CppGenerator::writeEnumsInitialization(TextStream &s, AbstractMetaEnumList &enums) const
{
    if (enums.isEmpty())
        return;
    s << "// Initialization of enums.\n\n";
    for (const AbstractMetaEnum &cppEnum : qAsConst(enums)) {
        if (cppEnum.isPrivate())
            continue;
        writeEnumInitialization(s, cppEnum);
    }
}

static QString mangleName(QString name)
{
    if (   name == QLatin1String("None")
        || name == QLatin1String("False")
        || name == QLatin1String("True"))
        name += QLatin1Char('_');
    return name;
}

void CppGenerator::writeEnumInitialization(TextStream &s, const AbstractMetaEnum &cppEnum) const
{
    const AbstractMetaClass *enclosingClass = cppEnum.targetLangEnclosingClass();
    bool hasUpperEnclosingClass = enclosingClass && enclosingClass->targetLangEnclosingClass() != nullptr;
    const EnumTypeEntry *enumTypeEntry = cppEnum.typeEntry();
    QString enclosingObjectVariable;
    if (enclosingClass)
        enclosingObjectVariable = cpythonTypeName(enclosingClass);
    else if (hasUpperEnclosingClass)
        enclosingObjectVariable = QLatin1String("enclosingClass");
    else
        enclosingObjectVariable = QLatin1String("module");

    s << "// Initialization of ";
    s << (cppEnum.isAnonymous() ? "anonymous enum identified by enum value" : "enum");
    s << " '" << cppEnum.name() << "'.\n";

    QString enumVarTypeObj;
    if (!cppEnum.isAnonymous()) {
        int packageLevel = packageName().count(QLatin1Char('.')) + 1;
        FlagsTypeEntry *flags = enumTypeEntry->flags();
        if (flags) {
            // The following could probably be made nicer:
            // We need 'flags->flagsName()' with the full module/class path.
            QString fullPath = getClassTargetFullName(cppEnum);
            fullPath.truncate(fullPath.lastIndexOf(QLatin1Char('.')) + 1);
            s << cpythonTypeNameExt(flags) << " = PySide::QFlags::create(\""
                << packageLevel << ':' << fullPath << flags->flagsName() << "\", "
                << cpythonEnumName(cppEnum) << "_number_slots);\n";
        }

        enumVarTypeObj = cpythonTypeNameExt(enumTypeEntry);

        s << enumVarTypeObj << " = Shiboken::Enum::"
            << ((enclosingClass || hasUpperEnclosingClass) ? "createScopedEnum" : "createGlobalEnum")
            << '(' << enclosingObjectVariable << ',' << '\n';
        {
            Indentation indent(s);
            s << '"' << cppEnum.name() << "\",\n"
                << '"' << packageLevel << ':' << getClassTargetFullName(cppEnum) << "\",\n"
                << '"' << cppEnum.qualifiedCppName() << '"';
            if (flags)
                s << ",\n" << cpythonTypeNameExt(flags);
            s << ");\n";
        }
        s << "if (!" << cpythonTypeNameExt(cppEnum.typeEntry()) << ")\n";
        {
            Indentation indent(s);
            s << returnStatement(m_currentErrorCode)  << "\n\n";
        }
    }

    for (const AbstractMetaEnumValue &enumValue : cppEnum.values()) {
        if (enumTypeEntry->isEnumValueRejected(enumValue.name()))
            continue;

        QString enumValueText;
        if (!avoidProtectedHack() || !cppEnum.isProtected()) {
            enumValueText = QLatin1String("(long) ");
            if (cppEnum.enclosingClass())
                enumValueText += cppEnum.enclosingClass()->qualifiedCppName() + QLatin1String("::");
            // Fully qualify the value which is required for C++ 11 enum classes.
            if (!cppEnum.isAnonymous())
                enumValueText += cppEnum.name() + QLatin1String("::");
            enumValueText += enumValue.name();
        } else {
            enumValueText += enumValue.value().toString();
        }

        const QString mangledName = mangleName(enumValue.name());
        switch (cppEnum.enumKind()) {
        case AnonymousEnum:
            if (enclosingClass || hasUpperEnclosingClass) {
                s << "{\n";
                {
                    Indentation indent(s);
                    s << "PyObject *anonEnumItem = PyInt_FromLong(" << enumValueText << ");\n"
                        << "if (PyDict_SetItemString(reinterpret_cast<PyTypeObject *>(reinterpret_cast<SbkObjectType *>("
                        << enclosingObjectVariable
                        << "))->tp_dict, \"" << mangledName << "\", anonEnumItem) < 0)\n";
                    {
                        Indentation indent(s);
                        s << returnStatement(m_currentErrorCode) << '\n';
                    }
                    s << "Py_DECREF(anonEnumItem);\n";
                }
                s << "}\n";
            } else {
                s << "if (PyModule_AddIntConstant(module, \"" << mangledName << "\", ";
                s << enumValueText << ") < 0)\n";
                {
                    Indentation indent(s);
                    s << returnStatement(m_currentErrorCode) << '\n';
                }
            }
            break;
        case CEnum: {
            s << "if (!Shiboken::Enum::";
            s << ((enclosingClass || hasUpperEnclosingClass) ? "createScopedEnumItem" : "createGlobalEnumItem");
            s << '(' << enumVarTypeObj << ',' << '\n';
            Indentation indent(s);
            s << enclosingObjectVariable << ", \"" << mangledName << "\", "
                << enumValueText << "))\n"
                << returnStatement(m_currentErrorCode) << '\n';
        }
            break;
        case EnumClass: {
            s << "if (!Shiboken::Enum::createScopedEnumItem("
                << enumVarTypeObj << ',' << '\n';
            Indentation indent(s);
            s << enumVarTypeObj<< ", \"" << mangledName << "\", "
               << enumValueText << "))\n"
               << returnStatement(m_currentErrorCode) << '\n';
        }
            break;
        }
    }

    writeEnumConverterInitialization(s, cppEnum);

    s << "// End of '" << cppEnum.name() << "' enum";
    if (cppEnum.typeEntry()->flags())
        s << "/flags";
    s << ".\n\n";
}

void CppGenerator::writeSignalInitialization(TextStream &s, const AbstractMetaClass *metaClass)
{
    // Try to check something and print some warnings
    const auto &signalFuncs = metaClass->cppSignalFunctions();
    for (const auto &cppSignal : signalFuncs) {
        if (cppSignal->declaringClass() != metaClass)
            continue;
        const AbstractMetaArgumentList &arguments = cppSignal->arguments();
        for (const AbstractMetaArgument &arg : arguments) {
            AbstractMetaType metaType = arg.type();
            const QByteArray origType =
                QMetaObject::normalizedType(qPrintable(metaType.originalTypeDescription()));
            const QByteArray cppSig =
                QMetaObject::normalizedType(qPrintable(metaType.cppSignature()));
            if ((origType != cppSig) && (!metaType.isFlags())) {
                qCWarning(lcShiboken).noquote().nospace()
                    << "Typedef used on signal " << metaClass->qualifiedCppName() << "::"
                    << cppSignal->signature();
                }
        }
    }

    s << "PySide::Signal::registerSignals(" << cpythonTypeName(metaClass) << ", &::"
       << metaClass->qualifiedCppName() << "::staticMetaObject);\n";
}

void CppGenerator::writeFlagsToLong(TextStream &s, const AbstractMetaEnum &cppEnum) const
{
    FlagsTypeEntry *flagsEntry = cppEnum.typeEntry()->flags();
    if (!flagsEntry)
        return;
    s << "static PyObject *" << cpythonEnumName(cppEnum) << "_long(PyObject *self)\n"
        << "{\n" << indent
        << "int val;\n";
    AbstractMetaType flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << cpythonToCppConversionFunction(flagsType) << "self, &val);\n"
        << "return Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<int>(), &val);\n"
        << outdent << "}\n";
}

void CppGenerator::writeFlagsNonZero(TextStream &s, const AbstractMetaEnum &cppEnum) const
{
    FlagsTypeEntry *flagsEntry = cppEnum.typeEntry()->flags();
    if (!flagsEntry)
        return;
    s << "static int " << cpythonEnumName(cppEnum) << "__nonzero(PyObject *self)\n";
    s << "{\n" << indent << "int val;\n";
    AbstractMetaType flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << cpythonToCppConversionFunction(flagsType) << "self, &val);\n"
        << "return val != 0;\n"
        << outdent << "}\n";
}

void CppGenerator::writeFlagsMethods(TextStream &s, const AbstractMetaEnum &cppEnum) const
{
    writeFlagsBinaryOperator(s, cppEnum, QLatin1String("and"), QLatin1String("&"));
    writeFlagsBinaryOperator(s, cppEnum, QLatin1String("or"), QLatin1String("|"));
    writeFlagsBinaryOperator(s, cppEnum, QLatin1String("xor"), QLatin1String("^"));

    writeFlagsUnaryOperator(s, cppEnum, QLatin1String("invert"), QLatin1String("~"));
    writeFlagsToLong(s, cppEnum);
    writeFlagsNonZero(s, cppEnum);

    s << '\n';
}

void CppGenerator::writeFlagsNumberMethodsDefinition(TextStream &s, const AbstractMetaEnum &cppEnum) const
{
    QString cpythonName = cpythonEnumName(cppEnum);

    s << "static PyType_Slot " << cpythonName << "_number_slots[] = {\n" << indent
        << "{Py_nb_bool,    reinterpret_cast<void *>(" << cpythonName << "__nonzero)},\n"
        << "{Py_nb_invert,  reinterpret_cast<void *>(" << cpythonName << "___invert__)},\n"
        << "{Py_nb_and,     reinterpret_cast<void *>(" << cpythonName  << "___and__)},\n"
        << "{Py_nb_xor,     reinterpret_cast<void *>(" << cpythonName  << "___xor__)},\n"
        << "{Py_nb_or,      reinterpret_cast<void *>(" << cpythonName  << "___or__)},\n"
        << "{Py_nb_int,     reinterpret_cast<void *>(" << cpythonName << "_long)},\n"
        << "{Py_nb_index,   reinterpret_cast<void *>(" << cpythonName << "_long)},\n"
        << "{0, " << NULL_PTR << "} // sentinel\n" << outdent
        << "};\n\n";
}

void CppGenerator::writeFlagsNumberMethodsDefinitions(TextStream &s, const AbstractMetaEnumList &enums) const
{
    for (const AbstractMetaEnum &e : enums) {
        if (!e.isAnonymous() && !e.isPrivate() && e.typeEntry()->flags()) {
            writeFlagsMethods(s, e);
            writeFlagsNumberMethodsDefinition(s, e);
            s << '\n';
        }
    }
}

void CppGenerator::writeFlagsBinaryOperator(TextStream &s, const AbstractMetaEnum &cppEnum,
                                            const QString &pyOpName, const QString &cppOpName) const
{
    FlagsTypeEntry *flagsEntry = cppEnum.typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject *" << cpythonEnumName(cppEnum) << "___" << pyOpName
        << "__(PyObject *self, PyObject *" << PYTHON_ARG << ")\n{\n" << indent;

    AbstractMetaType flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << "::" << flagsEntry->originalName() << " cppResult, " << CPP_SELF_VAR
        << ", cppArg;\n"
        << CPP_SELF_VAR << " = static_cast<::" << flagsEntry->originalName()
        << ">(int(PyLong_AsLong(self)));\n"
        << "cppArg = static_cast<" << flagsEntry->originalName()
        << ">(int(PyLong_AsLong(" << PYTHON_ARG << ")));\n"
        << "if (PyErr_Occurred())\n" << indent
            << "return nullptr;\n" << outdent
        << "cppResult = " << CPP_SELF_VAR << " " << cppOpName << " cppArg;\n"
        << "return ";
    writeToPythonConversion(s, flagsType, nullptr, QLatin1String("cppResult"));
    s << ";\n" << outdent << "}\n\n";
}

void CppGenerator::writeFlagsUnaryOperator(TextStream &s, const AbstractMetaEnum &cppEnum,
                                           const QString &pyOpName,
                                           const QString &cppOpName, bool boolResult) const
{
    FlagsTypeEntry *flagsEntry = cppEnum.typeEntry()->flags();
    Q_ASSERT(flagsEntry);

    s << "PyObject *" << cpythonEnumName(cppEnum) << "___" << pyOpName
        << "__(PyObject *self, PyObject *" << PYTHON_ARG << ")\n{\n" << indent;

    AbstractMetaType flagsType = buildAbstractMetaTypeFromTypeEntry(flagsEntry);
    s << "::" << flagsEntry->originalName() << " " << CPP_SELF_VAR << ";\n"
        << cpythonToCppConversionFunction(flagsType) << "self, &" << CPP_SELF_VAR
        << ");\n";
    if (boolResult)
        s << "bool";
    else
        s << "::" << flagsEntry->originalName();
    s << " cppResult = " << cppOpName << CPP_SELF_VAR << ";\n"
        << "return ";
    if (boolResult)
        s << "PyBool_FromLong(cppResult)";
    else
        writeToPythonConversion(s, flagsType, nullptr, QLatin1String("cppResult"));
    s << ";\n" << outdent << "}\n\n";
}

QString CppGenerator::getSimpleClassInitFunctionName(const AbstractMetaClass *metaClass) const
{
    QString initFunctionName;
    // Disambiguate namespaces per module to allow for extending them.
    if (metaClass->isNamespace())
        initFunctionName += moduleName();
    initFunctionName += metaClass->qualifiedCppName();
    initFunctionName.replace(QLatin1String("::"), QLatin1String("_"));
    return initFunctionName;
}

QString CppGenerator::getInitFunctionName(const GeneratorContext &context) const
{
    return !context.forSmartPointer()
        ? getSimpleClassInitFunctionName(context.metaClass())
        : getFilteredCppSignatureString(context.preciseType().cppSignature());
}

void CppGenerator::writeSignatureStrings(TextStream &s,
                                         const QString &signatures,
                                         const QString &arrayName,
                                         const char *comment) const
{
    s << "// The signatures string for the " << comment << ".\n"
        << "// Multiple signatures have their index \"n:\" in front.\n"
        << "static const char *" << arrayName << "_SignatureStrings[] = {\n" << indent;
    const auto lines = QStringView{signatures}.split(u'\n', Qt::SkipEmptyParts);
    for (auto line : lines) {
        // must anything be escaped?
        if (line.contains(QLatin1Char('"')) || line.contains(QLatin1Char('\\')))
            s << "R\"CPP(" << line << ")CPP\",\n";
        else
            s << '"' << line << "\",\n";
    }
    s << NULL_PTR << "}; // Sentinel\n" << outdent << '\n';
}

void CppGenerator::writeClassRegister(TextStream &s,
                                      const AbstractMetaClass *metaClass,
                                      const GeneratorContext &classContext,
                                      const QString &signatures) const
{
    const ComplexTypeEntry *classTypeEntry = metaClass->typeEntry();

    const AbstractMetaClass *enc = metaClass->targetLangEnclosingClass();
    QString enclosingObjectVariable = enc ? QLatin1String("enclosingClass") : QLatin1String("module");

    QString pyTypeName = cpythonTypeName(metaClass);
    QString initFunctionName = getInitFunctionName(classContext);

    // PYSIDE-510: Create a signatures string for the introspection feature.
    writeSignatureStrings(s, signatures, initFunctionName, "functions");
    s << "void init_" << initFunctionName;
    s << "(PyObject *" << enclosingObjectVariable << ")\n{\n" << indent;

    // Multiple inheritance
    QString pyTypeBasesVariable = chopType(pyTypeName) + QLatin1String("_Type_bases");
    const AbstractMetaClassList baseClasses = metaClass->typeSystemBaseClasses();
    if (metaClass->baseClassNames().size() > 1) {
        s << "PyObject *" << pyTypeBasesVariable
            << " = PyTuple_Pack(" << baseClasses.size() << ',' << '\n';
        Indentation indent(s);
        for (int i = 0, size = baseClasses.size(); i < size; ++i) {
            if (i)
                s << ",\n";
            s << "reinterpret_cast<PyObject *>("
                << cpythonTypeNameExt(baseClasses.at(i)->typeEntry()) << ')';
        }
        s << ");\n\n";
    }

    // Create type and insert it in the module or enclosing class.
    const QString typePtr = QLatin1String("_") + chopType(pyTypeName)
        + QLatin1String("_Type");

    s << typePtr << " = Shiboken::ObjectType::introduceWrapperType(\n";
    {
        Indentation indent(s);
        // 1:enclosingObject
        s << enclosingObjectVariable << ",\n";
        QString typeName;
        if (!classContext.forSmartPointer())
            typeName = metaClass->name();
        else
            typeName = classContext.preciseType().cppSignature();

        // 2:typeName
        s << "\"" << typeName << "\",\n";

        // 3:originalName
        s << "\"";
        if (!classContext.forSmartPointer()) {
            s << metaClass->qualifiedCppName();
            if (classTypeEntry->isObject())
                s << '*';
        } else {
            s << classContext.preciseType().cppSignature();
        }

        s << "\",\n";
        // 4:typeSpec
        s << '&' << chopType(pyTypeName) << "_spec,\n";

        // 5:cppObjDtor
        if (!metaClass->isNamespace() && !metaClass->hasPrivateDestructor()) {
            QString dtorClassName = metaClass->qualifiedCppName();
            if (((avoidProtectedHack() && metaClass->hasProtectedDestructor()) || classTypeEntry->isValue())
                && classContext.useWrapper()) {
                dtorClassName = classContext.wrapperName();
            }
            if (classContext.forSmartPointer())
                dtorClassName = classContext.smartPointerWrapperName();

            s << "&Shiboken::callCppDestructor< ::" << dtorClassName << " >,\n";
        } else {
            s << "0,\n";
        }

        // 6:baseType: Find a type that is not disabled.
        auto base = metaClass->isNamespace()
            ? metaClass->extendedNamespace() : metaClass->baseClass();
        if (!metaClass->isNamespace()) {
            for (; base != nullptr; base = base->baseClass()) {
                const auto ct = base->typeEntry()->codeGeneration();
                if (ct == TypeEntry::GenerateCode || ct == TypeEntry::GenerateForSubclass)
                    break;
            }
        }
        if (base) {
            s << "reinterpret_cast<SbkObjectType *>("
                << cpythonTypeNameExt(base->typeEntry()) << "),\n";
        } else {
            s << "0,\n";
        }

        // 7:baseTypes
        if (metaClass->baseClassNames().size() > 1)
            s << pyTypeBasesVariable << ',' << '\n';
        else
            s << "0,\n";

        // 8:wrapperflags
        QByteArrayList wrapperFlags;
        if (enc)
            wrapperFlags.append(QByteArrayLiteral("Shiboken::ObjectType::WrapperFlags::InnerClass"));
        if (metaClass->deleteInMainThread())
            wrapperFlags.append(QByteArrayLiteral("Shiboken::ObjectType::WrapperFlags::DeleteInMainThread"));
        if (wrapperFlags.isEmpty())
            s << '0';
        else
            s << wrapperFlags.join(" | ");
    }
    s << ");\nauto pyType = reinterpret_cast<PyTypeObject *>(" << typePtr << ");\n"
        << "InitSignatureStrings(pyType, " << initFunctionName << "_SignatureStrings);\n";

    if (usePySideExtensions())
        s << "SbkObjectType_SetPropertyStrings(reinterpret_cast<PyTypeObject *>(" << typePtr << "), "
                    << chopType(pyTypeName) << "_PropertyStrings);\n";

    if (!classContext.forSmartPointer())
        s << cpythonTypeNameExt(classTypeEntry) << '\n';
    else
        s << cpythonTypeNameExt(classContext.preciseType()) << '\n';
    s << "    = reinterpret_cast<PyTypeObject *>(" << pyTypeName << ");\n\n";

    // Register conversions for the type.
    writeConverterRegister(s, metaClass, classContext);
    s << '\n';

    // class inject-code target/beginning
    if (!classTypeEntry->codeSnips().isEmpty()) {
        writeClassCodeSnips(s, classTypeEntry->codeSnips(),
                            TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode,
                            classContext);
        s << '\n';
    }

    // Fill multiple inheritance data, if needed.
    const AbstractMetaClass *miClass = getMultipleInheritingClass(metaClass);
    if (miClass) {
        s << "MultipleInheritanceInitFunction func = ";
        if (miClass == metaClass) {
            s << multipleInheritanceInitializerFunctionName(miClass) << ";\n";
        } else {
            s << "Shiboken::ObjectType::getMultipleInheritanceFunction(reinterpret_cast<SbkObjectType *>("
                << cpythonTypeNameExt(miClass->typeEntry()) << "));\n";
        }
        s << "Shiboken::ObjectType::setMultipleInheritanceFunction("
            << cpythonTypeName(metaClass) << ", func);\n"
            << "Shiboken::ObjectType::setCastFunction(" << cpythonTypeName(metaClass)
            << ", &" << cpythonSpecialCastFunctionName(metaClass) << ");\n";
    }

    // Set typediscovery struct or fill the struct of another one
    if (metaClass->isPolymorphic() && metaClass->baseClass()) {
        s << "Shiboken::ObjectType::setTypeDiscoveryFunctionV2(" << cpythonTypeName(metaClass)
            << ", &" << cpythonBaseName(metaClass) << "_typeDiscovery);\n\n";
    }

    AbstractMetaEnumList classEnums = metaClass->enums();
    metaClass->getEnumsFromInvisibleNamespacesToBeGenerated(&classEnums);

    ErrorCode errorCode(QString::fromLatin1(""));
    writeEnumsInitialization(s, classEnums);

    if (metaClass->hasSignals())
        writeSignalInitialization(s, metaClass);

    // Write static fields
    const AbstractMetaFieldList &fields = metaClass->fields();
    for (const AbstractMetaField &field : fields) {
        if (!field.isStatic())
            continue;
        s << "PyDict_SetItemString(reinterpret_cast<PyTypeObject *>("
            << cpythonTypeName(metaClass) << ")->tp_dict, \""
            << field.name() << "\", ";
        writeToPythonConversion(s, field.type(), metaClass, field.qualifiedCppName());
        s << ");\n";
    }
    s << '\n';

    // class inject-code target/end
    if (!classTypeEntry->codeSnips().isEmpty()) {
        s << '\n';
        writeClassCodeSnips(s, classTypeEntry->codeSnips(),
                            TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode,
                            classContext);
    }

    if (usePySideExtensions()) {
        if (avoidProtectedHack() && classContext.useWrapper())
            s << classContext.wrapperName() << "::pysideInitQtMetaTypes();\n";
        else
            writeInitQtMetaTypeFunctionBody(s, classContext);
    }

    if (usePySideExtensions() && metaClass->isQObject()) {
        s << "Shiboken::ObjectType::setSubTypeInitHook(" << pyTypeName
            << ", &PySide::initQObjectSubType);\n"
            << "PySide::initDynamicMetaObject(" << pyTypeName << ", &::"
            << metaClass->qualifiedCppName() << "::staticMetaObject, sizeof(";
        if (shouldGenerateCppWrapper(metaClass))
            s << wrapperName(metaClass);
        else
            s << "::" << metaClass->qualifiedCppName();
        s << "));\n";
    }

    s << outdent << "}\n";
}

void CppGenerator::writeInitQtMetaTypeFunctionBody(TextStream &s, const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    // Gets all class name variants used on different possible scopes
    QStringList nameVariants;
    if (!context.forSmartPointer())
        nameVariants << metaClass->name();
    else
        nameVariants << context.preciseType().cppSignature();

    const AbstractMetaClass *enclosingClass = metaClass->enclosingClass();
    while (enclosingClass) {
        if (enclosingClass->typeEntry()->generateCode())
            nameVariants << (enclosingClass->name() + QLatin1String("::") + nameVariants.constLast());
        enclosingClass = enclosingClass->enclosingClass();
    }

    QString className;
    if (!context.forSmartPointer())
        className = metaClass->qualifiedCppName();
    else
        className = context.preciseType().cppSignature();

    if (!metaClass->isNamespace() && !metaClass->isAbstract())  {
        // Qt metatypes are registered only on their first use, so we do this now.
        bool canBeValue = false;
        if (!metaClass->isObjectType()) {
            // check if there's a empty ctor
            for (const auto &func : metaClass->functions()) {
                if (func->isConstructor() && !func->arguments().count()) {
                    canBeValue = true;
                    break;
                }
            }
        }

        if (canBeValue) {
            for (const QString &name : qAsConst(nameVariants)) {
                if (name == QLatin1String("iterator")) {
                    qCWarning(lcShiboken, "%s",
                              qPrintable(msgRegisterMetaTypeUnqualifiedName(context.metaClass(),
                                                                            __FILE__, __LINE__)));
                    continue;
                }
                s << "qRegisterMetaType< ::" << className << " >(\"" << name << "\");\n";
            }
        }
    }

    for (const AbstractMetaEnum &metaEnum : metaClass->enums()) {
        if (!metaEnum.isPrivate() && !metaEnum.isAnonymous()) {
            for (const QString &name : qAsConst(nameVariants)) {
                s << "qRegisterMetaType< ::"
                    << metaEnum.typeEntry()->qualifiedCppName() << " >(\""
                    << name << "::" << metaEnum.name() << "\");\n";
            }
            if (metaEnum.typeEntry()->flags()) {
                QString n = metaEnum.typeEntry()->flags()->originalName();
                s << "qRegisterMetaType< ::" << n << " >(\"" << n << "\");\n";
            }
        }
    }
}

void CppGenerator::writeTypeDiscoveryFunction(TextStream &s, const AbstractMetaClass *metaClass)
{
    QString polymorphicExpr = metaClass->typeEntry()->polymorphicIdValue();

    s << "static void *" << cpythonBaseName(metaClass)
        << "_typeDiscovery(void *cptr, SbkObjectType *instanceType)\n{\n" << indent;

    if (!polymorphicExpr.isEmpty()) {
        polymorphicExpr = polymorphicExpr.replace(QLatin1String("%1"),
                                                  QLatin1String(" reinterpret_cast< ::")
                                                  + metaClass->qualifiedCppName()
                                                  + QLatin1String(" *>(cptr)"));
        s << " if (" << polymorphicExpr << ")\n";
        {
            Indentation indent(s);
            s << "return cptr;\n";
        }
    } else if (metaClass->isPolymorphic()) {
        const AbstractMetaClassList &ancestors = metaClass->allTypeSystemAncestors();
        for (AbstractMetaClass *ancestor : ancestors) {
            if (ancestor->baseClass())
                continue;
            if (ancestor->isPolymorphic()) {
                s << "if (instanceType == reinterpret_cast<SbkObjectType *>(Shiboken::SbkType< ::"
                            << ancestor->qualifiedCppName() << " >()))\n";
                Indentation indent(s);
                s << "return dynamic_cast< ::" << metaClass->qualifiedCppName()
                            << " *>(reinterpret_cast< ::"<< ancestor->qualifiedCppName() << " *>(cptr));\n";
            } else {
                qCWarning(lcShiboken).noquote().nospace()
                    << metaClass->qualifiedCppName() << " inherits from a non polymorphic type ("
                    << ancestor->qualifiedCppName() << "), type discovery based on RTTI is "
                       "impossible, write a polymorphic-id-expression for this type.";
            }

        }
    }
    s << "return {};\n" << outdent << "}\n\n";
}

QString CppGenerator::writeSmartPointerGetterCast()
{
    return QLatin1String("const_cast<char *>(")
           + QLatin1String(SMART_POINTER_GETTER) + QLatin1Char(')');
}

void CppGenerator::writeSetattroDefinition(TextStream &s, const AbstractMetaClass *metaClass) const
{
    s << "static int " << ShibokenGenerator::cpythonSetattroFunctionName(metaClass)
        << "(PyObject *self, PyObject *name, PyObject *value)\n{\n" << indent;
    if (wrapperDiagnostics()) {
        s << R"(std::cerr << __FUNCTION__ << ' ' << Shiboken::debugPyObject(name)
        << ' ' << Shiboken::debugPyObject(value) << '\n';)" << '\n';
    }
}

inline void CppGenerator::writeSetattroDefaultReturn(TextStream &s)
{
    s << "return PyObject_GenericSetAttr(self, name, value);\n"
        << outdent << "}\n\n";
}

void CppGenerator::writeSetattroFunction(TextStream &s, AttroCheck attroCheck,
                                         const GeneratorContext &context) const
{
    Q_ASSERT(!context.forSmartPointer());
    const AbstractMetaClass *metaClass = context.metaClass();
    writeSetattroDefinition(s, metaClass);

    // PYSIDE-1019: Switch tp_dict before doing tp_setattro.
    if (usePySideExtensions())
        s << "PySide::Feature::Select(self);\n";

    // PYSIDE-803: Detect duck-punching; clear cache if a method is set.
    if (attroCheck.testFlag(AttroCheckFlag::SetattroMethodOverride)
            && context.useWrapper()) {
        s << "if (value && PyCallable_Check(value)) {\n" << indent
            << "auto plain_inst = " << cpythonWrapperCPtr(metaClass, QLatin1String("self")) << ";\n"
            << "auto inst = dynamic_cast<" << context.wrapperName() << " *>(plain_inst);\n"
            << "if (inst)\n" << indent
            << "inst->resetPyMethodCache();\n" << outdent << outdent
            << "}\n";
    }
    if (attroCheck.testFlag(AttroCheckFlag::SetattroQObject)) {
        s << "Shiboken::AutoDecRef pp(reinterpret_cast<PyObject *>(PySide::Property::getObject(self, name)));\n"
            << "if (!pp.isNull())\n";
        Indentation indent(s);
        s << "return PySide::Property::setValue(reinterpret_cast<PySideProperty *>(pp.object()), self, value);\n";
    }

    if (attroCheck.testFlag(AttroCheckFlag::SetattroUser)) {
        auto func = AbstractMetaClass::queryFirstFunction(metaClass->functions(),
                                                          FunctionQueryOption::SetAttroFunction);
        Q_ASSERT(func);
        s << "{\n";
        {
            Indentation indent(s);
            s << "auto " << CPP_SELF_VAR << " = "
                << cpythonWrapperCPtr(metaClass, QLatin1String("self")) << ";\n";
            writeClassCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionAny,
                                TypeSystem::TargetLangCode, context);
        }
        s << "}\n";
    }

    writeSetattroDefaultReturn(s);
}

void CppGenerator::writeSmartPointerSetattroFunction(TextStream &s,
                                                     const GeneratorContext &context) const
{
    Q_ASSERT(context.forSmartPointer());
    writeSetattroDefinition(s, context.metaClass());
    s << "// Try to find the 'name' attribute, by retrieving the PyObject for the corresponding C++ object held by the smart pointer.\n"
         << "PyObject *rawObj = PyObject_CallMethod(self, "
         << writeSmartPointerGetterCast() << ", 0);\n";
    s << "if (rawObj) {\n";
    {
        Indentation indent(s);
        s << "int hasAttribute = PyObject_HasAttr(rawObj, name);\n"
            << "if (hasAttribute) {\n";
        {
            Indentation indent(s);
            s << "return PyObject_GenericSetAttr(rawObj, name, value);\n";
        }
        s << "}\nPy_DECREF(rawObj);\n";
    }
    s << "}\n";
    writeSetattroDefaultReturn(s);
}

void CppGenerator::writeGetattroDefinition(TextStream &s, const AbstractMetaClass *metaClass)
{
    s << "static PyObject *" << cpythonGetattroFunctionName(metaClass)
        << "(PyObject *self, PyObject *name)\n{\n" << indent;
}

QString CppGenerator::qObjectGetAttroFunction() const
{
    static QString result;
    if (result.isEmpty()) {
        AbstractMetaClass *qobjectClass = AbstractMetaClass::findClass(classes(), qObjectT());
        Q_ASSERT(qobjectClass);
        result = QLatin1String("PySide::getMetaDataFromQObject(")
                 + cpythonWrapperCPtr(qobjectClass, QLatin1String("self"))
                 + QLatin1String(", self, name)");
    }
    return result;
}

void CppGenerator::writeGetattroFunction(TextStream &s, AttroCheck attroCheck,
                                         const GeneratorContext &context) const
{
    Q_ASSERT(!context.forSmartPointer());
    const AbstractMetaClass *metaClass = context.metaClass();
    writeGetattroDefinition(s, metaClass);

    // PYSIDE-1019: Switch tp_dict before doing tp_getattro.
    if (usePySideExtensions())
        s << "PySide::Feature::Select(self);\n";

    const QString getattrFunc = usePySideExtensions() && metaClass->isQObject()
        ? qObjectGetAttroFunction() : QLatin1String("PyObject_GenericGetAttr(self, name)");

    if (attroCheck.testFlag(AttroCheckFlag::GetattroOverloads)) {
        s << "// Search the method in the instance dict\n"
            << "if (auto ob_dict = reinterpret_cast<SbkObject *>(self)->ob_dict) {\n";
        {
            Indentation indent(s);
            s << "if (auto meth = PyDict_GetItem(ob_dict, name)) {\n";
            {
                Indentation indent(s);
                s << "Py_INCREF(meth);\n"
                    << "return meth;\n";
            }
            s << "}\n";
        }
        s << "}\n"
            << "// Search the method in the type dict\n"
            << "if (Shiboken::Object::isUserType(self)) {\n";
        {
            Indentation indent(s);
            // PYSIDE-772: Perform optimized name mangling.
            s << "Shiboken::AutoDecRef tmp(_Pep_PrivateMangle(self, name));\n"
               << "if (auto meth = PyDict_GetItem(Py_TYPE(self)->tp_dict, tmp))\n";
            {
                Indentation indent(s);
                s << "return PyFunction_Check(meth) ? SBK_PyMethod_New(meth, self) : " << getattrFunc << ";\n";
            }
        }
        s << "}\n";

        const auto &funcs = getMethodsWithBothStaticAndNonStaticMethods(metaClass);
        for (const auto &func : funcs) {
            QString defName = cpythonMethodDefinitionName(func);
            s << "static PyMethodDef non_static_" << defName << " = {\n";
            {
                Indentation indent(s);
                s << defName << ".ml_name,\n"
                    << defName << ".ml_meth,\n"
                    << defName << ".ml_flags & (~METH_STATIC),\n"
                    << defName << ".ml_doc,\n";
            }
            s << "};\n"
                << "if (Shiboken::String::compare(name, \""
                << func->definitionNames().constFirst() << "\") == 0)\n";
            Indentation indent(s);
            s << "return PyCFunction_NewEx(&non_static_" << defName << ", self, 0);\n";
        }
    }

    if (attroCheck.testFlag(AttroCheckFlag::GetattroUser)) {
        auto func = AbstractMetaClass::queryFirstFunction(metaClass->functions(),
                                                          FunctionQueryOption::GetAttroFunction);
        Q_ASSERT(func);
        s << "{\n";
        {
            Indentation indent(s);
            s << "auto " << CPP_SELF_VAR << " = "
                << cpythonWrapperCPtr(metaClass, QLatin1String("self")) << ";\n";
            writeClassCodeSnips(s, func->injectedCodeSnips(), TypeSystem::CodeSnipPositionAny,
                                TypeSystem::TargetLangCode, context);
        }
        s << "}\n";
    }

    s << "return " << getattrFunc << ";\n" << outdent << "}\n\n";
}

void CppGenerator::writeSmartPointerGetattroFunction(TextStream &s, const GeneratorContext &context)
{
    Q_ASSERT(context.forSmartPointer());
    const AbstractMetaClass *metaClass = context.metaClass();
    writeGetattroDefinition(s, metaClass);
    s << "PyObject *tmp = PyObject_GenericGetAttr(self, name);\n"
        << "if (tmp)\n";
    {
        Indentation indent(s);
        s << "return tmp;\n";
    }
    s << "if (!PyErr_ExceptionMatches(PyExc_AttributeError))\n";
    {
        Indentation indent(s);
        s << "return nullptr;\n";
    }
    s << "PyErr_Clear();\n";

    // This generates the code which dispatches access to member functions
    // and fields from the smart pointer to its pointee.
    s << "// Try to find the 'name' attribute, by retrieving the PyObject for "
                   "the corresponding C++ object held by the smart pointer.\n"
        << "if (auto rawObj = PyObject_CallMethod(self, "
        << writeSmartPointerGetterCast() << ", 0)) {\n";
    {
        Indentation indent(s);
        s << "if (auto attribute = PyObject_GetAttr(rawObj, name))\n";
        {
            Indentation indent(s);
            s << "tmp = attribute;\n";
        }
        s << "Py_DECREF(rawObj);\n";
    }
    s << "}\n"
        << "if (!tmp) {\n";
    {
        Indentation indent(s);
        s << R"(PyTypeObject *tp = Py_TYPE(self);
PyErr_Format(PyExc_AttributeError,
             "'%.50s' object has no attribute '%.400s'",
             tp->tp_name, Shiboken::String::toCString(name));
)";
    }
    s << "}\n"
        << "return tmp;\n" << outdent << "}\n\n";
}

// Write declaration and invocation of the init function for the module init
// function.
void CppGenerator::writeInitFunc(TextStream &declStr, TextStream &callStr,
                                 const QString &initFunctionName,
                                 const TypeEntry *enclosingEntry) const
{
    const bool hasParent =
        enclosingEntry && enclosingEntry->type() != TypeEntry::TypeSystemType;
    declStr << "void init_" << initFunctionName << "(PyObject *"
        << (hasParent ? "enclosingClass" : "module") << ");\n";
    callStr << "init_" << initFunctionName;
    if (hasParent) {
        callStr << "(reinterpret_cast<PyTypeObject *>("
            << cpythonTypeNameExt(enclosingEntry) << ")->tp_dict);\n";
    } else {
        callStr << "(module);\n";
    }
}

bool CppGenerator::finishGeneration()
{
    //Generate CPython wrapper file
    StringStream s_classInitDecl(TextStream::Language::Cpp);
    StringStream s_classPythonDefines(TextStream::Language::Cpp);

    QSet<Include> includes;
    StringStream s_globalFunctionImpl(TextStream::Language::Cpp);
    StringStream s_globalFunctionDef(TextStream::Language::Cpp);
    StringStream signatureStream(TextStream::Language::Cpp);

    const auto functionGroups = getGlobalFunctionGroups();
    for (auto it = functionGroups.cbegin(), end = functionGroups.cend(); it != end; ++it) {
        AbstractMetaFunctionCList overloads;
        for (const auto &func : it.value()) {
            if (!func->isModifiedRemoved()) {
                overloads.append(func);
                if (func->typeEntry())
                    includes << func->typeEntry()->include();
            }
        }

        if (overloads.isEmpty())
            continue;

        // Dummy context to satisfy the API.
        GeneratorContext classContext;
        writeMethodWrapper(s_globalFunctionImpl, overloads, classContext);
        writeSignatureInfo(signatureStream, overloads);
        writeMethodDefinition(s_globalFunctionDef, overloads);
    }

    //this is a temporary solution before new type revison implementation
    //We need move QMetaObject register before QObject
    Dependencies additionalDependencies;
    const AbstractMetaClassList &allClasses = classes();
    if (auto qObjectClass = AbstractMetaClass::findClass(allClasses, qObjectT())) {
        if (auto qMetaObjectClass = AbstractMetaClass::findClass(allClasses, qMetaObjectT())) {
            Dependency dependency;
            dependency.parent = qMetaObjectClass;
            dependency.child = qObjectClass;
            additionalDependencies.append(dependency);
        }
    }
    const AbstractMetaClassList lst = classesTopologicalSorted(additionalDependencies);

    for (const AbstractMetaClass *cls : lst){
        if (shouldGenerate(cls)) {
            writeInitFunc(s_classInitDecl, s_classPythonDefines,
                          getSimpleClassInitFunctionName(cls),
                          cls->typeEntry()->targetLangEnclosingEntry());
        }
    }

    // Initialize smart pointer types.
    const auto &smartPtrs = instantiatedSmartPointers();
    for (const AbstractMetaType &metaType : smartPtrs) {
        GeneratorContext context = contextForSmartPointer(nullptr, metaType);
        writeInitFunc(s_classInitDecl, s_classPythonDefines,
                      getInitFunctionName(context),
                      metaType.typeEntry()->targetLangEnclosingEntry());
    }

    QString moduleFileName(outputDirectory() + QLatin1Char('/') + subDirectoryForPackage(packageName()));
    moduleFileName += QLatin1Char('/') + moduleName().toLower() + QLatin1String("_module_wrapper.cpp");


    verifyDirectoryFor(moduleFileName);
    FileOut file(moduleFileName);

    TextStream &s = file.stream;
    s.setLanguage(TextStream::Language::Cpp);

    // write license comment
    s << licenseComment() << R"(
#include <sbkpython.h>
#include <shiboken.h>
#include <algorithm>
#include <signature.h>
)";
    if (usePySideExtensions()) {
        s << includeQDebug;
        s << R"(#include <pyside.h>
#include <pysideqenum.h>
#include <feature_select.h>
#include <qapp_macro.h>
)";
    }

    s << "#include \"" << getModuleHeaderFileName() << '"'  << "\n\n";
    for (const Include &include : qAsConst(includes))
        s << include;
    s << '\n';

    // Global enums
    AbstractMetaEnumList globalEnums = this->globalEnums();
    for (const AbstractMetaClass *nsp : invisibleTopNamespaces())
        nsp->getEnumsToBeGenerated(&globalEnums);

    TypeDatabase *typeDb = TypeDatabase::instance();
    const TypeSystemTypeEntry *moduleEntry = typeDb->defaultTypeSystemType();
    Q_ASSERT(moduleEntry);

    //Extra includes
    s  << '\n' << "// Extra includes\n";
    QList<Include> extraIncludes = moduleEntry->extraIncludes();
    for (const AbstractMetaEnum &cppEnum : qAsConst(globalEnums))
        extraIncludes.append(cppEnum.typeEntry()->extraIncludes());
    std::sort(extraIncludes.begin(), extraIncludes.end());
    for (const Include &inc : qAsConst(extraIncludes))
        s << inc;
    s << '\n'
       << "// Current module's type array.\n"
       << "PyTypeObject **" << cppApiVariableName() << " = nullptr;\n"
       << "// Current module's PyObject pointer.\n"
       << "PyObject *" << pythonModuleObjectName() << " = nullptr;\n"
       << "// Current module's converter array.\n"
       << "SbkConverter **" << convertersVariableName() << " = nullptr;\n";

    const CodeSnipList snips = moduleEntry->codeSnips();

    // module inject-code native/beginning
    if (!snips.isEmpty())
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::NativeCode);

    // cleanup staticMetaObject attribute
    if (usePySideExtensions()) {
        s << "void cleanTypesAttributes() {\n" << indent
            << "for (int i = 0, imax = SBK_" << moduleName()
            << "_IDX_COUNT; i < imax; i++) {\n" << indent
            << "PyObject *pyType = reinterpret_cast<PyObject *>(" << cppApiVariableName() << "[i]);\n"
            << "Shiboken::AutoDecRef attrName(Py_BuildValue(\"s\", \"staticMetaObject\"));\n"
            << "if (pyType && PyObject_HasAttr(pyType, attrName))\n" << indent
            << "PyObject_SetAttr(pyType, attrName, Py_None);\n" << outdent
            << outdent << "}\n" << outdent << "}\n";
    }

    s << "// Global functions "
        << "------------------------------------------------------------\n"
        << s_globalFunctionImpl.toString() << '\n'
        << "static PyMethodDef " << moduleName() << "_methods[] = {\n" << indent
        << s_globalFunctionDef.toString()
        << "{0} // Sentinel\n" << outdent << "};\n\n"
        << "// Classes initialization functions "
        << "------------------------------------------------------------\n"
        << s_classInitDecl.toString() << '\n';

    if (!globalEnums.isEmpty()) {
        StringStream convImpl(TextStream::Language::Cpp);

        s << "// Enum definitions "
            << "------------------------------------------------------------\n";
        for (const AbstractMetaEnum &cppEnum : qAsConst(globalEnums)) {
            if (cppEnum.isAnonymous() || cppEnum.isPrivate())
                continue;
            writeEnumConverterFunctions(s, cppEnum);
            s << '\n';
        }

        if (convImpl.size() > 0) {
            s << "// Enum converters "
                << "------------------------------------------------------------\n"
                << "namespace Shiboken\n{\n"
                << convImpl.toString() << '\n'
                << "} // namespace Shiboken\n\n";
        }

        writeFlagsNumberMethodsDefinitions(s, globalEnums);
        s << '\n';
    }

    const QStringList &requiredModules = typeDb->requiredTargetImports();
    if (!requiredModules.isEmpty())
        s << "// Required modules' type and converter arrays.\n";
    for (const QString &requiredModule : requiredModules) {
        s << "PyTypeObject **" << cppApiVariableName(requiredModule) << ";\n"
            << "SbkConverter **" << convertersVariableName(requiredModule) << ";\n";
    }

    s << "\n// Module initialization "
        << "------------------------------------------------------------\n";
    ExtendedConverterData extendedConverters = getExtendedConverters();
    if (!extendedConverters.isEmpty()) {
        s  << '\n' << "// Extended Converters.\n\n";
        for (ExtendedConverterData::const_iterator it = extendedConverters.cbegin(), end = extendedConverters.cend(); it != end; ++it) {
            const TypeEntry *externalType = it.key();
            s << "// Extended implicit conversions for "
                << externalType->qualifiedTargetLangName() << '.' << '\n';
            for (const AbstractMetaClass *sourceClass : it.value()) {
                AbstractMetaType sourceType = buildAbstractMetaTypeFromAbstractMetaClass(sourceClass);
                AbstractMetaType targetType = buildAbstractMetaTypeFromTypeEntry(externalType);
                writePythonToCppConversionFunctions(s, sourceType, targetType);
            }
        }
    }

    const QList<const CustomConversion *> &typeConversions = getPrimitiveCustomConversions();
    if (!typeConversions.isEmpty()) {
        s  << "\n// Primitive Type converters.\n\n";
        for (const CustomConversion *conversion : typeConversions) {
            s << "// C++ to Python conversion for type '" << conversion->ownerType()->qualifiedCppName() << "'.\n";
            writeCppToPythonFunction(s, conversion);
            writeCustomConverterFunctions(s, conversion);
        }
        s << '\n';
    }

    const auto &containers = instantiatedContainers();
    if (!containers.isEmpty()) {
        s << "// Container Type converters.\n\n";
        for (const AbstractMetaType &container : containers) {
            s << "// C++ to Python conversion for type '" << container.cppSignature() << "'.\n";
            writeContainerConverterFunctions(s, container);
        }
        s << '\n';
    }

    // Implicit smart pointers conversions
    const auto smartPointersList = instantiatedSmartPointers();
    if (!smartPointersList.isEmpty()) {
        s << "// SmartPointers converters.\n\n";
        for (const AbstractMetaType &smartPointer : smartPointersList) {
            s << "// C++ to Python conversion for type '" << smartPointer.cppSignature() << "'.\n";
            writeSmartPointerConverterFunctions(s, smartPointer);
        }
        s << '\n';
    }

    s << "static struct PyModuleDef moduledef = {\n"
        << "    /* m_base     */ PyModuleDef_HEAD_INIT,\n"
        << "    /* m_name     */ \"" << moduleName() << "\",\n"
        << "    /* m_doc      */ nullptr,\n"
        << "    /* m_size     */ -1,\n"
        << "    /* m_methods  */ " << moduleName() << "_methods,\n"
        << "    /* m_reload   */ nullptr,\n"
        << "    /* m_traverse */ nullptr,\n"
        << "    /* m_clear    */ nullptr,\n"
        << "    /* m_free     */ nullptr\n};\n\n";

    // PYSIDE-510: Create a signatures string for the introspection feature.
    writeSignatureStrings(s, signatureStream.toString(), moduleName(), "global functions");

    s << "extern \"C\" LIBSHIBOKEN_EXPORT PyObject *PyInit_"
        << moduleName() << "()\n{\n" << indent;

    ErrorCode errorCode(QLatin1String("nullptr"));
    // module inject-code target/beginning
    if (!snips.isEmpty())
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionBeginning, TypeSystem::TargetLangCode);

    for (const QString &requiredModule : requiredModules) {
        s << "{\n" << indent
             << "Shiboken::AutoDecRef requiredModule(Shiboken::Module::import(\"" << requiredModule << "\"));\n"
             << "if (requiredModule.isNull())\n" << indent
             << "return nullptr;\n" << outdent
             << cppApiVariableName(requiredModule)
             << " = Shiboken::Module::getTypes(requiredModule);\n"
             << convertersVariableName(requiredModule)
             << " = Shiboken::Module::getTypeConverters(requiredModule);\n" << outdent
             << "}\n\n";
    }

    int maxTypeIndex = getMaxTypeIndex() + instantiatedSmartPointers().size();
    if (maxTypeIndex) {
        s << "// Create an array of wrapper types for the current module.\n"
           << "static PyTypeObject *cppApi[SBK_" << moduleName() << "_IDX_COUNT];\n"
           << cppApiVariableName() << " = cppApi;\n\n";
    }

    s << "// Create an array of primitive type converters for the current module.\n"
        << "static SbkConverter *sbkConverters[SBK_" << moduleName()
        << "_CONVERTERS_IDX_COUNT" << "];\n"
        << convertersVariableName() << " = sbkConverters;\n\n"
        << "PyObject *module = Shiboken::Module::create(\""  << moduleName()
        << "\", &moduledef);\n\n"
        << "// Make module available from global scope\n"
        << pythonModuleObjectName() << " = module;\n\n"
        << "// Initialize classes in the type system\n"
        << s_classPythonDefines.toString();

    if (!typeConversions.isEmpty()) {
        s << '\n';
        for (const CustomConversion *conversion : typeConversions) {
            writePrimitiveConverterInitialization(s, conversion);
            s << '\n';
        }
    }

    if (!containers.isEmpty()) {
        s << '\n';
        for (const AbstractMetaType &container : containers) {
            writeContainerConverterInitialization(s, container);
            s << '\n';
        }
    }

    if (!smartPointersList.isEmpty()) {
        s << '\n';
        for (const AbstractMetaType &smartPointer : smartPointersList) {
            writeSmartPointerConverterInitialization(s, smartPointer);
            s << '\n';
        }
    }

    if (!extendedConverters.isEmpty()) {
        s << '\n';
        for (ExtendedConverterData::const_iterator it = extendedConverters.cbegin(), end = extendedConverters.cend(); it != end; ++it) {
            writeExtendedConverterInitialization(s, it.key(), it.value());
            s << '\n';
        }
    }

    writeEnumsInitialization(s, globalEnums);

    s << "// Register primitive types converters.\n";
    const PrimitiveTypeEntryList &primitiveTypeList = primitiveTypes();
    for (const PrimitiveTypeEntry *pte : primitiveTypeList) {
        if (!pte->generateCode() || !pte->isCppPrimitive())
            continue;
        const TypeEntry *referencedType = pte->basicReferencedTypeEntry();
        if (!referencedType)
            continue;
        QString converter = converterObject(referencedType);
        QStringList cppSignature = pte->qualifiedCppName().split(QLatin1String("::"), Qt::SkipEmptyParts);
        while (!cppSignature.isEmpty()) {
            QString signature = cppSignature.join(QLatin1String("::"));
            s << "Shiboken::Conversions::registerConverterName("
                << converter << ", \"" << signature << "\");\n";
            cppSignature.removeFirst();
        }
    }

    s << '\n';
    if (maxTypeIndex)
        s << "Shiboken::Module::registerTypes(module, " << cppApiVariableName() << ");\n";
    s << "Shiboken::Module::registerTypeConverters(module, " << convertersVariableName() << ");\n";

    s << "\nif (PyErr_Occurred()) {\n" << indent
        << "PyErr_Print();\n"
        << "Py_FatalError(\"can't initialize module " << moduleName() << "\");\n"
        << outdent << "}\n";

    // module inject-code target/end
    if (!snips.isEmpty())
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::TargetLangCode);

    // module inject-code native/end
    if (!snips.isEmpty())
        writeCodeSnips(s, snips, TypeSystem::CodeSnipPositionEnd, TypeSystem::NativeCode);

    if (usePySideExtensions()) {
        for (const AbstractMetaEnum &metaEnum : qAsConst(globalEnums))
            if (!metaEnum.isAnonymous()) {
                s << "qRegisterMetaType< ::" << metaEnum.typeEntry()->qualifiedCppName()
                  << " >(\"" << metaEnum.name() << "\");\n";
            }

        // cleanup staticMetaObject attribute
        s << "PySide::registerCleanupFunction(cleanTypesAttributes);\n\n";
    }

    // finish the rest of __signature__ initialization.
    s << "FinishSignatureInitialization(module, " << moduleName()
        << "_SignatureStrings);\n"
        << "\nreturn module;\n" << outdent << "}\n";

    return file.done() != FileOut::Failure;
}

static ArgumentOwner getArgumentOwner(const AbstractMetaFunctionCPtr &func, int argIndex)
{
    ArgumentOwner argOwner = func->argumentOwner(func->ownerClass(), argIndex);
    if (argOwner.index == ArgumentOwner::InvalidIndex)
        argOwner = func->argumentOwner(func->declaringClass(), argIndex);
    return argOwner;
}

bool CppGenerator::writeParentChildManagement(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                              int argIndex, bool useHeuristicPolicy) const
{
    const int numArgs = func->arguments().count();
    bool ctorHeuristicEnabled = func->isConstructor() && useCtorHeuristic() && useHeuristicPolicy;

    const auto &groups = func->implementingClass()
        ? getFunctionGroups(func->implementingClass())
        : getGlobalFunctionGroups();
    bool usePyArgs = pythonFunctionWrapperUsesListOfArguments(OverloadData(groups[func->name()], this));

    ArgumentOwner argOwner = getArgumentOwner(func, argIndex);
    ArgumentOwner::Action action = argOwner.action;
    int parentIndex = argOwner.index;
    int childIndex = argIndex;
    if (ctorHeuristicEnabled && argIndex > 0 && numArgs) {
        const AbstractMetaArgument &arg = func->arguments().at(argIndex-1);
        if (arg.name() == QLatin1String("parent") && arg.type().isObjectType()) {
            action = ArgumentOwner::Add;
            parentIndex = argIndex;
            childIndex = -1;
        }
    }

    QString parentVariable;
    QString childVariable;
    if (action != ArgumentOwner::Invalid) {
        if (!usePyArgs && argIndex > 1)
            qCWarning(lcShiboken).noquote().nospace()
                << "Argument index for parent tag out of bounds: " << func->signature();

        if (action == ArgumentOwner::Remove) {
            parentVariable = QLatin1String("Py_None");
        } else {
            if (parentIndex == 0) {
                parentVariable = QLatin1String(PYTHON_RETURN_VAR);
            } else if (parentIndex == -1) {
                parentVariable = QLatin1String("self");
            } else {
                parentVariable = usePyArgs
                    ? pythonArgsAt(parentIndex - 1) : QLatin1String(PYTHON_ARG);
            }
        }

        if (childIndex == 0) {
            childVariable = QLatin1String(PYTHON_RETURN_VAR);
        } else if (childIndex == -1) {
            childVariable = QLatin1String("self");
        } else {
            childVariable = usePyArgs
                ? pythonArgsAt(childIndex - 1) : QLatin1String(PYTHON_ARG);
        }

        s << "Shiboken::Object::setParent(" << parentVariable << ", " << childVariable << ");\n";
        return true;
    }

    return false;
}

void CppGenerator::writeParentChildManagement(TextStream &s, const AbstractMetaFunctionCPtr &func,
                                              bool useHeuristicForReturn) const
{
    const int numArgs = func->arguments().count();

    // -1    = return value
    //  0    = self
    //  1..n = func. args.
    for (int i = -1; i <= numArgs; ++i)
        writeParentChildManagement(s, func, i, useHeuristicForReturn);

    if (useHeuristicForReturn)
        writeReturnValueHeuristics(s, func);
}

void CppGenerator::writeReturnValueHeuristics(TextStream &s, const AbstractMetaFunctionCPtr &func) const
{
    const  AbstractMetaType &type = func->type();
    if (!useReturnValueHeuristic()
        || !func->ownerClass()
        || type.isVoid()
        || func->isStatic()
        || func->isConstructor()
        || !func->typeReplaced(0).isEmpty()) {
        return;
    }

    ArgumentOwner argOwner = getArgumentOwner(func, ArgumentOwner::ReturnIndex);
    if (argOwner.action == ArgumentOwner::Invalid || argOwner.index != ArgumentOwner::ThisIndex) {
        if (type.isPointerToWrapperType())
            s << "Shiboken::Object::setParent(self, " << PYTHON_RETURN_VAR << ");\n";
    }
}

void CppGenerator::writeHashFunction(TextStream &s, const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    const char hashType[] = "Py_hash_t";
    s << "static " << hashType << ' ' << cpythonBaseName(metaClass)
        << "_HashFunc(PyObject *self) {\n" << indent;
    writeCppSelfDefinition(s, context);
    s << "return " << hashType << '('
        << metaClass->typeEntry()->hashFunction() << '(';
    if (!metaClass->isObjectType())
        s << '*';
    s << CPP_SELF_VAR << "));\n"
        << outdent << "}\n\n";
}

void CppGenerator::writeDefaultSequenceMethods(TextStream &s,
                                               const GeneratorContext &context) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    ErrorCode errorCode(0);

    // __len__
    s << "Py_ssize_t " << cpythonBaseName(metaClass->typeEntry())
        << "__len__(PyObject *self)\n{\n" << indent;
    writeCppSelfDefinition(s, context);
    s << "return " << CPP_SELF_VAR << "->size();\n"
        << outdent << "}\n";

    // __getitem__
    s << "PyObject *" << cpythonBaseName(metaClass->typeEntry())
        << "__getitem__(PyObject *self, Py_ssize_t _i)\n{\n" << indent;
    writeCppSelfDefinition(s, context);
    writeIndexError(s, QLatin1String("index out of bounds"));

    QString value;
    s << metaClass->qualifiedCppName() << "::const_iterator _item = "
        << CPP_SELF_VAR << "->begin();\n"
        << "std::advance(_item, _i);\n";

    const AbstractMetaTypeList &instantiations = metaClass->templateBaseClassInstantiations();
    if (instantiations.isEmpty()) {
        qFatal("shiboken: %s: Internal error, no instantiations of \"%s\" were found.",
               __FUNCTION__, qPrintable(metaClass->qualifiedCppName()));
    }
    const AbstractMetaType &itemType = instantiations.constFirst();

    s << "return ";
    writeToPythonConversion(s, itemType, metaClass, QLatin1String("*_item"));
    s << ";\n" << outdent << "}\n";

    // __setitem__
    ErrorCode errorCode2(-1);
    s << "int " << cpythonBaseName(metaClass->typeEntry())
        << "__setitem__(PyObject *self, Py_ssize_t _i, PyObject *pyArg)\n{\n"
        << indent;
    writeCppSelfDefinition(s, context);
    writeIndexError(s, QLatin1String("list assignment index out of range"));

    s << "PythonToCppFunc " << PYTHON_TO_CPP_VAR << ";\n"
        << "if (!";
    writeTypeCheck(s, itemType, QLatin1String("pyArg"), isNumber(itemType.typeEntry()));
    s << ") {\n";
    {
        Indentation indent(s);
        s << "PyErr_SetString(PyExc_TypeError, \"attributed value with wrong type, '"
            << itemType.name() << "' or other convertible type expected\");\n"
            << "return -1;\n";
    }
    s << "}\n";
    writeArgumentConversion(s, itemType, QLatin1String("cppValue"), QLatin1String("pyArg"), metaClass);

    s << metaClass->qualifiedCppName() << "::iterator _item = "
        << CPP_SELF_VAR << "->begin();\n"
        << "std::advance(_item, _i);\n"
        << "*_item = cppValue;\n";

    s << "return {};\n" << outdent << "}\n";
}
void CppGenerator::writeIndexError(TextStream &s, const QString &errorMsg)
{
    s << "if (_i < 0 || _i >= (Py_ssize_t) " << CPP_SELF_VAR << "->size()) {\n";
    {
        Indentation indent(s);
        s << "PyErr_SetString(PyExc_IndexError, \"" << errorMsg << "\");\n"
            << returnStatement(m_currentErrorCode) << '\n';
    }
    s << "}\n";
}

QString CppGenerator::writeReprFunction(TextStream &s,
                                        const GeneratorContext &context,
                                        uint indirections) const
{
    const AbstractMetaClass *metaClass = context.metaClass();
    QString funcName = cpythonBaseName(metaClass) + reprFunction();
    s << "extern \"C\"\n{\n"
        << "static PyObject *" << funcName << "(PyObject *self)\n{\n" << indent;
    writeCppSelfDefinition(s, context);
    s << R"(QBuffer buffer;
buffer.open(QBuffer::ReadWrite);
QDebug dbg(&buffer);
dbg << )";
    if (metaClass->typeEntry()->isValue() || indirections == 0)
         s << '*';
    s << CPP_SELF_VAR << R"(;
buffer.close();
QByteArray str = buffer.data();
int idx = str.indexOf('(');
if (idx >= 0)
)";
    {
        Indentation indent(s);
        s << "str.replace(0, idx, Py_TYPE(self)->tp_name);\n";
    }
    s << "str = str.trimmed();\n"
        << "PyObject *mod = PyDict_GetItem(Py_TYPE(self)->tp_dict, Shiboken::PyMagicName::module());\n";
    // PYSIDE-595: The introduction of heap types has the side effect that the module name
    // is always prepended to the type name. Therefore the strchr check:
    s << "if (mod && !strchr(str, '.'))\n";
    {
        Indentation indent(s);
        s << "return Shiboken::String::fromFormat(\"<%s.%s at %p>\", Shiboken::String::toCString(mod), str.constData(), self);\n";
    }
    s << "else\n";
    {
        Indentation indent(s);
        s << "return Shiboken::String::fromFormat(\"<%s at %p>\", str.constData(), self);\n";
    }
    s << outdent << "}\n} // extern C\n\n";
    return funcName;
}
