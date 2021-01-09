#include "detectormain.h"
#include "ui_detectormain.h"


detectorMain::detectorMain(QPointer<Recorder> recorder, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::detectorMain)
{
    ui->setupUi(this);
    rec = recorder;

    ui->beginRecordingButton->setEnabled(true);
    ui->stopRecordingButton->setEnabled(false);

    connect(recorder, SIGNAL(UpdateNote(std::string)), this, SLOT(updateNote(std::string)));

}

detectorMain::~detectorMain()
{
    delete ui;
}



void detectorMain::on_GetDeviceCountButton_clicked()
{
    int dev_count = 0;
    dev_count = rec->get_device_count();
//    dev_count = Recorder.get_device_count();
    ui->DeviceCountLabel->setText(QString::number(dev_count));
}

void detectorMain::on_beginRecordingButton_clicked()
{
    int br;
    ui->beginRecordingButton->setEnabled(false);
    ui->GetDeviceCountButton->setEnabled(false);
    ui->stopRecordingButton->setEnabled(true);
    br = rec->begin_recording();
    if(br != paNoError)
    {
        QTextStream(stdout) << "Some Error Occured";
    }

}

void detectorMain::updateNote(std::string note)
{
    ui->noteLabel->setText(QString::fromStdString(note));
    QCoreApplication::processEvents();
//    QTextStream(stdout) <<  QString::fromStdString(note) << "\n";
}

void detectorMain::on_stopRecordingButton_clicked()
{
    rec->stop_recording();

    ui->stopRecordingButton->setEnabled(false);
    ui->beginRecordingButton->setEnabled(true);
    ui->GetDeviceCountButton->setEnabled(true);

    ui->noteLabel->setText("Done");
}
