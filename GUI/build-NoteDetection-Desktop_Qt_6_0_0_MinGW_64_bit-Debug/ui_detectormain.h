/********************************************************************************
** Form generated from reading UI file 'detectormain.ui'
**
** Created by: Qt User Interface Compiler version 6.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DETECTORMAIN_H
#define UI_DETECTORMAIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_detectorMain
{
public:
    QWidget *centralwidget;
    QPushButton *chooseInputDeviceButton;
    QPushButton *beginRecordingButton;
    QLabel *noteLabel;
    QPushButton *stopRecordingButton;
    QComboBox *inputDevicesComboBox;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *detectorMain)
    {
        if (detectorMain->objectName().isEmpty())
            detectorMain->setObjectName(QString::fromUtf8("detectorMain"));
        detectorMain->resize(800, 600);
        centralwidget = new QWidget(detectorMain);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        chooseInputDeviceButton = new QPushButton(centralwidget);
        chooseInputDeviceButton->setObjectName(QString::fromUtf8("chooseInputDeviceButton"));
        chooseInputDeviceButton->setGeometry(QRect(470, 10, 141, 31));
        beginRecordingButton = new QPushButton(centralwidget);
        beginRecordingButton->setObjectName(QString::fromUtf8("beginRecordingButton"));
        beginRecordingButton->setGeometry(QRect(250, 70, 141, 31));
        noteLabel = new QLabel(centralwidget);
        noteLabel->setObjectName(QString::fromUtf8("noteLabel"));
        noteLabel->setGeometry(QRect(170, 130, 451, 331));
        QFont font;
        font.setPointSize(48);
        noteLabel->setFont(font);
        noteLabel->setAlignment(Qt::AlignCenter);
        stopRecordingButton = new QPushButton(centralwidget);
        stopRecordingButton->setObjectName(QString::fromUtf8("stopRecordingButton"));
        stopRecordingButton->setGeometry(QRect(410, 70, 141, 31));
        inputDevicesComboBox = new QComboBox(centralwidget);
        inputDevicesComboBox->setObjectName(QString::fromUtf8("inputDevicesComboBox"));
        inputDevicesComboBox->setGeometry(QRect(10, 10, 451, 31));
        detectorMain->setCentralWidget(centralwidget);
        menubar = new QMenuBar(detectorMain);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        detectorMain->setMenuBar(menubar);
        statusbar = new QStatusBar(detectorMain);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        detectorMain->setStatusBar(statusbar);

        retranslateUi(detectorMain);

        QMetaObject::connectSlotsByName(detectorMain);
    } // setupUi

    void retranslateUi(QMainWindow *detectorMain)
    {
        detectorMain->setWindowTitle(QCoreApplication::translate("detectorMain", "detectorMain", nullptr));
        chooseInputDeviceButton->setText(QCoreApplication::translate("detectorMain", "Choose Input Device", nullptr));
        beginRecordingButton->setText(QCoreApplication::translate("detectorMain", "Begin Recording", nullptr));
        noteLabel->setText(QCoreApplication::translate("detectorMain", "Note Played", nullptr));
        stopRecordingButton->setText(QCoreApplication::translate("detectorMain", "Stop Recording", nullptr));
    } // retranslateUi

};

namespace Ui {
    class detectorMain: public Ui_detectorMain {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DETECTORMAIN_H
