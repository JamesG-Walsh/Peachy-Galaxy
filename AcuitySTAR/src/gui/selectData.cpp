#include "gui/selectData.h"
#include "ui_selectData.h"
#include <QMessageBox>

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


QStringList selectData::getSortFields() {
    return sortFields;
}

void selectData::setNext(int fieldNum, int currentIndex) {
    for (int i=fieldNum+1; i<numFields; i++) {
        fieldBoxes.at(i)->clear();
    }

    for (int i=0; i<fieldBoxes[fieldNum]->count(); i++) {
        fieldBoxes[fieldNum+1]->addItem(fieldBoxes[fieldNum]->itemText(i));
    }
    fieldBoxes[fieldNum+1]->removeItem(currentIndex);
}

void selectData::on_field_0_currentIndexChanged(int index) { setNext(0, index);}
void selectData::on_field_1_currentIndexChanged(int index) { setNext(1, index);}
void selectData::on_field_2_currentIndexChanged(int index) { setNext(2, index);}
void selectData::on_field_3_currentIndexChanged(int index) { setNext(3, index);}
void selectData::on_field_4_currentIndexChanged(int index) { setNext(4, index);}
void selectData::on_field_5_currentIndexChanged(int index) { setNext(5, index);}

void selectData::on_buttonBox_rejected() {
    done(0);
}

void selectData::on_buttonBox_accepted() {

}
