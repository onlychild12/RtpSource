#include "widget.h"
#include "ui_widget.h"
#include "rtpsource.h"
#include "h264naluparse.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    RtpSource *source=new RtpSource;
    H264NaluParse *parse=new H264NaluParse;
    connect(source,&RtpSource::rtpParseData,parse,&H264NaluParse::parsePacket);

    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}
