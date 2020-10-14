#ifndef RDCONFIGREAD_H
#define RDCONFIGREAD_H

#include<iostream>
#include<string.h>
#include<fstream>
#include<map>

#define _In_
#define _Inout_
#define _Out_

using namespace std;

class RDConfigRead
{
private:
    map<string,string> ConfData;
    fstream ConfFile;
public:
    RDConfigRead();
    ~RDConfigRead();
    bool ReadParseConfigFile();
    string getWebAppDirectory();
    string getCharSet();
    string getRDVersion();
    string getDefaultPage
    (
        _In_ string index
    );
    int getListenPort();
    int getMaxConnectNum();
};
#endif