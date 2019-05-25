#ifndef LZ77_H
#define LZ77_H

#include "QtGlobal"


class lz77
{
public:
    static QByteArray Compress(QByteArray data, bool header = false);
    static QByteArray Decompress(QByteArray source, bool header = false);
    static void Compress_Search(QByteArray data, int pos, int& match, int& length);
};

#endif // LZ77_H
