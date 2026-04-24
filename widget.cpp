#include "widget.h"
#include "ui_widget.h"
#include "rtpsource.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    RtpSource *source=new RtpSource;
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}
