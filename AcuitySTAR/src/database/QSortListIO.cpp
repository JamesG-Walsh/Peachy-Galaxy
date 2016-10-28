#include <QDataStream>
#include <QFile>
#include <QStringList>

#include "QSortListIO.h"

QSortListIO::QSortListIO(std::string _filename)
{
    filename = _filename;
}

void QSortListIO::saveList(QList<QStringList> sortFields)
{
    // open the file for writing
    QFile file(filename.c_str());
    file.open(QIODevice::WriteOnly);

    // we will serialize the data into the file
    QDataStream out(&file);

    // serialize the sort fields
    out << sortFields;

    // close the file, we're done
    file.close();
}

QList<QStringList> QSortListIO::readList()
{
    // open the file for reading
    QFile file(filename.c_str());
    file.open(QIODevice::ReadOnly);

    // read the data serialized from the file
    QDataStream in(&file);

    // extract sort fields
    QList<QStringList> sortFields;
    in >> sortFields;

    // close the file, we're done
    file.close();

    return sortFields;
}

void QSortListIO::savePath(QString path)
{
    // open the file for writing
    QFile file(filename.c_str());
    file.open(QIODevice::WriteOnly);

    // we will serialize the data into the file
    QDataStream out(&file);

    // serialize the sort fields
    out << path;

    // close the file, we're done
    file.close();
}

QString QSortListIO::readPath()
{
    // open the file for reading
    QFile file(filename.c_str());
    file.open(QIODevice::ReadOnly);

    // read the data serialized from the file
    QDataStream in(&file);

    // extract sort fields
    QString path;
    in >> path;

    // close the file, we're done
    file.close();

    return path;
}
