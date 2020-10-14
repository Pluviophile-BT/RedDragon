#ifndef RDMIMETYPE_H
#define RDMIMETYPE_H

/**
 * Author: Pluviophi;e
 * Date: 2020.7.10
 * 
 * 此类用于解析文件mimetype
 * 
 * */
#include<iostream>
#include<unistd.h>
#include<cstring>
#include<string.h>
#include<map>
#include<string>

#define _In_
#define _Out_
#define _Inout_

using namespace std;

class RDMimeType
{
private:
    map <string,string>MimeType;
    string getSuffix
    (
        _In_ string file_path
    );
public:
    RDMimeType();
    //get request file mimetype
    string getMimeType
    (
        _In_ string path
    );
    ~RDMimeType();
};
#endif