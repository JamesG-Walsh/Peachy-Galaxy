#ifndef selectData_H
#define selectData_H

#include <QDialog>
#include <string.h>
#include <QAbstractButton>
#include <QComboBox>
using namespace std;
namespace Ui

{
class selectData;
}

class selectData : public QDialog
{
    Q_OBJECT

public:
    explicit selectData(QWidget *parent = 0);
    ~selectData();
    void setFields(vector<string> names);
    vector<string> getSortFields();

private slots:

    void displayNames();


    void on_buttonBox_rejected();
    void on_buttonBox_accepted();


    void on_nameEntry_textChanged();

    void on_addButton_clicked();

    void on_removeButton_clicked();

private:
    Ui::selectData *ui;
    vector<string> sortFields;
    int numFields;
    string charInField;
    QList<string> fieldBoxes;
};

#endif // selectData_H
