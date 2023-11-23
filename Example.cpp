#include "ArmDetour.h"
#include <cstring>
#include <iostream>
#include <thread>

int findaddrinmaps(char *libname) {
    char buffer[1024];
    FILE *opendfile = fopen( "/proc/self/maps", "rt");
    int retur = 0;

    while (fgets(buffer, sizeof(buffer), opendfile)) {
        if (strstr(buffer, libname)) {
            retur = (int) strtoul(buffer, NULL, 16);
            break;
        }
    }

    
    return retur;
}

bool radarhackenabled=true;

bool(*oldradar)(int player);
bool radarhack(int player){
    bool toret = oldradar(player);
    if(radarhackenabled)toret = true;
    return toret;
}

void thread_hack(){
 for(;findaddrinmaps("libil2cpp.so")==NULL;sleep(1));
 
 oldradar=(bool(*)(int))ArmDetour(findaddrinmaps("libil2cpp.so")+0xDAEAF0,(uintptr_t)radarhack);
}

int mainlib(){
   std::thread(thread_hack).detach();
   return 0;
}

static int djjdjd= mainlib();