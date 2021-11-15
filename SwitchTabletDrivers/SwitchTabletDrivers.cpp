#include    <iostream>
#include    <string>
#define     WIN32_LEAN_AND_MEAN
#include    <windows.h>
#include    <tlhelp32.h>
#include    <filesystem>
#include    <fstream>
#include    <algorithm>
#include    "tinyxml2.h"

using namespace tinyxml2;
using namespace std;
namespace fs = std::filesystem;

//macro error check for xml files
#ifndef XMLCheckResult
	#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

//Function Declaration
void BannerAnimation(string AnimationText, int x);
void ProcessController(string ProcessName);
void ServiceManagement(string szSvcName[], int j);
void ServiceStart(string szSvcName[], int j);
bool ProcessChecker(string nameProcess);
void ProcessStartup(string pathProcess);


string process[5] = {"Wacom_Tablet.exe", "Pen_Tablet.exe", "WacomDesktopCenter.exe", "Wacom_Tablet.exe", "Pen_Tablet.exe"};
string szSvcName[2] = {"WTabletServicePro", "WTabletServiceCon"};

int main()
{
        string AnimationText = "Switch Tablet Driver\n\n";
        int x=0;
        string ris;
        string path;
        BannerAnimation(AnimationText,x);
    
        XMLDocument xmlDoc;

        if(!std::filesystem::exists("Data.xml")){
            //get the address of devocub drivers
            cout << "Enter the address of the drivers:" << endl;
            getline(cin,path);

            XMLNode * Root = xmlDoc.NewElement("Root");
            xmlDoc.InsertFirstChild(Root);

            XMLElement * pElement = xmlDoc.NewElement("AlredyRunned");
            pElement-> SetText(1);
            Root->InsertEndChild(pElement);

            pElement = xmlDoc.NewElement("Path");
            pElement-> SetText(path.c_str());
            Root->InsertEndChild(pElement);
            XMLError eResult = xmlDoc.SaveFile("Data.xml");
        }
        else{

            xmlDoc.LoadFile("Data.xml");
            
            XMLText *pElement = xmlDoc.FirstChildElement("Root")->FirstChildElement("AlredyRunned")->FirstChild()->ToText();
            if (pElement == nullptr) cout << XML_ERROR_FILE_READ_ERROR;
            string iOutInt = pElement->Value();

            pElement = xmlDoc.FirstChildElement("Root")->FirstChildElement("Path")->FirstChild()->ToText();
            if (pElement == nullptr) cout << XML_ERROR_FILE_READ_ERROR;
            path = pElement->Value();       
        }

        if(!ProcessChecker("Wacom_Tablet.exe"))
        {
            cout << "Wacom driver not detected. Do you want to use them?(y/n)"<< endl;
            cin >> ris;
            transform(ris.begin(), ris.end(), ris.begin(), ::tolower);

            if(ris == "y"){
                //Gets the "filename.exe" contained in the path
                string base_filename = path.substr(path.find_last_of("/\\") + 1);
                if(ProcessChecker(base_filename)){
                        ProcessController(base_filename);  
                }
                for(int j=0;j<2;j++){
                        ServiceManagement(szSvcName,j);
                }

            }
        }  
        else{
            cout << "Wacom drivers detected. Start of closing procedure..."<< endl;

            for(int i=0;i<5;i++){
                ProcessController(process[i]);
                if (i==1){
                    for(int j=0;j<2;j++){
                            ServiceManagement(szSvcName,j);    
                    }
                }
                else if(i==2){Sleep(1000);}
            }
            ProcessStartup(path);

        }        
        return 0;
}
//banner animation
void BannerAnimation(string AnimationText, int x)
{
    while(AnimationText[x] != '\0')
    {
        cout << AnimationText[x];
        Sleep(10);
        x++;
    }
}

bool ProcessChecker(string Process){

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

void ProcessStartup(string pathProcess){
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


void ProcessController(string ProcessName) {
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


void ServiceManagement(string szSvcName[],int j){
    
        SERVICE_STATUS Status;

        SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        SC_HANDLE SHandle = OpenService(SCManager, szSvcName[j].c_str(), SC_MANAGER_ALL_ACCESS);

        if(SHandle == NULL)
        {
            cout <<"|" << "ERROR " << GetLastError() << endl;
            cout << "Administrator rights required.";
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