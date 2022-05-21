#include "backend.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BackEnd w;
    w.show();
    return a.exec();
}
