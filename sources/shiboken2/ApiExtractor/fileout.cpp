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

#include "fileout.h"
#include "reporthandler.h"

#include <QtCore/QTextCodec>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <cstdio>

bool FileOut::dummy = false;
bool FileOut::diff = false;

#ifdef Q_OS_LINUX
static const char colorDelete[] = "\033[31m";
static const char colorAdd[] = "\033[32m";
static const char colorInfo[] = "\033[36m";
static const char colorReset[] = "\033[0m";
#else
static const char colorDelete[] = "";
static const char colorAdd[] = "";
static const char colorInfo[] = "";
static const char colorReset[] = "";
#endif

FileOut::FileOut(QString n):
        name(n),
        stream(&tmp),
        isDone(false)
{}

static int* lcsLength(QList<QByteArray> a, QList<QByteArray> b)
{
    const int height = a.size() + 1;
    const int width = b.size() + 1;

    int *res = new int[width * height];

    for (int row = 0; row < height; row++)
        res[width * row] = 0;

    for (int col = 0; col < width; col++)
        res[col] = 0;

    for (int row = 1; row < height; row++) {
        for (int col = 1; col < width; col++) {
            if (a[row-1] == b[col-1])
                res[width * row + col] = res[width * (row-1) + col-1] + 1;
            else
                res[width * row + col] = qMax(res[width * row     + col-1],
                                              res[width * (row-1) + col]);
        }
    }
    return res;
}

enum Type {
    Add,
    Delete,
    Unchanged
};

struct Unit
{
    Unit(Type type, int pos) :
            type(type),
            start(pos),
            end(pos) {}

    Type type;
    int start;
    int end;

    void print(QList<QByteArray> a, QList<QByteArray> b)
    {
            if (type == Unchanged) {
                if ((end - start) > 9) {
                    for (int i = start; i <= start + 2; i++)
                        std::printf("  %s\n", a[i].data());
                    std::printf("%s=\n= %d more lines\n=%s\n", colorInfo, end - start - 6, colorReset);
                    for (int i = end - 2; i <= end; i++)
                        std::printf("  %s\n", a[i].data());
                } else {
                    for (int i = start; i <= end; i++)
                        std::printf("  %s\n", a[i].data());
                }
            } else if (type == Add) {
                std::printf("%s", colorAdd);
                for (int i = start; i <= end; i++)
                    std::printf("+ %s\n", b[i].data());
                std::printf("%s", colorReset);
            } else if (type == Delete) {
                std::printf("%s", colorDelete);
                for (int i = start; i <= end; i++)
                    std::printf("- %s\n", a[i].data());
                std::printf("%s", colorReset);
            }
    }
};

static QList<Unit*> *unitAppend(QList<Unit*> *res, Type type, int pos)
{
    if (!res) {
        res = new QList<Unit*>;
        res->append(new Unit(type, pos));
        return res;
    }

    Unit *last = res->last();
    if (last->type == type)
        last->end = pos;
    else
        res->append(new Unit(type, pos));

    return res;
}

static QList<Unit*> *diffHelper(int *lcs, QList<QByteArray> a, QList<QByteArray> b, int row, int col)
{
    if (row > 0 && col > 0 && (a[row-1] == b[col-1])) {
        return unitAppend(diffHelper(lcs, a, b, row - 1, col - 1), Unchanged, row - 1);
    } else {
        int width = b.size() + 1;
        if ((col > 0)
            && (row == 0 || lcs[width * row + col-1] >= lcs[width *(row-1) + col])) {
            return unitAppend(diffHelper(lcs, a, b, row, col - 1), Add, col - 1);
        } else if ((row > 0)
            && (col == 0 || lcs[width * row + col-1] < lcs[width *(row-1) + col])) {
            return unitAppend(diffHelper(lcs, a, b, row - 1, col), Delete, row - 1);
        }
    }
    delete lcs;
    return 0;
}

static void diff(QList<QByteArray> a, QList<QByteArray> b)
{
    QList<Unit*> *res = diffHelper(lcsLength(a, b), a, b, a.size(), b.size());
    for (int i = 0; i < res->size(); i++) {
        Unit *unit = res->at(i);
        unit->print(a, b);
        delete(unit);
    }
    delete(res);
}


FileOut::State FileOut::done()
{
    Q_ASSERT(!isDone);
    if (name.isEmpty())
        return Failure;

    isDone = true;
    bool fileEqual = false;
    QFile fileRead(name);
    QFileInfo info(fileRead);
    stream.flush();
    QByteArray original;
    if (info.exists() && (diff || (info.size() == tmp.size()))) {
        if (!fileRead.open(QIODevice::ReadOnly)) {
            qCWarning(lcShiboken).noquote().nospace()
                << QStringLiteral("failed to open file '%1' for reading")
                                  .arg(QDir::toNativeSeparators(fileRead.fileName()));
            return Failure;
        }

        original = fileRead.readAll();
        fileRead.close();
        fileEqual = (original == tmp);
    }

    if (fileEqual)
        return Unchanged;

    if (!FileOut::dummy) {
        QDir dir(info.absolutePath());
        if (!dir.mkpath(dir.absolutePath())) {
            qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("unable to create directory '%1'")
                       .arg(QDir::toNativeSeparators(dir.absolutePath()));
            return Failure;
        }

        QFile fileWrite(name);
        if (!fileWrite.open(QIODevice::WriteOnly)) {
            qCWarning(lcShiboken).noquote().nospace()
                    << QStringLiteral("failed to open file '%1' for writing")
                       .arg(QDir::toNativeSeparators(fileWrite.fileName()));
            return Failure;
        }
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        stream.setCodec(codec);
        stream.setDevice(&fileWrite);
        stream << tmp;
    }
    if (diff) {
        std::printf("%sFile: %s%s\n", colorInfo, qPrintable(name), colorReset);
        ::diff(original.split('\n'), tmp.split('\n'));
        std::printf("\n");
    }

    return Success;
}
