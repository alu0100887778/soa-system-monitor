#include "hilo_sensores.h"

hilo_sensores::hilo_sensores(info_sensores *cont, QObject *parent):
    QThread(parent),
    buffer(cont){
}

void hilo_sensores::run(void){
    QString nombre, aux, inf;
    bool encontrado= false;
    while(1){

        sleep(1);
        QDir dir("/sys/class/hwmon");

        for(auto directory: dir.entryList(QDir::Dirs, QDir::NoSort)){
            encontrado = false;
            QString directorio_actual("/sys/class/hwmon/" + directory);

           /** if(!(directorio_actual== "/." && actual_dir == "/..")){**/
             if(!(directorio_actual== "/sys/class/hwmon/." && directorio_actual == "/sys/class/hwmon/..")){
                QDir directorio_aux(directorio_actual);

                for(auto file: directorio_aux.entryList(QDir::Files, QDir::NoSort)){
                    QFile fichero_actual(directorio_actual + "/" + file);

                    if(!fichero_actual.open(QIODevice::ReadOnly | QIODevice::Text)) return;

                    QTextStream in(&fichero_actual);
                    aux = in.readAll();
                    inf = aux.mid(0,aux.lastIndexOf('\n'));

                    if(!encontrado){
                        if(file == "name"){
                            nombre = inf;
                            encontrado = true;
                        }
                    }
                    else if(file.contains("temp") or file.contains("fan")){
                        aux = nombre + " : " + file;

                        QPair<QString,QString> pair(aux, inf);
                        buffer->insert_info(pair);
                    }
                    fichero_actual.close();
                }
            }
        }
        emit refreshInfo(0);
    }
}
int hilo_sensores::get_buffer_size(void){
    return buffer->get_counter();
}
