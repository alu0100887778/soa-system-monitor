#ifndef HILO_SENSORES_H
#define HILO_SENSORES_H

#include "info_sensores.h"
#include <QThread>
#include <QDir>
#include <QVector>


class hilo_sensores : public QThread
{
    Q_OBJECT
private:

    info_sensores *buffer;
public:

    explicit hilo_sensores(info_sensores *cont, QObject *parent = 0);
    int get_buffer_size(void);

public slots:

    void run (void);

signals:

    void refreshInfo(int);
};

#endif // HILO_SENSORES_H
