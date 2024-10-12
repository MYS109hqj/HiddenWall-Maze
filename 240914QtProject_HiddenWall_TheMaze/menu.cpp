#include "menu.h"
#include "ui_menu.h"
#include "widget.h"


Menu::Menu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Menu)
{

    ui->setupUi(this);
    QPixmap pix(":/images/OIP-C02.jpg");
    ui->label->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->label->setPixmap(pix.scaled(ui->label->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

}

Menu::~Menu()
{
    delete ui;
}

void Menu::on_pushButton_clicked()
{
    this->close();
    Widget * mainpic = new Widget();
    mainpic->show();
}

