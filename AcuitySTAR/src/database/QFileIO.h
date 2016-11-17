#ifndef QFILEIO_H
#define QFILEIO_H
#include <vector>
#include <string>

class QFileIO
{
public:
    QFileIO(std::string _filename);

    void savePath(QString path);
    QString readPath();
    void saveEdit(std::vector<std::string>::iterator vector);
    std::vector<std::string>::iterator readEdit();


private:
    std::string filename;
};

#endif // QFILEIO_H
