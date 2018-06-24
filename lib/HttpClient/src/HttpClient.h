#ifndef __HTTP_CLIENT_H_
#define __HTTP_CLIENT_H_

#include <functional>
#include <queue>

#include "application.h"
#include "spark_wiring_string.h"
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_usbserial.h"
#include "http_request.h"

#define MAX_QUEUE_SIZE				8
#define DEFAULT_HTTP_TIMEOUT_MS 	5000

class HttpClient {
public:
    /**
    * Public references to variables.
    */
    TCPClient client;

    /**
    * Constructor.
    */
    HttpClient(uint16_t timeout=DEFAULT_HTTP_TIMEOUT_MS);

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
    bool pushRequest(HTTP_Request& r);

private:
    /*
     * Private variables
     */
    uint16_t responseTimeoutMs;
    uint8_t responseBuffer[1024];

    /* HTTP Request queue */
    uint8_t max_queue_size;
    std::queue<HTTP_Request> http_q;
};

#endif /* __HTTP_CLIENT_H_ */
