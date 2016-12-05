#ifndef TEST_H
#define TEST_H

#include <QtTest/QtTest>
#include <QtGui>
#include <QtTestGui>
#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/CustomSort.h"
#include "gui/ErrorEditDialog.h"
#include "gui/mainwindow.h"
#include "gui/editsort.h"
#include "ui_editsort.h"


#include "database/CSVReader.h"
#include "database/QSortListIO.h"
#include "database/RecordsManager.h"
#include "datamodel/TreeModel.h"
#include "datamodel/GrantFundingTreeModel.h"
#include "datamodel/PresentationTreeModel.h"
#include "datamodel/PublicationTreeModel.h"
#include "datamodel/TeachingTreeModel.h"

class Test: public QObject
{
    Q_OBJECT

private:
    MainWindow w;
    EditSort editui;

public:
    Test() = default;
private slots:

    void test1();
    void test2();
    void test3();
    void test4();
    void test5();
    void test6();
    void test7();
    void test8();
    void test9();
    void test10();
    void test11();
    void test12();
    void test13();
    void test14();


    void NoSaveTest();
    void SaveTestTeach();
    void SaveTestPub();
    void SaveTestPres();
    void SaveTestFund();

    void testSortByDivision();


    //test new graph
    void test_on_teach_line_button_toggled();
    void test_on_pres_line_button_toggled();
    void test_on_pub_line_button_toggled();
    void test_on_fund_line_button_toggled();
    void test_setupLineChart();

    //test edit sort order

    void test_editsort_setFields();
    void test_editsort_getSortFields();
    void testCustomList();
    void testFixDateFormatting();



};

#endif // TESTGUI_H
