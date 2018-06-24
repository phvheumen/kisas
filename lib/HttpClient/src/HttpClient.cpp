#include "HttpClient.h"
#include "Applications.h"
#include <application.h>
#include <string>

static const uint16_t TIMEOUT = 5000; // Allow maximum 5s between data packets.

/**
* Constructor.
*/
HttpClient::HttpClient(uint16_t timeout)
{
	this->responseTimeoutMs = timeout;
}

/**
	run() always tries to jump to next state and shift states in state triplet accordingly.
	When state_next is nullptr, run stays in current state.
	The state triplet is used inside states to determine state entry and state exit
 */
void HttpClient::run(void) {
	// If state_curr is null pointer start with stateMachineEntry
	if( this->stateMachine.state_curr == nullptr ) {
		this->stateMachine.state_curr = stateMachineEntry;
	}

	// If state_next is null pointer stay in current state
	if (this->stateMachine.state_next != nullptr) {
		this->stateMachine.state_curr = this->stateMachine.state_next;
	}

	// Execute state function once
	stateMachine.state_curr(this); // Execute current state

	// Previous state is same as current state now
	this->stateMachine.state_prev = this->stateMachine.state_curr;
}

/**
	stateMachineEntry decides where to start the state machine.
	Possibly some global variables for the state machine can be initialised here.
	@param HttpClient instance pointer.
 */
void HttpClient::stateMachineEntry(HttpClient * obj) {
	obj->stateMachine.state_next = stateIdle;
}

void HttpClient::stateIdle(HttpClient * obj) {
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateIdle> Entry");

	}
	/* State entry end */

	/* State during */
	if( ( !( obj->testRequest.ip == (uint32_t) 0 ) ||  obj->testRequest.hostname != "" ) && obj->testRequest.port != 0 ){
		// If hostname or ip available start request
		obj->stateMachineVariables.currentRequest = &obj->testRequest;
		obj->stateMachine.state_next = stateConnect;
	}
	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {
		APP_MSG("HttpClient::stateIdle> Exit");
	}
	/* State exit end */
}

void HttpClient::stateConnect(HttpClient * obj) {
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateConnect> Entry");
	}
	/* State entry end */

	/* State during */
	do {
		http_request_t * req = obj->stateMachineVariables.currentRequest;
		if ( req->ip == (uint32_t) 0 ) {
			APP_MSG_FORMATTED("HttpClient::stateConnect> Resolving hostname: %s", req->hostname.c_str());
			// First start resolving hostname
			req->ip = Cellular.resolve(req->hostname.c_str());
			if(req->ip) {
				// Hostname resolved!
				APP_MSG("HttpClient::stateConnect> Hostname resolved");
			} else {
				// Hostname could not be resolved. Go to error state
				APP_MSG("HttpClient::stateConnect> Hostname not resolved");
				obj->stateMachine.state_next = stateError;
				break;
			}
		} else {
			APP_MSG_FORMATTED("HttpClient::stateConnect> Connecting to: %s", req->ip.toString().c_str());
			if( obj->client.connect(req->ip, req->port) ) {
				// Connected, now its time to send HTTP request
				APP_MSG("HttpClient::stateConnect> connected");
				obj->stateMachine.state_next = stateTransmit;
			} else {
				// Connection failed. Go to error state
				APP_MSG("HttpClient::stateConnect> Connection failed");
				obj->stateMachine.state_next = stateError;
				break;
			}
		}
	}
	while(0);
	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {
		APP_MSG("HttpClient::stateConnect> Exit");
	}
	/* State exit end */
}

