#include <QDataStream>
#include <QFile>
#include <QString>

#include "QFileIO.h"

QFileIO::QFileIO(std::string _filename)
{
    filename = _filename;
}

void QFileIO::savePath(QString path)
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

QString QFileIO::readPath()
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
