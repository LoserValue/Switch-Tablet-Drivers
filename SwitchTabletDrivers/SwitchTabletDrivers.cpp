#include    "DriversManagement.h"
#include    "tinyxml2.h"
#include    <filesystem>

using namespace tinyxml2;
using namespace std;
using namespace Management;
namespace fs = std::filesystem;

//macro error check for xml files
#ifndef XMLCheckResult
	#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

//Function Declaration
void BannerAnimation(string AnimationText, int x);

string process[5] = {"Wacom_Tablet.exe", "Pen_Tablet.exe", "WacomDesktopCenter.exe", "Wacom_Tablet.exe", "Pen_Tablet.exe"};
string szSvcName[2] = {"WTabletServicePro", "WTabletServiceCon"};

int main()
{  
        HANDLE hToken;
        Service service;
        Process processObj;

        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)){printf("OpenProcessToken() error %u\n", GetLastError());}
        LPCTSTR lpszPrivilege = "SeSecurityPrivilege";
        if(!service.SetPrivilege(hToken,lpszPrivilege,TRUE)) {Sleep(2000); return 1;}

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
            xmlDoc.SaveFile("Data.xml");
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

        if(!processObj.ProcessChecker("Wacom_Tablet.exe"))
        {
            cout << "Wacom driver not detected. Do you want to use them?(y/n)"<< endl;
            cin >> ris;
            transform(ris.begin(), ris.end(), ris.begin(), ::tolower);

            if(ris == "y"){
                //Gets the "filename.exe" contained in the path
                string base_filename = path.substr(path.find_last_of("/\\") + 1);
                if(processObj.ProcessChecker(base_filename)){
                        processObj.ProcessController(base_filename);  
                }
                for(int j=0;j<2;j++){
                        service.ServiceManagement(szSvcName,j);
                }

            }
        }  
        else{
            cout << "Wacom drivers detected. Start of closing procedure..."<< endl;

            for(int i=0;i<5;i++){
                processObj.ProcessController(process[i]);
                if (i==1){
                    for(int j=0;j<2;j++){
                            service.ServiceManagement(szSvcName,j);    
                    }
                }
                else if(i==2){Sleep(1000);}
            }
            processObj.ProcessStartup(path);

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
