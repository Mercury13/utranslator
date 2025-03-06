#include "FmFind.h"
#include "ui_FmFind.h"

// Qt
#include <QMessageBox>

// Qt ex
#include "QtConsts.h"
#include "u_Qstrings.h"
#include "BalloonTip.h"


const FindOptions::Channels FindOptions::Channels::NONE;


///// FindOptions //////////////////////////////////////////////////////////////


inline bool FindOptions::matchEntity(const tr::Entity& x) const
{
    return matchChan(channels.id,                 x.id)
        || matchChan(channels.importersComment,   x.comm.importersIfVisible())
        || matchChan(channels.authorsComment,     x.comm.authors)
        || matchChan(channels.translatorsComment, x.comm.translators);
}


inline bool FindOptions::matchText1(const tr::Text& x) const
{
    return matchChan(channels.original,    x.tr.original)
        || matchChan(channels.translation, x.tr.translationSv());
}


bool FindOptions::matchText(const tr::Text& x) const
{
    return matchEntity(x) || matchText1(x);
}


bool FindOptions::matchGroup(const tr::VirtualGroup& x) const
{
    return matchEntity(x);
}


bool FindOptions::matchChan(bool isEnabled, std::u8string_view channel) const
{
    static constexpr auto FROM_START = 0;
    return isEnabled
            && (str::toQ(channel).indexOf(text, FROM_START, qCaseSen()) >= 0);
}


std::u8string FindOptions::caption() const
{
    std::u8string r = u8"Find “{1}”";
    str::replace(r, u8"{1}", str::toU8sv(text.toStdString()));
    return r;
}

FindIssue FindOptions::firstIssue() const
{
    if (text.isEmpty())
        return FindIssue::TEXT;
    if (channels == Channels::NONE)
        return FindIssue::CHANNELS;
    return FindIssue::OK;
}


///// FmFind ///////////////////////////////////////////////////////////////////

FmFind::FmFind(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmFind)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::acceptIf);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
    ui->grpPlace->setEnabled(false);
}

void FmFind::acceptIf()
{
    static constexpr int TIMEOUT = 15'000;
    copyTo(opts);
    switch (opts.firstIssue()) {
    case FindIssue::OK:
        accept();
        break;
    case FindIssue::TEXT:
        ui->edFind->setFocus();
        BalloonTip::showBalloon(QMessageBox::Critical,
                "Find", "Please enter some text.",
                ui->edFind, TIMEOUT, BalloonDir::BLN_5_OC);
        break;
    case FindIssue::CHANNELS:
        BalloonTip::showBalloon(QMessageBox::Critical,
                "Find", "Please check at least one channel.",
                ui->grpChannels, TIMEOUT, BalloonDir::BLN_3_OC);
        break;
    }
}


FmFind::~FmFind()
{
    delete ui;
}

void FmFind::copyTo(FindOptions& r)
{
    r.text = ui->edFind->text();
    r.channels.id = ui->chkChanId->isChecked();
    r.channels.original = ui->chkChanOriginal->isChecked();
    r.channels.importersComment = ui->chkChanImportersComment->isChecked();
    r.channels.authorsComment = ui->chkChanAuthorsComment->isChecked();
    if (isTransl) {
        r.channels.translation = ui->chkChanTranslation->isChecked();
        r.channels.translatorsComment = ui->chkChanTranslatorsComment->isChecked();
    } else {
        r.channels.translation = false;
        r.channels.translatorsComment = false;
    }
    r.options.matchCase = ui->chkMatchCase->isChecked();
}

const FindOptions* FmFind::exec(tr::PrjType prjType)
{
    switch (prjType) {
    case tr::PrjType::ORIGINAL:
        isTransl = false;
        break;
    case tr::PrjType::FULL_TRANSL:
        isTransl = true;
        break;
    }
    ui->chkChanTranslation->setEnabled(isTransl);
    ui->chkChanTranslatorsComment->setEnabled(isTransl);
    ui->edFind->setFocus();

    if (Super::exec() && opts) {
        return &opts;
    } else {
        return nullptr;
    }
}
