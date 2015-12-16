#include "PalmOS_dll.h"
/*wstring retr(wstring dir)
{
    wstring path(dir.begin(),dir.end());
    path.append(L"\\*");
    WIN32_FIND_DATAW FindData;
    HANDLE hFind;
    hFind = FindFirstFileW( path.c_str(), &FindData );
    wstring toRet = dir;
    toRet.push_back(wchar_t('|'));
    if( hFind == INVALID_HANDLE_VALUE ) {
        return L"";
    }
    do
    {
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            toRet.append(L">");
            toRet.append(FindData.cFileName);
        }
        else {
            toRet.append(FindData.cFileName);
        }
        toRet.push_back('|');
    }
    while( FindNextFileW(hFind, &FindData) > 0 );
    if( GetLastError() != ERROR_NO_MORE_FILES ) {
        cout << "Something went wrong during searching\n";
    }
    return toRet;
}

int cntDown = 0;
string driveList_old;
string getDriveList()
{
    string toRet = "";
    char dLetter = 'A';
    DWORD dMask = GetLogicalDrives();
    while(dMask) {
        if(dMask & 1) {
            string getType = "";
            getType.push_back(dLetter);
            getType.append(":\\");
            int res = GetDriveType(getType.c_str());
            if(res != DRIVE_UNKNOWN && res != DRIVE_NO_ROOT_DIR) {
                char str[1000];
                if(GetVolumeInformation(getType.c_str(),str,1000,NULL,NULL,NULL,NULL,0) != 0) {
                    toRet.push_back(dLetter);
                    toRet.push_back('|');
                }
            }
        }
        dLetter++;
        dMask >>= 1;
    }
    return toRet;
}*/

extern "C" __declspec(dllexport) string who()
{
    return "File Module";
}
extern "C" __declspec(dllexport) string version()
{
    return "1.0";
}
extern "C" __declspec(dllexport) packetS exec(packetS p)
{
    cout << "FILE : exec\n";
    string opcode = p.opcode;
    wstring decodeMsg = p.data;
    /*if(opcode.compare("RETR") == 0) { /// RETR [DIR] : retrieve file list at DIR
        wstring toRet = retr(decodeMsg);
        packetS toRet = {'2',"DIRLST",p.id,translate_ws_to_s2(toRet),"",};
        server.sendTo(wsEncodeMsg("DIRLST",translate_ws_to_s2(toRet),pData.id,WS_OP_TXT,'2'),i);
    }
    else if(opcode.compare("EXEC") == 0) { /// EXEC [PATH] : Execute file
        wstring fullpath = decodeMsg;
        wstring dir = fullpath.substr(0,fullpath.find_last_of(L"/\\"));
        if(isLogEnable("OS")) cout << logHeader("OS",2) << reqClientInfo(i) << "EXEC" << endl;
        ShellExecuteW(NULL, NULL, fullpath.c_str(), NULL, dir.c_str(), SW_SHOWNORMAL);
    }
    else if(opcode.compare("RETD") == 0) { /// RETD [] : retrieve available physical drives letter
        if(isLogEnable("OS")) cout << logHeader("OS",2) << reqClientInfo(i) << "RETD" << endl;
        server.sendTo(wsEncodeMsg("DRIVE",driveList_old,pData.id,WS_OP_TXT,'1'),i);
    }
    else if(opcode.compare("READ") == 0) { /// READ [PATH] : read file data
        wstring fullpath = decodeMsg;
        FILE* f = _wfopen(fullpath.c_str(),L"rb");
        if(f == NULL) {
            // file not exist
            server.sendTo(wsEncodeMsg("ERROR","Can't open file",pData.id,WS_OP_BIN,'2'),i);
            continue;
        }
        wstring toSendH = fullpath;
        toSendH.append(L"|");
        string realData = translate_ws_to_s2(toSendH);
        char c;
        int sizeCnt = 0;
        c=fgetc(f);
        while(!feof(f)) {
            realData.push_back(cvt(c));
            sizeCnt++;
            c=fgetc(f);
        }
        //cout << "    " << "File size : " << sizeCnt << endl;
        if(ferror(f) != 0) {
            //cout << "    " << "File read error code : " << ferror(f) << endl;
            if(isLogEnable("OS")) cout << logHeader("OS",2) << reqClientInfo(i) << "READ " << translate_ws_to_s1(fullpath) << " , Error code " << ferror(f) << endl;
        }
        else {
            if(isLogEnable("OS")) cout << logHeader("OS",2) << reqClientInfo(i) << "READ " << translate_ws_to_s1(fullpath) << " , size " << sizeCnt << endl;
        }
        fclose(f);
        server.sendTo(wsEncodeMsg("FILE",realData,pData.id,WS_OP_BIN,'2'),i);
    }
    else if(opcode.compare("SAVE") == 0) { /// SAVE [PATH | BINARYDATA] : save binary data to file
        cout << "    " << "<SAVE>" << endl;
        wstring param = decodeMsg;
        int splitPos = param.find(L"|");
        if(splitPos == string::npos) continue; // not correct packet
        wstring filepath = param.substr(0,splitPos);
        string binaryData = pData.trimData.substr((filepath.size()+1) * (pData.enc=='2'?2:1));
        cout << "    " << "File will be save to " << translate_ws_to_s1(filepath) << endl;
        cout << "    " << "File size : " << binaryData.size() << endl;
        cout << "    " << "File begin : " << hex(binaryData.substr(0,20)) << endl;
        FILE* f = _wfopen(filepath.c_str(),L"wb");
        // add data to file
        for(int i = 0;i < binaryData.size();i++) fputc(binaryData[i],f);
        fclose(f);
    }
    return toRet;*/
}
extern "C" __declspec(dllexport) void registerGlobalVar()
{

}
extern "C" __declspec(dllexport) vector<string> load()
{
    vector<string> toRet;
    toRet.push_back("OPNAME|FILE");
    toRet.push_back("FLAG|FLAG_DRIVECHANGE");
    toRet.push_back("POLL|60");
    return toRet;
}
extern "C" __declspec(dllexport) void poll()
{
    cout << "FILE : poll\n";
}
extern "C" __declspec(dllexport) void unload()
{
    cout << "FILE : unload\n";
}
