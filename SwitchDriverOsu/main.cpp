#include<iostream>
#include<string>
#include<windows.h>
#include<tlhelp32.h>
#include<tchar.h>


using namespace std;

//Dichiarazione funzioni
void BannerAnimation(string AnimationText, int x);
void ProcessController(string process[], int i);
void ServiceManagement(string szSvcName[],int j);
void ServiceStart(string szSvcName[],int j);
bool ProcessChecker(string nameProcess);


string process[] = {"Wacom_Tablet.exe", "Pen_Tablet.exe","WacomDesktopCenter.exe","Wacom_Tablet.exe","Pen_Tablet.exe"};
string szSvcName[] =  {"WTabletServicePro","WTabletServiceCon"};



int main()
{
        string AnimationText = "Switch Tablet Driver\n\n";
        int x=0;
        char ris;
        BannerAnimation(AnimationText,x);

        if(!ProcessChecker("Wacom_Tablet.exe"))
        {
            cout << "Driver wacom non rilevati. Vuoi utilizzarli? (y/n)"<< endl;
            cin >> ris;
            ris =  tolower(ris);
            if(ris == 'y'){
                for(int j=0;j<2;j++){
                        ServiceManagement(szSvcName,j);
                }
                
            }
            
        }  

        else {
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

bool ProcessChecker(string ProcessName){

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (!Process32First(snapshot, &entry)) {
            CloseHandle(snapshot);
            return false;
        }
        do {
            if (!strcmp(entry.szExeFile, ProcessName.c_str() )) {
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