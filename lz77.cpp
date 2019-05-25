#include "lz77.h"
#include "QtGlobal"
#include "QDataStream"

lz77::lz77()
{

}

QByteArray lz77::Compress(QByteArray data, bool header/* = false*/)
{
    QByteArray newdata;
    QDataStream res(&newdata, QIODevice::WriteOnly);
    res.setByteOrder(QDataStream::LittleEndian);

    if (header)
    {
        res << static_cast<quint32>(0x37375A4C); //LZ77
    }

    res << static_cast<qint32>((data.size() << 8) | 0x10);

    quint8 tempBuffer[16];

    //Current byte to compress.
    int current = 0;

    while (current < data.size())
    {
        int tempBufferCursor = 0;
        quint8 blockFlags = 0;
        for (int i = 0; i < 8; i++)

        {
            //Not sure if this is needed. The DS probably ignores this data.
            if (current >= data.size())
            {
                tempBuffer[tempBufferCursor++] = 0;
                continue;
            }

            int searchPos = 0;
            int searchLen = 0;
            Compress_Search(data, current, searchPos, searchLen);
            int searchDisp = current - searchPos - 1;
            if (searchLen > 2) //We found a big match, let's write a compressed block.
            {
                blockFlags |= static_cast<quint8>(1 << (7 - i));
                tempBuffer[tempBufferCursor++] = static_cast<quint8>((((searchLen - 3) & 0xF) << 4) + ((searchDisp >> 8) & 0xF));
                tempBuffer[tempBufferCursor++] = static_cast<quint8>(searchDisp & 0xFF);
                current += searchLen;
            }
            else
            {
                tempBuffer[tempBufferCursor++] = static_cast<quint8>(data[current++]);
            }
        }

        res << static_cast<quint8>(blockFlags);
        for (int i = 0; i < tempBufferCursor; i++)
            res << static_cast<quint8>(tempBuffer[i]);
    }

    return newdata;
}

void lz77::Compress_Search(QByteArray data, int pos, int& match, int& length)
{
    int maxMatchDiff = 4096;
    int maxMatchLen = 18;
    match = 0;
    length = 0;

    int start = pos - maxMatchDiff;
    if (start < 0) start = 0;

    for (int thisMatch = start; thisMatch < pos; thisMatch++)
    {
        int thisLength = 0;
        while(thisLength < maxMatchLen
            && thisMatch + thisLength < pos
            && pos + thisLength < data.size()
            && data[pos+thisLength] == data[thisMatch+thisLength])
            thisLength++;

        if(thisLength > length)
        {
            match = thisMatch;
            length = thisLength;
        }

        //We can't improve the max match length again...
        if(length == maxMatchLen)
            return;
    }
}
