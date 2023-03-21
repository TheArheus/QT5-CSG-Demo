#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QWheelEvent>

#include "mat_h.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Run();

private slots:
    void on_pushButton_clicked();
    void mouseMoveEvent(QMouseEvent *event);
    void mouseWheel(QWheelEvent *event);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
