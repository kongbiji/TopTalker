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

extern vector<pair<MAC,Beacon_values> > beacon_v;
extern vector<pair<CONV_MAC,Probe_values> > probe_v;
extern vector<pair<MAC,Beacon_values>>::iterator beacon_v_iter;
extern vector<pair<CONV_MAC,Probe_values>>::iterator probe_v_iter;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer;
    timer->start();
    timer->setInterval(1000);
}

MainWindow::~MainWindow()
{
    beacon_v.clear();
    probe_v.clear();
    beacon_map.clear();
    probe_map.clear();
    delete ui;
}

void MainWindow::graph_init(){
    ui->qGraph->addGraph(); // blue line
    ui->qGraph->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->qGraph->addGraph(); // red line
    ui->qGraph->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->qGraph->xAxis->setTicker(timeTicker);
    ui->qGraph->axisRect()->setupFullAxesBox();
    ui->qGraph->yAxis->setRange(0, 400);
    ui->qGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
    ui->qGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->qGraph->legend->setVisible(true);
    ui->qGraph->legend->setBrush(QBrush(QColor(255,255,255,150)));
    ui->qGraph->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignLeft|Qt::AlignTop);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->qGraph->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->qGraph->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->qGraph->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->qGraph->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&DataTimer, SIGNAL(timeout()), this, SLOT(realtime_data_slot()));
    DataTimer.start(1000); // Interval 0 means to refresh as fast as possible
}

void MainWindow::realtime_data_slot(){
    static QTime time(QTime::currentTime());
    qsrand(QTime::currentTime().msecsSinceStartOfDay());

    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    double value=qrand()%400+0;
    ui->qGraph->graph(0)->addData(key, value);
    QCPItemText *item=new QCPItemText(ui->qGraph);

    item->setPositionAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
    //item->position->setType(QCPItemPosition::ptAxisRectRatio);
    item->position->setCoords(key,value+10);
    item->setColor(ui->qGraph->graph(0)->pen().color());
    item->setText(QString::number(value));

    ui->qGraph->graph(1)->addData(key, qrand()%400+0);

    // if (key-lastPointKey > 0.002) // at most add point every 2 ms
    // {
    // add data to lines:

    // rescale value (vertical) axis to fit the current data:
    //    ui->customPlot->graph(0)->rescaleValueAxis();
    //    ui->widget_RealTimeGraph->graph(1)->rescaleValueAxis(true);
    //    lastPointKey = key;
    //  }
    // make key axis range scroll with the data (at a constant range size of 8):

    ui->qGraph->xAxis->setRange(key,8,Qt::AlignRight);
    ui->qGraph->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
//    if (key-lastFpsKey > 1) // average fps over 2 seconds
//    {
//        ui->statusBar->showMessage(
//                    QString("%1 FPS, Total Data points: %2")
//                    .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
//                    .arg(ui->widget_RealTimeGraph->graph(0)->data()->size()+ui->widget_RealTimeGraph->graph(1)->data()->size())
//                    , 0);
//        lastFpsKey = key;
//        frameCount = 0;
//    }
}

void MainWindow::output(){
    beacon_v.clear();
    for (auto iter = beacon_map.begin(); iter != beacon_map.end(); ++iter)
        beacon_v.emplace_back(std::make_pair(iter->first, iter->second));
    sort(beacon_v.begin(), beacon_v.end(), compare_pair_second<std::less>());
    probe_v.clear();
    for (auto iter = probe_map.begin(); iter != probe_map.end(); ++iter)
        probe_v.emplace_back(std::make_pair(iter->first, iter->second));
    sort(probe_v.begin(), probe_v.end(), compare_pair_second<std::less>());

    ui->textBrowser->setText(QString("[CH: %1]").arg(chan));
    for(beacon_v_iter = beacon_v.begin(); beacon_v_iter != beacon_v.begin(); beacon_v_iter++){
        char mac[18] = {0,};
        print_MAC((uint8_t *)beacon_v_iter->first.mac, mac);
        QString pwr = QString("%1(%2) >> -%3").arg(beacon_v_iter->second.ssid).arg(mac).arg(beacon_v_iter->second.PWR);
        ui->textBrowser->append(pwr);
    } ui->textBrowser->append("============");
    for(probe_v_iter = probe_v.begin(); probe_v_iter != probe_v.end(); probe_v_iter++){
        char mac[18] = {0, };
        print_MAC((uint8_t *)probe_v_iter->first.station, mac);
        QString pwr = QString("%1 >> -%2").arg(mac).arg(probe_v_iter->second.PWR);
        ui->textBrowser->append(pwr);
    }

}

void * MainWindow::channel_hop(void * data){
    char command[50], chan_num[3], real_command[50];
    int CHAN_NUM[14]= {1,7,13,2,8,3,14,9,4,10,5,11,6,12};
    int i = 0;

    memset(command, 0, sizeof(command));
    strcpy(command, "iwconfig ");
    strcat(command, (char *)data );
    strcat(command, " channel ");

    while(run == 1){
        real_command[0] = '\0';
        chan_num[0] = '\0';
        chan = CHAN_NUM[i];
        sprintf(chan_num, "%d", CHAN_NUM[i]);
        strcpy(real_command, command);
        strcat(real_command, chan_num);
        system(real_command);
        i++;
        if(i > 14){
            i = 0;
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

void * MainWindow::capture(void * handle){
    struct pcap_pkthdr * header;
    const u_char * data;

    while(run == 1){
        int res = pcap_next_ex((pcap_t *)handle, &header, &data);
        if(res == 0) return 0;
        if(res == -1 || res == -2) return 0;

        int type = check_packet_type(data);
        switch (type){
        case BEACON_FRAME:
            save_Beacon_info(data);
            break;
        case PROBE_REQUEST:
            save_Probe_info(data, PROBE_REQUEST);
            break;
        case PROBE_RESPONSE:
            save_Probe_info(data, PROBE_REQUEST);
            break;
        case QOS_DATA:
            save_QoS_info(data, QOS_DATA);
            break;
        case QOS_NULL:
            save_QoS_info(data, QOS_NULL);
            break;
        default:
            break;
        }
    }
    pthread_exit(NULL);
}

void MainWindow::on_startBtn_clicked()
{
    QString dev = ui->textEdit->toPlainText();

    char iface[10] = {0,};
    strcpy(iface, dev.toStdString().c_str());

    if(dev == NULL){
        ui->textBrowser->setText("[-] Please Enter the interface name!");
        return;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_live(iface, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        ui->textBrowser->setText("[-] Couldn't open device. Plz retry.");
        return;
    }
    ui->textBrowser->setText("[*] Start scanning...");

    run = 1;
    pthread_create(&hop_th, NULL, &MainWindow::channel_hop, (void *)iface);
    pthread_detach(hop_th);

    pthread_create(&cap_th, NULL, &MainWindow::capture, (void *)handle);
    pthread_detach(cap_th);

    connect(timer, SIGNAL(timeout()), this, SLOT(output()));
    graph_init();
    DataTimer.start(1000);
}

void MainWindow::on_stopBtn_clicked()
{
    run = 0;
    disconnect(timer, SIGNAL(timeout()), this, SLOT(output()));
    ui->textBrowser->append("[-] Stop scanning");
    beacon_map.clear();
    probe_map.clear();
    DataTimer.stop();
}
