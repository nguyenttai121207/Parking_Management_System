// tst_PasswordHasher.cpp
#include <QtTest>
#include "PasswordHasher.h"
#include <QCryptographicHash>

class tst_PasswordHasher : public QObject {
    Q_OBJECT

private slots:
    // ── Hash đúng mật khẩu ────────────────────────────────────

    void correct_password_verifies() {
        QString h = PasswordHasher::hash("admin123");
        QCOMPARE(PasswordHasher::verify("admin123", h),
                 PasswordHasher::VerifyResult::OkPbkdf2);
    }

    // ── Hash sai mật khẩu ─────────────────────────────────────

    void wrong_password_rejected() {
        QString h = PasswordHasher::hash("admin123");
        QCOMPARE(PasswordHasher::verify("wrong_password", h),
                 PasswordHasher::VerifyResult::Wrong);
    }

    void empty_password_rejected() {
        QString h = PasswordHasher::hash("admin123");
        QCOMPARE(PasswordHasher::verify("", h),
                 PasswordHasher::VerifyResult::Wrong);
    }

    // ── 2 lần hash cùng mật khẩu → kết quả khác nhau (salt) ─

    void two_hashes_differ_due_to_salt() {
        QString h1 = PasswordHasher::hash("samePassword");
        QString h2 = PasswordHasher::hash("samePassword");
        QVERIFY(h1 != h2);
        // Nhưng cả 2 đều verify được
        QCOMPARE(PasswordHasher::verify("samePassword", h1),
                 PasswordHasher::VerifyResult::OkPbkdf2);
        QCOMPARE(PasswordHasher::verify("samePassword", h2),
                 PasswordHasher::VerifyResult::OkPbkdf2);
    }

    // ── Format pbkdf2$ hợp lệ ─────────────────────────────────

    void hash_has_correct_prefix() {
        QVERIFY(PasswordHasher::hash("test").startsWith("pbkdf2$"));
    }

    void hash_has_four_parts() {
        QString h = PasswordHasher::hash("test");
        QCOMPARE(h.split('$').size(), 4);
    }

    void iterations_in_hash_is_correct() {
        QString h = PasswordHasher::hash("test");
        int iter = h.split('$')[1].toInt();
        QCOMPARE(iter, PasswordHasher::ITERATIONS);
    }

    // ── Tương thích ngược: SHA-256 cũ vẫn verify được ────────

    void legacy_sha256_verifies() {
        // Giả lập hash cũ (không có tiền tố pbkdf2$)
        QString legacyHash = QString::fromLatin1(
            QCryptographicHash::hash("admin123", QCryptographicHash::Sha256).toHex()
        );
        QCOMPARE(PasswordHasher::verify("admin123", legacyHash),
                 PasswordHasher::VerifyResult::OkLegacy);
    }

    void legacy_sha256_wrong_password_rejected() {
        QString legacyHash = QString::fromLatin1(
            QCryptographicHash::hash("admin123", QCryptographicHash::Sha256).toHex()
        );
        QCOMPARE(PasswordHasher::verify("wrongPass", legacyHash),
                 PasswordHasher::VerifyResult::Wrong);
    }

    // ── Hash malformed ────────────────────────────────────────

    void malformed_pbkdf2_rejected() {
        QCOMPARE(PasswordHasher::verify("any", "pbkdf2$abc"),
                 PasswordHasher::VerifyResult::Wrong);
    }

    void empty_stored_hash_rejected() {
        QCOMPARE(PasswordHasher::verify("any", ""),
                 PasswordHasher::VerifyResult::Wrong);
    }
};

QTEST_MAIN(tst_PasswordHasher)
#include "tst_PasswordHasher.moc"
