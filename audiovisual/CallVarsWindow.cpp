#include "CallVarsWindow.h"
#include <QApplication>
#include <QTextCursor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QVideoFrame>
#include <QPixmap>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>

namespace {
constexpr int kSampleRate = 24000;
QAudioFormat pcmFormat() {
    QAudioFormat f;
    f.setSampleRate(kSampleRate);
    f.setChannelCount(1);
    f.setSampleFormat(QAudioFormat::Int16);
    return f;
}
}

CallVarsWindow::CallVarsWindow(QWidget* parent)
    : QMainWindow(parent), m_mail(m_storage) {
    buildUi();
    m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(m_socket, &QWebSocket::connected, this, [this]() {
        m_connected = true;
        setStatus("Connected — FOX is listening");
        m_connectButton->setText("Disconnect");
        setFoxState("LISTENING");
        QJsonObject session;
        session["type"] = "realtime";
        session["model"] = "gpt-realtime";
        session["instructions"] = m_fox.systemIdentity()
            + " You are in a live audiovisual conversation. Use camera frames when supplied. "
              "If the user interrupts, stop speaking and listen. Do not claim facial-expression "
              "or gaze detection unless actually available from the frame.";
        session["output_modalities"] = QJsonArray{"audio", "text"};
        QJsonObject inputAudio;
        inputAudio["format"] = QJsonObject{{"type", "audio/pcm"}, {"rate", kSampleRate}};
        inputAudio["turn_detection"] = QJsonObject{{"type", "server_vad"}, {"create_response", true}, {"interrupt_response", true}};
        QJsonObject outputAudio;
        outputAudio["format"] = QJsonObject{{"type", "audio/pcm"}, {"rate", kSampleRate}};
        outputAudio["voice"] = m_voice->currentText();
        session["audio"] = QJsonObject{{"input", inputAudio}, {"output", outputAudio}};
        sendJson(QJsonObject{{"type", "session.update"}, {"session", session}});
        startAudio();
        m_storage.appendLog("SYSTEM", "Realtime session connected");
    });
    connect(m_socket, &QWebSocket::disconnected, this, [this]() {
        m_connected = false;
        stopAudio();
        setStatus("Disconnected");
        m_connectButton->setText("Connect FOX");
        setFoxState("IDLE");
    });
    connect(m_socket, &QWebSocket::textMessageReceived, this, &CallVarsWindow::onSocketText);
    connect(m_socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        setStatus("Connection error: " + m_socket->errorString());
        m_storage.appendLog("ERROR", m_socket->errorString());
    });

    m_cameraTimer = new QTimer(this);
    m_cameraTimer->setInterval(1800);
    connect(m_cameraTimer, &QTimer::timeout, this, &CallVarsWindow::captureFrame);
}

CallVarsWindow::~CallVarsWindow() {
    stopAudio();
    stopCamera();
    if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) m_socket->close();
}