void HttpClient::stateTransmit(HttpClient * obj) {
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateTransmit> Entry");

		obj->httpRequestString = "";
		http_request_t * req = obj->stateMachineVariables.currentRequest;

		obj->httpRequestString += req->method + " " + req->path + " HTTP/1.0\r\n";	// Send initial headers (only HTTP 1.0 is supported for now).
		obj->httpRequestString += "Connection: close\r\n"; 						// Not supporting keep-alive for now.
		if(req->hostname!=NULL) {
			obj->httpRequestString += "HOST: " + req->hostname + "\r\n";
		}
		if (req->body != NULL) {
			obj->httpRequestString += "Content-Length: " + String( (req->body).length() ) + "\r\n";
		} else if (req->method == HTTP_METHOD_POST) { //Check to see if its a Post method.
			obj->httpRequestString += "Content-Length: 0\r\n";
		}

// TODO: Add additional headers
//	    if (headers != NULL)
//	    {
//	        int i = 0;
//	        while (headers[i].header != NULL)
//	        {
//	            if (headers[i].value != NULL) {
//	                this->httpRequestString += String(headers[i].header) + ": " + String(headers[i].value) + "\r\n";
//	            } else {
//	                this->httpRequestString += String(headers[i].header) + "\r\n";
//	            }
//	            i++;
//	        }
//	    }

	    // Empty line to finish headers
		obj->httpRequestString += "\r\n";
		obj->httpRequestString += req->body;

		APP_MSG_FORMATTED("HttpClient::stateTransmit> HTTP request: \r\n%s", obj->httpRequestString.c_str());
	}
	/* State entry end */

	/* State during */
	do {
		uint16_t httpLength = obj->httpRequestString.length();

		if ( obj->client.write((uint8_t *) obj->httpRequestString.c_str(), httpLength) != httpLength ) {
			obj->stateMachine.state_next = stateError;
			break;
		} else {
			obj->httpRequestString = "";
			obj->stateMachine.state_next = stateWaitResponse;
		}
	} while (0);
	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {
		APP_MSG("HttpClient::stateTransmit> Exit");
	}
	/* State exit end */
}

void HttpClient::stateWaitResponse(HttpClient * obj) {
	static unsigned long tickMs;
	static uint16_t bytesReceived;
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateWaitResponse> Entry");

		tickMs = millis();
		bytesReceived = 0;
	}
	/* State entry end */

	/* State during */
	int bytesAvailable = obj->client.available();
	if ( bytesAvailable > 0 ) {
		obj->client.read(&obj->responseBuffer[bytesReceived], bytesAvailable);
		bytesReceived += bytesAvailable;
	} else if ( bytesAvailable < 0 ){
		// Error
		obj->stateMachine.state_next = stateError;
	} else if ( (millis() > tickMs + (unsigned long) obj->responseTimeoutMs) && obj->responseTimeoutMs != 0 ) {
		// Time out condition
		obj->stateMachine.state_next = stateTimeout;
	} else if ( !obj->client.connected() ) {
		// Connection closed. An indicator for end of response
		obj->responseBuffer[bytesReceived] = '\0';
		obj->httpResponseString = String((char *) obj->responseBuffer);
		APP_MSG_FORMATTED("HttpClient::stateWaitResponse> Response: \r\n%s", obj->httpResponseString.c_str());

		// Get status code in a not so elegant way
		String statusCode = obj->httpResponseString.substring(9,12);
		obj->testResponse.status = statusCode;
		APP_MSG_FORMATTED("HttpClient::stateWaitResponse> Response code: %s", obj->testResponse.status.c_str());

		// Get body
		int bodyPos = obj->httpResponseString.indexOf("\r\n\r\n");
		if (bodyPos == -1) {
			APP_MSG("HttpClient::stateWaitResponse> No response body");
		} else {
			obj->testResponse.body = "";
			obj->testResponse.body += obj->httpResponseString.substring(bodyPos+4);
		}

		if (obj->testResponse.status == String("200")) {
			obj->stateMachine.state_next = stateDone;
		} else {
			obj->stateMachine.state_next = stateError;
		}

	}

	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {

		APP_MSG("HttpClient::stateWaitResponse> Exit");
	}
	/* State exit end */
}

void HttpClient::stateTimeout(HttpClient * obj) {
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateTimeout> Entry");
	}
	/* State entry end */

	/* State during */

	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {
		APP_MSG("HttpClient::stateTimeout> Exit");
	}
	/* State exit end */
}

void HttpClient::stateDone(HttpClient * obj) {
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateDone> Entry");
	}
	/* State entry end */

	/* State during */

	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {
		APP_MSG("HttpClient::stateDone> Exit");
	}
	/* State exit end */
}

void HttpClient::stateError(HttpClient * obj) {
	/* State entry */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_prev ) {
		APP_MSG("HttpClient::stateError> Entry");
	}
	/* State entry end */

	/* State during */

	/* State during end*/

	/* State exit */
	if ( obj->stateMachine.state_curr != obj->stateMachine.state_next ) {
		APP_MSG("HttpClient::stateError> Exit");
	}
	/* State exit end */
}

bool HttpClient::sendRequest(http_callback_func * callback) {

	return true;
}
