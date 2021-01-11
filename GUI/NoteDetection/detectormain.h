#ifndef DETECTORMAIN_H
#define DETECTORMAIN_H

#include <QMainWindow>
#include <QThread>
#include <QPointer>
#include "utils/rectools.h"


QT_BEGIN_NAMESPACE
namespace Ui { class detectorMain; }
QT_END_NAMESPACE

class detectorMain : public QMainWindow
{
    Q_OBJECT

public:
    detectorMain(QPointer<Recorder> recorder, QWidget *parent = nullptr);
    ~detectorMain();


private slots:

    void on_beginRecordingButton_clicked();

    void updateNote(std::string);

    void on_stopRecordingButton_clicked();

    void on_chooseInputDeviceButton_clicked();

private:
    Ui::detectorMain *ui;
    QPointer<Recorder> rec;
    int deviceCount = 0;
    int inputDeviceID = -1;

};
#endif // DETECTORMAIN_H
