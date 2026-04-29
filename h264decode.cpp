#include "h264decode.h"
#include <QDebug>
H264Decode::H264Decode(QObject *parent)
    : QObject{parent}
{}

void H264Decode::init()
{
    m_avCodec=avcodec_find_decoder(AV_CODEC_ID_H264);
    if(!m_avCodec)
    {
        qDebug()<<"init codec failure";
        return;
    }
    m_avContext=avcodec_alloc_context3(m_avCodec);
    if(!m_avContext)
    {
        qDebug()<<"init m_avContext failure";
        return;
    }
    m_avContext->flags|=AV_CODEC_FLAG_LOW_DELAY;
    m_avContext->flags2|=AV_CODEC_FLAG2_FAST;
    m_avContext->thread_count=2;
    int ret=avcodec_open2(m_avContext,m_avCodec,nullptr);
    if(ret)
    {
        avcodec_free_context(&m_avContext);
        qDebug()<<"open failure";
    }

}
