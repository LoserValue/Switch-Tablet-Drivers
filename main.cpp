    #include<iostream>
    #include<string>
    #include<windows.h>
    #include<tlhelp32.h>
    #include<tchar.h>


    using namespace std;

    //Dichiarazione funzioni
    void __stdcall DoStartSvc(const string szSvcName[],int j);
    VOID __stdcall DoStopSvc(const string szSvcName[],int j);

    void BannerAnimation(string AnimationText, int x);
    bool ProcessController(const string process[], int i);
    SC_HANDLE schSCManager;
    SC_HANDLE schService;


    int main()
    {
    string AnimationText = "Switch Tablet Driver\n\n";
    int x = 0;

    const string process[5] = {"Wacom_Tablet.exe", "Pen_Tablet.exe","WacomDesktopCenter.exe","Wacom_Tablet.exe","Pen_Tablet.exe"};
    const string szSvcName[2] = {"WTabletServicePro","WTabletServiceCon"};
    BannerAnimation(AnimationText,x);
    int j;


    for(int i=0;i<5;i++){
        ProcessController(process,i);
        if (i==1){

            j=0;
            DoStartSvc(szSvcName,j);
            DoStopSvc(szSvcName,j);

            j=1;
            DoStopSvc(szSvcName,j);
            DoStartSvc(szSvcName,j);
        }
        if(i==2)
            Sleep(1000);
    }
    return 0;
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


    void __stdcall DoStartSvc(const string szSvcName[],int j)
    {
        SERVICE_STATUS_PROCESS ssStatus; 
        DWORD dwOldCheckPoint; 
        DWORD dwStartTickCount;
        DWORD dwWaitTime;
        DWORD dwBytesNeeded;
        DWORD dwStartTime = GetTickCount();
        DWORD dwTimeout = 30000;

        // Get a handle to the SCM database. 
    
        schSCManager = OpenSCManager( NULL,NULL,SC_MANAGER_ALL_ACCESS); 
    
        if (NULL == schSCManager) 
        {
            printf("OpenSCManager failed (%d)\n", GetLastError());
            return;
        }

        // Get a handle to the service.

        schService = OpenService( schSCManager,szSvcName[j].c_str(),SERVICE_ALL_ACCESS|SERVICE_STOP|SERVICE_QUERY_STATUS); 
    
        if (schService == NULL)
        { 
            printf("OpenService failed (%d)\n", GetLastError()); 
            CloseServiceHandle(schSCManager);
            return;
        }    

        // Check the status in case the service is not stopped. 

        if (!QueryServiceStatusEx( 
                schService,                     // handle to service 
                SC_STATUS_PROCESS_INFO,         // information level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) )              // size needed if buffer is too small
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            goto stop_cleanup;
            return; 
        }

        // Check if the service is already running. It would be possible 
        // to stop the service here, but for simplicity this example just returns. 

        if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
        {
            cout <<"ERROR "<< szSvcName[j]<< ": impossibile avviare il seguente servizio perchè già in esecuzione "<< endl;  
            DoStopSvc(szSvcName,j);
    
            goto stop_cleanup;
            return;

        }
        //dsds
        // Save the tick count and initial checkpoint.

        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = ssStatus.dwCheckPoint;

        // Wait for the service to stop before attempting to start it.

        while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
        {
            // Do not wait longer than the wait hint. A good interval is 
            // one-tenth of the wait hint but not less than 1 second  
            // and not more than 10 seconds. 
    
            dwWaitTime = ssStatus.dwWaitHint / 10;

            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            // Check the status until the service is no longer stop pending. 
    
            if (!QueryServiceStatusEx( 
                    schService,                     // handle to service 
                    SC_STATUS_PROCESS_INFO,         // information level
                    (LPBYTE) &ssStatus,             // address of structure
                    sizeof(SERVICE_STATUS_PROCESS), // size of structure
                    &dwBytesNeeded ) )              // size needed if buffer is too small
            {
                printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
                goto stop_cleanup;
                return; 
            }

            if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
            {
                // Continue to wait and check.

                dwStartTickCount = GetTickCount();
                dwOldCheckPoint = ssStatus.dwCheckPoint;
            }
            else
            {
                if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
                {
                    printf("Timeout waiting for service to stop\n");
                    goto stop_cleanup;
                    return; 
                }
            }
        }

        // Attempt to start the service.

        if (!StartService(
                schService,  // handle to service 
                0,           // number of arguments 
                NULL) )      // no arguments 
        {
            printf("StartService failed (%d)\n", GetLastError());
            goto stop_cleanup;
            return; 
        }
        else printf("Service start pending...\n"); 

        // Check the status until the service is no longer start pending. 
    
        if (!QueryServiceStatusEx( 
                schService,                     // handle to service 
                SC_STATUS_PROCESS_INFO,         // info level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) )              // if buffer too small
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
            goto stop_cleanup;
            return; 
        }
    
        // Save the tick count and initial checkpoint.

        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = ssStatus.dwCheckPoint;

        while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
        { 
            // Do not wait longer than the wait hint. A good interval is 
            // one-tenth the wait hint, but no less than 1 second and no 
            // more than 10 seconds. 
    
            dwWaitTime = ssStatus.dwWaitHint / 10;

            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            // Check the status again. 
    
            if (!QueryServiceStatusEx( 
                schService,             // handle to service 
                SC_STATUS_PROCESS_INFO, // info level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) )              // if buffer too small
            {
                printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
                break; 
            }
    
            if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
            {
                // Continue to wait and check.

                dwStartTickCount = GetTickCount();
                dwOldCheckPoint = ssStatus.dwCheckPoint;
            }
            else
            {
                if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
                {
                    // No progress made within the wait hint.
                    break;
                }
            }
        } 

        // Determine whether the service is running.

        if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
        {
            printf("Service started successfully.\n"); 
        }
        else 
        { 
            printf("Service not started. \n");
            printf("  Current State: %d\n", ssStatus.dwCurrentState); 
            printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
            printf("  Check Point: %d\n", ssStatus.dwCheckPoint); 
            printf("  Wait Hint: %d\n", ssStatus.dwWaitHint); 
        } 

        goto stop_cleanup;

        stop_cleanup:
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);   
    }




    VOID __stdcall DoStopSvc(const string szSvcName[],int j)
    {
        SERVICE_STATUS_PROCESS ssStatus;
        DWORD dwStartTime = GetTickCount();
        DWORD dwBytesNeeded;
        DWORD dwTimeout = 30000; // 30-second time-out
        DWORD dwWaitTime;

        // Get a handle to the SCM database. 
    
        schSCManager = OpenSCManager( 
            NULL,                    // local computer
            NULL,                    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS);  // full access rights 
    
        if (NULL == schSCManager) 
        {
            printf("OpenSCManager failed (%d)\n", GetLastError());
            return;
        }

        // Get a handle to the service.

        schService = OpenService( 
            schSCManager,         // SCM database 
            szSvcName[j].c_str(),            // name of service 
            SERVICE_STOP | 
            SERVICE_QUERY_STATUS | 
            SERVICE_ENUMERATE_DEPENDENTS);  
    
        if (schService == NULL)
        { 
            printf("OpenService failed (%d)\n", GetLastError()); 
            CloseServiceHandle(schSCManager);
            return;
        }    

        // Make sure the service is not already stopped.

        if ( !QueryServiceStatusEx( 
                schService, 
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&ssStatus, 
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded ) )
        {
            printf("QueryServiceStatusEx failed (%d)\n", GetLastError()); 
            goto stop_cleanup;
        }

        if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
        {
            printf("Service is already stopped.\n");
            goto stop_cleanup;
        }

        // If a stop is pending, wait for it.

        while ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING ) 
        {
            printf("Service stop pending...\n");

            // Do not wait longer than the wait hint. A good interval is 
            // one-tenth of the wait hint but not less than 1 second  
            // and not more than 10 seconds. 
    
            dwWaitTime = ssStatus.dwWaitHint / 10;

            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

            if ( !QueryServiceStatusEx( 
                    schService, 
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ssStatus, 
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded ) )
            {
                printf("QueryServiceStatusEx failed (%d)\n", GetLastError()); 
                goto stop_cleanup;
            }

            if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
            {
                printf("Service stopped successfully.\n");
                goto stop_cleanup;
            }

            if ( GetTickCount() - dwStartTime > dwTimeout )
            {
                printf("Service stop timed out.\n");
                goto stop_cleanup;
            }
        }

        // If the service is running, dependencies must be stopped first.

        /* StopDependentServices(); */

        // Send a stop code to the service.

        if ( !ControlService( 
                schService, 
                SERVICE_CONTROL_STOP, 
                (LPSERVICE_STATUS) &ssStatus ) )
        {
            printf( "ControlService failed (%d)\n", GetLastError() );
            goto stop_cleanup;
        }

        // Wait for the service to stop.

        while ( ssStatus.dwCurrentState != SERVICE_STOPPED ) 
        {
            Sleep( ssStatus.dwWaitHint );
            if ( !QueryServiceStatusEx( 
                    schService, 
                    SC_STATUS_PROCESS_INFO,
                    (LPBYTE)&ssStatus, 
                    sizeof(SERVICE_STATUS_PROCESS),
                    &dwBytesNeeded ) )
            {
                printf( "QueryServiceStatusEx failed (%d)\n", GetLastError() );
                goto stop_cleanup;
            }

            if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                break;

            if ( GetTickCount() - dwStartTime > dwTimeout )
            {
                printf( "Wait timed out\n" );
                goto stop_cleanup;
            }
        }
        printf("Service stopped successfully\n");



        stop_cleanup:
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);

    }

    /* BOOL __stdcall StopDependentServices()
    {
        DWORD i;
        DWORD dwBytesNeeded;
        DWORD dwCount;

        LPENUM_SERVICE_STATUS   lpDependencies = NULL;
        ENUM_SERVICE_STATUS     ess;
        SC_HANDLE               hDepService;
        SERVICE_STATUS_PROCESS  ssStatus;

        DWORD dwStartTime = GetTickCount();
        DWORD dwTimeout = 30000; // 30-second time-out

        // Pass a zero-length buffer to get the required buffer size.
        if ( EnumDependentServices( schService, SERVICE_ACTIVE, 
            lpDependencies, 0, &dwBytesNeeded, &dwCount ) ) 
        {
            // If the Enum call succeeds, then there are no dependent
            // services, so do nothing.
            return TRUE;
        } 
        else 
        {
            if ( GetLastError() != ERROR_MORE_DATA )
                return FALSE; // Unexpected error

            // Allocate a buffer for the dependencies.
            lpDependencies = (LPENUM_SERVICE_STATUS) HeapAlloc( 
                GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded );
    
            if ( !lpDependencies )
                return FALSE;

            __try {
                // Enumerate the dependencies.
                if ( !EnumDependentServices( schService, SERVICE_ACTIVE, 
                    lpDependencies, dwBytesNeeded, &dwBytesNeeded,
                    &dwCount ) )
                return FALSE;

                for ( i = 0; i < dwCount; i++ ) 
                {
                    ess = *(lpDependencies + i);
                    // Open the service.
                    hDepService = OpenService( schSCManager, 
                    ess.lpServiceName, 
                    SERVICE_STOP | SERVICE_QUERY_STATUS );

                    if ( !hDepService )
                    return FALSE;

                    __try {
                        // Send a stop code.
                        if ( !ControlService( hDepService, 
                                SERVICE_CONTROL_STOP,
                                (LPSERVICE_STATUS) &ssStatus ) )
                        return FALSE;

                        // Wait for the service to stop.
                        while ( ssStatus.dwCurrentState != SERVICE_STOPPED ) 
                        {
                            Sleep( ssStatus.dwWaitHint );
                            if ( !QueryServiceStatusEx( 
                                    hDepService, 
                                    SC_STATUS_PROCESS_INFO,
                                    (LPBYTE)&ssStatus, 
                                    sizeof(SERVICE_STATUS_PROCESS),
                                    &dwBytesNeeded ) )
                            return FALSE;

                            if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
                                break;

                            if ( GetTickCount() - dwStartTime > dwTimeout )
                                return FALSE;
                        }
                    } 
                    __finally
                    {
                        // Always release the service handle.
                        CloseServiceHandle( hDepService );
                    }
                }
            } 

            {
                // Always free the enumeration buffer.
                HeapFree( GetProcessHeap(), 0, lpDependencies );
            }
        }
        
        return TRUE;
    } */
