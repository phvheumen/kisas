/*
 * http_request.h
 *
 *  Created on: 24 Jun 2018
 *      Author: P.H. van Heumen
 */

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <string>
#include <application.h>

extern const char* HTTP_METHOD_OPTIONS;
extern const char* HTTP_METHOD_GET;
extern const char* HTTP_METHOD_HEAD;
extern const char* HTTP_METHOD_POST;
extern const char* HTTP_METHOD_DELETE;
extern const char* HTTP_METHOD_TRACE;
extern const char* HTTP_METHOD_CONNECT;

typedef struct http_header_field {
    const char * field;
    const char * value;
} http_header_field_T;

class HTTP_Request {
public:
	typedef std::function<void(HTTP_Request&)> http_request_callback_fn;

    HTTP_Request();
    HTTP_Request(const char* host, uint16_t port, const char* method, const char* URI, http_header_field_T header_fields[], const char* body);
    ~HTTP_Request();

    void addCallback(http_request_callback_fn callback_) { callback = std::move(callback_); }

    template <typename T>
    void addCallback(void (T::*fun)(HTTP_Request&), T& instance) { addCallback(std::bind(fun, &instance, std::placeholders::_1)); }

    std::string request() { return httpRequestString; }
    std::string response() { return httpResponseString; }

    void addResponse(const char* response) { httpResponseString = response; parseResponse(); }
    int responseStatus(void) { return statusCode; }
    std::string responseBody(void) { return httpResponseBody; }

    std::string host;
	IPAddress ip;
	uint16_t port;

private:
    std::string httpRequestString; // Complete request string
    std::string httpResponseString; // Complete response string

    void parseResponse(void);
    int statusCode;
    std::string httpResponseBody;

    http_request_callback_fn callback;
};

#endif /* _HTTP_REQUEST_H_ */
