
#include "testdatabase.h"
#include "RecordsManager.h"
#include "QSortListIO.h"

#define FUNDORDER_SAVE  "fundsortorder.dat"


void TestDatabase::toUpper()
{
    QString str = "Hello";
    QVERIFY(str.toUpper() == "HELLO");
}


void TestDatabase::testDatabase1(){
    QStringList testOrder;
    testOrder << "Default";
    QList<QStringList> allTestOrders;
    allTestOrders << testOrder;
    QSortListIO saveTestSort(FUNDORDER_SAVE);
    saveTestSort.saveList(allTestOrders);
    QList<QStringList> read = saveTestSort.readList();
    QVERIFY(saveTestSort.readList() == allTestOrders);





}
