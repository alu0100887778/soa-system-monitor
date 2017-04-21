#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sensores = new info_sensores();
    hilo_sens = new hilo_sensores(sensores,this);

    ui->tabla_proc->setColumnCount(5);

    connect(hilo_sens, SIGNAL(refreshInfo(int)), this, SLOT(on_Tabs_currentChanged(int)));

    hilo_sens->start();

    temp.start(5000);
    //connect(temp,SIGNAL(QTimer::timeout()),this, SLOT(on_Tabs_currentChanged(1)));

    connect(&hardware_,SIGNAL(ejecute(int)),this,SLOT(on_tabWidget_tabBarClicked(int)));
    hardware_.moveToThread(&hilohard);
    connect(&hilohard,SIGNAL(started()),&hardware_,SLOT(lshw()));
    hilohard.start();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Tabs_currentChanged(int index)
{
    QPair<QString,QString> text;
    QDir directory("/proc");
    switch(index){

    case 0:
        ui->text_sensores->clear();
        for(int i = 0; i < hilo_sens->get_buffer_size(); i++){
            text = sensores->extract_info();
            ui->text_sensores->insertPlainText(text.first + '\t' + text.second + '\n');
        }
    break;

    case 1:
        qDebug()<<"huli";


        ui->tabla_proc->setRowCount(0);
        for(auto dir:directory.entryList(QDir::Dirs, QDir::NoSort)){
            std::string aux_string = dir.toUtf8().constData();

            if(is_numeric(aux_string)){
                QString actual_dir("/proc/" + dir);
                    QFuture<QVector<QString>> fut = QtConcurrent::run(this, &MainWindow::explorar, actual_dir, dir);
                     QFutureWatcher<QVector<QString>> *watcher = new QFutureWatcher<QVector<QString>>;
                        watcher->setFuture(fut);

                            qDebug()<<watcher->isRunning();
                            connect(watcher, &QFutureWatcher<QVector<QString>>::finished, [this,watcher](){
                            QVector<QString> aux;
                            qDebug()<<"holi";
                            aux = watcher->result();

                            int j = ui->tabla_proc->rowCount();

                            ui->tabla_proc->setRowCount(ui->tabla_proc->rowCount()+1);

                            for(int i = 0; i < 5; i++){
                                ui->tabla_proc->setItem(j-1,i,new QTableWidgetItem (aux[i]));
                            }

                            watcher->deleteLater();
                });
            }
        }
    break;
    case 2:{

        QByteArray aux = hardware_.transferenciadatos();
        QJsonModel * modelo = new QJsonModel;

        ui->treeView->setColumnWidth(0,100);
        ui->treeView->setModel(modelo);
        modelo->loadJson(aux);
}
    break;

    default:
        break;
   }
}

bool MainWindow::is_numeric(const std::string s){
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c){
        return !std::isdigit(c);
    }) == s.end();
}

QVector<QString> MainWindow::explorar(QString dir, QString PID){


    QVector<QString> row;
    QDir directory(dir);
    QString cmd, status, name, threads;

    QFile file_aux1(dir + "/cmdline"), file_aux2(dir + "/status");
    QTextStream in1(&file_aux1), in2(&file_aux2);

    row.push_back(PID);

    if(file_aux1.open(QIODevice::ReadOnly)){
        cmd = in1.readAll();
        row.push_back(cmd);
    }

    if(file_aux2.open(QIODevice::ReadOnly)){
        status = in2.readAll();
        name = status.mid(status.lastIndexOf("Name:"), status.indexOf("\n"));
        name = name.remove("Name:\t");
        threads = status.mid(status.lastIndexOf("Threads:"),10);
        threads = threads.remove("Threads:\t");
        status = status.mid(status.lastIndexOf("State:"), 8);
        status = status.remove("State:\t");
        row.push_back(name);
        row.push_back(threads);
        row.push_back(status);
    }

    file_aux1.close();
    file_aux2.close();

    return row;
}
