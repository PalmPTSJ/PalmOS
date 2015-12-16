#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
// DLL function signature
typedef string (*who)();
typedef int (*sum)();
typedef void (*masterFunc)();
typedef void (*global)(int*,masterFunc);
typedef int (*getGlobal)();
typedef void (*store)(int);
vector<string> moduleListF;

int myG;

void mf() {
    printf("Master Function Run !!!\n");
}

int main(int argc, char **argv)
{
	moduleListF.push_back("test.dll");
    //moduleListF.push_back("modB.dll");
	// Load DLL file
	vector<HINSTANCE> moduleList;
	for(int i = 0;i < moduleListF.size();i++) {
        moduleList.push_back(LoadLibrary(moduleListF[i].c_str()));
        HINSTANCE* hin = &moduleList[i];
        if (hin  == NULL) {
            printf("ERROR: unable to load DLL\n");
            return 1;
        }
	}

	// Get function pointer
	//vector<who> moduleLoadWho;
	//moduleLoadWho.push_back((who) GetProcAddress(hinstLib, "who"));
	myG = 1;
	((global)GetProcAddress(moduleList[0], "global"))(&myG,&mf);
	while(true) {
        printf(">>> ");
        int t;
        scanf("%d",&t);
        if(t==0) {
            printf("%d\n",(sum)GetProcAddress(moduleList[0], "sum")());
            printf("Global : %d\n",(getGlobal)GetProcAddress(moduleList[0], "getGlobal")());
        }
        else if(t<0) {
            break;
        }
        else {
            ((store)GetProcAddress(moduleList[0], "store"))(t);
            myG *= t;
        }

        /*printf("%d modules loaded. Execute ? : ",moduleList.size());
        int t;
        scanf("%d",&t);
        if(t == -1) break;*/
        //printf("Output : %s\n\n",((who)GetProcAddress(moduleList[t], "who"))().c_str());
	}

	for(int i = 0;i < moduleList.size();i++) {
        FreeLibrary(moduleList[i]);
	}
	return 0;
}
