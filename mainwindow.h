#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "qcustomplot.h"
#include <pcap.h>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static void * channel_hop(void * data);
    static void * capture(void * data);

private slots:
    void realtime_data_slot();
    void output();
    void on_startBtn_clicked();
    void on_stopBtn_clicked();
    
private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer DataTimer;
    pcap_t* handle;
    void graph_init();
};
#endif // MAINWINDOW_H
