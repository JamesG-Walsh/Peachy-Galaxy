#ifndef GUITEST_H
#define GUITEST_H


#include <QtTest/QtTest>

class TestQString: public QObject
{
    Q_OBJECT
private slots:
    void toUpper();
};

#endif // GUITEST_H
