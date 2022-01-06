#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <vector>
#include <math.h>
#include <thread>

#include <QFileDialog>
#include <QTcpSocket>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    stopFlag = true;
    audio->stop();
    delete ui;
}

void MainWindow::play(const PlayParams& params) {

    QAudioFormat format;
    int samplePerSec = params.sampleRate;
    format.setSampleRate(samplePerSec);
    format.setChannelCount(params.nChannel);
    format.setSampleSize(params.sampleSize);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
     qWarning() << "Raw audio format not supported by backend, cannot play audio.";
     return;
    }

    audio = std::make_unique<QAudioOutput>(format, this);
    audio->stop();
    device = audio->start();
    stopFlag = false;
    pos = 0;


    QTcpSocket socket;
    socket.connectToHost(ui->tcpHost->text(), ui->tcpPort->value());
    if(!socket.waitForConnected(3000)) {
        qDebug("Couldn't connect to host");
    }

    while(pos < buffer.size()) {
        auto rem = buffer.size() - pos;
        uint8_t* pStart = (uint8_t*)buffer.data() + pos;

        auto nVolumeSample = std::min(rem, 200ull);

        uint8_t min = 255;
        uint8_t max = 0;
        for(int i=0;i<nVolumeSample;++i) {
            if(pStart[i]<min) min = pStart[i];
            if(pStart[i]>max) max = pStart[i];
        }

        auto n = device->write((const char*)pStart, rem);
        pos += n;

        ui->progressBar->setValue(max - min);
        if(socket.isOpen()) {
            char val = static_cast<char>(max - min);
            socket.write(&val, 1);
        }

        QApplication::processEvents();
        if(stopFlag) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void MainWindow::on_playBtn_clicked()
{
    QFile sourceFile;
    sourceFile.setFileName(ui->fileName->text());
    sourceFile.open(QIODevice::ReadOnly);
    if(!sourceFile.isOpen()) {
        return;
    }
    buffer = sourceFile.readAll();
    sourceFile.close();

    PlayParams params;
    params.sampleRate = ui->sampleRate->value();
    params.sampleSize = ui->sampleSize->value();
    params.nChannel = ui->nChannel->value();
    play(params);
}

void MainWindow::on_stopBtn_clicked()
{
    stopFlag = true;
    audio->stop();
}

void MainWindow::on_browseBtn_clicked()
{
    QFileDialog fd;
    fd.exec();
    auto files = fd.selectedFiles();
    if(files.size() == 0) return;
    ui->fileName->setText(files[0]);
}
