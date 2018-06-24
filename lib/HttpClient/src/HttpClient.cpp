#include <application.h>
#include <string>

#include "HttpClient.h"
#include "Applications.h"
#include "http_request.h"

/**
* Constructor.
*/
HttpClient::HttpClient(uint16_t timeout)
{
	this->responseTimeoutMs = timeout;
	this->max_queue_size = MAX_QUEUE_SIZE;
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
	// When there are HTTP requests in the queue, start to process them
	if( obj->http_q.size() > 0 ) {
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
		if ( ( obj->http_q.front().ip == (uint32_t) 0 ) ) {
			APP_MSG_FORMATTED("HttpClient::stateConnect> Resolving hostname: %s", obj->http_q.front().host.c_str());
			// First start resolving hostname
			obj->http_q.front().ip = Cellular.resolve(obj->http_q.front().host.c_str());
			if( obj->http_q.front().ip ) {
				// Hostname resolved!
				APP_MSG("HttpClient::stateConnect> Hostname resolved");
			} else {
				// Hostname could not be resolved. Go to error state
				APP_MSG("HttpClient::stateConnect> Hostname not resolved");
				obj->stateMachine.state_next = stateError;
				break;
			}
		} else {
			APP_MSG_FORMATTED("HttpClient::stateConnect> Connecting to: %s", obj->http_q.front().ip.toString().c_str());
			if( obj->client.connect(obj->http_q.front().ip, obj->http_q.front().port) ) {
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
	} while(0);
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
		APP_MSG_FORMATTED("HttpClient::stateTransmit> HTTP request: \r\n%s", obj->http_q.front().request().c_str());
	}
	/* State entry end */

	/* State during */
	do {
		uint16_t httpRequestLen = obj->http_q.front().request().size();

		if ( obj->client.write((uint8_t *) obj->http_q.front().request().c_str(), httpRequestLen) != httpRequestLen ) {
			obj->stateMachine.state_next = stateError;
			break;
		} else {
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
		// Connection closed. An indicator for end of response.
		// This is a real weakness for the system now.
		// A bad server could keep the connection open for forever and also keep pushing bytes. Memory will overflow and particle craches.
		obj->responseBuffer[bytesReceived] = '\0';
		obj->http_q.front().addResponse(String((char *) obj->responseBuffer));

		APP_MSG_FORMATTED("HttpClient::stateWaitResponse> Response: \r\n%s", obj->http_q.front().response().c_str());
		APP_MSG_FORMATTED("HttpClient::stateWaitResponse> Response code: %d", obj->http_q.front().responseStatus());
		APP_MSG_FORMATTED("HttpClient::stateWaitResponse> Response body: %s", obj->http_q.front().responseBody().c_str());

		if( obj->http_q.front().responseStatus() == 200 ) {
			// OK
			obj->stateMachine.state_next = stateDone;
		} else if( obj->http_q.front().responseStatus() == 0 ) {
			// Response could not be parsed
			obj->stateMachine.state_next = stateError;
		} else {
			// Response NOT OK
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
	// Pop processed request from the queue and go back to stateIdle
	obj->http_q.pop();
	obj->stateMachine.state_next = stateIdle;

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

/**
 * Add an HTTP request to the HTTP request service queue
 * This method is probably not thread safe.
 *
 * @param r The HTTP request object
 * @return True if added successfully to the queue, False if queue is full.
 */
bool HttpClient::pushRequest(HTTP_Request& r) {
	if( this->http_q.size() >= this->max_queue_size ) {
		return false;
	} else {
		this->http_q.push(r);
		return true;
	}
}
