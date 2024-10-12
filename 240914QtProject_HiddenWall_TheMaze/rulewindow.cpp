#include "rulewindow.h"
#include "ui_rulewindow.h"

RuleWindow::RuleWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RuleWindow)
{
    ui->setupUi(this);
    QPixmap pix(":/images/wall.png");
    ui->exampleLB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->exampleLB->setPixmap(pix.scaled(ui->exampleLB->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
}

RuleWindow::~RuleWindow()
{
    delete ui;
}
