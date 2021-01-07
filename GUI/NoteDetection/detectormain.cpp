#include "detectormain.h"
#include "ui_detectormain.h"
#include <QThread>
//#include "utils/rectools.h"

detectorMain::detectorMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::detectorMain)
{
    recorder = new Recorder;
    ui->setupUi(this);
    QThread* thread = new QThread;
    recorder->moveToThread(thread);
    thread->start();

    ui->beginRecordingButton->setEnabled(true);
    ui->stopRecordingButton->setEnabled(false);
    connect(recorder, SIGNAL(UpdateNote(std::string)), this, SLOT(updateNote(std::string)));
    connect(recorder, SIGNAL(UpdateBars(std::string)), this, SLOT(updateBars(std::string)));
}

detectorMain::~detectorMain()
{
    delete ui;
}



void detectorMain::on_GetDeviceCountButton_clicked()
{
    int dev_count = 0;
    dev_count = recorder->get_device_count();
//    dev_count = Recorder.get_device_count();
    ui->DeviceCountLabel->setText(QString::number(dev_count));
}

void detectorMain::on_beginRecordingButton_clicked()
{
    int br;
//    ui->beginRecordingButton->setText("Stop Recording");
    ui->beginRecordingButton->setEnabled(false);
    ui->stopRecordingButton->setEnabled(true);
    br = recorder->begin_recording();
//    ui->beginRecordingButton->setText("Begin Recording");

}

void detectorMain::updateNote(std::string note)
{
    ui->noteLabel->setText(QString::fromStdString(note));
    QCoreApplication::processEvents();
//    QTextStream(stdout) <<  QString::fromStdString(note) << "\n";
}

void detectorMain::updateBars(std::string bars)
{
    ui->barsLabel->setText(QString::fromStdString(bars));
    QCoreApplication::processEvents();
//    QTextStream(stdout) <<  QString::fromStdString(bars) << "\n";
}

void detectorMain::on_stopRecordingButton_clicked()
{
    int sr;
    ui->beginRecordingButton->setEnabled(true);
    ui->stopRecordingButton->setEnabled(false);
    sr = recorder->stop_recording();
}
