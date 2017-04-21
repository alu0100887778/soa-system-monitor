#ifndef HARDWARE_H
#define HARDWARE_H


#include <QDebug>
#include <iostream>
#include <QMainWindow>
#include <QTabWidget>
#include <QThreadPool>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QDir>
#include <QPair>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QProcess>



using namespace std ;


class Hardware : public QObject
{

    Q_OBJECT

private:

     QByteArray datoslshw;

public slots :

     void lshw();
    QByteArray transferenciadatos();

signals:
    void ejecute(int);


};

#endif // HARDWARE_H
