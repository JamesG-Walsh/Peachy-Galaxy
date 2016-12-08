#ifndef EditSort_H
#define EditSort_H

#include <QDialog>
#include <QAbstractButton>
#include <QComboBox>

namespace Ui
{
class EditSort;
}

class EditSort : public QDialog
{
    Q_OBJECT
    friend class Test;

public:
    explicit EditSort(QWidget *parent = 0);
    ~EditSort();
    void setFields(const std::vector<std::string> &headers);
    QStringList getSortFields();

private slots:
    void setNext(int fieldNum, int currentIndex);

    void on_field_0_currentIndexChanged(int index);
    void on_field_1_currentIndexChanged(int index);
    void on_field_2_currentIndexChanged(int index);
    void on_field_3_currentIndexChanged(int index);
    void on_field_4_currentIndexChanged(int index);
    void on_field_5_currentIndexChanged(int index);
    void on_field_6_currentIndexChanged(int index);

    void on_buttonBox_rejected();
    void on_buttonBox_accepted();


private:
    Ui::EditSort *ui;
    QStringList sortFields;
    int numFields;
    QList<QComboBox*> fieldBoxes;
};

#endif // EditSort_H
