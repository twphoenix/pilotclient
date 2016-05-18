/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \cond PRIVATE_TESTS

/*!
 * \file
 * \ingroup testblackgui
 */

#include "testutils.h"
#include "blackgui/guiutility.h"

#include <QtTest>

using namespace BlackGui;

namespace BlackCoreTest
{

    CTestGuiUtilities::CTestGuiUtilities(QObject *parent) :
        QObject(parent)
    { }

    void CTestGuiUtilities::utilityFunctions()
    {
        QVERIFY2(CGuiUtility::lenientTitleComparison("foo", "foo"), "wrong title match");
        QVERIFY2(CGuiUtility::lenientTitleComparison("foo&", "&Foo"), "wrong title match");
        QVERIFY2(!CGuiUtility::lenientTitleComparison("foo", "bar"), "wrong title mismatch");
    }
} // ns

//! \endcond
