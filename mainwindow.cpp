#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
    }

MainWindow::~MainWindow()
    {
        delete ui;
    }

void MainWindow::on_Button_open_can_clicked()
{

}

void MainWindow::on_Button__closecan_clicked()
{

}
