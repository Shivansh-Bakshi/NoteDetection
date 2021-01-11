#include "detectormain.h"
#include "ui_detectormain.h"
#include <QTextStream>

detectorMain::detectorMain(QPointer<Recorder> recorder, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::detectorMain)
{
    ui->setupUi(this);

    rec = recorder;

    ui->beginRecordingButton->setEnabled(true);
    ui->stopRecordingButton->setEnabled(false);

    deviceCount = rec->get_device_count();

    for(int i = 0; i < deviceCount; i++)
    {
        PaDeviceInfo di = rec->get_device_info(i);
        if(di.maxInputChannels == 0)
            continue;
        ui->inputDevicesComboBox->addItem(di.name);
    }

    connect(recorder, SIGNAL(UpdateNote(std::string)), this, SLOT(updateNote(std::string)));

}

detectorMain::~detectorMain()
{
    delete ui;
}


void detectorMain::on_beginRecordingButton_clicked()
{
    int br;
    ui->beginRecordingButton->setEnabled(false);
    ui->chooseInputDeviceButton->setEnabled(false);
    ui->stopRecordingButton->setEnabled(true);
    br = rec->begin_recording(inputDeviceID);
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
    ui->chooseInputDeviceButton->setEnabled(true);

    ui->noteLabel->setText("Done");
}

void detectorMain::on_chooseInputDeviceButton_clicked()
{
//    QTextStream(stdout) << ui->inputDevicesComboBox->currentIndex();
    inputDeviceID = ui->inputDevicesComboBox->currentIndex();
}
