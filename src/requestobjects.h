#include "config.h"

class Request {
public:
    enum RequestType { DEFAULT, HTTP, HTTPS };
    struct {
        std::string blueprint = "DEFAULT BLUEPRINT";
        RequestType request_type = DEFAULT;
        size_t port = 0;
        const char *host = NULL;
        signed int socket_handle = -9;
        long int request_size = 0;
        char request_string[4096];
    } ConnectionData;
    virtual void ResolveDNS(char *hostname) {
        struct hostent *he;
        struct in_addr **addr_list;
        if ((he = gethostbyname(hostname)) == NULL) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
        addr_list = (struct in_addr **)he->h_addr_list;
        char ip[100];
        for (int i = 0; addr_list[i] != NULL; i++) {
            strcpy(ip, inet_ntoa(*addr_list[i]));
        }
        if ((strcpy(hostname, ip)) == NULL) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
    }
    virtual void SpawnFile() {
        std::ofstream fHeader("6fa308f71c48414410dab60370771ebe73b3ac2a");
        if (!fHeader.is_open()) {
            fHeader.close();
            ErrorLeave(__PRETTY_FUNCTION__);
        }
        fHeader << ConnectionData.blueprint;
        fHeader.close();
    }
    virtual void EditFile() {
        system("vi 6fa308f71c48414410dab60370771ebe73b3ac2a");
    }
    virtual void SaveFile() {
        FILE *file = fopen("6fa308f71c48414410dab60370771ebe73b3ac2a", "r");
        if (!file) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
        fseek(file, 0L, SEEK_END);
        ConnectionData.request_size = ftell(file);
        rewind(file);
        if (fread(ConnectionData.request_string, ConnectionData.request_size, 1,
                  file) > 1) {
            fclose(file);
            ErrorLeave(__PRETTY_FUNCTION__);
        } else {
            fclose(file);
        }
    }
    virtual void DeleteFile() {
        if (remove("6fa308f71c48414410dab60370771ebe73b3ac2a") != 0) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
    }
    virtual void InitSocket() {}
    virtual void Connect() {}
    virtual void Send() {}
    virtual void Receive() {}
    virtual void ResponseParsing(char *response, int response_size) {}
    virtual void Close() {}
    void ErrorLeave(const char *function_name) {
        std::cout << "Error, check userinput." << std::endl
                  << "struct = { "
                  << "\n\tenum\t\t.request_type = "
                  << ConnectionData.request_type
                  << "\n\tsize_t\t\t.port = " << ConnectionData.port
                  << "\n\tchar*\t\t.host = " << ConnectionData.host
                  << "\n\tsigned int\t.socket_handle = "
                  << ConnectionData.socket_handle
                  << "\n\tlong int\t.request_size (bytes) = "
                  << ConnectionData.request_size << "} ConnectionData; "
                  << std::endl;
        std::cerr << function_name << " failed.\t\terrno(" << strerror(errno)
                  << ")\n";
        exit(0);
    }
    virtual ~Request() {}

protected:
    //"specialrequestobjects.h" tend to inherit from here.
};

class HTTP_Request : public Request {
public:
    void InitSocket() {
        ConnectionData.socket_handle = socket(AF_INET, SOCK_STREAM, 0);
        if (ConnectionData.socket_handle < 0) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
    }
    void Connect() {
        struct sockaddr_in server;
        server.sin_addr.s_addr = inet_addr(ConnectionData.host);
        server.sin_family = AF_INET;
        server.sin_port = htons(ConnectionData.port);
        std::cout << "Trying " << ConnectionData.host << "... " << std::endl;
        if (connect(ConnectionData.socket_handle, (struct sockaddr *)&server,
                    sizeof(server)) != 0) {
            close(ConnectionData.socket_handle);
            ErrorLeave(__PRETTY_FUNCTION__);
        } else {
            std::cout << "Connected to " << ConnectionData.host << "."
                      << std::endl;
        }
    }
    void Send() {
        if (send(ConnectionData.socket_handle, ConnectionData.request_string,
                 ConnectionData.request_size, 0) < 0) {
            close(ConnectionData.socket_handle);
            ErrorLeave(__PRETTY_FUNCTION__);
        }
    }
    void Receive() {
        std::cout << "Receiving response..." << std::endl << std::endl;
        char *server_reply = (char *)calloc(65535, sizeof(char));
        char *header = server_reply;
        signed int receive = 1;
        int retry = 0;
        while (receive > 0 || retry < 2) {
            sleep(2);
            receive =
                recv(ConnectionData.socket_handle, server_reply, 65535, 0);
            if (receive == 0) {
                ++retry;
            }
        }
        ResponseParsing(server_reply, 65535);
        std::cout << header << std::endl;
        free(server_reply);
    }
    void ResponseParsing(char *response, int response_size) {
        for (int i = 0; i < response_size; i++) {
            if ((response[i] == '\r') && (response[i + 1] == '\n') &&
                (response[i + 2] == '\r') && (response[i + 3] == '\n')) {
                response[i + 3] = 0;
                break;
            }
        }
    }
    void Close() {
        if (close(ConnectionData.socket_handle) != 0) {
            ErrorLeave(__PRETTY_FUNCTION__);
        } else {
            std::cout << "Socket closed." << std::endl;
        }
    }
    HTTP_Request(char *host, size_t port) {
        ConnectionData = {
            .blueprint = "GET / HTTP/1.1\r\nHost: \r\n"
                         "User-Agent: yourfriend\r\nAccept: */*\r\nConnection:"
                         " close\r\n\r\n",
            .request_type = HTTP,
            .port = port,
            .host = host};
    }
};

