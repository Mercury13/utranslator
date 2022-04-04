#ifndef FMDECODER_H
#define FMDECODER_H

#include <QDialog>

namespace Ui {
class FmDecoder;
}

class QPlainTextEdit;

class StrObject     // interface
{
public:
    virtual std::u32string toU32() = 0;
    virtual void set(std::u32string_view x) = 0;

    template <class T> T get();
};

template <> inline std::u32string StrObject::get<std::u32string>()
    { return toU32(); }

class QstrObject : public StrObject
{
public:
    virtual QString toQ() = 0;
    virtual void set(const QString& x) = 0;

    // Different strings — they are final now
    std::u32string toU32() final
        { return toQ().toStdU32String(); }
    void set(std::u32string_view x) final
        { set(QString::fromUcs4(x.data(), x.size())); }
};

class MemoObject : public QstrObject
{
public:
    QPlainTextEdit* const memo;

    MemoObject(QPlainTextEdit* x) : memo(x) {}

    QString toQ() override;
    void set(const QString& x) override;
};

class FmDecoder : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmDecoder;
public:
    explicit FmDecoder(QWidget *parent = nullptr);
    ~FmDecoder();
    int exec() override;
private slots:
    void decodeCpp();
private:
    Ui::FmDecoder *ui;
    template <class Intermed, class Body> void decodeMemo(const Body& body);
};

#endif // FMDECODER_H
