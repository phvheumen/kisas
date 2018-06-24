#ifndef __HTTP_CLIENT_H_
#define __HTTP_CLIENT_H_

#include "application.h"
#include "spark_wiring_string.h"
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_usbserial.h"
#include <functional>

/**
 * Defines for the HTTP methods.
 */
static const char* HTTP_METHOD_GET    = "GET";
static const char* HTTP_METHOD_POST   = "POST";
static const char* HTTP_METHOD_PUT    = "PUT";
static const char* HTTP_METHOD_DELETE = "DELETE";
static const char* HTTP_METHOD_PATCH = "PATCH";

/**
 * This struct is used to pass additional HTTP headers such as API-keys.
 * Normally you pass this as an array. The last entry must have NULL as key.
 */
typedef struct
{
  const char* header;
  const char* value;
} http_header_t;

/**
 * HTTP Request struct.
 * hostname request host
 * path	 request path
 * port     request port
 * body	 request body
 */
typedef struct
{
  String hostname;
  IPAddress ip;
  String path;
  int port;
  String body;
  String method;
} http_request_t;

/**
 * HTTP Response struct.
 * status  response status code.
 * body	response body
 */
typedef struct
{
  String status;
  String body;
} http_response_t;

typedef enum http_request_status {
	HTTP_STATUS_OK,
	HTTP_STATUS_CONNECTION_TIMEOUT,
	HTTP_STATUS_RESPONSE_TIMEOUT,
	HTTP_STATUS_RESPONSE_ERROR
} http_request_status_t;

using http_callback_func = void(http_request_t *, http_response_t *, http_request_status_t);

class HttpClient {
public:
    /**
    * Public references to variables.
    */
    TCPClient client;

    /**
    * Constructor.
    */
    HttpClient(uint16_t timeout=0);

    http_request_t testRequest;
    http_response_t testResponse;

    /*
     * State machine methods and members
     */
    using stateFunction = void(HttpClient *);

    struct state_triplet {
    	stateFunction * state_prev;
    	stateFunction * state_curr;
    	stateFunction * state_next;
    } stateMachine;

    struct state_machine_variables {
    	http_request_t * currentRequest;
    } stateMachineVariables;

    void run(void);
    static void stateMachineEntry(HttpClient * obj);

    static void stateIdle(HttpClient * obj);
    static void stateConnect(HttpClient * obj);
    static void stateTransmit(HttpClient * obj);
    static void stateWaitResponse(HttpClient * obj);
    static void stateTimeout(HttpClient * obj);
    static void stateDone(HttpClient * obj);
    static void stateError(HttpClient * obj);

    /*
     * Request queue methods
     */
    bool pushRequest(http_request_t &pRequest);
    bool popRequest(http_request_t &pRequest);

    /*
     * Send request
     */
    bool sendRequest(http_callback_func * callback);

private:
    /*
     * Private variables
     */
    String httpRequestString;
    String httpResponseString;

    http_request_t requestQueue[8];
    http_response_t responseQueue[8];

    uint16_t responseTimeoutMs;
    uint8_t responseBuffer[1024];
};

#endif /* __HTTP_CLIENT_H_ */
