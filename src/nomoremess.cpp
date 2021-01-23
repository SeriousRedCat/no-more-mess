#include "mainwindow.hpp"
#include "ui_nomoremess.h"

NoMoreMess::NoMoreMess(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NoMoreMess)
{
    ui->setupUi(this);
}

NoMoreMess::~NoMoreMess()
{
    delete ui;
}

