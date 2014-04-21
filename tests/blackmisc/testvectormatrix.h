/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISCTEST_TESTVECTORMATRIX_H
#define BLACKMISCTEST_TESTVECTORMATRIX_H

#include "blackmisc/mathmatrix3x3.h"
#include <QtTest/QtTest>

namespace BlackMiscTest
{

    /*!
     * \brief Vector and Matrix classes tests
     */
    class CTestVectorMatrix : public QObject
    {
        Q_OBJECT

    public:
        //! Standard test case constructor
        explicit CTestVectorMatrix(QObject *parent = nullptr) : QObject(parent) {}

    private slots:
        //! Vector tests
        void vectorBasics();

        //! Matrix tests
        void matrixBasics();
    };

} // namespace

#endif // guard
