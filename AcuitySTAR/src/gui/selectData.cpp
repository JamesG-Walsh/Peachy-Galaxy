#include "gui/selectData.h"
#include "ui_selectData.h"
#include <QMessageBox>
#include <algorithm>

selectData::selectData(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::selectData)
{
    ui->setupUi(this);

}

selectData::~selectData()
{
    delete ui;
}

void selectData::setFields(vector<string> names){
    sortFields = names;
    displayNames();
}

vector<string> selectData::getSortFields() {
    return sortFields;
}

void selectData::displayNames(){
    QStringList names;
    ui->textBrowser->setReadOnly(true);
    for(int i = 0; i < sortFields.size(); i++){
        names.append(QString::fromStdString(sortFields.at(i)));
    }
    for(int i = 0; i < sortFields.size(); i++){
        ui->textBrowser->append(names.at(i));
    }
}

void selectData::on_buttonBox_rejected() {
    done(0);
}

void selectData::on_buttonBox_accepted() {
    done(0);
}

void selectData::on_nameEntry_textChanged()
{
    charInField = ui->nameEntry->toPlainText().toStdString();
}



void selectData::on_addButton_clicked()
{
    sortFields.push_back(charInField);
}


void selectData::on_removeButton_clicked()
{
    if(std::find(sortFields.begin(), sortFields.end(), charInField) != sortFields.end()){
        sortFields.erase(std::remove(sortFields.begin(), sortFields.end(), charInField), sortFields.end());
    }
}

void selectData::on_textBrowser_textChanged()
{
    ui->textBrowser->clear();
    displayNames();
}
