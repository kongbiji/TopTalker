#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TCheckPacket.h"
#include "TPacketData.h"
#include <pthread.h>

pthread_t hop_th, cap_th, show_th;
volatile int chan = 0;
volatile int run = 0;

extern map<MAC, Beacon_values> beacon_map;
extern map<MAC, Beacon_values>::iterator beacon_iter;
extern map<CONV_MAC, Probe_values> probe_map;
extern map<CONV_MAC, Probe_values>::iterator probe_iter;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

