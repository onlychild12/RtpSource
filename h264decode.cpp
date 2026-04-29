#include "h264decode.h"
#include <QDebug>
H264Decode::H264Decode(QObject *parent)
    : QObject{parent}
{}

void H264Decode::esureSwsContext(int nWidth, int nHeight, AVPixelFormat sourceFormat)
{
    if(nWidth==m_nWidth&&nHeight==m_nHeight)
    {
        return;
    }
}

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
    m_pAvFrame=av_frame_alloc();
    if(!m_pAvFrame)
    {
        qDebug()<<"av frame init failure";
    }
    m_pAvPacket=av_packet_alloc();
    if(!m_pAvPacket)
    {
        qDebug()<<"av packet init failure";
    }

}

void H264Decode::decodeFrame(const QByteArray &data)
{
    if(data.isEmpty())
        return;
    av_packet_unref(m_pAvPacket);
    m_pAvPacket->data=(uint8_t*)data.constData();
    m_pAvPacket->size=data.size();
    int ret= avcodec_send_packet(m_avContext,m_pAvPacket);
    if(ret!=0)
    {
        qDebug()<<"send packet failure";
        return;
    }
    while(1)
    {
        int nRet=  avcodec_receive_frame(m_avContext,m_pAvFrame);
        if(nRet!=0)
        {
            qDebug()<<"receive Frame not all";
            return;
        }


    }
}
