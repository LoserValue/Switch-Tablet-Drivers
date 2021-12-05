#define  WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>

#ifndef DriverManagement
#define DriverManagement
using namespace std;

namespace Management{
    class Service;
    class Process;

class Service{
    public:
    BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege,BOOL bEnablePrivilege);
    void ServiceManagement(string szSvcName[],int j);
};

class Process{
    public:
    void ProcessController(string ProcessName);
    void ProcessStartup(string pathProcess);
    bool ProcessChecker(string Process);
};
}
#endif 