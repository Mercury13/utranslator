#pragma once

#include <QDialog>

#include "TrProject.h"

namespace Ui {
class FmFind;
}

enum class FindIssue : unsigned char {
    OK, TEXT, CHANNELS
};

struct FindOptions : public tr::FindCriterion
{
public:
    QString text;
    struct Channels {
        bool id = false, original = false, importersComment = false,
             authorsComment = false, translation = false, translatorsComment = false;
        static const Channels NONE;
        // default C++20 op==
        bool operator == (const Channels&) const = default;
    } channels;
    struct Options {
        bool matchCase = false;
    } options;

    explicit operator bool () const { return !text.isEmpty(); }
    FindIssue firstIssue() const;
    bool areSet() const { return (firstIssue() != FindIssue::OK); }

    Qt::CaseSensitivity qCaseSen() const
    {
        // Needed for this to work
        static_assert(static_cast<int>(Qt::CaseInsensitive) == 0);
        static_assert(static_cast<int>(Qt::CaseSensitive) == 1);
        // Go!
        return static_cast<Qt::CaseSensitivity>(options.matchCase);
    }

    // FindCriterion
    bool matchText(const tr::Text&) const override;
    bool matchGroup(const tr::VirtualGroup&) const override;
    std::u8string caption() const override;
private:
    inline bool matchEntity(const tr::Entity& x) const;
    bool matchChan(bool isEnabled, std::u8string_view channel) const;
    inline bool matchText1(const tr::Text&) const;
};

class FmFind : public QDialog
{
    Q_OBJECT
    using Super = QDialog;
    using This = FmFind;
public:
    explicit FmFind(QWidget *parent = nullptr);
    ~FmFind() override;

    const FindOptions* exec(tr::PrjType prjType);
private slots:
    void acceptIf();
private:
    Ui::FmFind *ui;
    FindOptions opts;
    bool isTransl = false;

    using Super::exec;
    void copyTo(FindOptions& r);
};
