#include <QString>
#include <QtTest>
#include "database/TestCSVReader.h"
#include "database/CSVReader.h"
#include "gui/Test.h"
#include "gui/mainwindow.h"
#include "gui/editsort.h"
#include <vector>
#include <iostream>
#include "database/QFileIO.h"
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


//Will only work the first time it is run
void Test::NoSaveTest(){
    MainWindow w;
    w.show();
    w.close();
    QFileIO teachFile("teachfile.dat");
    QFileIO pubFile("pubfile.dat");
    QFileIO presFile("presFile.dat");
    QFileIO fundFile("fundFile.dat");
    QString teachPath = teachFile.readPath();
    QString pubPath = pubFile.readPath();
    QString presPath = presFile.readPath();
    QString fundPath = fundFile.readPath();
    QVERIFY2(teachPath == "" && pubPath == "" && presPath == "" && fundPath == "", "No save test failed");
}

void Test::SaveTestTeach(){
    MainWindow w;
    w.show();
    w.load_teach("../Project Information/Sample Data/Program_Teaching_expanded.csv");
    w.close();
    QFileIO teachFile("teachfile.dat");
    QString readTeachPath = teachFile.readPath();
    QVERIFY2(readTeachPath != "", "Save Test for Teaching failed");
}

void Test::SaveTestPub(){
    MainWindow w;
    w.show();
    w.load_pub("../Project Information/Sample Data/Publications_sample.csv");
    w.close();
    QFileIO pubFile("pubfile.dat");
    QString readPubPath = pubFile.readPath();
    QVERIFY2(readPubPath != "", "Save Test for Publications failed");
}

void Test::SaveTestPres(){
    MainWindow w;
    w.show();
    w.load_pres("../Project Information/Sample Data/Presentations_sample.csv");
    w.close();
    QFileIO presFile("presfile.dat");
    QString readPresPath = presFile.readPath();
    QVERIFY2(readPresPath != "", "Save Test for Presentations failed");
}

void Test::SaveTestFund(){
    MainWindow w;
    w.show();
    w.load_fund("../Project Information/Sample Data/GrantsClinicalFunding_sample.csv");
    w.close();
    QFileIO fundFile("fundfile.dat");
    QString readFundPath = fundFile.readPath();
    QVERIFY2(readFundPath != "", "Save Test for Funding failed");
}

void Test::testSortByDivision()
{
    bool testPasses = false;
    //QString path = "../Project Information/Sample Data/Teaching_sample.csv";
    //w.load_teach(path, false);      //load teaching file

    QStringList newSortOrder = (QStringList() << "Division" << "Program"); //create new sort order (simulates w.on_teach_new_sort_clicked())

    w.allTeachOrders << newSortOrder;
    w.ui->teach_sort->addItem(newSortOrder.at(0));  //add new sort order to mainwindow attributes

    for (int i=0; i < w.allTeachOrders.size(); i++)  //check if sort orders contain a tier for "Division"
    {
        QStringList qsl = w.allTeachOrders.at(i);
        if (qsl.contains("Division"))
        {
            testPasses = true;
            //maybe could also be done by evaluating the tree model
        }
    }

    QVERIFY(testPasses);


}

//test on_teach_line_button_toggled
void Test::test_on_teach_line_button_toggled(){
    w.on_teach_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test_on_pres_line_button_toggled(){
    w.on_pres_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test_on_pub_line_button_toggled(){
    w.on_pub_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test_on_fund_line_button_toggled(){
    w.on_fund_line_button_toggled();
    QCOMPARE(w.ui->teach_graph_stackedWidget->currentIndex(),2);
}

void Test::test_setupLineChart() {
    int size = 5;
    std::vector<std::pair <std::string, double>> chartList;
    for (int i = 0; i < size; i++) {
        chartList.emplace_back("test", static_cast<double>(0.0));
    }
    w.setupLineChart(w.ui->teachLineChart,chartList);
    QCOMPARE(w.ui->teachLineChart->plottableCount(),(int) chartList.size());
}



void Test::test_editsort_setFields(){
    std::vector<std::string> testString;
    testString.push_back("a");
    editui.setFields(testString);
    QCOMPARE(editui.fieldBoxes.at(0)->currentText(),QString::fromStdString("a"));
}

void Test::test_editsort_getSortFields(){
    std::vector<std::string> testString;
    editui.setFields(testString);
    QStringList testList;
    QCOMPARE(editui.getSortFields(),testList);
}

void Test::testFixDateFormatting()
{
    CSVReader reader("../Project Information/Sample Data/Grants_expanded.csv");

    int numRecords = reader.getData().size();


    QVERIFY(numRecords == 4241);
}




