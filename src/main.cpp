#include "nomoremess.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NoMoreMess w;
    w.show();
    return a.exec();
}
