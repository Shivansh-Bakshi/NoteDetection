#include "detectormain.h"
//#include "utils/rectools.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QThread* thread = new QThread;
    QPointer<Recorder> recorder = new Recorder();
//    Recorder recorder = new Recorder();
    recorder->moveToThread(thread);
    detectorMain w(recorder);
    thread->start();

    w.show();

    return a.exec();
}
