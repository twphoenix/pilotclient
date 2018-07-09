/* Copyright (C) 2018
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#ifndef BLACKSIMPLUGINFSXP3D_TESTFSXP3DCOMMON_H
#define BLACKSIMPLUGINFSXP3D_TESTFSXP3DCOMMON_H

//! \cond PRIVATE_TESTS
//! \file
//! \ingroup testblacksimplugin

#include <QObject>

namespace BlackSimPluginFsxP3D
{
    //! FSX/P3D common tests
    class CTestFsxP3DCommon : public QObject
    {
        Q_OBJECT

    public:
        //! Standard test case constructor
        explicit CTestFsxP3DCommon(QObject *parent = nullptr) : QObject(parent) {}

    private slots:
        //! Request IDs
        void requestIds();
    };
} // namespace

//! \endcond

#endif // guard