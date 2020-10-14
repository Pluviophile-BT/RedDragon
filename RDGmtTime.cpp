#include"RDGmtTime.h"

RDGmtTime::RDGmtTime(/* args */)
{
}

RDGmtTime::~RDGmtTime()
{
}

string RDGmtTime::getGmtTime()
{
    string GmtTime;
    time_t rawTime;
    struct tm* timeInfo;
    char szTemp[30]={0};
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    strftime(szTemp,sizeof(szTemp),"%a, %d %b %Y %H:%M:%S GMT",timeInfo);
    GmtTime = szTemp;
    
    return GmtTime;
}