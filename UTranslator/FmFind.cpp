#include "FmFind.h"
#include "ui_FmFind.h"

// Qt
#include <QMessageBox>

// Qt ex
#include "QtConsts.h"
#include "u_Qstrings.h"


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
    copyTo(opts);
    if (opts.areSet()) {
        accept();
    } else {
        QMessageBox::warning(this,
                "Find",
                "Please enter non-empty text, and check at least one channel");
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

FindOptions FmFind::exec(tr::PrjType prjType)
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

    if (Super::exec()) {
        return opts;
    } else {
        return {};
    }
}
