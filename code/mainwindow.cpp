#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setAutoRepeat(true);

    ui->ObjPosX->setPlainText(QString::number(ui->widget->Cylinder.Position.x));
    ui->ObjPosY->setPlainText(QString::number(ui->widget->Cylinder.Position.y));
    ui->ObjPosZ->setPlainText(QString::number(ui->widget->Cylinder.Position.z));

    setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    vec3 NewPos = vec3(ui->MoveToX->toPlainText().toFloat(), ui->MoveToY->toPlainText().toFloat(), ui->MoveToZ->toPlainText().toFloat());
    float Vel = ui->Vel->toPlainText().toFloat();
    ui->widget->MoveTo(NewPos, Vel);
    ui->ObjPosX->setPlainText(QString::number(ui->widget->Cylinder.Position.x));
    ui->ObjPosY->setPlainText(QString::number(ui->widget->Cylinder.Position.y));
    ui->ObjPosZ->setPlainText(QString::number(ui->widget->Cylinder.Position.z));
}

void MainWindow::
mouseMoveEvent(QMouseEvent *Event)
{
    ui->widget->SetNewCamera(vec3(((Event->localPos().x() / (ui->widget->width())) * 2 - 1) / 100, 0, 0));
}

void MainWindow::
mouseWheel(QWheelEvent *event)
{

}

