#ifndef ERROREDITDIALOG_H
#define ERROREDITDIALOG_H

#include <QDialog>
#include <vector>
#include <string>
#include <QString>

class QAbstractButton;

namespace Ui {
    class ErrorEditDialog;
}

class ErrorEditDialog : public QDialog
{
    Q_OBJECT

public:
     ErrorEditDialog(QWidget *parent,
                     std::vector<std::vector<std::string>*>& errors,
                     std::vector<std::string>& headers,
                     std::vector<std::string>& mandatory,
                     QString &type);
    ~ErrorEditDialog();

private slots:
     void on_save_clicked();
     void on_cancel_clicked();

     void on_findNext_clicked();

     void on_findPrev_clicked();

private:
    std::vector<std::vector<std::string>*> errorList;
    std::vector<std::string> headerList;
    std::vector<std::string> mandatoryList;
    Ui::ErrorEditDialog *ui;
    QString type;
    std::vector<int> mandatoryColumnIndexes;

//    struct coord{
//        int row, col;
//    };

//    QList<coord> errorCoords;
    std::vector<std::vector<bool>> errCoords;

    void saveData();
    int countRemainingErrors();
};

#endif // ERROREDITDIALOG_H
