#include "gui/selectData.h"
#include "ui_selectData.h"
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <algorithm>

selectData::selectData(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::selectData)
{
    ui->setupUi(this);
    displayNames();
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
    for(unsigned int i = 0; i < sortFields.size(); i++){
        names.append(QString::fromStdString(sortFields.at(i)));
    }
    for(unsigned int i = 0; i < sortFields.size(); i++){
        ui->listWidget->addItem(names.at(i));
    }
}

void selectData::on_buttonBox_rejected() {
    done(0);
}

void selectData::on_buttonBox_accepted() {
    done(1);
}

void selectData::on_nameEntry_textChanged()
{
    charInField = ui->nameEntry->toPlainText().toStdString();
}



void selectData::on_addButton_clicked()
{
    if(charInField == ""){
        QMessageBox prompt;
        QString mainText = "Please type a name";
        prompt.setText(mainText);
        prompt.setStandardButtons(QMessageBox::Yes);
        prompt.setButtonText(QMessageBox::Yes, "Ok");
        prompt.exec();
        if(QMessageBox::Yes){
        }
    }
    else{
        if(std::find(sortFields.begin(), sortFields.end(), charInField) != sortFields.end()){
            QMessageBox prompt;
            QString mainText = QString::fromStdString(charInField) + " is already in the list";
            prompt.setText(mainText);
            prompt.setStandardButtons(QMessageBox::Yes);
            prompt.setButtonText(QMessageBox::Yes, "Ok");
            prompt.exec();
            if(QMessageBox::Yes){
            }
        }
        else{
            sortFields.push_back(charInField);
            ui->listWidget->clear();
            displayNames();
        }
    }
}


void selectData::on_removeButton_clicked()
{
    if(ui->listWidget->currentItem()){
        charInField = ui->listWidget->currentItem()->text().toStdString();
        if(std::find(sortFields.begin(), sortFields.end(), charInField) != sortFields.end()){
            sortFields.erase(std::remove(sortFields.begin(), sortFields.end(), charInField), sortFields.end());
            ui->listWidget->removeItemWidget(ui->listWidget->currentItem());
        }
        ui->listWidget->clear();
        displayNames();
        charInField = "";
    }
    else{
        QMessageBox prompt;
        QString mainText = "Please select a name";
        prompt.setText(mainText);
        prompt.setStandardButtons(QMessageBox::Yes);
        prompt.setButtonText(QMessageBox::Yes, "Ok");
        prompt.exec();
        if(QMessageBox::Yes){
        }
    }
}

