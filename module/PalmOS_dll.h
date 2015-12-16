#include <windows.h>
#include <bits/stdc++.h>
#include <tchar.h>
#include <wchar.h>
using namespace std;
struct packetS {
    char enc;
    string opcode;
    string id;
    wstring data;
    string trimData;
    int type;
};

string translate_s1_to_s2(string str) // ABCDE -> _A_B_C_D_E
{
    string toRetStr = "";
    for(int i = 0;i < str.size();i++) {
        toRetStr.push_back(0);
        toRetStr.push_back(str[i]);
    }
    return toRetStr;
}
string translate_ws_to_s1(wstring str) // ABCD(Unicode char) -> ABCD*
{
    string toRetStr = "";
    for(int i = 0;i < str.size();i++) {
        if(int(str[i]) > 255) toRetStr.push_back('*');
        else toRetStr.push_back(str[i]);
    }
    return toRetStr;
}
string translate_ws_to_s2(wstring str) // ABCDX -> _A_B_C_DXX
{
    string toRetStr = "";
    for(int i = 0;i < str.size();i++) {
        toRetStr.push_back(int(str[i])>>8);
        toRetStr.push_back(int(str[i])%256);
    }
    return toRetStr;
}
wstring translate_s2_to_ws(string str) // _A_B_C_DXX -> ABCDX
{
    /// NOT TEST
    wstring toRet;
    for(int i = 0;i < str.size();i+=2) {
        if(i == str.size()-1) toRet.push_back(wchar_t(str[i]));
        else toRet.push_back(wchar_t(str[i]<<8+str[i+1]));
    }
    return toRet;
}
vector<string> split(string str) {
    vector<string> args;
    args.push_back("");
    for(int i = 0;i < str.size();i++) {
        if(str[i] == '|') args.push_back("");
        else args[args.size()-1].push_back(str[i]);
    }
    return args;
}
vector<wstring> split(wstring str) {
    vector<wstring> args;
    args.push_back(L"");
    for(int i = 0;i < str.size();i++) {
        if(str[i] == wchar_t('|')) args.push_back(L"");
        else args[args.size()-1].push_back(str[i]);
    }
    return args;
}

/*packetS encodePacket(string opcode,string data,string id="0",int type=WS_OP_TXT,char encodingType='2')
{
    packetS toRet = {encodingType,opcode,id,data,data,type};
    return toRet;
}*/
