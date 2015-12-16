#include "stringUtil.h"
unsigned int cvt(int a) {
    if(a>=0) return a;
    return 256+a;
}
unsigned int rotl(unsigned int value, int shift) {
    return (value << shift) | (value >> (sizeof(value)*8 - shift));
}
string bin(string str) {
    string toRet = "";
    for(unsigned int i = 0;i < str.size();i++)
        for(int j = 7;j >= 0;j--)
            toRet.push_back(((str[i]>>j)&1) + '0');
    return toRet;
}
string bin(unsigned int i)
{
    string toRet = "";
    for(int j = 31;j >= 0;j--)
        toRet.push_back(((i>>j) & 1) + '0');
    return toRet;
}
string hex(string str)
{
    string toRet = "";
    char mp[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    for(int i = 0;i < str.size();i++) { // 1 byte of str to 2 bytes
        char b1 = (str[i]>>4)&15;
        char b2 = str[i]&15;
        toRet.push_back(mp[b1]);
        toRet.push_back(mp[b2]);
    }
    return toRet;
}
string hex(byteArray data)
{
    string toRet = "";
    char mp[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    for(int i = 0;i < data.size();i++) { // 1 byte of str to 2 bytes
        char b1 = (data[i]>>4)&15;
        char b2 = data[i]&15;
        toRet.push_back(mp[b1]);
        toRet.push_back(mp[b2]);
    }
    return toRet;
}
string SHA1(string str)
{
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xEFCDAB89;
    unsigned int h2 = 0x98BADCFE;
    unsigned int h3 = 0x10325476;
    unsigned int h4 = 0xC3D2E1F0;
    // preprocessing
    unsigned long long int ml = str.size()*8;
    str.push_back(char(128));
    while(str.size()%64 != 56) str.push_back(0); // padding until %512 = 448 bits
    for(int i = 7;i >= 0;i--) {
        // add ml
        str.push_back((ml>>(8*i)) & 255);
    }
    for(int chk = 0;chk < str.size()/64;chk++)
    {
        unsigned int w[80];
        for(int i = 0;i < 16;i++) {
            unsigned int thisW = 0;
            for(int j = 0;j < 4;j++) {
                thisW<<=8;
                thisW+=cvt(str[chk*64+i*4+j]);
            }
            w[i] = thisW;
        }
        // extend 16 -> 80
        for(int i = 16;i < 80;i++) {
            w[i] = rotl(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16],1);
        }
        unsigned int a = h0;
        unsigned int b = h1;
        unsigned int c = h2;
        unsigned int d = h3;
        unsigned int e = h4;
        unsigned int f,k;
        for(int i = 0;i < 80;i++) {
            if(i < 20) {
                f = (b&c)|((~b)&d);
                k = 0x5A827999;
            }
            else if(i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if(i < 60) {
                f = (b&c)|(b&d)|(c&d);
                k = 0x8F1BBCDC;
            }
            else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            unsigned int tmp = rotl(a,5)+f+e+k+w[i];
            e = d;
            d = c;
            c = rotl(b,30);
            b = a;
            a = tmp;
        }
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }
    unsigned int hh[5];
    hh[0] = h0;
    hh[1] = h1;
    hh[2] = h2;
    hh[3] = h3;
    hh[4] = h4;
    // done , translate hh to string
    string toRet = "";
    for(int i = 0;i < 5;i++)
        for(int j = 3;j >= 0;j--)
            toRet.push_back((hh[i]>>(j*8)) & 255);
    return toRet;
}
char b64mp(int i)
{
    if(i <= 25) return 'A'+i;
    else if(i <= 51) return 'a'+i-26;
    else if(i <= 61) return '0'+i-52;
    else if(i == 62) return '+';
    return '/';
}
string base64(string str)
{
    // A-Z a-z 0-9 + /
    string output = "";
    for(int i = 0;i < str.size();i+=3)
    {
        unsigned int c1,c2,c3;
        if(i+2<str.size()) {
            // full
            c1 = cvt(str[i]);
            c2 = cvt(str[i+1]);
            c3 = cvt(str[i+2]);
            output.push_back(b64mp( (c1>>2) ));
            output.push_back(b64mp( ((c1&3)<<4) + (c2>>4) ));
            output.push_back(b64mp( ((c2&15)<<2) + (c3>>6) ));
            output.push_back(b64mp( (c3&63) ));
        }
        else if(i+1<str.size()) {
            // 1 pad
            c1 = cvt(str[i]);
            c2 = cvt(str[i+1]);
            output.push_back(b64mp( (c1>>2) ));
            output.push_back(b64mp( ((c1&3)<<4) + (c2>>4) ));
            output.push_back(b64mp( (c2&15)<<2 ));
            output.push_back('=');
        }
        else {
            // 2 pad
            c1 = cvt(str[i]);
            output.push_back(b64mp( (c1>>2) ));
            output.push_back(b64mp( (c1&3)<<4 ));
            output.push_back('=');
            output.push_back('=');
        }
    }
    return output;
}
string translate_s1_to_s2(string str) // ABCDE -> _A_B_C_D_E
{
    string toRetStr = "";
    for(int i = 0;i < str.size();i++) {
        toRetStr.push_back(0);
        toRetStr.push_back(str[i]);
    }
    return toRetStr;
}
string translate_ws_to_s1(wstring str) // ABCD[Unicode char] -> ABCD*
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
