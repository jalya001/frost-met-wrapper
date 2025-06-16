#include <iostream>
#include <cstring>
#include <curl/curl.h>
#include <string>

constexpr size_t RESPONSE_BUFFER_SIZE = 8 * 1024 * 1024;  // 8 MB

struct ResponseData {
    char* buffer;
    size_t size;
    size_t length;

    ResponseData(char* buf, size_t sz) : buffer(buf), size(sz), length(0) {
        buffer[0] = 0; // null-terminate
    }
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    auto* resp = static_cast<ResponseData*>(userp);

    size_t spaceLeft = resp->size - resp->length - 1; // leave room for null terminator
    size_t bytesToWrite = (totalSize > spaceLeft) ? spaceLeft : totalSize;

    if (bytesToWrite > 0) {
        std::memcpy(resp->buffer + resp->length, contents, bytesToWrite);
        resp->length += bytesToWrite;
        resp->buffer[resp->length] = '\0'; // null-terminate
    }

    return totalSize;
}

/**
 * Perform HTTP GET request.
 * @param url - URL to fetch.
 * @param out_json_ptr - will point to malloc'd response data (must be freed by caller).
 * @return size of response data in bytes, or 0 on failure.
 */
size_t http_get_response(const std::string& url, char** out_json_ptr) {
    char* buffer = static_cast<char*>(malloc(RESPONSE_BUFFER_SIZE));
    if (!buffer) {
        std::cerr << "Failed to allocate response buffer.\n";
        return 0;
    }

    ResponseData resp(buffer, RESPONSE_BUFFER_SIZE);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Failed to initialize CURL.\n";
        free(buffer);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << '\n';
        free(buffer);
        buffer = nullptr;
        resp.length = 0;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    *out_json_ptr = buffer;
    return resp.length;
}