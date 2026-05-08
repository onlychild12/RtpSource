#include "widget.h"
#include "ui_widget.h"
#include "rtpsource.h"
#include "h264naluparse.h"
#include "h264decode.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    RtpSource *source=new RtpSource;
    H264NaluParse *parse=new H264NaluParse;
    H264Decode *decode=new H264Decode;
    connect(source,&RtpSource::rtpParseData,parse,&H264NaluParse::parsePacket);
    connect(source,&RtpSource::lostFrame,parse,&H264NaluParse::slot_lostFrame);
    connect(source,&RtpSource::lostFrame,decode,&H264Decode::slot_flush);

    connect(parse,&H264NaluParse::sig_EmitNewFrame,decode,&H264Decode::decodeFrame);

    connect(decode,&H264Decode::lostFrame,this,[=](){
        ui->label->clear();
    });
    connect(decode,&H264Decode::sig_emitImageFrame,this,[=](QImage img){
        ui->label->setPixmap(QPixmap::fromImage(img).scaled(
            ui->label->size(),
            Qt::KeepAspectRatio,
            Qt::FastTransformation));
    });
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}
