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
    timer = new QTimer;
    timer->start();
    timer->setInterval(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::output(){
    ui->textBrowser->setText(QString("[CH: %1]").arg(chan));
    for(beacon_iter = beacon_map.begin();beacon_iter != beacon_map.end();beacon_iter++){
        char mac[18] = {0,};
        print_MAC((uint8_t *)beacon_iter->first.mac, mac);
        QString pwr = QString("%1(%2) >> -%3").arg(beacon_iter->second.ssid).arg(mac).arg(beacon_iter->second.PWR);
        ui->textBrowser->append(pwr);
    } ui->textBrowser->append("============");
    for(probe_iter = probe_map.begin();probe_iter != probe_map.end();probe_iter++){
        char mac[18] = {0, };
        print_MAC((uint8_t *)probe_iter->first.station, mac);
        QString pwr = QString("%1 >> -%2").arg(mac).arg(probe_iter->second.PWR);
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
        beacon_v.clear();
        for (auto iter = beacon_map.begin(); iter != beacon_map.end(); ++iter)
            beacon_v.emplace_back(std::make_pair(iter->first, iter->second));
        sort(beacon_v.begin(), beacon_v.end(), compare_pair_second<std::less>());
        probe_v.clear();
        for (auto iter = probe_map.begin(); iter != probe_map.end(); ++iter)
            probe_v.emplace_back(std::make_pair(iter->first, iter->second));
        sort(probe_v.begin(), probe_v.end(), compare_pair_second<std::less>());
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
}

void MainWindow::on_stopBtn_clicked()
{
    run = 0;
    disconnect(timer, SIGNAL(timeout()), this, SLOT(output()));
    ui->textBrowser->append("[-] Stop scanning");
    beacon_map.clear();
    probe_map.clear();
}
