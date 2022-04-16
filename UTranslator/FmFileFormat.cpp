#include "FmFileFormat.h"
#include "ui_FmFileFormat.h"

// Libs
#include "u_Qstrings.h"

// Project
#include "TrFile.h"


FmFileFormat::FmFileFormat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FmFileFormat)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &This::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &This::reject);
    connect(ui->comboFormat, &QComboBox::currentIndexChanged, this, &This::comboChanged);
    connect(ui->comboLineBreaksInStrings, &QComboBox::currentIndexChanged, this, &This::reenable);
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


void FmFileFormat::reenable()
{
    ui->edLineBreakChar->setEnabled(
                escapeMode() == tf::LineBreakEscapeMode::SPECIFIED_TEXT);
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
                static_cast<int>(sets.textEscape.mode));
    ui->edLineBreakChar->setText(
                sets.textEscape.mode == tf::LineBreakEscapeMode::SPECIFIED_TEXT
                    ? str::toQ(sets.textEscape.specifiedText)
                    : QString{'^'});

    // Unified: multitier
    ui->edMultitierChar->setText(str::toQ(sets.multitierSeparator));
}

void FmFileFormat::copyTo(std::unique_ptr<tf::FileFormat>& r)
{
    tf::UnifiedSets sets;
    // Unified: text format
    sets.textFormat.writeBom = ui->chkBom->isChecked();
    sets.textFormat.lineBreakStyle = static_cast<tf::LineBreakStyle>(
                ui->comboLineBreaksInFile->currentIndex());


    // Unified: text escape
    sets.textEscape.mode = escapeMode();
    if (sets.textEscape.mode == tf::LineBreakEscapeMode::SPECIFIED_TEXT)
        sets.textEscape.specifiedText = str::toU8(ui->edLineBreakChar->text());

    // Unified: multitier
    sets.multitierSeparator = str::toU8(ui->edMultitierChar->text());

    auto currProto = filteredProtos[ui->comboFormat->currentIndex()];
    if (currProto->isDummy()) {
        r.reset();
    } else {
        if (!r || &r->proto() != currProto) {
            r = currProto->make();
        }
        r->setUnifiedSets(sets);
    }
}

bool FmFileFormat::exec(
        std::unique_ptr<tf::FileFormat>& x,
        const tf::ProtoFilter& filter)
{
    auto& obj = x ? *x : tf::Dummy::INST;
    collectFormats(&obj.proto(), filter);
    copyFrom(obj);
    reenable();
    ui->comboFormat->setFocus();
    bool r = Super::exec();
    if (r) {
        copyTo(x);
    }
    return r;
}
