/*
 * http_request.cpp
 *
 *  Created on: 24 Jun 2018
 *      Author: P.H. van Heumen
 */

#include "http_request.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>

#include "http_request.h"

const char* HTTP_METHOD_OPTIONS = "OPTIONS";
const char* HTTP_METHOD_GET     = "GET";
const char* HTTP_METHOD_HEAD    = "HEAD";
const char* HTTP_METHOD_POST    = "POST";
const char* HTTP_METHOD_DELETE  = "DELETE";
const char* HTTP_METHOD_TRACE   = "TRACE";
const char* HTTP_METHOD_CONNECT = "CONNECT";

const char* HTTP_METHODS[] = {
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_CONNECT
};

HTTP_Request::HTTP_Request() {
	httpRequestString = "";
	httpResponseString = "";
	statusCode = 0;
	httpResponseBody = "";
}

HTTP_Request::HTTP_Request(const char* host, uint16_t port, const char* method, const char* URI, http_header_field_T header_fields[], const char* body) {
	// Host information
	this->host = host;
	this->port = port;

	// Construct request line
	const int HTTP_METHODS_size = sizeof(HTTP_METHODS)/sizeof(HTTP_METHODS[0]);

	int k = 0;
	while( k < HTTP_METHODS_size ) {
		if ( 0 == strcmp(HTTP_METHODS[k], method) ) {
			break;
		}
		++k;
	}

	if( k >= HTTP_METHODS_size ) {
		// method argument was not a valid HTTP method.
		// It suffices to leave httpRequestString empty.
		return;
	}

	httpRequestString = method;
	httpRequestString += " ";
	httpRequestString += URI;
	httpRequestString += " ";
	httpRequestString += "HTTP/1.1";
	httpRequestString += "\r\n";

	// Construct header
	// Also check if user gives Content-Length and Host field.
	// If not, the constructor will append it automatically
	// At last we always include "Connection: close" entry to header. This library is not intended to support keep-alive conversations
	k = 0;
	bool contentLengthField = false;
	bool hostField = false;

	if (header_fields != nullptr) {
		while(header_fields[k].field != nullptr && header_fields[k].value != nullptr) {
			if( 0 == strcmp(header_fields[k].field, "Content-Length") )
				contentLengthField = true;

			if( 0 == strcmp(header_fields[k].field, "Host") )
				hostField = true;

			httpRequestString += header_fields[k].field;
			httpRequestString += ": ";
			httpRequestString += header_fields[k].value;
			httpRequestString += "\r\n";
			++k;
		}
	}

	if( !contentLengthField && (strlen(body) > 0) ) {
		char bodyLenStr[8];
		snprintf(bodyLenStr, 8, "%d", strlen(body));
		httpRequestString += "Content-Length: ";
		httpRequestString += bodyLenStr;
		httpRequestString += "\r\n";
	}

	if( !hostField ) {
		httpRequestString += "Host: " + this->host + "\r\n";
	}

	// Close connection
	httpRequestString += "Connection: close\r\n";

	// End header
	httpRequestString += "\r\n";

	// Append body
	httpRequestString += body;

	// Initialise response members
	httpResponseString = "";
	statusCode = 0;
	httpResponseBody = "";

}

HTTP_Request::~HTTP_Request() {

}

void HTTP_Request::parseResponse(void) {
	statusCode = 0;
	httpResponseBody = "";

	if( httpResponseString.size() > 0 ) {
		std::string statusLine = httpResponseString.substr(0, httpResponseString.find("\r\n", 0));

		/* Tokenize statusline */
		std::vector<std::string> v;
		size_t startPos = 0, endPos = 0;
		while( endPos != std::string::npos ) {
			endPos = statusLine.find(" ", startPos);
			v.push_back(statusLine.substr( startPos, (endPos == std::string::npos) ? std::string::npos : endPos - startPos ));
			startPos = (endPos == std::string::npos) ? std::string::npos : ++endPos;
		}

		/* Check if statusLine was valid */
		if( v.size() != 3 )
			return;

		if( v[0].compare("HTTP/1.1") != 0 )
			return;

		statusCode = strtol(v[1].c_str(), NULL, 10);
		if ( statusCode == 0 )
			return;

		// Search for body, by simple take everything after the first \r\n\r\n
		size_t bodyPos = httpResponseString.find("\r\n\r\n", 0);
		if ( bodyPos != std::string::npos )
			httpResponseBody = httpResponseString.substr(bodyPos + 4, std::string::npos);
	}
}



