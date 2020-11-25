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

#ifndef TEXTSTREAM_H
#define TEXTSTREAM_H

#include <QtCore/QTextStream>

/// A text stream based on QTextStream with built-in indent.
class TextStream
{
public:
    Q_DISABLE_COPY_MOVE(TextStream)

    using ManipulatorFunc = void(TextStream &);

    enum class Language
    {
        None, Cpp
    };

    enum class CharClass
    {
        Other, NewLine, Hash
    };

    explicit TextStream(QIODevice *device, Language l = Language::None);
    explicit TextStream(QString *string, Language l = Language::None);
    explicit TextStream(QByteArray *array, Language l = Language::None);
    virtual ~TextStream();

    Language language() const { return m_language; }
    void setLanguage(const Language &language) { m_language = language; }

    bool isIndentationEnabled() const { return m_indentationEnabled; }
    void setIndentationEnabled(bool m)
    { m_indentationEnabled = m; }

    int tabWidth() const { return m_tabWidth; }
    void setTabWidth(int tabWidth) { m_tabWidth = tabWidth; }

    void setFieldWidth(int f) { m_str.setFieldWidth(f); }
    int fieldWidth() const { return m_str.fieldWidth(); }

    int indentation() const { return m_indentation; }
    void setIndentation(int i);

    void indent(int n = 1) { m_indentation += n; }
    void outdent(int n = 1);

    // QTextStream API
    qint64 pos() const;
    QTextStream::FieldAlignment fieldAlignment() const
    { return m_str.fieldAlignment();  }
    void setFieldAlignment(QTextStream::FieldAlignment al)
    { m_str.setFieldAlignment(al); }
    void setString(QString *string, QIODeviceBase::OpenMode openMode = QIODeviceBase::ReadWrite)
    { m_str.setString(string, openMode); }
    QString *string() const { return m_str.string(); }
    void flush() { m_str.flush(); }
    void setDevice(QIODevice *device) { m_str.setDevice(device); }
    QIODevice *device() const { return m_str.device(); }
    QTextStream &textStream() { return m_str; }

    // Last character written, works only for streams on strings
    QChar lastChar() const;

    void putString(QStringView v);
    void putChar(QChar c) { putCharHelper(c); }
    void putString(const char *s);
    void putChar(char c) { putCharHelper(c); }

    void putInt(int t);
    void putSizeType(qsizetype t);

    TextStream &operator<<(QStringView v) { putString(v); return *this; }
    TextStream &operator<<(QChar c) { putChar(c); return *this; }
    TextStream &operator<<(const char *s) { putString(s); return *this; }
    TextStream &operator<<(char c) { putChar(c); return *this; }
    TextStream &operator<<(int t) { putInt(t); return *this; }
    TextStream &operator<<(qsizetype t) { putSizeType(t); return *this; }

    inline TextStream &operator<<(QTextStreamManipulator m) { m_str << m; return *this; }
    inline TextStream &operator<<(ManipulatorFunc f) { f(*this); return *this; }

    void putRepetitiveChars(char c, int count);

protected:
    void setLastCharClass(CharClass c);

private:
    void writeIndent();
    void checkIndent(CharClass upComingCharClass);
    template <class Char>
    void putCharHelper(Char c);

    QTextStream m_str;
    CharClass m_lastCharClass = CharClass::NewLine;
    int m_tabWidth = 4;
    int m_indentation = 0;
    bool m_indentationEnabled = true;
    Language m_language;
};

/// Stream into a string (cf std::ostringstream)
class StringStream : public TextStream
{
public:
    StringStream(Language l = Language::None);

    qsizetype size() const { return m_buffer.size(); }
    void clear();

    const QString &toString() const { return m_buffer; }
    operator const QString &() const { return m_buffer; }

private:
    QString m_buffer;
};

void indent(TextStream &s);
void outdent(TextStream &s);
void enableIndent(TextStream &s);
void disableIndent(TextStream &s);
// Works only for streams on strings
void ensureEndl(TextStream &s);

/// Format an aligned field
template <class T>
class AlignedField
{
public:
    explicit AlignedField(T value, int fieldWidth,
                          QTextStream::FieldAlignment a = QTextStream::AlignLeft) :
        m_value(value), m_fieldWidth(fieldWidth), m_alignment(a)
    {
    }

    void put(TextStream &s) const
    {
        const int oldFieldWidth = s.fieldWidth();
        const auto oldFieldAlignment = s.fieldAlignment();
        s.setFieldWidth(m_fieldWidth);
        s.setFieldAlignment(m_alignment);
        const auto oldPos = s.pos();
        s << m_value;
        // Ensure something is written when an empty string is encountered
        if (oldPos == s.pos())
            s << ' ';
        s.setFieldAlignment(oldFieldAlignment);
        s.setFieldWidth(oldFieldWidth);
    }

private:
    const T m_value;
    const int m_fieldWidth;
    const QTextStream::FieldAlignment m_alignment;
};

template <class T>
TextStream &operator<<(TextStream &str, const AlignedField<T> &fa)
{
    fa.put(str);
    return str;
}

class Indentation
{
public:
    Q_DISABLE_COPY_MOVE(Indentation)

    Indentation(TextStream &s, int n = 1) : m_s(s), m_n(n) { m_s.indent(m_n); }
    ~Indentation() { m_s.outdent(m_n); }

private:
    TextStream &m_s;
    const int m_n;
};

#endif // TEXTSTREAM_H
