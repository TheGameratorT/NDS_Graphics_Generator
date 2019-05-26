#include "lz77.h"
#include "QtGlobal"
#include "QDataStream"

//Ported from C# WinForms to Qt C++
//Original code can be found in Dirbaio's NSMB Editor

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

QByteArray lz77::Decompress(QByteArray source, bool header/* = false*/)
{
    int DataLen;
    if(header)
        DataLen = source[5] | (source[6] << 8) | (source[7] << 16);
    else
        DataLen = source[1] | (source[2] << 8) | (source[3] << 16);
    QByteArray dest = QByteArray::number(DataLen);
    int i, j, xin, xout;
    if(header)
        xin = 8;
    else
        xin = 4;
    xout = 0;
    int length, offset, windowOffset, data;
    quint8 d;
    while (DataLen > 0)
    {
        d = static_cast<quint8>(source[xin++]);
        if (d != 0)
        {
            for (i = 0; i < 8; i++)
            {
                if ((d & 0x80) != 0)
                {
                    data = ((source[xin] << 8) | source[xin + 1]);
                    xin += 2;
                    length = (data >> 12) + 3;
                    offset = data & 0xFFF;
                    windowOffset = xout - offset - 1;
                    for (j = 0; j < length; j++)
                    {
                        dest[xout++] = dest[windowOffset++];
                        DataLen--;
                        if (DataLen == 0)
                        {
                            return dest;
                        }
                    }
                }
                else
                {
                    dest[xout++] = source[xin++];
                    DataLen--;
                    if (DataLen == 0)
                    {
                        return dest;
                    }
                }
                d <<= 1;
            }
        }
        else
        {
            for (i = 0; i < 8; i++)
            {
                dest[xout++] = source[xin++];
                DataLen--;
                if (DataLen == 0)
                {
                    return dest;
                }
            }
        }
    }
    return dest;
}