void CallVarsWindow::buildUi() {
    setWindowTitle("CALL-VARS | FOX Audiovisual AI");
    resize(1050, 720);
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    auto* header = new QHBoxLayout();
    m_foxFace = new QLabel("  ◢\n◢◣ FOX ◢◣\n  ◥", central);
    m_foxFace->setMinimumWidth(220);
    m_foxFace->setAlignment(Qt::AlignCenter);
    m_foxFace->setStyleSheet("QLabel { background:#101820; color:#64c8ff; border:2px solid #17628a; border-radius:14px; font-size:24px; font-weight:700; padding:18px; }");
    auto* titleBox = new QVBoxLayout();
    auto* title = new QLabel("CALL-VARS", central);
    title->setStyleSheet("font-size:28px;font-weight:800;");
    auto* subtitle = new QLabel("Full-duplex voice • camera perception • reasoning • FOX character", central);
    subtitle->setWordWrap(true);
    m_status = new QLabel("Offline", central);
    m_status->setStyleSheet("color:#b7791f;font-weight:700;");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    titleBox->addWidget(m_status);
    header->addWidget(m_foxFace);
    header->addLayout(titleBox, 1);
    root->addLayout(header);

    auto* controls = new QGridLayout();
    m_key = new QLineEdit(central);
    m_key->setEchoMode(QLineEdit::Password);
    m_key->setPlaceholderText("OpenAI API key (kept in memory only; not saved)");
    m_connectButton = new QPushButton("Connect FOX", central);
    m_voice = new QComboBox(central);
    m_voice->addItems({"marin", "cedar", "alloy", "ash", "ballad", "coral", "echo", "sage", "shimmer", "verse"});
    m_cameraButton = new QPushButton("Start camera", central);
    controls->addWidget(new QLabel("API key:"), 0, 0);
    controls->addWidget(m_key, 0, 1, 1, 3);
    controls->addWidget(m_connectButton, 0, 4);
    controls->addWidget(new QLabel("Voice:"), 1, 0);
    controls->addWidget(m_voice, 1, 1);
    controls->addWidget(m_cameraButton, 1, 4);
    root->addLayout(controls);

    auto* body = new QHBoxLayout();
    m_transcript = new QPlainTextEdit(central);
    m_transcript->setReadOnly(true);
    m_transcript->setPlaceholderText("Conversation transcript and system events appear here...");
    m_cameraPreview = new QLabel("Camera stopped", central);
    m_cameraPreview->setAlignment(Qt::AlignCenter);
    m_cameraPreview->setMinimumSize(300, 220);
    m_cameraPreview->setStyleSheet("background:#111;color:#aaa;border:1px solid #444;border-radius:8px;");
    auto* right = new QVBoxLayout();
    right->addWidget(new QLabel("VISUAL PERCEPTION", central));
    right->addWidget(m_cameraPreview, 1);
    auto* mathBox = new QGroupBox("FOX MATH", central);
    auto* mathLayout = new QVBoxLayout(mathBox);
    auto* mathHint = new QLabel("Type /math 12 * 4 in the message box to calculate locally.", mathBox);
    mathHint->setWordWrap(true);
    mathLayout->addWidget(mathHint);
    right->addWidget(mathBox);
    body->addWidget(m_transcript, 2);
    body->addLayout(right, 1);
    root->addLayout(body, 1);

    auto* composer = new QHBoxLayout();
    m_message = new QLineEdit(central);
    m_message->setPlaceholderText("Talk to FOX or type a message...");
    auto* send = new QPushButton("Send", central);
    composer->addWidget(m_message, 1);
    composer->addWidget(send);
    root->addLayout(composer);
    setCentralWidget(central);

    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        if (m_connected) disconnectLive(); else connectLive();
    });
    connect(send, &QPushButton::clicked, this, &CallVarsWindow::sendText);
    connect(m_message, &QLineEdit::returnPressed, this, &CallVarsWindow::sendText);
    connect(m_cameraButton, &QPushButton::clicked, this, [this]() {
        if (m_camera) stopCamera(); else startCamera();
    });
    appendTranscript("FOX", m_fox.greeting());
}

QString CallVarsWindow::apiKey() const {
    const QString fromEnvironment = qEnvironmentVariable("OPENAI_API_KEY");
    return fromEnvironment.isEmpty() ? m_key->text().trimmed() : fromEnvironment;
}

void CallVarsWindow::connectLive() {
    const QString key = apiKey();
    if (key.isEmpty()) {
        QMessageBox::warning(this, "API key required", "Enter an OpenAI API key or set OPENAI_API_KEY in your Windows environment.");
        return;
    }
    QNetworkRequest request(QUrl("wss://api.openai.com/v1/realtime?model=gpt-realtime"));
    request.setRawHeader("Authorization", QByteArray("Bearer ") + key.toUtf8());
    request.setRawHeader("OpenAI-Safety-Identifier", "callvars-desktop-user");
    setStatus("Connecting...");
    m_socket->open(request);
}

void CallVarsWindow::disconnectLive() {
    stopAudio();
    m_socket->close();
}

