    #include<iostream>
    #include<string>
    #include<windows.h>
    #include<tlhelp32.h>
    #include<tchar.h>


    using namespace std;

    //Dichiarazione funzioni
    void __stdcall DoStartSvc(const string szSvcName[],int j);
    VOID __stdcall DoStopSvc(const string szSvcName[],int j);

    void StartService();
    void BannerAnimation(string AnimationText, int x);
    bool ProcessController(const string process[], int i);
    const string process[5] = {"Wacom_Tablet.exe", "Pen_Tablet.exe","WacomDesktopCenter.exe","Wacom_Tablet.exe","Pen_Tablet.exe"};
    const string szSvcName[2] = {"WTabletServicePro","WTabletServiceCon"};
    int j;
    SC_HANDLE SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    SC_HANDLE SHandle = OpenService(SCManager, szSvcName[j].c_str(), SC_MANAGER_ALL_ACCESS);



    int main()
    {

    string AnimationText = "Switch Tablet Driver\n\n";
    int x = 0;

    BannerAnimation(AnimationText,x);

    for(int i=0;i<5;i++){
        ProcessController(process,i);
        if (i==1){
            j=1;
            StartService();    
        }
    }
    }

    void BannerAnimation(string AnimationText, int x)
    {
    while ( AnimationText[x] != '\0')
    {
        cout << AnimationText[x];
        Sleep(30);
        x++;
    };
    }

    bool ProcessController(const string process[], int i) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (!Process32First(snapshot, &entry)) {
            CloseHandle(snapshot);
            return false;
        }
        
        do {
            if (!strcmp(entry.szExeFile, process[i].c_str() )) {
                cout <<"|" <<process[i]<< " in esecuzione"<< endl;
                //chiusura processo
                cout<<"|Chiusura in corso..."<<endl<<endl;
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_TERMINATE,FALSE,entry.th32ProcessID); 
                TerminateProcess(hProcess,0);
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32Next(snapshot, &entry));

        CloseHandle(snapshot);

        return false;


    }

    void StartService(){
        SERVICE_STATUS Status;
        if(SHandle == NULL)
        {
            std::cout << "ERROR " << GetLastError() << std::endl;
        }
        
        if(!ControlService(SHandle, SERVICE_CONTROL_STOP, &Status))
        {
            std::cout << "FAILED TO SEND STOP SERVICE COMMAND: " << GetLastError();
        }
        else
        {
            std::cout << "Service Stop Command Sent\n";
        }
        
        do
        {
            QueryServiceStatus(SHandle, &Status);
            std::cout << "Checking Service Status...\n";
        }while(Status.dwCurrentState != SERVICE_STOPPED);   

        if(!StartService(SHandle, 0, NULL))
        {
            std::cout << "Service Did Not Start Up Again: " << GetLastError() << std::endl;
        }
        else
        {
            std::cout << "Service Started\n";
        }
        
        cin.get();
        
        CloseServiceHandle(SCManager);
        CloseServiceHandle(SHandle);
       
        }