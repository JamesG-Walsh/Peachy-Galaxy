#include "ui_ErrorEditDialog.h"
#include "ErrorEditDialog.h"

#include <QTableWidgetItem>
#include <QStringList>
#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QAbstractButton>
#include <QMessageBox>
#include <QString>
#include "database/QFileIO.h"



/*
 * Load data contained in the errors vector into a QWidgetTable
 * Fields will be marked red and editable if they are a mandatory field
 * and editable.  Otherwise all other fields will not be editable.
 * Clicking Save in the dialog will return the corrected entries to the main
 * program through the errors parameter.  If not all marked fields are edited
 * then a warning message will be displayed.  If cancel is clicked all errors
 * are discarded.
 */
ErrorEditDialog::ErrorEditDialog(QWidget *parent,
                                 std::vector<std::vector<std::string>*>& errors,
                                 std::vector<std::string>& headers,
                                 std::vector<std::string>& mandatory,
                                 QString &type) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    errorList(errors),
    headerList(headers),
    mandatoryList(mandatory),
    ui(new Ui::ErrorEditDialog)
{
    ui->setupUi(this);
    ui->tableWidget->setRowCount((int) errors.size());
    ui->tableWidget->setColumnCount((int) headers.size());

    for (int col = 0, mandIndex = 0; col < (int) headers.size() ; col++)
    {
        if(mandatory[mandIndex].compare(headers.at(col)) == 0)
        {
            //qDebug() << "mandatory index: " << col;
            mandatoryColumnIndexes.push_back(col);
            mandIndex++;
        }
    }

    errCoords.resize(errors.size());

    for(int row = 0; row < errors.size(); row++)
    {
        errCoords.at(row).resize(headers.size());
        for (int col = 0; col < headers.size(); col++)
        {
            errCoords.at(row).at(col) = false;
        }
    }

    QStringList listHeaders;
    for (int i = 0; i < (int) headers.size(); i++) {
        listHeaders << headers[i].c_str();
    }

    ui->tableWidget->setHorizontalHeaderLabels(listHeaders);
    QTableWidgetItem* item;
    QBrush brush(QColor(255, 0, 0, 100));
    std::vector<std::vector<std::string>*>::iterator it;
    int row = 0;
    for (it = errors.begin(); it != errors.end(); it++)
    {
        for (int col = 0; col < (int) headers.size()/* && col < (int) (*it)->size()*/; col++)
        {
            item = new QTableWidgetItem();
            Qt::ItemFlags flag = item->flags();
            item->setFlags(Qt::ItemIsSelectable);
            if(col < (*it)->size())
            {
                item->setText((*it)->at(col).c_str());
            }
            else
            {
                item->setText("");
            }
            for (int i = 0; i < (int) mandatory.size(); i++)
            {
                if (mandatory[i].compare(headers.at(col)) == 0 && item->text().compare("") == 0)
                {
                    item->setBackground(brush);
                    item->setFlags(flag);
                    //                    coord theCoord;
                    //                    theCoord.row = row;
                    //                    theCoord.col = col;
                    //                    errorCoords.append(theCoord);
                    errCoords.at(row).at(col) = true;
                }
            }
            ui->tableWidget->setItem(row, col, item);
        }
        row++;
    }
    int errorsRem = this->countRemainingErrors();
    ui->numErrorsRemaining->display(errorsRem);

    if(errorsRem == 0)
    {
        ui->findNext->setEnabled(false);
        ui->findPrev->setEnabled(false);
    }

    ui->numErrorsRemaining->setSegmentStyle(QLCDNumber::Flat);
    ui->numErrorsRemaining->show();
}

//Clean up allocated memory for the table items
ErrorEditDialog::~ErrorEditDialog()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        for (int j = 0; j < ui->tableWidget->columnCount(); j++) {
            delete ui->tableWidget->item(i,j);
        }
    }
    delete ui;
}

//Save the new data entered by the user via the error reference var
void ErrorEditDialog::saveData()
{
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        for (int col = 0; col < ui->tableWidget->columnCount() && col < (int) errorList[row]->size(); col++)
        {
            std::vector<std::string>::iterator it = errorList[row]->begin()+col;
            if (errorList[row]->at(col).compare("") == 0)
            {
                it = errorList[row]->erase(it);
            }
            errorList[row]->insert(it, ui->tableWidget->item(row, col)->text().toStdString());
        }
    }

    accept();
}

