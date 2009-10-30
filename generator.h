/*
 * This file is part of the API Extractor project.
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

#ifndef GENERATOR_H
#define GENERATOR_H

#include <QtCore/QObject>
#include <QtCore/QDir>
#include <QtCore/QLinkedList>
#include <apiextractor/abstractmetalang.h>

class ApiExtractor;
class AbstractMetaBuilder;
class QFile;

#define EXPORT_GENERATOR_PLUGIN(X)\
extern "C" Q_DECL_EXPORT GeneratorList getGenerators()\
{\
    return GeneratorList() << X;\
}\

QTextStream& formatCode(QTextStream &s, const QString& code, Indentor &indentor);

/**
 *   Base class for all generators. The default implementations does nothing,
 *   you must subclass this to create your own generators.
 */
class Generator
{
public:
    /// Optiosn used around the generator code
    enum Option {
        NoOption                 = 0x00000000,
        BoxedPrimitive           = 0x00000001,
        ExcludeConst             = 0x00000002,
        ExcludeReference         = 0x00000004,
        UseNativeIds             = 0x00000008,

        EnumAsInts               = 0x00000010,
        SkipName                 = 0x00000020,
        NoCasts                  = 0x00000040,
        SkipReturnType           = 0x00000080,
        OriginalName             = 0x00000100,
        ShowStatic               = 0x00000200,
        UnderscoreSpaces         = 0x00000400,
        ForceEnumCast            = 0x00000800,
        ArrayAsPointer           = 0x00001000,
        VirtualCall              = 0x00002000,
        SkipTemplateParameters   = 0x00004000,
        SkipAttributes           = 0x00008000,
        OriginalTypeDescription  = 0x00010000,
        SkipRemovedArguments     = 0x00020000,
        IncludeDefaultExpression = 0x00040000,
        NoReturnStatement        = 0x00080000,
        NoBlockedSlot            = 0x00100000,

        SuperCall                = 0x00200000,

        GlobalRefJObject         = 0x00100000,

        SkipDefaultValues        = 0x00400000,

        WriteSelf                = 0x00800000,
        ExcludeMethodConst       = 0x01000000,

        ForceValueType           = ExcludeReference | ExcludeConst
    };
    Q_DECLARE_FLAGS(Options, Option)

    Generator();
    virtual ~Generator();

    bool setup(const ApiExtractor& extractor, const QMap<QString, QString> args);

    virtual QMap<QString, QString> options() const;

    /// Returns the classes used to generate the binding code.
    AbstractMetaClassList classes() const;

    AbstractMetaFunctionList globalFunctions() const;

    AbstractMetaEnumList globalEnums() const;

    QList<const PrimitiveTypeEntry*> primitiveTypes() const;

    QList<const ContainerTypeEntry*> containerTypes() const;

    /// Returns the output directory
    QString outputDirectory() const;

    /// Set the output directory
    void setOutputDirectory(const QString &outDir);

    /**
    *   Start the code generation, be sure to call setClasses before callign this method.
    *   For each class it creates a QTextStream, call the write method with the current
    *   class and the associated text stream, then write the text stream contents if needed.
    *   \see #write
    */
    void generate();

    /// Returns the number of generated items
    int numGenerated()
    {
        return m_numGenerated;
    }

    /// Returns the number of generated items written
    int numGeneratedAndWritten()
    {
        return m_numGeneratedWritten;
    }

    virtual const char* name() const = 0;

    /// Returns true if the generator should generate any code for the AbstractMetaClass
    virtual bool shouldGenerate(const AbstractMetaClass *) const;

    /// Returns the subdirectory used to write the binding code of an AbstractMetaClass.
    virtual QString subDirectoryForClass(const AbstractMetaClass* clazz) const;

    /**
    *   Translate metatypes to binding source format.
    *   \param metatype a pointer to metatype
    *   \param context the current meta class
    *   \param option some extra options
    *   \return the metatype translated to binding source format
    */
    QString translateType(const AbstractMetaType *metatype,
                          const AbstractMetaClass *context,
                          Options options = NoOption) const;

    /**
    *   Function used to write the fucntion arguments on the class buffer.
    *   \param s the class output buffer
    *   \param metafunction the pointer to metafunction information
    *   \param count the number of function arguments
    *   \param options some extra options used during the parser
    */
    virtual void writeFunctionArguments(QTextStream &s,
                                        const AbstractMetaFunction *metafunction,
                                        Options options = NoOption) const = 0;

    virtual void writeArgumentNames(QTextStream &s,
                                    const AbstractMetaFunction *metafunction,
                                    Options options = NoOption) const = 0;

