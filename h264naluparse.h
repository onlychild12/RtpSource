#ifndef H264NALUPARSE_H
#define H264NALUPARSE_H

#include <QObject>
class rtpData;
class H264NaluParse : public QObject
{
    Q_OBJECT
public:
    explicit H264NaluParse(QObject *parent = nullptr);
    void parsePacket(const QByteArray& data);
    void outPutFrame(const QByteArray& data);
public slots:
    void slot_lostFrame();
signals:
    void sig_EmitNewFrame(QByteArray data);
private:
    QByteArray m_FrameByteArray;
    QByteArray m_LastSpsFrameData;
    QByteArray m_LastPpsFrameData;
    bool m_bWaitIDRFrame=true;
};

#endif // H264NALUPARSE_H
