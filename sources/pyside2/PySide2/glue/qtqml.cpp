/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt for Python.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// @snippet qmlregistertype
int %0 = PySide::qmlRegisterType(%ARGUMENT_NAMES);
%PYARG_0 = %CONVERTTOPYTHON[int](%0);
// @snippet qmlregistertype

// @snippet qmlregistersingletontype_qobject_callback
int %0 = PySide::qmlRegisterSingletonType(%ARGUMENT_NAMES, true, true);
%PYARG_0 = %CONVERTTOPYTHON[int](%0);
// @snippet qmlregistersingletontype_qobject_callback

// @snippet qmlregistersingletontype_qobject_nocallback
int %0 = PySide::qmlRegisterSingletonType(%ARGUMENT_NAMES, nullptr, true, false);
%PYARG_0 = %CONVERTTOPYTHON[int](%0);
// @snippet qmlregistersingletontype_qobject_nocallback

// @snippet qmlregistersingletontype_qjsvalue
int %0 = PySide::qmlRegisterSingletonType(nullptr, %ARGUMENT_NAMES, false, true);
%PYARG_0 = %CONVERTTOPYTHON[int](%0);
// @snippet qmlregistersingletontype_qjsvalue

// @snippet qmlregisteruncreatabletype
int %0 = PySide::qmlRegisterType(%ARGUMENT_NAMES, false);
%PYARG_0 = %CONVERTTOPYTHON[int](%0);
// @snippet qmlregisteruncreatabletype

// @snippet init
PySide::initQmlSupport(module);
// @snippet init

// @snippet qjsengine-toscriptvalue
%RETURN_TYPE retval = %CPPSELF.%FUNCTION_NAME(%1);
return %CONVERTTOPYTHON[%RETURN_TYPE](retval);
// @snippet qjsengine-toscriptvalue

// @snippet qmlelement
%PYARG_0 = PySide::qmlElementMacro(%ARGUMENT_NAMES);
// @snippet qmlelement