void CallVarsWindow::sendJson(const QJsonObject& object) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->sendTextMessage(QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact)));
}

void CallVarsWindow::sendText() {
    const QString text = m_message->text().trimmed();
    if (text.isEmpty()) return;
    m_message->clear();
    appendTranscript("YOU", text);
    if (text.startsWith("/math ", Qt::CaseInsensitive)) {
        const QString result = m_math.calculate(text.mid(6));
        appendTranscript("FOX MATH", result);
        m_storage.appendLog("MATH", text + " = " + result);
        return;
    }
    if (text.startsWith("/draft ", Qt::CaseInsensitive)) {
        const bool ok = m_mail.saveDraft(QStringLiteral("unspecified"), QStringLiteral("Call-Vars draft"), text.mid(7));
        appendTranscript("FOX MAIL", ok ? "Draft saved locally." : "Could not save draft.");
        return;
    }
    if (!m_connected) {
        appendTranscript("SYSTEM", "Connect FOX to send a live AI message.");
        return;
    }
    QJsonObject content{{"type", "input_text"}, {"text", text}};
    QJsonObject item{{"type", "message"}, {"role", "user"}, {"content", QJsonArray{content}}};
    sendJson(QJsonObject{{"type", "conversation.item.create"}, {"item", item}});
    sendJson(QJsonObject{{"type", "response.create"}});
    m_storage.appendLog("USER_TEXT", text);
}

void CallVarsWindow::startAudio() {
    const auto format = pcmFormat();
    const auto input = QMediaDevices::defaultAudioInput();
    const auto output = QMediaDevices::defaultAudioOutput();
    if (input.isNull() || output.isNull()) {
        setStatus("Connected, but microphone or speaker is unavailable");
        return;
    }
    if (!input.isFormatSupported(format) || !output.isFormatSupported(format)) {
        setStatus("Audio device does not support 24 kHz mono PCM. Use a compatible device.");
        return;
    }
    m_audioSource = new QAudioSource(input, format, this);
    m_audioSink = new QAudioSink(output, format, this);
    m_micDevice = m_audioSource->start();
    m_speakerDevice = m_audioSink->start();
    if (m_micDevice) connect(m_micDevice, &QIODevice::readyRead, this, &CallVarsWindow::onMicReady);
    setStatus("Connected — microphone streaming; server VAD enabled");
}

void CallVarsWindow::stopAudio() {
    if (m_audioSource) { m_audioSource->stop(); m_audioSource->deleteLater(); m_audioSource = nullptr; }
    if (m_audioSink) { m_audioSink->stop(); m_audioSink->deleteLater(); m_audioSink = nullptr; }
    m_micDevice = nullptr;
    m_speakerDevice = nullptr;
}

void CallVarsWindow::onMicReady() {
    if (!m_connected || !m_micDevice) return;
    const QByteArray pcm = m_micDevice->readAll();
    if (pcm.isEmpty()) return;
    sendJson(QJsonObject{{"type", "input_audio_buffer.append"},
                         {"audio", QString::fromLatin1(pcm.toBase64())}});
}

void CallVarsWindow::playPcmChunk(const QByteArray& bytes) {
    if (!m_speakerDevice || bytes.isEmpty()) return;
    m_speakerDevice->write(bytes);
    m_playing = true;
    setFoxState("SPEAKING");
}

