#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtMultimedia/QAudio>
#include <QtMultimedia/QAudioOutput>
#include <QFile>
#include <QDebug>

#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void testAudio();

    struct PlayParams {
        int sampleRate = 44100;
        int sampleSize = 8;
        int nChannel = 1;
    };

private slots:
    void on_playBtn_clicked();

    void on_stopBtn_clicked();

    void play(const PlayParams& params);

    void on_browseBtn_clicked();

    void on_sendTcp_clicked();

private:
    std::unique_ptr<QAudioOutput> audio;
    std::size_t pos = 0;
    QByteArray buffer;
    QIODevice* device = nullptr;
    bool stopFlag = false;

private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
