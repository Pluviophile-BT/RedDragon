#ifndef RDGZIPENCODE_H
#define RDGZIPENCODE_H

#include<iostream>
#include<zlib.h>

#define _In_
#define _Inout_
#define _Out_

using namespace std;

class RDGzipEncode
{
private:
    
public:
    RDGzipEncode(/* args */);

    static string Compress
    (
        _In_ string inputData
    );

    static string Uncompress
    (
        _In_ string inputData
    );
    ~RDGzipEncode();
};
#endif