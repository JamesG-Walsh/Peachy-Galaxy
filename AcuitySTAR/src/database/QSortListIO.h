#ifndef QSORTLISTIO_H
#define QSORTLISTIO_H

#include <string>

class QSortListIO
{
public:
    QSortListIO(std::string _filename);

    void saveList(QList<QStringList> sortFields);
    QList<QStringList> readList();
    void saveLastSort(int index);
    int readLastSort();

private:
    std::string filename;
};

#endif // QSORTLISTIO_H
