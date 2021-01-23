#ifndef NOMOREMESS_HPP
#define NOMOREMESS_HPP

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class NoMoreMess; }
QT_END_NAMESPACE

class NoMoreMess : public QMainWindow
{
    Q_OBJECT

public:
    NoMoreMess(QWidget *parent = nullptr);
    ~NoMoreMess();

private:
    Ui::NoMoreMess *ui;
};
#endif // NOMOREMESS_HPP
