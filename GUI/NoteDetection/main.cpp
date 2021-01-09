#include "detectormain.h"
//#include "utils/rectools.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QThread* thread = new QThread;
    QPointer<Recorder> recorder = new Recorder;
    recorder->moveToThread(thread);
    thread->start();
    detectorMain w(recorder);

    w.show();

    return a.exec();
}
