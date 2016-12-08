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
    friend class Test;

    Q_OBJECT

public:
    explicit selectData(QWidget *parent = 0);
    ~selectData();
    void setFields(vector<string> names);
    void setCustomFields(vector<string> names);
    vector<string> getCustomSortFields();
    vector<string> getSortFields();

private slots:

    void displayNames();


    void on_buttonBox_rejected();
    void on_buttonBox_accepted();


    void on_nameEntry_textChanged();

    void on_addButton_clicked();

    void on_removeButton_clicked();


    void on_listWidget_doubleClicked(const QModelIndex &index);


    void on_customListWidget_doubleClicked(const QModelIndex &index);

    void on_reset_clicked();

private:
    Ui::selectData *ui;
    vector<string> sortFields;
    vector<string> customSortFields;
    int numFields;
    string charInField;
    QList<string> fieldBoxes;
};

#endif // selectData_H
