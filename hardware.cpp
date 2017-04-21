#include "hardware.h"

void Hardware::lshw(){

          QString program = "lshw";
          QStringList arguments;

          arguments << "-json";
          QProcess *myProcess = new QProcess(this);

          myProcess->start(program,arguments);
          myProcess->waitForFinished();
          myProcess->readyRead();
          datoslshw = myProcess->readAllStandardOutput();
          emit ejecute(2);


}

QByteArray Hardware::transferenciadatos(){

    return datoslshw;

}
