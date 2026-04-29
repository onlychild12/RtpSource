#ifndef H264DECODE_H
#define H264DECODE_H

#include <QObject>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}
class H264Decode : public QObject
{
    Q_OBJECT
public:
    explicit H264Decode(QObject *parent = nullptr);
    void esureSwsContext(int nWidth,int nHeight,AVPixelFormat sourceFormat);
    void init();
public slots:
    void decodeFrame(const QByteArray&data);
signals:

private:
    AVCodecContext *m_avContext=nullptr;
    const AVCodec *m_avCodec=nullptr;
    SwsContext *m_swsContext=nullptr;
    AVFrame *m_pAvFrame;
    AVPacket *m_pAvPacket;
    int m_nWidth=0;
    int m_nHeight=0;
};

#endif // H264DECODE_H
