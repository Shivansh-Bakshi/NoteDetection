#include "detectormain.h"
#include "ui_detectormain.h"
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>

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

    connect(recorder, SIGNAL(UpdateNote(int)), this, SLOT(updateNote(int)));

}

detectorMain::~detectorMain()
{
    delete ui;
}


void detectorMain::on_beginRecordingButton_clicked()
{
    bool generateMidi = ui->generateMidiCheckBox->isChecked();
    ui->beginRecordingButton->setEnabled(false);
    ui->chooseInputDeviceButton->setEnabled(false);
    ui->generateMidiCheckBox->setEnabled(false);
    ui->outputFileName->setEnabled(false);
    ui->stopRecordingButton->setEnabled(true);

    filename = ui->outputFileName->toPlainText();

    int br = rec->begin_recording(inputDeviceID, generateMidi, filename);
//    QFuture<int> future = QtConcurrent::run(this->rec, &Recorder::begin_recording, inputDeviceID, generateMidi);

//    QCoreApplication::processEvents();
//    QTextStream(stdout) << "Called in a seperate thread";

//    int br = future.result();
//    if(br != paNoError)
//    {
//        QTextStream(stdout) << "Some Error Occured";
//    }
}

void detectorMain::updateNote(int note)
{
    if(note != -1)
    {
        ui->noteLabel->setText(QString::fromStdString(note_names[note]));
    }
    else
        ui->noteLabel->setText("");

    QCoreApplication::processEvents();
}

void detectorMain::on_stopRecordingButton_clicked()
{
    QTextStream(stdout) << "Stopping Recording\n";
    rec->stop_recording();

    ui->stopRecordingButton->setEnabled(false);
    ui->beginRecordingButton->setEnabled(true);
    ui->chooseInputDeviceButton->setEnabled(true);
    ui->generateMidiCheckBox->setEnabled(true);
    ui->outputFileName->setEnabled(true);

    ui->noteLabel->setText("Done");
    QCoreApplication::processEvents();
}

void detectorMain::on_chooseInputDeviceButton_clicked()
{
    inputDeviceID = ui->inputDevicesComboBox->currentIndex();
}



