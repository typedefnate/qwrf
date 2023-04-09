#include "config.h"
#include "requestobjects.h"
#include "specialrequestobjects.h"
#include "uierrorcheck.h"

int main(int argc, char** argv) {

    UI_Error_Check* ui_error_check = new UI_Error_Check(argc, argv);
    delete ui_error_check;

    Request* request;

    if (strcmp(argv[3], "HTTP") == 0 || strcmp(argv[3], "http") == 0) {
        request = new HTTP_Request(argv[1], atoi(argv[2]));
    } else if (strcmp(argv[3], "HTTPS") == 0 || strcmp(argv[3], "https") == 0) {
        request = new HTTPS_Request(argv[1], atoi(argv[2]));
    } else {
        std::cout << "Check " << argv[3] << std::endl;
        std::cout << "Try '" << argv[0] << " -h' for more information"
                  << std::endl;
        return 0;
    }

    request->ResolveDNS(argv[1]);
    request->SpawnFile();
    request->EditFile();
    request->SaveFile();
    request->DeleteFile();
    request->InitSocket();
    request->Connect();
    request->Send();
    request->Receive();
    request->Close();

    delete request;
    return 0;
}
