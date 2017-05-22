/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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


#ifndef FASTLIST_H
#define FASTLIST_H

#include "smallobject.h"

template <typename Tp>
struct ListNode {
    Tp element;
    int index;
    mutable const ListNode<Tp> *next;

    static ListNode *create(const Tp &element, pool *p) {
        ListNode<Tp> *node = new(p->allocate(sizeof(ListNode), strideof(ListNode))) ListNode();
        node->element = element;
        node->index = 0;
        node->next = node;

        return node;
    }

    static ListNode *create(const ListNode *n1, const Tp &element, pool *p) {
        ListNode<Tp> *n2 = ListNode::create(element, p);

        n2->index = n1->index + 1;
        n2->next = n1->next;
        n1->next = n2;

        return n2;
    }

    inline ListNode<Tp>() { }

    inline const ListNode<Tp> *at(int index) const {
        const ListNode<Tp> *node = this;
        while (index != node->index)
            node = node->next;

        return node;
    }

    inline bool hasNext() const {
        return index < next->index;
    }

    inline int count() const {
        return 1 + toBack()->index;
    }

    inline const ListNode<Tp> *toFront() const {
        return toBack()->next;
    }

    inline const ListNode<Tp> *toBack() const {
        const ListNode<Tp> *node = this;
        while (node->hasNext())
            node = node->next;

        return node;
    }
};

template <class Tp>
inline const ListNode<Tp> *snoc(const ListNode<Tp> *list,
                                const Tp &element, pool *p)
{
    if (!list)
        return ListNode<Tp>::create(element, p);

    return ListNode<Tp>::create(list->toBack(), element, p);
}

#endif // FASTLIST_H

// kate: space-indent on; indent-width 2; replace-tabs on;

