#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QTimer>
#include <QTableWidgetItem>
#include "hilo_sensores.h"
#include "info_sensores.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_Tabs_currentChanged(int index);

private:

    bool is_numeric(const std::string s);
    QVector<QString> explorar(QString actual_dir, QString dir);

    Ui::MainWindow *ui;
    info_sensores *sensores;
    hilo_sensores *hilo_sens;
    QTimer temp;


};

#endif // MAINWINDOW_H
