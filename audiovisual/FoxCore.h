#pragma once

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QMutex>
#include <QMutexLocker>
#include <optional>

// Preserved application concepts: FoxCharacter, FoxMathEngine, FoxMail, FoxStorage.
// This file is the local application layer; live multimodal inference is in CallVarsWindow.

class FoxCharacter {
public:
    QString name() const { return QStringLiteral("FOX"); }
    QString greeting() const {
        return QStringLiteral("Greetings. I'm FOX, your Call-Vars AI companion. How may I help?");
    }
    QString systemIdentity() const {
        return QStringLiteral(
            "You are FOX, the black fox AI character inside Call-Vars. Be curious, precise, "
            "helpful, candid about uncertainty, and respectful. Preserve the user's intent. "
            "Do not claim to see, hear, remember, calculate, or execute anything unless the "
            "system actually supplied that information or completed that action."
        );
    }
};

class FoxMathEngine {
public:
    static double StandardMultiply(double a, double b) { return a * b; }
    static double FoxMultiply(double a, double b) { return 2.0 * a * b; }

    QString calculate(const QString& expression) const;
};

class FoxStorage {
public:
    FoxStorage();
    QString dataDirectory() const { return m_directory; }
    bool appendLog(const QString& category, const QString& message);
    QString readRecent(int maxLines = 100) const;
private:
    QString m_directory;
    QString m_logPath;
    mutable QMutex m_mutex;
};

class FoxMail {
public:
    explicit FoxMail(FoxStorage& storage) : m_storage(storage) {}
    bool saveDraft(const QString& recipient, const QString& subject, const QString& body);
private:
    FoxStorage& m_storage;
};