int ErrorEditDialog::countRemainingErrors()
{
    int remainingErrors = 0;
    QTableWidgetItem* currItem;
    QString currStr;
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        int col;
        //qDebug() << "row: " << row;
        for(int mandIndex = 0; mandIndex < mandatoryColumnIndexes.size(); mandIndex++)
        {
            col = mandatoryColumnIndexes.at(mandIndex);
            //qDebug() << "col: " << col;
            currItem = ui->tableWidget->item(row, col);
            //qDebug() << "got item";
            currStr = currItem->text();
            //qDebug() << "got string";
            if(errCoords.at(row).at(col) && (currStr.isNull() || currStr.isEmpty()))
            {
                //qDebug() << "found error at: ";
                //qDebug() << "\trow: " << row;
                //qDebug() << "\tcol: " << col;
                remainingErrors++;
                //qDebug() << "\tRemaining Errors: " << remainingErrors;
            }
            //qDebug() << "finished if block";
        }
        //qDebug() << "finished inner loop";
    }
    //qDebug() << "Final Remaining Errors: " << remainingErrors;
    return remainingErrors;
}

void ErrorEditDialog::on_save_clicked()
{
    bool search = true;
    //check if mandatory fields have been filled
    for (int row = 0; row < ui->tableWidget->rowCount() && search; row++)
    {
        for (int j = 0; j < (int) mandatoryList.size() && search; j++)
        {
            std::vector<std::string>::iterator it = std::find(headerList.begin(), headerList.end(), mandatoryList[j]);
            int col = it - headerList.begin();
            QTableWidgetItem* item = ui->tableWidget->item(row, col);
            if (item->text().compare("") == 0)
            {
                QMessageBox::critical(this, "Error", "Mandatory fields are still empty.");
                search = false;
            }
        }
    }
    if (search)
    {
        saveData();
    }
}

void ErrorEditDialog::on_cancel_clicked()
{
    reject();
}

void ErrorEditDialog::on_findNext_clicked()
{
    int errorsRem = this->countRemainingErrors();
    ui->numErrorsRemaining->display(errorsRem);

    if(errorsRem == 0)
    {
        ui->findNext->setEnabled(false);
        ui->findPrev->setEnabled(false);
    }

    int initRow = ui->tableWidget->currentRow();
    int initCol = ui->tableWidget->currentColumn();

    if(initRow < 0)
    {
        initRow = 0;
    }
    if(initCol < 0)
    {
        initCol = 0;
    }

    int destRow, destCol;
    bool checkedFirstRow = false;

    //qDebug() << "initRow: " << initRow;

    if(!(initCol == (ui->tableWidget->columnCount() - 1))){
        initCol++;
    }
    else{
        initRow++;
        initCol = 0;
    }

    //qDebug() << "initCol: " << initCol;

    bool foundError = false;

    for(int row = initRow ; row < ui->tableWidget->rowCount() && !foundError ; row++)
    {
        //qDebug() << "row: " << row;
        if(checkedFirstRow)
        {
            initCol = 0;
        }
        for(int col = initCol ; col < ui->tableWidget->columnCount() && !foundError ; col++)
        {
            //qDebug() << "col" << col;
            if(errCoords.at(row).at(col))
            {
                foundError = true;
                destRow = row;
                destCol = col;
                //qDebug() << "destRow: " << destRow;
                //qDebug() << "destCol: " << destCol;
            }
            checkedFirstRow = true;
        }
    }
    ui->tableWidget->setCurrentCell(destRow,destCol);
}

void ErrorEditDialog::on_findPrev_clicked()
{
    int errorsRem = this->countRemainingErrors();
    ui->numErrorsRemaining->display(errorsRem);

    if(errorsRem == 0)
    {
        ui->findNext->setEnabled(false);
        ui->findPrev->setEnabled(false);
    }
    int initRow = ui->tableWidget->currentRow();
    int initCol = ui->tableWidget->currentColumn();

    if(initRow < 0)
    {
        initRow = 0;
    }
    if(initCol < 0)
    {
        initCol = 0;
    }

    int destRow, destCol;
    bool checkedFirstRow = false;

    //qDebug() << "initRow: " << initRow;

    if(!(initCol == 0)){
        initCol--;
    }
    else{
        initRow--;
        initCol = (ui->tableWidget->columnCount() - 1);
    }

    //qDebug() << "initCol: " << initCol;

    bool foundError = false;

    for(int row = initRow ; row >= 0 && !foundError ; row--)
    {
        //qDebug() << "row: " << row;

        if(checkedFirstRow){
            initCol = (ui->tableWidget->columnCount() - 1);
        }

        for(int col = initCol ; col >= 0 && !foundError ; col--)
        {
            //qDebug() << "col" << col;
            if(errCoords.at(row).at(col))
            {
                foundError = true;
                destRow = row;
                destCol = col;
                //qDebug() << "destRow: " << destRow;
                //qDebug() << "destCol: " << destCol;
            }
            checkedFirstRow = true;
        }
    }

    ui->tableWidget->setCurrentCell(destRow,destCol);
}
