#include "FmFileFormat.h"
#include "ui_FmFileFormat.h"

// Libs
#include "u_Qstrings.h"
#include "QtConsts.h"

// Project
#include "TrFile.h"

// UI forms
#include "FmAboutFormat.h"


namespace {

    template <class Elem, size_t N>
    void fillComboWithLocName(QComboBox* combo, const Elem (&arr)[N]) {
        for (auto& v : arr) {
            combo->addItem(str::toQ(v.locName));
        }
    }

}   // anon namespace


FmFileFormat::FmFileFormat(QWidget *parent) :
    QDialog(parent, QDlgType::FIXED),
    ui(new Ui::FmFileFormat)
{
    ui->setupUi(this);

    radioExisting.setRadio(tf::Existing::KEEP, ui->radioExistingKeep);
    radioExisting.setRadio(tf::Existing::OVERWRITE, ui->radioExistingOverwrite);

    radioLoadTo.setRadio(tf::LoadTo::ROOT, ui->radioPlaceRoot);
    radioLoadTo.setRadio(tf::LoadTo::SELECTED, ui->radioPlaceSelected);

    fillComboWithLocName(ui->comboLineBreaksInFile, tf::textLineBreakStyleInfo);
    fillComboWithLocName(ui->comboSpaceEscape, tf::spaceEscapeModeInfo);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
    connect(ui->comboFormat, &QComboBox::currentIndexChanged, this, &This::comboChanged);
    connect(ui->comboLineBreaksInStrings, &QComboBox::currentIndexChanged, this, &This::reenable);
    connect(ui->btAbout, &QPushButton::clicked, this, &This::aboutFormat);
}

FmFileFormat::~FmFileFormat()
{
    delete ui;
}


void FmFileFormat::reenableToFormat(const tf::FormatProto& proto)
{
    // Big pieces
    auto flags = proto.workingSets();
    ui->grpText->setVisible(flags.have(tf::Usfg::TEXT_FORMAT));
    ui->wiLineBreaksInStrings->setVisible(flags.have(tf::Usfg::TEXT_ESCAPE));
    ui->grpMultitier->setVisible(flags.have(tf::Usfg::MULTITIER));

    // Legend
        // Marking ¹ (write-only) on two settings of text format
    constexpr Flags USFGS_LEGEND = tf::Usfg::TEXT_FORMAT;
    ui->lbLegend->setVisible(flags.haveAny(USFGS_LEGEND));
}


void FmFileFormat::comboChanged(int index)
{
    if (static_cast<size_t>(index) < filteredProtos.size()) {
        reenableToFormat(*filteredProtos[index]);
    }
}

void FmFileFormat::collectFormats(
        const tf::FormatProto* myProto,
        const tf::ProtoFilter& filter)
{
    filteredProtos.clear();
    size_t iSelected = 0;
    ui->comboFormat->clear();    
    for (auto v : tf::allProtos) {
        if (v->isWithin(filter)) {
            if (v == myProto)
                iSelected = filteredProtos.size();
            filteredProtos.add(v);
            ui->comboFormat->addItem(str::toQ(v->locName()));
        }
    }
    ui->comboFormat->setCurrentIndex(iSelected);
    comboChanged(iSelected);
}


tf::LineBreakEscapeMode FmFileFormat::escapeMode() const
{
    return static_cast<tf::LineBreakEscapeMode>(
                    ui->comboLineBreaksInStrings->currentIndex());
}


bool FmFileFormat::isC() const
{
    return tf::TextEscape::isC(escapeMode());
}


void FmFileFormat::reenable()
{
    ui->edLineBreakChar->setEnabled(
                escapeMode() == tf::LineBreakEscapeMode::SPECIFIED_TEXT);
    ui->wiCSubformat->setEnabled(isC());
}

void FmFileFormat::copyFrom(const tf::FileFormat& fmt)
{
    auto sets = fmt.unifiedSets();
    reenableToFormat(fmt.proto());

    // Unified: text format
    ui->chkBom->setChecked(sets.textFormat.writeBom);
    ui->comboLineBreaksInFile->setCurrentIndex(
                static_cast<int>(sets.textFormat.lineBreakStyle));

    // Unified: text escape
    ui->comboLineBreaksInStrings->setCurrentIndex(
                static_cast<int>(sets.textEscape.lineBreak));
    ui->edLineBreakChar->setText(
                sets.textEscape.lineBreak == tf::LineBreakEscapeMode::SPECIFIED_TEXT
                    ? str::toQ(sets.textEscape.lineBreakText)
                    : QString{'^'});
    ui->comboSpaceEscape->setCurrentIndex(
                static_cast<int>(sets.textEscape.spaceEscape));

    // Unified: multitier
    ui->edMultitierChar->setText(str::toQ(sets.multitier.separator));
}

const tf::FormatProto* FmFileFormat::currentProto() const
    { return filteredProtos[ui->comboFormat->currentIndex()]; }


void FmFileFormat::copyTo(std::unique_ptr<tf::FileFormat>& r)
{
    tf::UnifiedSets sets;
    // Unified: text format
    sets.textFormat.writeBom = ui->chkBom->isChecked();
    sets.textFormat.lineBreakStyle = static_cast<tf::TextLineBreakStyle>(
                ui->comboLineBreaksInFile->currentIndex());

    // Unified: text escape
    sets.textEscape.lineBreak = escapeMode();
    if (sets.textEscape.lineBreak == tf::LineBreakEscapeMode::SPECIFIED_TEXT) {
        sets.textEscape.lineBreakText = str::toU8(ui->edLineBreakChar->text());
        if (sets.textEscape.lineBreakText.empty())
            sets.textEscape.lineBreak = tf::LineBreakEscapeMode::BANNED;
    }
    sets.textEscape.spaceEscape = static_cast<tf::SpaceEscapeMode>(ui->comboSpaceEscape->currentIndex());

    // Unified: multitier
    sets.multitier.separator = str::toU8(ui->edMultitierChar->text());

    auto currProto = currentProto();
    if (currProto->isDummy()) {
        r.reset();
    } else {
        if (!r || &r->proto() != currProto) {
            r = currProto->make();
        }
        r->setUnifiedSets(sets);
    }

    // Specific (not unified) — right now do not exist
}

void FmFileFormat::copyFrom(const tf::LoadTextsSettings* x)
{
    ui->grpLoadTexts->setVisible(x);
    if (x) {
        radioLoadTo.set(x->loadTo);
        radioExisting.set(x->existing);
    }
}

void FmFileFormat::copyTo(tf::LoadTextsSettings* r)
{
    if (r) {
        r->loadTo = radioLoadTo.get();
        r->existing = radioExisting.get();
    }
}


bool FmFileFormat::exec(
        std::unique_ptr<tf::FileFormat>& x,
        tf::LoadTextsSettings* loadSets,
        const tf::ProtoFilter& filter)
{
    auto& obj = x ? *x : tf::Dummy::INST;
    collectFormats(&obj.proto(), filter);
    copyFrom(obj);
    copyFrom(loadSets);
    reenable();
    ui->btAbout->setFocus();     // sensitive form → focus About button
    bool r = Super::exec();
    if (r) {
        copyTo(x);
        copyTo(loadSets);
    }
    return r;
}

void FmFileFormat::aboutFormat()
{
    fmAboutFormat.ensure(this).exec(currentProto());
}
