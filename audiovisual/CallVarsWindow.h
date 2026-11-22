#pragma once

#include "FoxCore.h"
#include <QMainWindow>
#include <QAudioSource>
#include <QAudioSink>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QWebSocket>
#include <QTimer>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QIODevice>
#include <QByteArray>
#include <QNetworkRequest>
#include <QImage>
#include <QBuffer>

class CallVarsWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CallVarsWindow(QWidget* parent = nullptr);
    ~CallVarsWindow() override;

private slots:
    void connectLive();
    void disconnectLive();
    void sendText();
    void startCamera();
    void stopCamera();
    void captureFrame();
    void onSocketText(const QString& message);
    void onMicReady();
    void onVideoFrame(const QVideoFrame& frame);

private:
    void buildUi();
    void setStatus(const QString& status);
    void setFoxState(const QString& state);
    void appendTranscript(const QString& who, const QString& text);
    void sendJson(const QJsonObject& object);
    void startAudio();
    void stopAudio();
    void playPcmChunk(const QByteArray& bytes);
    QString apiKey() const;
    QByteArray imageAsDataUrl(const QImage& image) const;

    FoxCharacter m_fox;
    FoxStorage m_storage;
    FoxMathEngine m_math;
    FoxMail m_mail;

    QWebSocket* m_socket = nullptr;
    QAudioSource* m_audioSource = nullptr;
    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_micDevice = nullptr;
    QIODevice* m_speakerDevice = nullptr;
    QCamera* m_camera = nullptr;
    QMediaCaptureSession* m_captureSession = nullptr;
    QVideoSink* m_videoSink = nullptr;
    QTimer* m_cameraTimer = nullptr;

    QLabel* m_foxFace = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_cameraPreview = nullptr;
    QPlainTextEdit* m_transcript = nullptr;
    QLineEdit* m_message = nullptr;
    QLineEdit* m_key = nullptr;
    QPushButton* m_connectButton = nullptr;
    QPushButton* m_cameraButton = nullptr;
    QComboBox* m_voice = nullptr;

    QImage m_latestFrame;
    bool m_connected = false;
    bool m_playing = false;
};
