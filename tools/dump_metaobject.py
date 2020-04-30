#############################################################################
##
## Copyright (C) 2020 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the Qt for Python project.
##
## $QT_BEGIN_LICENSE:LGPL$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 3 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL3 included in the
## packaging of this file. Please review the following information to
## ensure the GNU Lesser General Public License version 3 requirements
## will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 2.0 or (at your option) the GNU General
## Public license version 3 or any later version approved by the KDE Free
## Qt Foundation. The licenses are as published by the Free Software
## Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-2.0.html and
## https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

"""Helper functions for formatting information on QMetaObject"""

from PySide2.QtCore import (QMetaClassInfo, QMetaEnum, QMetaMethod,
                            QMetaProperty, QMetaObject, QObject)


def _qbytearray_to_string(b):
    return bytes(b.data()).decode('utf-8')


def _dump_metaobject_helper(meta_obj, indent):
    print('{}class {}:'.format(indent, meta_obj.className()))
    indent += '    '

    info_offset = meta_obj.classInfoOffset()
    info_count = meta_obj.classInfoCount()
    if info_offset < info_count:
        print('{}Info:'.format(indent))
        for i in range(info_offset, info_count):
            name = meta_obj.classInfo(i).name()
            value = meta_obj.classInfo(i).value()
            print('{}{:4d} {}+{}'.format(indent, i, name, value))

    enumerator_offset = meta_obj.enumeratorOffset()
    enumerator_count = meta_obj.enumeratorCount()
    if enumerator_offset < enumerator_count:
        print('{}Enumerators:'.format(indent))
        for e in range(enumerator_offset, enumerator_count):
            meta_enum = meta_obj.enumerator(e)
            name = meta_enum.name()
            value_str = ''
            for k in range(0, meta_enum.keyCount()):
                if k > 0:
                    value_str += ', '
                value_str += '{} = {}'.format(meta_enum.key(k),
                                              meta_enum.value(k))
            print('{}{:4d} {} ({})'.format(indent, e, name, value_str))

    property_offset = meta_obj.propertyOffset()
    property_count = meta_obj.propertyCount()
    if property_offset < property_count:
        print('{}Properties:'.format(indent))
        for p in range(property_offset, property_count):
            meta_property = meta_obj.property(p)
            name = meta_property.name()
            desc = ''
            if meta_property.isConstant():
                desc += ', constant'
            if meta_property.isDesignable:
                desc += ', designable'
            if meta_property.isFlagType:
                desc += ', flag'
            if meta_property.isEnumType():
                desc += ', enum'
            if meta_property.isStored():
                desc += ', stored'
            if meta_property.isWritable():
                desc += ', writable'
            if meta_property.isResettable:
                desc += ', resettable'
            if meta_property.hasNotifySignal():
                notify_name = meta_property.notifySignal().name()
                desc += ', notify={}'.format(_qbytearray_to_string(notify_name))
            print('{}{:4d} {} {}{}'.format(indent, p, meta_property.typeName(),
                                           name, desc))

    method_offset = meta_obj.methodOffset()
    method_count = meta_obj.methodCount()
    if method_offset < method_count:
        print('{}Methods:'.format(indent))
        for m in range(method_offset, method_count):
            method = meta_obj.method(m)
            signature = _qbytearray_to_string(method.methodSignature())
            access = ''
            if method.access() == QMetaMethod.Protected:
                access += 'protected '
            elif method.access() == QMetaMethod.Private:
                access += 'private '
            type = method.methodType()
            typeString = ''
            if type == QMetaMethod.Signal:
                typeString = ' (Signal)'
            elif type == QMetaMethod.Slot:
                typeString = ' (Slot)'
            elif type == QMetaMethod.Constructor:
                typeString = ' (Ct)'
            desc = '{}{:4d} {}{} {}{}'.format(indent, m, access,
                                              method.typeName(), signature,
                                              typeString)
            parameter_names = method.parameterNames()
            if parameter_names:
                parameter_types = method.parameterTypes()
                desc += ' Parameters:'
                for p, bname in enumerate(parameter_names):
                    name = _qbytearray_to_string(bname)
                    type = _qbytearray_to_string(parameter_types[p])
                    desc += ' {}: {}'.format(name if name else '<unnamed>', type)
            print(desc)


def dump_metaobject(meta_obj):
    super_classes = [meta_obj]
    super_class = meta_obj.superClass()
    while super_class:
        super_classes.append(super_class)
        super_class = super_class.superClass()
    indent = ''
    for c in reversed(super_classes):
        _dump_metaobject_helper(c, indent)
        indent += '    '