    void replaceTemplateVariables(QString &code, const AbstractMetaFunction *func);

    bool hasDefaultConstructor(const AbstractMetaType *type);

    // QtScript
    QSet<QString> qtMetaTypeDeclaredTypeNames() const
    {
        return m_qmetatypeDeclaredTypenames;
    }

    /**
    *   Returns the license comment to be prepended to each source file generated.
    */
    QString licenseComment()
    {
        return m_licenseComment;
    }

    /**
    *   Sets the license comment to be prepended to each source file generated.
    */
    void setLicenseComment(const QString &licenseComment)
    {
        m_licenseComment = licenseComment;
    }

    /**
     *   Returns the package name.
     */
    QString packageName()
    {
        return m_packageName;
    }

    /**
     *   Sets the package name.
     */
    void setPackageName(const QString &packageName)
    {
        m_packageName = packageName;
    }

    /**
     *    Retrieves the name of the currently processed module. While package name
     *    is a complete package idetification, e.g. 'PySide.QtCore', a module name
     *    represents the last part of the package, e.g. 'QtCore'.
     *    If the target language separates the modules with characters other than
     *    dots ('.') the generator subclass must overload this method.
     *    /return a string representing the last part of a package name
     */
    virtual QString moduleName() const
    {
        return QString(m_packageName).remove(0, m_packageName.lastIndexOf('.') + 1);
    }

    /// returns the code snips of a function
    CodeSnipList getCodeSnips(const AbstractMetaFunction *func);

protected:
    QString m_packageName;

    /**
     *   Returns the file name used to write the binding code of an AbstractMetaClass.
     *   /param metaClass the AbstractMetaClass for which the file name must be
     *   returned
     *   /return the file name used to write the binding code for the class
     */
    virtual QString fileNameForClass(const AbstractMetaClass* metaClass) const = 0;

    static FunctionModificationList functionModifications(const AbstractMetaFunction *meta_function);
    AbstractMetaFunctionList filterFunctions(const AbstractMetaClass *cppClass);
    AbstractMetaFunctionList queryFunctions(const AbstractMetaClass *cpp_class, bool all_function = false);
    AbstractMetaFunctionList queryGlobalOperators(const AbstractMetaClass *cpp_class);
    AbstractMetaFunctionList sortConstructor(AbstractMetaFunctionList list);

    virtual bool doSetup(const QMap<QString, QString>& args) = 0;

    /**
     *    Returns the subdirectory path for a given package
     *    (aka module, aka library) name.
     *    If the target language separates the package modules with characters other
     *    than dots ('.') the generator subclass must overload this method.
     *    /param packageName complete package name for which to return the subdirectory path
     *    or nothing the use the name of the currently processed package
     *    /return a string representing the subdirectory path for the given package
     */
    virtual QString subDirectoryForPackage(QString packageName = QString()) const;

    /**
     *   Write the bindding code for an AbstractMetaClass.
     *   This is called by the default implementation of generate method.
     *   \param  s   text stream to write the generated output
     *   \param  metaClass  the class that should be generated
     */
    virtual void generateClass(QTextStream& s, const AbstractMetaClass* metaClass) = 0;
    virtual void finishGeneration() = 0;

    void verifyDirectoryFor(const QFile &file);

    int m_numGenerated;
    int m_numGeneratedWritten;

private:
    AbstractMetaClassList m_classes;
    AbstractMetaFunctionList m_globalFunctions;
    AbstractMetaEnumList m_globalEnums;
    QString m_outDir;

    QList<const PrimitiveTypeEntry*> m_primitiveTypes;
    QList<const ContainerTypeEntry*> m_containerTypes;

    // QtScript
    QSet<QString> m_qmetatypeDeclaredTypenames;

    // License comment
    QString m_licenseComment;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Generator::Options)
typedef QLinkedList<Generator*> GeneratorList;

/**
* Utility class to store the identation level, use it in a QTextStream.
*/
class Indentor
{
public:
    Indentor():
            indent(0) {}
    int indent;
};

/**
*   Class that use the RAII idiom to set and unset the identation level.
*/
class Indentation
{
public:
    Indentation(Indentor &indentor) : indentor(indentor)
    {
        indentor.indent++;
    }
    ~Indentation()
    {
        indentor.indent--;
    }

private:
    Indentor &indentor;
};

inline QTextStream &operator <<(QTextStream &s, const Indentor &indentor)
{
    for (int i = 0; i < indentor.indent; ++i)
        s << "    ";
    return s;
}

#endif // GENERATOR_H

