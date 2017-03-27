#ifndef INFO_SENSORES_H
#define INFO_SENSORES_H

#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>
#include <QPair>

class info_sensores  : public QObject{
private:
    int contador;
    QVector<QPair<QString, QString>> info;
    QMutex mutex;
public:
    info_sensores();
    ~info_sensores();

    int get_counter();

    void insert_info(const QPair<QString,QString> &pair);
    QPair<QString,QString> extract_info();
};

#endif // INFO_SENSORES_H