class HTTPS_Request : public Request {
public:
    struct {
        const SSL_METHOD *method = SSLv23_client_method();
        SSL_CTX *ctx = SSL_CTX_new(method);
        BIO *bio = BIO_new_ssl_connect(ctx);
        SSL *ssl = NULL;
    } SSLData;
    void InitSocket() {
        std::cout << "Trying " << ConnectionData.host << "... " << std::endl;
        char hostname[2048];
        char portstring[500];
        snprintf(portstring, 500, "%zu", ConnectionData.port);
        sprintf(hostname, "%s:%s", ConnectionData.host, portstring);
        BIO_set_conn_hostname(SSLData.bio, hostname);
        if (BIO_get_ssl(SSLData.bio, &SSLData.ssl) != 1) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
    }
    void Connect() {
        if (BIO_do_connect(SSLData.bio) != 1) {
            SSL_CTX_free(SSLData.ctx);
            BIO_free_all(SSLData.bio);
            ErrorLeave(__PRETTY_FUNCTION__);
        }
        if (!SSL_CTX_load_verify_locations(SSLData.ctx,
                                           "/etc/ssl/certs/ca-certificates.crt",
                                           "/etc/ssl/certs/")) {
            ErrorLeave(__PRETTY_FUNCTION__);
        }
        long verify_flag = SSL_get_verify_result(SSLData.ssl);
        if (verify_flag != X509_V_OK) {
            std::cout
                << "Certificate verification code: " << verify_flag
                << ".  Continuing..." << std::endl
                << "Read here for more: https://linux.die.net/man/1/verify"
                << std::endl;
        }
        std::cout << "Connected to " << ConnectionData.host << ".  ";
        std::cout << "Cipher: " << SSL_get_cipher(SSLData.ssl) << "."
                  << std::endl;
    }
    void Send() { BIO_puts(SSLData.bio, ConnectionData.request_string); }
    void Receive() {
        std::cout << "Receiving response..." << std::endl << std::endl;
        char *server_reply = (char *)calloc(65535, sizeof(char));
        BIO_read(SSLData.bio, server_reply, 65535);
        ResponseParsing(server_reply, 65535);
        std::cout << server_reply << std::endl;
        while (true) {
            sleep(1);
            int n = BIO_read(SSLData.bio, server_reply, 65535);
            if (n <= 0) {
                break;
            }
        }
        free(server_reply);
    }
    void ResponseParsing(char *response, int response_size) {
        for (int i = 0; i < response_size; i++) {
            if ((response[i] == '\r') && (response[i + 1] == '\n') &&
                (response[i + 2] == '\r') && (response[i + 3] == '\n')) {
                response[i + 3] = '\0';
                break;
            }
        }
    }
    void Close() {
        SSL_CTX_free(SSLData.ctx);
        BIO_free_all(SSLData.bio);
        std::cout << "Socket closed." << std::endl;
    }
    HTTPS_Request(char *host, size_t port) {
        ConnectionData = {
            .blueprint = "GET / HTTP/1.1\r\nHost: \r\n"
                         "User-Agent: yourfriend\r\nAccept: */*\r\nConnection:"
                         " close\r\n\r\n",
            .request_type = HTTPS,
            .port = port,
            .host = host};
        SSL_library_init();
        SSL_load_error_strings();
    }
};
