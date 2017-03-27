#include "info_sensores.h"

info_sensores::info_sensores():
contador(0){
    qDebug() << "Declarando nuevo contenedor de sensores.";
}

info_sensores::~info_sensores(){
}

int info_sensores::get_counter(void){
    return contador;
}

void info_sensores::insert_info(const QPair<QString,QString> &pair){
    mutex.lock();
    info.push_back(pair);
    contador++;
    mutex.unlock();
}

QPair<QString, QString> info_sensores::extract_info(){
    if(contador != 0){
        mutex.lock();
        QPair<QString, QString> datos = info[info.size()-1];
        info.pop_back();
        contador--;
        mutex.unlock();
        return datos;
    }
    else{
        QPair<QString, QString> datos;
        return datos;
    }
}

