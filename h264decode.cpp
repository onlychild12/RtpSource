#include "h264decode.h"
#include <QDebug>
#include <QImage>
H264Decode::H264Decode(QObject *parent)
    : QObject{parent}
{

    init();
}

void H264Decode::esureSwsContext(int nWidth, int nHeight, AVPixelFormat sourceFormat)
{
    if(nWidth==m_nWidth&&nHeight==m_nHeight)
    {
        return;
    }
    if(m_swsContext)
    {
        sws_freeContext(m_swsContext);
    }
    m_nHeight=nHeight;
    m_nWidth=nWidth;
    m_swsContext = sws_getContext(
        nWidth, nHeight, sourceFormat,
        nWidth, nHeight, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
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
    m_avContext->err_recognition = 0;  // 不要过度报错
    m_avContext->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;  // 开启错误隐藏
    m_avContext->flags|=AV_CODEC_FLAG_LOW_DELAY;
    m_avContext->flags2|=AV_CODEC_FLAG2_FAST;
    m_avContext->thread_count=2;
    int ret=avcodec_open2(m_avContext,m_avCodec,nullptr);
    if(ret)
    {
        avcodec_free_context(&m_avContext);
        qDebug()<<"open failure";
        return;
    }
    m_pAvFrame=av_frame_alloc();
    if(!m_pAvFrame)
    {
        qDebug()<<"av frame init failure";
    }



}

void H264Decode::decodeFrame( QByteArray data)
{
    if(data.isEmpty())
        return;
    qDebug() << ">> send to decoder, size =" << data.size()
             << "hex:" << data.left(20).toHex();
    m_pAvPacket = av_packet_alloc();
    av_new_packet(m_pAvPacket, data.size());
    memcpy(m_pAvPacket->data, data.constData(), data.size());
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
            av_packet_free(&m_pAvPacket);
            return;
        }

        // if(m_pAvFrame->decode_error_flags)
        // {
        //     qDebug() << "corrupted frame, dropped!";
        //     av_frame_unref(m_pAvFrame);
        //     continue;
        // }
        qDebug() << ">> decoded:" << m_pAvFrame->width << "x" << m_pAvFrame->height
                 << "format:" << m_pAvFrame->format;
        esureSwsContext(m_pAvFrame->width,m_pAvFrame->height,(AVPixelFormat)m_pAvFrame->format);
        // 构造 dstData/dstLinesize 指向 QImage 内部的缓冲区
        uint8_t *dstData[4] = {nullptr};
        int dstLinesize[4] = {0};
        av_image_alloc(dstData, dstLinesize,
                       m_pAvFrame->width, m_pAvFrame->height,
                       AV_PIX_FMT_RGB32, 32);

        sws_scale(m_swsContext,
                  m_pAvFrame->data, m_pAvFrame->linesize,
                  0, m_pAvFrame->height,
                  dstData, dstLinesize);

        QImage img(dstData[0], m_pAvFrame->width, m_pAvFrame->height,
                   dstLinesize[0], QImage::Format_RGB32);
        sig_emitImageFrame(img.copy());

        av_freep(&dstData[0]);   // ★ 用完释放
        av_frame_unref(m_pAvFrame);

    }

}

void H264Decode::slot_flush()
{
    // if(m_avContext)
        // avcodec_flush_buffers(m_avContext);
        qDebug()<<"flush decode context";//丢包刷新上下文
        lostFrame();
}
