#include <windows.h>
#include <string>
#include <vector>
using namespace std;
vector<int> V;
int* test;
typedef void (*masterFunc)();
masterFunc mf;
extern "C" __declspec(dllexport) string who()
{
    return "Module C";
}
extern "C" __declspec(dllexport) void store(int i)
{
    V.push_back(i);
}
extern "C" __declspec(dllexport) void global(int* testPtr,masterFunc m)
{
    test = testPtr;
    mf = m;
}
extern "C" __declspec(dllexport) int sum()
{
    int toRet = 0;
    for(int i = 0;i < V.size();i++) toRet+=V[i];
    mf();
    return toRet;
}
extern "C" __declspec(dllexport) int getGlobal()
{
    return *test;
}
