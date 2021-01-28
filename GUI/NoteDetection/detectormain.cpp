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

    connect(recorder, SIGNAL(UpdateNote(int)), this, SLOT(updateNote(int)));

}

detectorMain::~detectorMain()
{
    delete ui;
}


void detectorMain::on_beginRecordingButton_clicked()
{
    int br;
    bool generateMidi = ui->generateMidiCheckBox->isChecked();
//    midi_notes.clear();
    ui->beginRecordingButton->setEnabled(false);
    ui->chooseInputDeviceButton->setEnabled(false);
    ui->stopRecordingButton->setEnabled(true);
    br = rec->begin_recording(inputDeviceID, generateMidi);
    if(br != paNoError)
    {
        QTextStream(stdout) << "Some Error Occured";
    }

}

void detectorMain::updateNote(int note)
{
    if(note != -1)
    {
        ui->noteLabel->setText(QString::fromStdString(note_names[note]));
    }
    else
        ui->noteLabel->setText("");

//    midi_notes.push_back(note);
    QCoreApplication::processEvents();
//    QTextStream(stdout) <<  QString::fromStdString(note) << "\n";
}

void detectorMain::on_stopRecordingButton_clicked()
{
    QTextStream(stdout) << "Stopping Recording\n";
    rec->stop_recording();
//    midi_notes = rec->get_midi_notes();
//    QTextStream(stdout) << "Generating Midi\n";
//    rec->generate_midi(midi_notes, filename);
//    QTextStream(stdout) << "Midi Saved\n";
//    QTextStream(stdout) << "Midi Notes: \n";
//    for (const auto& i : midi_notes)
//    {
//        QTextStream(stdout) <<"Note: " << i.first << "\tDuration: " << i.second << "\n";
//    }

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



