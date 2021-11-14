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

#ifndef XMLCheckResult
	#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

//Dichiarazione funzioni
void BannerAnimation(string AnimationText, int x);
void ProcessController(string process[], int i);
void ServiceManagement(string szSvcName[], int j);
void ServiceStart(string szSvcName[], int j);
bool ProcessChecker(string nameProcess);

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
            
            cout << "Inserire l'indirizzo dei driver:" << endl;
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
            cout << "Driver wacom non rilevati. Vuoi utilizzarli? (y/n)"<< endl;
            cin >> ris;
            transform(ris.begin(), ris.end(), ris.begin(), ::tolower);
            if(ris == "y"){
                for(int j=0;j<2;j++){
                        ServiceManagement(szSvcName,j);
                }
            }
        }  
        else{
            cout << "Driver wacom rilevati. Inizio procedura di chiusura..."<< endl;
            for(int i=0;i<5;i++){
                    ProcessController(process,i);
                    if (i==1){
                            for(int j=0;j<2;j++){
                                    ServiceManagement(szSvcName,j);    
                            }
                    }
                else if(i==2){Sleep(1000);}
            } 
        }        
        return 0;
}
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
void ProcessController(string process[], int i) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (!Process32First(snapshot, &entry)) {
            CloseHandle(snapshot);
        }
        
        do {
            if (!strcmp(entry.szExeFile, process[i].c_str() )) {
                
                cout <<endl<<"|" <<process[i]<< " in esecuzione"<< endl;
                //chiusura processo
                cout<<"|Chiusura in corso..."<<endl<<endl;
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