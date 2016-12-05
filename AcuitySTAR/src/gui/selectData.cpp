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
    ui->listWidget->clear();
    ui->customListWidget->clear();
    displayNames();
}

void selectData::setCustomFields(vector<string> names){
    customSortFields = names;
    ui->listWidget->clear();
    ui->customListWidget->clear();
    displayNames();
}

vector<string> selectData::getSortFields() {
    return sortFields;
}

vector<string> selectData::getCustomSortFields(){
    return customSortFields;

}

void selectData::displayNames(){
    QStringList names;
    QStringList names2;
    for(unsigned int i = 0; i < sortFields.size(); i++){
        names.append(QString::fromStdString(sortFields.at(i)));
        std::sort(names.begin(), names.end());
    }
    for(unsigned int i = 0; i < sortFields.size(); i++){
        ui->listWidget->addItem(names.at(i));
    }
    for(unsigned int i = 0; i < customSortFields.size(); i++){
        names2.append(QString::fromStdString(customSortFields.at(i)));
        std::sort(names2.begin(), names2.end());
    }
    for(unsigned int i = 0; i < customSortFields.size(); i++){
        ui->customListWidget->addItem(names2.at(i));
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
        if(std::find(customSortFields.begin(), customSortFields.end(), charInField) != customSortFields.end()){
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
            if(std::find(sortFields.begin(), sortFields.end(), charInField) != sortFields.end()){
                sortFields.erase(std::remove(sortFields.begin(), sortFields.end(), charInField), sortFields.end());
                ui->listWidget->removeItemWidget(ui->listWidget->currentItem());
            }
            customSortFields.push_back(charInField);
            ui->nameEntry->clear();
            ui->listWidget->clear();
            ui->customListWidget->clear();
            displayNames();
        }
    }
}


void selectData::on_removeButton_clicked()
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
            QString mainText = QString::fromStdString(charInField) + " is not in the list";
            prompt.setText(mainText);
            prompt.setStandardButtons(QMessageBox::Yes);
            prompt.setButtonText(QMessageBox::Yes, "Ok");
            prompt.exec();
            if(QMessageBox::Yes){
            }
        }
        else{
            if(std::find(customSortFields.begin(), customSortFields.end(), charInField) != customSortFields.end()){
                customSortFields.erase(std::remove(customSortFields.begin(), customSortFields.end(), charInField), customSortFields.end());
                ui->customListWidget->removeItemWidget(ui->customListWidget->currentItem());
            }
            sortFields.push_back(charInField);
            ui->nameEntry->clear();
            ui->listWidget->clear();
            ui->customListWidget->clear();
            displayNames();
        }
    }
}

void selectData::on_listWidget_doubleClicked(const QModelIndex &index)
{
    if(ui->listWidget->currentItem()){
        charInField = ui->listWidget->currentItem()->text().toStdString();
    }

    if(std::find(sortFields.begin(), sortFields.end(), charInField) != sortFields.end()){
        sortFields.erase(std::remove(sortFields.begin(), sortFields.end(), charInField), sortFields.end());
        ui->listWidget->removeItemWidget(ui->listWidget->currentItem());
    }
    customSortFields.push_back(charInField);
    ui->nameEntry->clear();
    ui->customListWidget->clear();
    ui->listWidget->clear();
    displayNames();
}


void selectData::on_customListWidget_doubleClicked(const QModelIndex &index)
{
    charInField = ui->customListWidget->currentItem()->text().toStdString();

    if(std::find(customSortFields.begin(), customSortFields.end(), charInField) != customSortFields.end()){
        customSortFields.erase(std::remove(customSortFields.begin(), customSortFields.end(), charInField), customSortFields.end());
        ui->customListWidget->removeItemWidget(ui->customListWidget->currentItem());
    }
    sortFields.push_back(charInField);
    ui->nameEntry->clear();
    ui->customListWidget->clear();
    ui->listWidget->clear();
    displayNames();
}


void selectData::on_reset_clicked()
{
    for(unsigned int i = 0; i < customSortFields.size(); i++){
        sortFields.push_back(customSortFields.at(i));
    }
    customSortFields.clear();
    ui->customListWidget->clear();
    ui->listWidget->clear();
    displayNames();
}
