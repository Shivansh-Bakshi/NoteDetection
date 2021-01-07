#ifndef DETECTORMAIN_H
#define DETECTORMAIN_H

#include <QMainWindow>
#include "utils/rectools.h"
QT_BEGIN_NAMESPACE
namespace Ui { class detectorMain; }
QT_END_NAMESPACE

class detectorMain : public QMainWindow
{
    Q_OBJECT

public:
    detectorMain(QWidget *parent = nullptr);
    ~detectorMain();


private slots:
    void on_GetDeviceCountButton_clicked();

    void on_beginRecordingButton_clicked();

    void updateNote(std::string);
    void updateBars(std::string);

    void on_stopRecordingButton_clicked();

private:
    Ui::detectorMain *ui;
    Recorder* recorder;
//    QString note_detected = "";
//    QString bars = "";
};
#endif // DETECTORMAIN_H
