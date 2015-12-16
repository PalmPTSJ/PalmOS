#ifndef STRINGUTIL_H_INCLUDED
#define STRINGUTIL_H_INCLUDED
#include "network_ext.h"
#include <string.h>
#include <tchar.h>
#include <string>
#include <wchar.h>
#include <sstream>

using namespace std;

unsigned int cvt(int a);
unsigned int rotl(unsigned int value, int shift);

string bin(string str);
string bin(unsigned int i);

string hex(string str);
string hex(byteArray data);

string SHA1(string str);
char b64mp(int i);
string base64(string str);

string translate_s1_to_s2(string str);
string translate_ws_to_s1(wstring str);
string translate_ws_to_s2(wstring str);
wstring translate_s2_to_ws(string str);
vector<string> split(string str);
vector<wstring> split(wstring str);

#endif // STRINGUTIL_H_INCLUDED
