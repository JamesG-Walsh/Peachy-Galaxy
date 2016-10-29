#include <iostream>
#include <stdio.h>
#include <QApplication>
#include <QTreeView>
#include <qstring.h>

#include "gui/mainwindow.h"
#include "database/TestCSVReader.h"
#include "gui/Test.h"

using namespace std;

int main(int argc, char *argv[])
{
    bool testing = true; //testing flag

    QApplication app(argc, argv);

    if (testing)
    {
       // Test *test_obj = new Test();
        //QTest::qExec(test_obj);
    }

    MainWindow w;
    w.show();

    return app.exec();
}
