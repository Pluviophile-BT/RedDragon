#ifndef RDHTTPHEADER_H
#define RDHTTPHEADER_H

#include<iostream>
#include<cstring>
#include<string.h>
#include<map>
#include"RDHttpRepCode.h"

#define _In_
#define _Out_
#define _Inout_

using namespace std;

class RDHttpHeader
{
private:
    map <string,string> RequestHeader;
    map <int,string> RequestHeaderKey;
    map <string,string> ResponseHeader;
    map <string,string> RequestMUV;
public:
    explicit RDHttpHeader();
    ~RDHttpHeader();
    string header;
    map <int,string> getequestHeaderKey();

    void addHeader
    (
        _In_ string key,
        _In_ string value
    );
    
    string getResponseData
    (
        _In_ const char* StatusCode
    );
    
    map <string,string> getRequestHeaderMap
    (
        _In_ const char* headers
    );

    map <string,string>  getRequestMUV
    (
        _In_ const char* headers
    );
};
#endif