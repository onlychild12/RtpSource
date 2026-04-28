#include "h264naluparse.h"
#include "rtpsource.h"
static const QByteArray START_CODE = QByteArray::fromHex("00000001");
H264NaluParse::H264NaluParse(QObject *parent)
    : QObject{parent}
{

}

void H264NaluParse::parsePacket(const QByteArray& data)
{
    if(data.isEmpty())
        return;
    quint8 firstByte=(quint8)data[0];
    quint8 type=firstByte&0x1F;
    QByteArray frameData=data;
    switch(type)
    {
    /*   RTP 载荷格式:
                    *   +--------+--------+--------+--------+     +--------+--------+--------+--------+
                    *   | 24(1B) | Size_1 (2B,BE) | NALU_1  | ... | Size_n (2B,BE) | NALU_n  |
        *   +--------+--------+--------+--------+     +--------+--------+--------+--------+
                                                                                                           *
                                                                                                           *   解析步骤:
                                                                                                                       *   1. 跳过第 1 个字节（0x18 = 24）
    *   2. 循环读取: 2 字节大端序长度 → 跟对应长度的 NAL 数据 → 直到读完*/
    case 24://stap-a 聚合包 里面有很多nalu
        qDebug()<<"stap-a packet!";
        frameData=frameData.mid(1);
        while(frameData.size()>2)
        {
            quint16 frameDataSize=qFromBigEndian<quint16>(frameData.constData());
            outPutFrame(frameData.mid(2,frameDataSize));
            frameData=frameData.mid(frameDataSize+2);
            qDebug()<<"stap-a packet size "<<frameDataSize;
        }

        break;
    case 25:
    case 26:
    case 27:
        qDebug()<<"h264 other type packet"<<type;
        break;
    case 28:
        /*
 * 3.3 FU-A 分片模式（type = 28）★ 最重要
 * -------------------------------------------------
 * 当 NAL 单元太大（超过 MTU ≈ 1500 字节）时，拆成多个 RTP 包传输。
 *
 *   FU-A RTP 载荷格式:
 *   +---------------+---------------+
 *   | FU Indicator  |  FU Header    |  FU Payload (NAL 单元数据片段)...
 *   |   (1 字节)    |   (1 字节)    |
 *   +---------------+---------------+
 *
 *
 *   3.3.1 FU Indicator（1 字节）:
 *     bit:  7    6    5    4    3    2    1    0
 *          +----+----+----+----+----+----+----+----+
 *          | F  |     NRI       |     28 (FU-A)    |
 *          +----+----+----+----+----+----+----+----+
 *
 *     高 3 位 (F+NRI) = 原始 NAL header 的高 3 位（原样拷贝）
 *     低 5 位 = 总是 28（表明这是一个 FU-A 包）
 *
 *     含义: 告诉接收端这个分片的原始重要性和原始 NAL 是什么类型
 *           （NRI 先保留，类型信息丢失了但会在 FU Header 里补回来）
 *
 *
 *   3.3.2 FU Header（1 字节）:
 *     bit:  7    6    5    4    3    2    1    0
 *          +----+----+----+----+----+----+----+----+
 *          | S  | E  | R  |  原始 NAL Unit Type   |
 *          +----+----+----+----+----+----+----+----+
 *
 *      S  (Start bit, bit7, 0x80): 1 = 这是第一个分片，需要重建 NAL header
 *      E  (End bit,   bit6, 0x40): 1 = 这是最后一个分片，NAL 单元完整了
 *      R  (Reserved,  bit5, 0x20): 保留位，必须为 0
 *      低5位 = 原始 NAL 单元的 Type（比如 IDR=5, SPS=7 等）
 *
 *
 *   3.3.3 ★ 核心逻辑：原始 NAL Header 的恢复
 *
 *   FU Indicator:  [F | NRI | 11100]    (高3位有效)
 *   FU Header:     [S | E | R | Type ]   (低5位有效)
 *                              ↓
 *   原始 NAL Header:  [F | NRI | Type ]   ← 把两部分拼起来
 *
 *   计算公式:
 *      originalNalHeader = (fuIndicator & 0xE0)   // 取高3位: F + NRI
 *                        | (fuHeader    & 0x1F);  // 取低5位: Type
 *
 *   对应 live555 代码 (H264VideoRTPSource.cpp 第93行):
 *      headerStart[1] = (headerStart[0]&0xE0) | (headerStart[1]&0x1F);
 */
        {
            QByteArray naluHeader;
            naluHeader.append((char)((frameData[0]&0xE0)|(frameData[1]&0x1F)));
            quint8 fuFlag=(frameData[1]&0xE0)>>5;
            frameData=frameData.mid(2);
            if(fuFlag==0x04)
            {
                m_FrameByteArray.clear();
                frameData= naluHeader+frameData;
            }
            if(!m_FrameByteArray.isEmpty()||fuFlag==0x04)
            {
                m_FrameByteArray.append(frameData);
            }
            if(fuFlag==0x02)
            {
                outPutFrame(m_FrameByteArray);

                m_FrameByteArray.clear();
            }
            qDebug()<<"fu-a nalu type="<<fuFlag;
            break;
        }
    default:
        //Single NAL Unit
        qDebug()<<"Singlenalu type="<<type;
        outPutFrame(frameData);
        break;

    }
}

void H264NaluParse::outPutFrame(const QByteArray &data)
{
    quint8 type=data[0]&0x1F;
    if(type==7)//sps帧
    {
        m_LastSpsFrameData=data;
    }
    else if(type==8)//pps帧
    {
        m_LastPpsFrameData=data;
    }
    else if(type==5)//idr帧 给他补上 sps和pps
    {
        if(!m_LastPpsFrameData.isEmpty()&&!m_LastSpsFrameData.isEmpty())
        {
            emit sig_EmitNewFrame(START_CODE+m_LastSpsFrameData);
            emit sig_EmitNewFrame(START_CODE+m_LastPpsFrameData);
        }
    }
    emit sig_EmitNewFrame(START_CODE+data);

}
