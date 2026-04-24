#include "rtpsource.h"
#include <QThread>
#include <QNetworkDatagram>
#include<QDebug>
RtpSource::RtpSource(QObject *parent):QUdpSocket(parent) {
    bind(QHostAddress::Any,1234);
    connect(this,&RtpSource::readyRead,this,&RtpSource::slot_receiveRtp);

}

void RtpSource::slot_receiveRtp()
{
    while(this->hasPendingDatagrams())
    {
        auto datagrame=this->receiveDatagram();
        auto data=datagrame.data();
        qDebug()<<"    enter"<<this->pendingDatagramSize();
        if(data.size()>12)
        {
            rtpHeader *header=(rtpHeader*)(data.data());
            qDebug()<<"cc="<<header->cc<<" x="<<header->x<<" p="<<header->p<<" v"<<header->v
                     <<"payload="<<header->payload<<" m"<<header->m<<" seqNumber="<<header->seqNumber
                     <<"timeStamp="<<header->timestamp<<" ssrc="<<header->ssrc;

            QThread::msleep(1000);
            int nLength=12;
            nLength+=header->cc*4;
            auto frameData=data.mid(nLength);
            if(header->x)
            {
                frameData=frameData.mid(2);

                quint16_be length=*(quint16_be*)(frameData.constData());
                qDebug()<<"length"<<length;
                frameData= frameData.mid(2+4*length);
            }
            if(header->p)
            {
                quint8 paddingLength=(quint8)frameData[frameData.length()-1];
                frameData=frameData.mid(0,frameData.length()-paddingLength);
            }
            auto newRtpData=new rtpData;
            newRtpData->data=frameData;
            newRtpData->seqNumber=header->seqNumber;
            storagePacket(newRtpData);
        }
    }

}
//因为序列号是一个16位数 他满了以后会重复从零开始计数，为了解决循环计数 以半程距离为限制 去判断是不是循环使用了 这个半程也是live555自定义的
//live555不可能缓存超过半程的包
bool RtpSource::seqNumLt(quint16 s1, quint16 s2)
{
    int diff=s2-s1;
    if(diff>0)
    {
        return diff<0x8000;
    }
    else if(diff ==0)
        return false;
    else
    {
        return diff<-0x8000;
    }
}

void RtpSource::storagePacket(rtpData *pRtpData)
{
    if(head==nullptr)
    {
        head=pRtpData;
    }
    else
    {
        if(!seqNumLt( pRtpData->seqNumber,head->seqNumber))
        {
            if(pRtpData->seqNumber==head->seqNumber)
            {
                delete pRtpData;
                return;
            }
            if(tail==nullptr)
            {
                head->next=pRtpData;
                tail=pRtpData;
            }
            else
            {
                if(seqNumLt(tail->seqNumber,pRtpData->seqNumber))
                {
                    tail->next=pRtpData;
                    tail=pRtpData;
                }
                else{
                    auto cursor=head->next;
                    while(cursor)
                    {
                        if(cursor->next==nullptr)
                        {
                            cursor->next=pRtpData;
                            tail=pRtpData;
                            break;
                        }
                        if(seqNumLt(cursor->seqNumber,pRtpData->seqNumber)&&!seqNumLt(cursor->next->seqNumber,pRtpData->seqNumber))
                        {
                            pRtpData->next=cursor->next;
                            cursor->next=pRtpData;
                            break;
                        }
                        cursor=cursor->next;

                    }
                }

            }
        }
        else
        {

            pRtpData->next=head;
            if(tail==nullptr)
            {
                tail=head;
            }
            head=pRtpData;

        }
    }
    auto cursor=head;
    while(cursor)
    {
        qDebug()<<cursor->seqNumber;
        cursor=cursor->next;
    }
    qDebug()<<"    end";
}