void CallVarsWindow::onSocketText(const QString& message) {
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    const QJsonObject event = doc.object();
    const QString type = event.value("type").toString();

    if (type == "response.output_audio.delta") {
        playPcmChunk(QByteArray::fromBase64(event.value("delta").toString().toLatin1()));
    } else if (type == "response.output_audio_transcript.delta" || type == "response.output_text.delta") {
        const QString delta = event.value("delta").toString();
        if (!delta.isEmpty()) {
            m_transcript->moveCursor(QTextCursor::End);
            m_transcript->insertPlainText(delta);
        }
    } else if (type == "response.output_audio_transcript.done" || type == "response.output_text.done") {
        m_transcript->appendPlainText("");
        setFoxState("LISTENING");
        m_playing = false;
    } else if (type == "input_audio_buffer.speech_started") {
        // Barge-in: cancel current generation and clear queued playback when the user speaks.
        sendJson(QJsonObject{{"type", "response.cancel"}});
        sendJson(QJsonObject{{"type", "output_audio_buffer.clear"}});
        if (m_audioSink) m_audioSink->reset();
        setFoxState("LISTENING — INTERRUPTED");
    } else if (type == "conversation.item.input_audio_transcription.completed") {
        appendTranscript("YOU (VOICE)", event.value("transcript").toString());
    } else if (type == "error") {
        const QString error = event.value("error").toObject().value("message").toString();
        appendTranscript("API ERROR", error);
        setStatus("API error: " + error);
    } else if (type == "session.updated") {
        appendTranscript("SYSTEM", "Realtime session configuration accepted.");
    }
}

void CallVarsWindow::startCamera() {
    const auto cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        setStatus("No camera detected");
        return;
    }
    m_camera = new QCamera(cameras.first(), this);
    m_captureSession = new QMediaCaptureSession(this);
    m_videoSink = new QVideoSink(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSink);
    connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &CallVarsWindow::onVideoFrame);
    m_camera->start();
    m_cameraTimer->start();
    m_cameraButton->setText("Stop camera");
    setStatus("Camera active — frames are sent to FOX while connected");
}

void CallVarsWindow::stopCamera() {
    if (m_camera) { m_camera->stop(); m_camera->deleteLater(); m_camera = nullptr; }
    if (m_captureSession) { m_captureSession->deleteLater(); m_captureSession = nullptr; }
    if (m_videoSink) { m_videoSink->deleteLater(); m_videoSink = nullptr; }
    if (m_cameraTimer) m_cameraTimer->stop();
    m_cameraButton->setText("Start camera");
    m_cameraPreview->setText("Camera stopped");
}

void CallVarsWindow::onVideoFrame(const QVideoFrame& frame) {
    QVideoFrame copy(frame);
    if (!copy.map(QVideoFrame::ReadOnly)) return;
    m_latestFrame = copy.toImage().convertToFormat(QImage::Format_RGB888);
    copy.unmap();
    if (!m_latestFrame.isNull()) {
        m_cameraPreview->setPixmap(QPixmap::fromImage(m_latestFrame).scaled(
            m_cameraPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QByteArray CallVarsWindow::imageAsDataUrl(const QImage& image) const {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.scaled(768, 768, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(&buffer, "JPEG", 75);
    return "data:image/jpeg;base64," + bytes.toBase64();
}

void CallVarsWindow::captureFrame() {
    if (!m_connected || m_latestFrame.isNull()) return;
    const QString dataUrl = QString::fromLatin1(imageAsDataUrl(m_latestFrame));
    QJsonObject imageContent{{"type", "input_image"}, {"image_url", dataUrl}, {"detail", "low"}};
    QJsonObject promptContent{{"type", "input_text"}, {"text", "Camera frame update. Describe only what is visibly supported; do not infer identity or hidden details."}};
    QJsonObject item{{"type", "message"}, {"role", "user"}, {"content", QJsonArray{promptContent, imageContent}}};
    sendJson(QJsonObject{{"type", "conversation.item.create"}, {"item", item}});
    m_storage.appendLog("VISION", "Camera frame transmitted to live session");
}

void CallVarsWindow::setStatus(const QString& status) { m_status->setText(status); }

void CallVarsWindow::setFoxState(const QString& state) {
    QString face = "◢◣\n FOX \n◥◤";
    if (state.startsWith("SPEAKING")) face = "◢◣\n F◡X \n◥◤";
    else if (state.startsWith("LISTENING")) face = "◢◣\n F◉X \n◥◤";
    m_foxFace->setText(face + "\n" + state);
}

void CallVarsWindow::appendTranscript(const QString& who, const QString& text) {
    m_transcript->appendPlainText(QString("[%1] %2: %3")
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), who, text));
}
