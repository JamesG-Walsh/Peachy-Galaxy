#ifndef TESTDATABASE_H
#define TESTDATABASE_H

#include <QtTest/QtTest>
#include "testdatabase.h"
#include "QSortListIO.h"

class TestDatabase: public QObject
{

    Q_OBJECT
public:
    TestDatabase() = default;
private slots:
    void toUpper();
    void testDatabase1();
};

#endif // TESTDATABASE_H
