#ifndef PASSWORD_HASHER_H
#define PASSWORD_HASHER_H

// PasswordHasher: PBKDF2-HMAC-SHA256 với salt ngẫu nhiên.
// Format lưu DB: "pbkdf2$<iterations>$<salt_base64>$<hash_base64>"
// Tương thích ngược: hash cũ dạng SHA-256 hex (64 ký tự, không có "$") vẫn verify được.

#include <QByteArray>
#include <QString>
#include <QRandomGenerator>
#include <QPasswordDigestor>
#include <QCryptographicHash>

class PasswordHasher {
public:
    static constexpr int ITERATIONS  = 100000;
    static constexpr int SALT_BYTES  = 16;
    static constexpr int KEY_BYTES   = 32; // SHA-256 output

    // Tạo hash mới với salt ngẫu nhiên. Trả về chuỗi lưu DB.
    static QString hash(const QString& password) {
        QByteArray salt(SALT_BYTES, Qt::Uninitialized);
        QRandomGenerator::securelySeeded().fillRange(
            reinterpret_cast<quint32*>(salt.data()),
            static_cast<qsizetype>(SALT_BYTES / sizeof(quint32))
        );

        QByteArray key = QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            password.toUtf8(),
            salt,
            ITERATIONS,
            KEY_BYTES
        );

        return QStringLiteral("pbkdf2$%1$%2$%3")
            .arg(ITERATIONS)
            .arg(QString::fromLatin1(salt.toBase64()))
            .arg(QString::fromLatin1(key.toBase64()));
    }

    // Constant-time verify — bảo vệ khỏi timing attack.
    // Hỗ trợ cả hash mới (pbkdf2$...) và hash cũ (SHA-256 hex).
    // Trả về: 0 = sai, 1 = đúng (legacy SHA-256), 2 = đúng (PBKDF2 mới)
    enum class VerifyResult { Wrong = 0, OkLegacy = 1, OkPbkdf2 = 2 };

    static VerifyResult verify(const QString& password, const QString& storedHash) {
        if (storedHash.startsWith(QLatin1String("pbkdf2$"))) {
            return verifyPbkdf2(password, storedHash) ? VerifyResult::OkPbkdf2
                                                       : VerifyResult::Wrong;
        }
        // Legacy SHA-256 hex
        QByteArray expected = QByteArray::fromHex(storedHash.toLatin1());
        QByteArray actual   = QCryptographicHash::hash(password.toUtf8(),
                                                       QCryptographicHash::Sha256);
        return constantTimeEqual(expected, actual) ? VerifyResult::OkLegacy
                                                    : VerifyResult::Wrong;
    }

private:
    static bool verifyPbkdf2(const QString& password, const QString& stored) {
        // Format: pbkdf2$<iter>$<salt_b64>$<hash_b64>
        QStringList parts = stored.split(QLatin1Char('$'));
        if (parts.size() != 4) return false;

        bool ok = false;
        int iterations = parts[1].toInt(&ok);
        if (!ok || iterations <= 0) return false;

        QByteArray salt     = QByteArray::fromBase64(parts[2].toLatin1());
        QByteArray expected = QByteArray::fromBase64(parts[3].toLatin1());

        QByteArray actual = QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            password.toUtf8(),
            salt,
            iterations,
            expected.size()
        );

        return constantTimeEqual(expected, actual);
    }

    // Constant-time comparison — tránh early-exit timing leak
    static bool constantTimeEqual(const QByteArray& a, const QByteArray& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (int i = 0; i < a.size(); ++i) {
            diff |= static_cast<unsigned char>(a[i]) ^
                    static_cast<unsigned char>(b[i]);
        }
        return diff == 0;
    }
};

#endif // PASSWORD_HASHER_H
