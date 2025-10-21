#include "cabrilloheader.h"
#include "ui_cabrilloheader.h"
#include "QLogBook.h"

cabrilloheader::cabrilloheader(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::cabrilloheader)   {
    ui->setupUi(this);
    uip = ui;
    prepare();
}

cabrilloheader::~cabrilloheader() {
    delete ui;
}

void cabrilloheader::prepare() {
QStringList qlcontest = { "ARRL-10", "ARRL-10-GHZ", "ARRL-160", "ARRL-DIGI", "ARRL-DX-CW",
                         "ARRL-DX-SSB", "ARRL-EME", "ARRL-SS-CW", "ARRL-SS-SSB",
                         "BARTG-RTTY", "CQ-160-CW", "CQ-160-SSB", "CQ-WPX-CW", "CQ-WPX-RTTY",
                         "CQ-WPX-SSB", "CQ-VHF", "CQ-WW-CW", "CQ-WW-RTTY", "CQ-WW-SSB", "IARU-HF",
                             "NAQP-CW", "NAQP-SSB", "NAQP-RTTY", "RDXC", "RSGB-IOTA", "SPDXC",
                             "SPDXC-RTTY", "TARA-RTTY", "WAG", "WW-DIGI" };
    ui->contest->addItems(qlcontest);
}


void cabrilloheader::on_buttonBox_accepted() {
    this->hide();
    ready = true;
}


void cabrilloheader::on_buttonBox_rejected() {
    ready = true;
}

