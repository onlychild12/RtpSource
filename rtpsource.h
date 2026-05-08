#ifndef RTPSOURCE_H
#define RTPSOURCE_H
#include <QtGlobal>
#include <QtEndian>
#include <QUdpSocket>
#include <QObject>
#include <QElapsedTimer>
typedef  struct{
    quint8 cc:4;
    quint8 x:1;
    quint8 p:1;
    quint8 v:2;

    quint8 payload:7;
    quint8 m:1;

    quint16_be seqNumber;

    quint32_be timestamp;

    quint32_be ssrc;

}__attribute__((packed)) rtpHeader;
struct rtpData{
    qint32 seqNumber;
    QByteArray data;
    rtpData*next=nullptr;
 QElapsedTimer arriveTime;
};
class RtpSource : public QUdpSocket
{
    Q_OBJECT
public:
    RtpSource(QObject *parent=nullptr);
public slots:
    void slot_receiveRtp();
private:
    bool seqNumLt(quint16 s1,quint16 s2);
    void storagePacket(rtpData *data);
    void tryDeliverPackets();
signals:
    void rtpParseData(const QByteArray& data);
    void lostFrame();
private:
    rtpData*head=nullptr;
    rtpData*tail=nullptr;
    quint16 m_nExpectSeq=0;
};

#endif // RTPSOURCE_H
