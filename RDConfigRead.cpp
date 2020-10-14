#include"RDConfigRead.h"

RDConfigRead::RDConfigRead()
{
}

RDConfigRead::~RDConfigRead()
{
}

bool RDConfigRead::ReadParseConfigFile()
{
    string line;
    int linesize=0;
    
    ConfFile.open("./conf/RedDragon.conf", ios::in);
    if( !ConfFile)
        return false;
    while(getline(ConfFile,line))
    {
        if(line[0] != '#')
        {
            if(line.find(":") != string::npos)
            {
                char* p = NULL;
                linesize = line.length();
                char temp[2048];// = new  char(linesize);
                memset(temp, 0x00, 2048);
                memcpy(temp, line.c_str(), linesize);
                char* u = strtok_r(temp, ":", &p);
                ConfData.insert(make_pair(u, p));
                //delete temp;
            }
        }
    }
    return true;
}

string RDConfigRead::getDefaultPage
(
     _In_ string index
)
{
    return ConfData[index];
}

string RDConfigRead::getRDVersion()
{
    return ConfData["Version"];
}
string RDConfigRead::getWebAppDirectory()
{
    return ConfData["Directory"];
}

string RDConfigRead::getCharSet()
{
    return ConfData["Charset"];
}

int RDConfigRead::getListenPort()
{
    return atoi(ConfData["Port"].c_str());
}

int RDConfigRead::getMaxConnectNum()
{
    return atoi(ConfData["Maxconnectnum"].c_str());
}