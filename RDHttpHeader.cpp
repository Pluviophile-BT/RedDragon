#include"RDHttpHeader.h"

RDHttpHeader::RDHttpHeader()
{
    nullptr;
}

RDHttpHeader::~RDHttpHeader()
{
    nullptr;
}

void RDHttpHeader::addHeader
(
    _In_ string key,
    _In_ string value
)
{
    header.append(key)
                .append(": ")
                .append(value)
                .append("\r\n");
}

string RDHttpHeader::getResponseData
(
    _In_ const char* StatusCode
)
{
    string BeginHeader= "HTTP/1.1 ";
    BeginHeader += StatusCode;
    BeginHeader.append("\r\n");

    BeginHeader.append(header).append("\r\n");
    return BeginHeader;
}
/**
 * 将接受到的字节数据转为Map存储
 * */
map <string,string> RDHttpHeader::getRequestHeaderMap
(
    _In_ const char* header
)
{
    char* tp;
	char* tp2;
    char* tp3;
    char headers[1024];
    memset(headers, 0x00, 1024);
    memcpy(headers, header, strlen(header)+1);

	char* u=strtok_r(headers,"\r\n",&tp);

    //获取请求头
    u=strtok_r(NULL,"\r\n",&tp);
	for(int i=0; u; i++)
	{
		char* toc_u=strtok_r(u, ":",&tp2);
        string header_key = toc_u;
        string header_value = (tp2+1);
        RequestHeaderKey.insert(make_pair(i, header_key));
        RequestHeader.insert(make_pair(header_key, header_value));
		u=strtok_r(NULL,"\r\n",&tp);
	}
    return RequestHeader;
}

map <int,string>RDHttpHeader::getequestHeaderKey()
{
    return RequestHeaderKey;
}

map <string,string> RDHttpHeader::getRequestMUV
(
     _In_ const char* header
)
{
     char* tp;
	char* tp2;

    char headers[1024];
    memset(headers, 0x00, 1024);
    memcpy(headers, header, strlen(header)+1);
	char* u=strtok_r(headers,"\r\n",&tp);

    //获取请求方式,URI,HTTP版本信息
    char data[400];
	memset(data, 0x0, 400);
	memcpy(data, u, strlen(u)+1);
	char* req=strtok_r(data, " ", &tp2);
    string li[] = {"method", "uri", "version"};
	for(int i=0; req; i++)
	{
		RequestMUV.insert(make_pair(li[i], req));
		req=strtok_r(NULL, " ", &tp2);
	}
    return RequestMUV;
}