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

#ifndef CONVERTERGENERATOR_H
#define CONVERTERGENERATOR_H

// #include <QRegExp>
#include <QtCore/QSet>
#include "boostpythongenerator.h"

/**
*   Generator for convertions between python collections and Qt collections.
*
*   It generates a file called converter_register_MODULENAME.hpp with only one
*   function called register_type_converters_MODULENAME, where MODULENAME is the current module name.
*   QPair are converted to python tuples, QList, QVector and QLinkedList to python lists, QHash and QMap to python dicts.
*/
class ConverterGenerator : public BoostPythonGenerator
{
public:
    ConverterGenerator();

    const char* name() const
    {
        return "ConverterGenerator";
    }

protected:
    void generateClass(QTextStream& s, const AbstractMetaClass* clazz)
    {
    }

    void finishGeneration();
    QString fileNameForClass(const AbstractMetaClass* cppClass) const
    {
        return QString();
    }
private:
    void checkFunctionMetaTypes(AbstractMetaFunction* func);
    void checkMetaType(const QString& cppSignature);
    QString extractTemplateArgument(const QString& templateParams);

    void writeConverterRegistration(QTextStream& out, const QString& func_name, const QString& type, const QSet<QString>& params);

    typedef QPair<QString, QSet<QString>* > Conversion;
    typedef QList<Conversion> ConversionList;
    ConversionList m_conversions;
    QSet<QString> m_qpairTypes;
    QSet<QString> m_qlistTypes;
    QSet<QString> m_qvectorTypes;
    QSet<QString> m_qmapTypes;
    QSet<QString> m_qhashTypes;
    QSet<QString> m_qmultiMapTypes;
};

#endif // CONVERSIONGENERATOR_H

