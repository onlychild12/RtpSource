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
    void init();
signals:

private:
    AVCodecContext *m_avContext=nullptr;
    const AVCodec *m_avCodec=nullptr;
    SwsContext *m_swsContext=nullptr;
};

#endif // H264DECODE_H
