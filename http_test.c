#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "response.h"

#include "http.h"

void test_get_request_uri(const char *request_buffer);

int main(void) {
    const char req1_dir[] = "GET /home/k/compsys HTTP/1.0\r\n";
    const char req2_file[] = "GET /home/k/compsys/comp30023-2022-project-2/.clang-format HTTP/1.0\r\n";
    const char req3_file_http11[] = "GET /home/k/compsys/comp30023-2022-project-2/.clang-format HTTP/1.1\r\n";
    const char req4_not_crlf[] = "GET /home/k/compsys/comp30023-2022-project-2/.clang-format HTTP/1.0\rabc\n";
    const char req5_wrong_method[] = "POST /home/k/compsys/comp30023-2022-project-2/.clang-format HTTP/1.0\r\n";
    const char req6_no_http[] = "POST /home/k/compsys/comp30023-2022-project-2/.clang-format\r\n";
    const char req7_just_crlf[] = "\r\n";
    const char req8_dir_trailing_slash[] = "GET /home/k/compsys/ HTTP/1.0\r\n";
    const char req9_garbage[] = "23890759290fd42398723894789237489237  adsdasd \r asds489327898";
    const char req10_garbage_pcsign[] = "23890759290fd42398%d%d723894789237489237  adsdasd \r asds489327898";
    const char req11_reversed[] = "HTTP/1.0 /home/k/compsys/comp30023-2022-project-2/.clang-format GET\r\n";
    const char req12_reversed_twist[] = "\r\nHTTP/1.0 /home/k/compsys/comp30023-2022-project-2/.clang-format GET";

    // const char *get_uri_test[] = {
    //     req1_dir,       req2_file,           req3_file_http11,        req4_not_crlf, req5_wrong_method,
    //     req6_no_http,   req7_just_crlf,      req8_dir_trailing_slash, req9_garbage,  req10_garbage_pcsign,
    //     req11_reversed, req12_reversed_twist};

    // for (int i = 0; i < 12; i++) {
    //     if (i == 9)
    //         continue;
    //     test_get_request_uri(get_uri_test[i]);
    // }

    test_get_request_uri(req8_dir_trailing_slash);

    return 0;
}

void test_get_request_uri(const char *request_buffer) {
    printf("TEST_get_request_uri: ");

    char *uri;
    int uri_len = get_request_uri(request_buffer, &uri);
    if (uri_len < 0) {
        printf("400 BAD REQUEST: %s", request_buffer);
        printf("------------------------------------\n");
        return;
    }

    if (uri == NULL) {
        printf("Unexpected NULL uri when not a bad request");
        printf("------------------------------------\n");
        return;
    }
    int printed_len = printf("%s", uri);
    printf("\n");
    printf("%-5s, printed_len=%3d, uri_len=%3d\n", uri_len == printed_len ? "true" : "false", printed_len, uri_len);
    printf("------------------------------------\n");
}
