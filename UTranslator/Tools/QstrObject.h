#pragma once

// STL
#include <string>

// Qt core
#include <QString>

class QPlainTextEdit;

class StrObject     // interface
{
public:
    virtual std::u32string toU32() = 0;
    virtual std::wstring toW() = 0;
    virtual void set(std::u32string_view x) = 0;
    virtual void set(std::wstring_view x) = 0;

    template <class T> T get();
};

template <> inline std::u32string StrObject::get<std::u32string>()
    { return toU32(); }
template <> inline std::wstring StrObject::get<std::wstring>()
    { return toW(); }

class QstrObject : public StrObject
{
public:
    virtual QString toQ() = 0;
    virtual void set(const QString& x) = 0;

    // Different strings — they are final now
    std::u32string toU32() final
        { return toQ().toStdU32String(); }
    std::wstring toW() final
        { return toQ().toStdWString(); }

    void set(std::u32string_view x) final
        { set(QString::fromUcs4(x.data(), x.size())); }
    void set(std::wstring_view x) final
        { set(QString::fromWCharArray(x.data(), x.size())); }
};

class MemoObject : public QstrObject
{
public:
    QPlainTextEdit* const memo;

    MemoObject(QPlainTextEdit* x) : memo(x) {}

    QString toQ() override;
    void set(const QString& x) override;
};
