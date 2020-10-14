#include"RDEpollSocket.h"
#include"RDMimeType.h"
#include"RDHttpHeader.h"
#include"RDMimeType.h"
#include"RDGmtTime.h"
#include"RDConfigRead.h"

RDConfigRead cfg;
int MaxConnect = 0;
int lport = 0 ;
string Verison = "";
string RootDircectory = "";
string PageCharset = "";

/**
 * 处理客户端的http交互用string来存储header与body,切记最后一个header头不需要在最后添加"\r\n"
 * */
void callback
(
    _In_ io_context_t ctx,
    _In_ int fd,
    _In_ int io_fd,
    _In_ const char* buff
)
{
    //获取请求头信息以及请求方式与URI,并初始化后期所用的对象
    string data;
    string MimeTypeStr;
    fstream PageFile;
    RDMimeType MimeType;
    RDHttpHeader HttpHeader;
	map<string,string> requestheader=HttpHeader.getRequestHeaderMap(buff);
	map<string,string> muv=HttpHeader.getRequestMUV(buff);
	map<int,string> requestheaderkey=HttpHeader.getequestHeaderKey();

    //拼接URI与默认访问目录,以便从文件系统读取页面文件
    string uri = muv["uri"];
    if(uri == "/")
        uri = cfg.getDefaultPage("/");

    string directory = RootDircectory;
    directory.append(uri);

    MimeTypeStr = MimeType.getMimeType(directory);
    //初始化一些必要的响应头信息
    HttpHeader.addHeader("Server", Verison);
    HttpHeader.addHeader("Date", RDGmtTime::getGmtTime());

    //header.append("Content-Encoding: ").append("br").append("\r\n");

    //对URI长度进行限制
    if(uri.length() > 800)
    {
        //uri too long 414 
        string pagePath = cfg.getDefaultPage("414");
        PageFile.open(pagePath, ios::in);
        if(PageFile)
        {
            string line;
            string PageData = "";
            while (getline(PageFile, line))
                PageData.append(line).append("\n");
            
            HttpHeader.addHeader("Content-Length", to_string(PageData.length()));
            HttpHeader.addHeader("Content-Type", MimeType.getMimeType(pagePath).append("; charset=").append(PageCharset));

            string WriteData = HttpHeader.getResponseData(RD_URITOOLONG)
                                            .append(PageData);

            RDEpollSocket::SubmitWrite(ctx, fd, io_fd, (char*)WriteData.c_str(), WriteData.length());
        }
        else
        {
            string PageData = "<html>"
                                            "<head>"
                                            "<title>414</title>"
                                            "</head>"
                                            "<body>"
                                            "<h1>414</h1>"
                                            "</body>"
                                            "</html>";
            
            HttpHeader.addHeader("Content-Length", to_string(PageData.length()));
            HttpHeader.addHeader("Content-Type", string("text/html; charset=").append(PageCharset));

            string WriteData = HttpHeader.getResponseData(RD_URITOOLONG)
                                            .append(PageData);
                        
            RDEpollSocket::SubmitWrite(ctx, fd, io_fd, (char*)WriteData.c_str(), WriteData.length());
        }
        PageFile.close();
        close(fd);
        return;
    }
    else
    {
        //打开页面文件
        PageFile.open(directory, ios::in);
        //如果页面文件不存在
        if(!PageFile)
        {
            //not found 404
            string pagePath = cfg.getDefaultPage("404");
            PageFile.open(pagePath, ios::in);
            if(PageFile)
            {
                string line;
                string pageData = "";
                while(getline(PageFile, line))
                    pageData.append(line).append("\n");

                HttpHeader.addHeader("Content-Length", to_string(pageData.length()));
                HttpHeader.addHeader("Content-Type", MimeType.getMimeType(pagePath).append("; charset=").append(PageCharset));

                string WriteData = HttpHeader.getResponseData(RD_NOTFOUND)
                                                .append(pageData);

                RDEpollSocket::SubmitWrite(ctx, fd, io_fd, (char*)WriteData.c_str(), WriteData.length());
            }
            else
            {
                string ND = "<html>"
                                    "<head><title>404</title></head>"
                                    "<body><h1>404 page not found 404</h1></body>"
                                    "</html>";

                HttpHeader.addHeader("Content-Length", to_string(ND.length()));
                HttpHeader.addHeader("Content-Type",string("text/html; charset=").append(PageCharset));

                string WriteData = HttpHeader.getResponseData(RD_NOTFOUND)
                                                .append(ND);

                RDEpollSocket::SubmitWrite(ctx, fd, io_fd, (char*)WriteData.c_str(), WriteData.length());
            }
        }
        //如果页面文件存在
        else
        {
            string PageData;
            string line;
            while(getline(PageFile, line))
                PageData.append(line).append("\n");

            HttpHeader.addHeader("Content-Length", to_string(PageData.length()));

            if(MimeTypeStr == "text/html")
                HttpHeader.addHeader("Content-Type",MimeTypeStr.append(";charset=").append(PageCharset));
            else
                HttpHeader.addHeader("Content-Type",MimeTypeStr);
                
            string WriteData = HttpHeader.getResponseData(RD_OK)
                                            .append(PageData);
            
            unsigned long DataSize = WriteData.length();
            //小于1M的文件
            if(DataSize < 1048576)
                RDEpollSocket::SubmitWrite(ctx, fd, io_fd, (char*)WriteData.c_str(), DataSize);
            //大于1M的文件
            else
            {
                for(int i =0; i< DataSize; i+=16384)
                {
                    int o=0xffffff;
                    char SendData[16384];
                    memset(SendData, 0x00, 16384);
                    memcpy(SendData, (char*)(WriteData.c_str()+i), 16384);

                    write(fd, SendData, 16384);
                    while(o--);
                }
            }
        }
    }
    PageFile.close();
    close(fd);
}

int main()
{
    cfg.ReadParseConfigFile();
    lport = cfg.getListenPort();
    Verison = cfg.getRDVersion();
    MaxConnect = cfg.getMaxConnectNum();
    RootDircectory = cfg.getWebAppDirectory();
    PageCharset = cfg.getCharSet();
    if((lport < 1024)||(MaxConnect < 1)||(RootDircectory == "")||(PageCharset == ""))
    {
        return 0;
    }
    //cout<<lport<<"\n"<<Verison<<"\n"<<MaxConnect<<"\n"<<RootDircectory<<"\n"<<PageCharset<<"\n"<<cfg.getDefaultPage("405")<<"\n";
    RDEpollSocket s;
    s.setLport(lport);
    s.setMaxSize(MaxConnect);
    s.setRDCallBack(callback);
    s.start();
    return 0;
    /*RDMimeType m;
    cout<<m.getFileType("html");*/
}
