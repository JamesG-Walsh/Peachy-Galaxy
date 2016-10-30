#include <QString>
#include <QtTest>
#include "database/TestCSVReader.h"
#include "database/CSVReader.h"
#include "gui/Test.h"
#include "gui/mainwindow.h"
#include <vector>
#include <iostream>
using namespace std;

//QSortListIO
void Test::test1()
{
    QStringList testOrder;
    testOrder << "Default";
    QList<QStringList> allTestOrders;
    allTestOrders << testOrder;
    QSortListIO saveTestSort("testsortorder.dat");
    saveTestSort.saveList(allTestOrders);
    QList<QStringList> read = saveTestSort.readList();
    QVERIFY(saveTestSort.readList() == allTestOrders);
}

//Test RecordsManager
void Test::test2(){
    CSVReader reader("../Project Information/Sample Data/Grants_expanded.csv");
    std::vector<std::string> header = reader.getHeaders();
    RecordsManager* testRM = new RecordsManager(&header);
    QVERIFY(testRM->getHeaders() == header);
}

//Test TeachingTreeModel
void Test::test3(){
    CSVReader reader("../Project Information/Sample Data/Teaching_sample.csv");
    std::vector<std::string> header = reader.getHeaders();;
    RecordsManager* testRM = new RecordsManager(&header);
    TeachingTreeModel* teachTree = new TeachingTreeModel(testRM);
    QVERIFY(teachTree!=NULL);
}

//Test GrantFundingTreeModel
void Test::test4(){
    CSVReader reader("../Project Information/Sample Data/Grants_expanded.csv");
    std::vector<std::string> header = reader.getHeaders();;
    RecordsManager* testRM = new RecordsManager(&header);
    GrantFundingTreeModel* grandTree = new GrantFundingTreeModel(testRM);
    QVERIFY(grandTree!=NULL);
}

//Test PresentationTreeModel
void Test::test5(){
    CSVReader reader("../Project Information/Sample Data/Presentations_expanded.csv");
    std::vector<std::string> header = reader.getHeaders();;
    RecordsManager* testRM = new RecordsManager(&header);
    PresentationTreeModel* presentationTree = new PresentationTreeModel(testRM);
    QVERIFY(presentationTree!=NULL);
}

//Test PublicationTreeModel
void Test::test6(){
    CSVReader reader("../Project Information/Sample Data/Publications_expanded.csv");
    std::vector<std::string> header = reader.getHeaders();
    RecordsManager* testRM = new RecordsManager(&header);
    PublicationTreeModel* publicationTree = new PublicationTreeModel(testRM);
    QVERIFY(publicationTree!=NULL);
}

//Tests for CSV Reader
void Test::test7(){
    CSVReader reader("../Project Information/Sample Data/Grants_expanded.csv");
    vector<string> header = reader.getHeaders();
    QVERIFY2(header.size() !=0, "Test1 Failed");
}

void Test::test8(){
    CSVReader reader("../Project Information/Sample Data/GrantsClinicalFunding_sample.csv");
    vector<string> header = reader.getHeaders();
    QVERIFY2(header.size() !=0, "Test2 Failed");
}

void Test::test9(){
    CSVReader reader("../Project Information/Sample Data/Presentations_expanded.csv");
    vector<string> header = reader.getHeaders();
    QVERIFY2(header.size() !=0, "Test3 Failed");
}

void Test::test10(){
    CSVReader reader("../Project Information/Sample Data/Presentations_sample.csv");
    vector<string> header = reader.getHeaders();
    QVERIFY2(header.size() !=0, "Test4 Failed");
}

void Test::test11(){
    CSVReader reader("../Project Information/Sample Data/Program_Teaching_expanded.csv");
    vector<string> header = reader.getHeaders();
    QVERIFY2(header.size() !=0, "Test5 Failed");
}

void Test::test12(){
    CSVReader reader("NO FILE");
    vector<string> header = reader.getHeaders();
    QVERIFY2(header.size() ==0, "Test6 Failed");
}

void Test::test13(){
    CSVReader reader("../Project Information/Sample Data/Presentations_sample.csv");
    vector< vector<string> > all_data = reader.getData();
    QVERIFY2(all_data.size() !=0, "Test7 Failed");
}

void Test::test14(){
    CSVReader reader("../Project Information/Sample Data/Program_Teaching_expanded.csv");
    vector< vector<string> > all_data = reader.getData();
    QVERIFY2(all_data.size() !=0, "Test8 Failed");
}

//test on_teach_line_button_toggled
void Test::test15(){
    w.on_teach_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test16(){
    w.on_pres_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test17(){
    w.on_pub_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test18(){
    w.on_fund_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

