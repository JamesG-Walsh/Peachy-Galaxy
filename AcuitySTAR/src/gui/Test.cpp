#include <QtTest/QtTest>

class test: public QObject
{
    Q_OBJECT
private slots:
    void toUpper();
};

void test::toUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

QTEST_MAIN(test)
#include "test.moc"
