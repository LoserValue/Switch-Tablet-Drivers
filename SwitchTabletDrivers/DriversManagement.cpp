#include    "DriversManagement.h"
#include    <tlhelp32.h>
#include    <filesystem>
#include    <fstream>
#include    <algorithm>


using namespace Management;

BOOL Management::Service::SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
    ) 
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if ( !LookupPrivilegeValue( 
            NULL,            // lookup privilege on local system
            lpszPrivilege,   // privilege to lookup 
            &luid ) )        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError() ); 
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if ( !AdjustTokenPrivileges(
           hToken, 
           FALSE, 
           &tp, 
           sizeof(TOKEN_PRIVILEGES), 
           (PTOKEN_PRIVILEGES) NULL, 
           (PDWORD) NULL) )
    { 
          printf("AdjustTokenPrivileges error: %u\n", GetLastError() ); 
          return FALSE; 
    } 

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {

          printf("The process does not have the required privileges. \n");
          return FALSE;
    } 

    return TRUE;
}

void Service::ServiceManagement(string szSvcName[],int j){
    
        SERVICE_STATUS Status;

        SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        SC_HANDLE SHandle = OpenService(SCManager, szSvcName[j].c_str(), SC_MANAGER_ALL_ACCESS);

        if(SHandle == NULL)
        {
            cout <<"|" << "ERROR " << GetLastError() << endl;
        }
        
        else if(!ControlService(SHandle, SERVICE_CONTROL_STOP, &Status))
        {
            cout <<"|" << "FAILED TO SEND STOP SERVICE COMMAND: " << GetLastError()<<endl;
        }
        else
        {
            cout <<"|" << "Service stopped successfully"<<endl;
        }
 
        do
        {
            QueryServiceStatus(SHandle, &Status);
            //cout <<"|" << "Checking Service Status...\n";
        }while(Status.dwCurrentState != SERVICE_STOPPED);   

        if(!StartService(SHandle, 0, NULL))
        {
            cout <<"|" << "Service Did Not Start Up: " << GetLastError() << endl;
        }
        else
        {
            cout <<"|" << "Service Started successfully"<<endl;
        }
        
        CloseServiceHandle(SCManager);
        CloseServiceHandle(SHandle);
}

void Process::ProcessController(string ProcessName) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (!Process32First(snapshot, &entry)) {
            CloseHandle(snapshot);
        }
        
        do {
            if (!strcmp(entry.szExeFile, ProcessName.c_str() )) {
                
                cout <<endl<<"|" <<ProcessName<< " is running"<< endl;
                //chiusura processo
                cout<<"|Closing..."<<endl<<endl;
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,entry.th32ProcessID); 

                TerminateProcess(hProcess,0);
                CloseHandle(hProcess);
            }
        } while (Process32Next(snapshot, &entry));

        CloseHandle(snapshot);

}

void Process::ProcessStartup(string pathProcess){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    LPSTR CPath = const_cast<char *>(pathProcess.c_str());

    if( !CreateProcessA( NULL,   // No module name (use command line)
        CPath,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ){
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return;
    }  

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

bool Process::ProcessChecker(string Process){

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (!Process32First(snapshot, &entry)) {
            CloseHandle(snapshot);
            return false;
        }
        do {
            if (!strcmp(entry.szExeFile, Process.c_str() )) {
                    CloseHandle(snapshot);
                    return true;
            }
        } while (Process32Next(snapshot, &entry));
        
        CloseHandle(snapshot);
        return false;
}