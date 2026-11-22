#include "FoxCore.h"
#include <QRegularExpression>
#include <QStringList>
#include <QFileInfo>
#include <cmath>

QString FoxMathEngine::calculate(const QString& expression) const {
    // Deliberately small, safe calculator: two numeric operands and + - * /.
    static const QRegularExpression re(
        R"(^\s*(-?(?:\d+(?:\.\d*)?|\.\d+))\s*([+*/-])\s*(-?(?:\d+(?:\.\d*)?|\.\d+))\s*$)"
    );
    const auto match = re.match(expression);
    if (!match.hasMatch())
        return QStringLiteral("Use a simple expression such as 12 * 4 or 8 / 2.");

    bool okA = false, okB = false;
    const double a = match.captured(1).toDouble(&okA);
    const double b = match.captured(3).toDouble(&okB);
    if (!okA || !okB) return QStringLiteral("Invalid number.");

    double result = 0.0;
    const QString op = match.captured(2);
    if (op == "+") result = a + b;
    else if (op == "-") result = a - b;
    else if (op == "*") result = a * b;
    else {
        if (b == 0.0) return QStringLiteral("Division by zero is undefined.");
        result = a / b;
    }
    return QString::number(result, 'g', 15);
}

FoxStorage::FoxStorage() {
    m_directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (m_directory.isEmpty()) m_directory = QDir::homePath() + "/.callvars";
    QDir().mkpath(m_directory);
    m_logPath = m_directory + "/callvars.log";
}

bool FoxStorage::appendLog(const QString& category, const QString& message) {
    QMutexLocker lock(&m_mutex);
    QFile file(m_logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return false;
    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString(Qt::ISODate)
        << " [" << category << "] " << message << "\n";
    return true;
}

QString FoxStorage::readRecent(int maxLines) const {
    QMutexLocker lock(&m_mutex);
    QFile file(m_logPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) lines.push_back(in.readLine());
    const int start = qMax(0, lines.size() - qMax(1, maxLines));
    return lines.mid(start).join('\n');
}

bool FoxMail::saveDraft(const QString& recipient, const QString& subject, const QString& body) {
    const QString payload = QStringLiteral("TO: %1\nSUBJECT: %2\n\n%3\n")
        .arg(recipient, subject, body);
    return m_storage.appendLog(QStringLiteral("MAIL_DRAFT"), payload);
}
