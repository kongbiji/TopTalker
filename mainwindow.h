#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void show();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    pcap_t* handle;
};
#endif // MAINWINDOW_H
