#include "config.h"

class UI_Error_Check {
public:
    int argc;
    char** argv;
    UI_Error_Check(int arg_c, char** arg_v) {
        argc = arg_c;
        argv = arg_v;
        if (argc == 2 && strcmp(argv[1], "-h") == 0) {
            std::cout
                << argv[0]
                << " - quick web resource fetcher\n\nSend requests to servers."
                   "  Automatic domain name resolutions.\n\nSupported request "
                   "types:\n\tHTTP/http\n\t\t"
                   "Usage: "
                << argv[0]
                << " [HOST] [PORT] [REQUEST TYPE]\n\t\tExample: " << argv[0]
                << " www.example.com 80 http\n\t\tCommon ports: 80, 8080"
                   "\n\tHTTPS/https\n\t\t"
                   "Usage: "
                << argv[0]
                << " [HOST] [PORT] [REQUEST TYPE]\n\t\tExample: " << argv[0]
                << " httpbin.org 443 https\n\t\tCommon ports: 443, 4443, 8443"
                << std::endl
                << std::endl;
            exit(0);
        }
        if (argc != 4) {
            std::cout << "Try '" << argv[0] << " -h' for more information."
                      << std::endl;
            exit(0);
        }
    }
};
