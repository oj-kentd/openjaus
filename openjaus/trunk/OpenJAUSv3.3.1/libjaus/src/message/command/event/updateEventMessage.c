/*****************************************************************************
 *  Copyright (c) 2008, University of Florida
 *  All rights reserved.
 *  
 *  This file is part of OpenJAUS.  OpenJAUS is distributed under the BSD 
 *  license.  See the LICENSE file for details.
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the University of Florida nor the names of its 
 *       contributors may be used to endorse or promote products derived from 
 *       this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
// File Name: updateEventMessage.c
//
// Written By: Danny Kent (jaus AT dannykent DOT com), Tom Galluzzo (galluzzo AT gmail DOT com)
//
// Version: 3.3.0
//
// Date: 07/09/08
//
// Description: This file defines the functionality of a UpdateEventMessage


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jaus.h"

static const int commandCode = JAUS_UPDATE_EVENT;
static const int maxDataSizeBytes = 512000; // Max Message size: 500K

static JausBoolean headerFromBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static JausBoolean headerToBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static int headerToString(UpdateEventMessage message, char **buf);

static JausBoolean dataFromBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static int dataToBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static void dataInitialize(UpdateEventMessage message);
static void dataDestroy(UpdateEventMessage message);
static unsigned int dataSize(UpdateEventMessage message);

// ************************************************************************************************************** //
//                                    USER CONFIGURED FUNCTIONS
// ************************************************************************************************************** //

// Initializes the message-specific fields
static void dataInitialize(UpdateEventMessage message)
{
	// Set initial values of message fields

	message->presenceVector = newJausByte(JAUS_BYTE_PRESENCE_VECTOR_ALL_ON);	// 1: Presence Vector
	message->requestId = newJausByte		(0);			// Local request ID for use in confirm event
	message->reportMessageCode = newJausUnsignedShort(0);			// Command Code of the resulting query
	message->eventType = newJausByte(0);					// Enumeration of Event types
	message->eventBoundary = newJausByte(0);				// Enumeration of Event Boundary Conditions
	message->limitDataField = newJausByte(0);				// Field from Report for Limit Trigger
	message->lowerLimit = jausEventLimitCreate();			// Lower Event Limit
	message->lowerLimit->dataType = EVENT_LIMIT_BYTE_TYPE;
	message->upperLimit = jausEventLimitCreate();			// Upper Event Limit
	message->upperLimit->dataType = EVENT_LIMIT_BYTE_TYPE;
	message->stateLimit = jausEventLimitCreate();			// State Event Limit used for Equal Boundary
	message->stateLimit->dataType = EVENT_LIMIT_BYTE_TYPE;
	message->requestedMinimumRate = newJausDouble(0.0);		// For Periodic Events for unchanging value, Scaled UnsignedShort (0, 1092)
	message->requestedUpdateRate = newJausDouble(0.0);		// For Periodic Events, Scaled UnsignedShort (0, 1092)
	message->eventId = newJausByte(0);						// ID of event to be updated
	message->queryMessage = NULL;			// Query Message (including header) to use for response	
}

// Destructs the message-specific fields
static void dataDestroy(UpdateEventMessage message)
{
	// Free message fields
	if(message->queryMessage) jausMessageDestroy(message->queryMessage);
	if(message->lowerLimit) jausEventLimitDestroy(message->lowerLimit);
	if(message->upperLimit) jausEventLimitDestroy(message->upperLimit);
	if(message->stateLimit) jausEventLimitDestroy(message->stateLimit);
}

// Return boolean of success
static JausBoolean dataFromBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	int index = 0;
	JausUnsignedShort tempUShort;
	JausUnsignedInteger queryMessageSize;
	
	if(bufferSizeBytes == message->dataSize)
	{
		// Unpack Message Fields from Buffer
		// Presence Vector
		if(!jausByteFromBuffer(&message->presenceVector, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;

		// Request ID
		if(!jausByteFromBuffer(&message->requestId, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;

		// Message Code
		if(!jausUnsignedShortFromBuffer(&message->reportMessageCode, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;		

		// Event Type
		if(!jausByteFromBuffer(&message->eventType, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_BOUNDARY_BIT))
		{
			// Event Boundary
			if(!jausByteFromBuffer(&message->eventBoundary, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}

		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_DATA_FIELD_BIT))
		{
			// Data Field
			if(!jausByteFromBuffer(&message->limitDataField, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_LOWER_LIMIT_BIT))
		{		
			// Lower Limit
			if(!jausEventLimitFromBuffer(&message->lowerLimit, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += jausEventLimitSize(message->lowerLimit);
		}
		else
		{
			message->lowerLimit = NULL;
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_UPPER_LIMIT_BIT))
		{		
			// Upper Limit
			if(!jausEventLimitFromBuffer(&message->upperLimit, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += jausEventLimitSize(message->upperLimit);
		}
		else
		{
			message->upperLimit = NULL;
		}

		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_STATE_LIMIT_BIT))
		{		
			// State Limit
			if(!jausEventLimitFromBuffer(&message->stateLimit, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += jausEventLimitSize(message->stateLimit);
		}
		else
		{
			message->stateLimit = NULL;
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_MINIMUM_RATE_BIT))
		{		
			// Minimum Periodic Rate 
			if(!jausUnsignedShortFromBuffer(&tempUShort, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
			
			// Scaled Int (0, 1092);
			message->requestedMinimumRate = jausUnsignedShortToDouble(tempUShort, 0, 1092);
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_REQUESTED_RATE_BIT))
		{		
			// Periodic Rate 
			if(!jausUnsignedShortFromBuffer(&tempUShort, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
			
			// Scaled Int (0, 1092);
			message->requestedUpdateRate = jausUnsignedShortToDouble(tempUShort, 0, 1092);
		}
		
		if(!jausByteFromBuffer(&message->eventId, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;		
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_QUERY_MESSAGE_BIT))
		{		
			// Query Message Size
			if(!jausUnsignedIntegerFromBuffer(&queryMessageSize, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_INTEGER_SIZE_BYTES;
			
			// Query Message Body
			if(bufferSizeBytes-index < queryMessageSize) return JAUS_FALSE;

			// Setup our Query Message
			message->queryMessage = jausMessageCreate();
			if(!message->queryMessage) return JAUS_FALSE;

			message->queryMessage->dataSize = queryMessageSize;
			jausAddressCopy(message->queryMessage->source, message->source);
			jausAddressCopy(message->queryMessage->destination, message->destination);
			
			message->queryMessage->commandCode = jausMessageGetComplementaryCommandCode(message->reportMessageCode);

			// Allocate Memory
			message->queryMessage->data = (unsigned char *) malloc(queryMessageSize);
			
			// Copy query message body to the data member
			memcpy(message->queryMessage->data, buffer+index, queryMessageSize);
			index += queryMessageSize;
		}
		return JAUS_TRUE;
	}
	else
	{
		return JAUS_FALSE;
	}
}

// Returns number of bytes put into the buffer
static int dataToBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	int index = 0;
	JausUnsignedShort tempUShort;

	if(bufferSizeBytes >= dataSize(message))
	{
		// Unpack Message Fields from Buffer
		// Presence Vector
		if(!jausByteToBuffer(message->presenceVector, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;
		
		// Request Id
		if(!jausByteToBuffer(message->requestId, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;

		// Message Code
		if(!jausUnsignedShortToBuffer(message->reportMessageCode, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
		
		// Event Type
		if(!jausByteToBuffer(message->eventType, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_BOUNDARY_BIT))
		{
			// Event Boundary
			if(!jausByteToBuffer(message->eventBoundary, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}

		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_DATA_FIELD_BIT))
		{
			// Data Field
			if(!jausByteToBuffer(message->limitDataField, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_LOWER_LIMIT_BIT))
		{		
			// Lower Limit
			if(!jausEventLimitToBuffer(message->lowerLimit, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += jausEventLimitSize(message->lowerLimit);
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_UPPER_LIMIT_BIT))
		{		
			// Upper Limit
			if(!jausEventLimitToBuffer(message->upperLimit, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += jausEventLimitSize(message->upperLimit);
		}

		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_STATE_LIMIT_BIT))
		{		
			// State Limit
			if(!jausEventLimitToBuffer(message->stateLimit, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += jausEventLimitSize(message->stateLimit);
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_MINIMUM_RATE_BIT))
		{		
			// Scaled Int (0, 1092);
			tempUShort = jausUnsignedShortFromDouble(message->requestedMinimumRate, 0, 1092);

			// Minimum Periodic Rate 
			if(!jausUnsignedShortToBuffer(tempUShort, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
		}
		
		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_REQUESTED_RATE_BIT))
		{		
			// Scaled Int (0, 1092);
			tempUShort = jausUnsignedShortFromDouble(message->requestedUpdateRate, 0, 1092);

			// Minimum Periodic Rate 
			if(!jausUnsignedShortToBuffer(tempUShort, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
		}
	
		if(!jausByteToBuffer(message->eventId, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_SIZE_BYTES;

		if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_QUERY_MESSAGE_BIT))
		{	
			if(message->queryMessage)
			{
				if(!jausUnsignedIntegerToBuffer(message->queryMessage->dataSize, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
				index += JAUS_UNSIGNED_INTEGER_SIZE_BYTES;
			
				// Query Message Body
				if(bufferSizeBytes-index < message->queryMessage->dataSize) return JAUS_FALSE;
				memcpy(buffer+index, message->queryMessage->data, message->queryMessage->dataSize);
				index += message->queryMessage->dataSize;
			}
			else
			{
				if(!jausUnsignedIntegerToBuffer(0, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
				index += JAUS_UNSIGNED_INTEGER_SIZE_BYTES;
			}
		}
	}

	return index;
}

static int dataToString(UpdateEventMessage message, char **buf)
{
  //message already verified 

  char* lowerLimitStr = NULL;
  char* upperLimitStr = NULL;
  char* stateLimitStr = NULL;
  char* msgString = NULL;
  unsigned int bufSize = 2000;

  //Setup temporary string buffer
  (*buf) = (char*)malloc(sizeof(char)*bufSize);

  strcpy((*buf), "\nPresence Vector: " );
    
  jausByteToHexString(message->presenceVector, (*buf)+strlen(*buf));

  strcat((*buf), "\nRequest Id: " );
    
  jausByteToString(message->requestId, (*buf)+strlen(*buf));

  strcat((*buf), "\nMessage Code: " );
    
  jausUnsignedShortToString(message->reportMessageCode, (*buf)+strlen(*buf));

  strcat((*buf), "\nEvent Type: " );
      
  jausByteToString(message->eventType, (*buf)+strlen(*buf));

  switch(message->eventType)
  {
  case EVENT_PERIODIC_TYPE:
    strcat((*buf), " Periodic(SC)");
    break;
    
  case EVENT_EVERY_CHANGE_TYPE:
    strcat((*buf), " Every Change");
    break;
    
  case EVENT_FIRST_CHANGE_TYPE:
    strcat((*buf), " First Change");
    break;
    
  case EVENT_FIRST_CHANGE_IN_AND_OUT_TYPE:
    strcat((*buf), " First change in and out of boundary");
    break;
    
  case EVENT_PERIODIC_NO_REPEAT_TYPE:
    strcat((*buf), " Periodic w/o replacement");
    break;
    
  case EVENT_ONE_TIME_ON_DEMAND_TYPE:
    strcat((*buf), " One time, on demand");
    break;
  }
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_BOUNDARY_BIT))
  {
    strcat((*buf), "\nEvent Boundary: " );
      
    jausByteToString(message->eventBoundary, (*buf)+strlen(*buf));
    
    switch(message->eventBoundary)
    {
    case EQUAL_BOUNDARY:
      strcat((*buf), " Equal");
      break;
      
    case NOT_EQUAL_BOUNDARY:
      strcat((*buf), " Not Equal");
      break;
      
    case INSIDE_INCLUSIVE_BOUNDARY:
      strcat((*buf), " Inside Inclusive");
      break;
      
    case INSIDE_EXCLUSIVE_BOUNDARY:
      strcat((*buf), " Inside Exclusive");
      break;
      
    case OUTSIDE_INCLUSIVE_BOUNDARY:
      strcat((*buf), " Outside Inclusive");
      break;
      
    case OUTSIDE_EXCLUSIVE_BOUNDARY:
      strcat((*buf), " Outside Exclusive");
      break;
      
    case GREATER_THAN_OR_EQUAL_BOUNDARY:
      strcat((*buf), " Greater than or Equal");
      break;
      
    case GREATER_THAN_BOUNDARY:
      strcat((*buf), " Strictly Greater than");
      break;
      
    case LESS_THAN_OR_EQUAL_BOUNDARY:
      strcat((*buf), " Less than or Equal");
      break;
      
    case LESS_THAN_BOUNDARY:
      strcat((*buf), " Strictly Less than");
      break;
    }
  }

  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_DATA_FIELD_BIT))
  {
    strcat((*buf), "\nLimit Data Field: " );
      
    jausByteToString(message->limitDataField, (*buf)+strlen(*buf));
  }
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_LOWER_LIMIT_BIT))
  {
    strcat((*buf), "\nLower Limit\n" );
    lowerLimitStr = jausEventLimitToString(message->lowerLimit); 
    strcat((*buf), lowerLimitStr);
    free(lowerLimitStr);
  }
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_UPPER_LIMIT_BIT))
  {
    strcat((*buf), "\nUpper Limit\n" );
    upperLimitStr = jausEventLimitToString(message->upperLimit);
    strcat((*buf), upperLimitStr);
    free(upperLimitStr);
  }
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_STATE_LIMIT_BIT))
  {
    strcat((*buf), "\nState Limit\n" );
    stateLimitStr =jausEventLimitToString(message->stateLimit); 
    strcat((*buf), stateLimitStr);
    free(stateLimitStr);
  }
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_MINIMUM_RATE_BIT))
  {
    strcat((*buf), "\nRequested Minimum Periodic Rate(Hz): " );
    jausDoubleToString(message->requestedMinimumRate, (*buf)+strlen(*buf));
  }
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_REQUESTED_RATE_BIT))
  {
    strcat((*buf), "\nRequested Periodic Update Rate(Hz): " );
    jausDoubleToString(message->requestedUpdateRate, (*buf)+strlen(*buf));
  }
  
  strcat((*buf), "\nEvent Id: ");
  jausByteToString(message->eventId, (*buf)+strlen(*buf));
  
  if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_QUERY_MESSAGE_BIT))
  {
    strcat((*buf), "\nQuery Message" );
    msgString = jausMessageToString(message->queryMessage);
    strcat((*buf), msgString);
    free(msgString);
  }

  return (int)strlen(*buf);
}

// Returns number of bytes put into the buffer
static unsigned int dataSize(UpdateEventMessage message)
{
	int index = 0;

	// Presence Vector
	index += JAUS_BYTE_SIZE_BYTES;

	// Request ID
	index += JAUS_BYTE_SIZE_BYTES;
	
	// Message Code
	index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
	
	// Event Type
	index += JAUS_BYTE_SIZE_BYTES;
	
	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_BOUNDARY_BIT))
	{
		index += JAUS_BYTE_SIZE_BYTES;
	}

	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_DATA_FIELD_BIT))
	{
		index += JAUS_BYTE_SIZE_BYTES;
	}
	
	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_LOWER_LIMIT_BIT))
	{		
		index += jausEventLimitSize(message->lowerLimit);
	}
	
	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_UPPER_LIMIT_BIT))
	{		
		index += jausEventLimitSize(message->upperLimit);
	}

	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_STATE_LIMIT_BIT))
	{		
		index += jausEventLimitSize(message->stateLimit);
	}
	
	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_MINIMUM_RATE_BIT))
	{		
		index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
	}
	
	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_REQUESTED_RATE_BIT))
	{		
		index += JAUS_UNSIGNED_SHORT_SIZE_BYTES;
	}

	// Event ID
	index += JAUS_BYTE_SIZE_BYTES;

	if(jausByteIsBitSet(message->presenceVector, UPDATE_EVENT_PV_QUERY_MESSAGE_BIT))
	{		
		index += JAUS_UNSIGNED_INTEGER_SIZE_BYTES;
		// Jaus Message
		if(message->queryMessage) index += message->queryMessage->dataSize;
	}


	return index;
}


// ************************************************************************************************************** //
//                                    NON-USER CONFIGURED FUNCTIONS
// ************************************************************************************************************** //

UpdateEventMessage updateEventMessageCreate(void)
{
	UpdateEventMessage message;

	message = (UpdateEventMessage)malloc( sizeof(UpdateEventMessageStruct) );
	if(message == NULL)
	{
		return NULL;
	}
	
	// Initialize Values
	message->properties.priority = JAUS_DEFAULT_PRIORITY;
	message->properties.ackNak = JAUS_ACK_NAK_NOT_REQUIRED;
	message->properties.scFlag = JAUS_NOT_SERVICE_CONNECTION_MESSAGE;
	message->properties.expFlag = JAUS_NOT_EXPERIMENTAL_MESSAGE;
	message->properties.version = JAUS_VERSION_3_3;
	message->properties.reserved = 0;
	message->commandCode = commandCode;
	message->destination = jausAddressCreate();
	message->source = jausAddressCreate();
	message->dataFlag = JAUS_SINGLE_DATA_PACKET;
	message->dataSize = maxDataSizeBytes;
	message->sequenceNumber = 0;
	
	dataInitialize(message);
	message->dataSize = dataSize(message);
	
	return message;	
}

void updateEventMessageDestroy(UpdateEventMessage message)
{
	dataDestroy(message);
	jausAddressDestroy(message->source);
	jausAddressDestroy(message->destination);
	free(message);
	message = NULL;
}

JausBoolean updateEventMessageFromBuffer(UpdateEventMessage message, unsigned char* buffer, unsigned int bufferSizeBytes)
{
	int index = 0;
	
	if(headerFromBuffer(message, buffer+index, bufferSizeBytes-index))
	{
		index += JAUS_HEADER_SIZE_BYTES;
		if(dataFromBuffer(message, buffer+index, bufferSizeBytes-index))
		{
			return JAUS_TRUE;
		}
		else
		{
			return JAUS_FALSE;
		}
	}
	else
	{
		return JAUS_FALSE;
	}
}

JausBoolean updateEventMessageToBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	if(bufferSizeBytes < updateEventMessageSize(message))
	{
		return JAUS_FALSE; //improper size	
	}
	else
	{	
		message->dataSize = dataToBuffer(message, buffer+JAUS_HEADER_SIZE_BYTES, bufferSizeBytes - JAUS_HEADER_SIZE_BYTES);
		if(headerToBuffer(message, buffer, bufferSizeBytes))
		{
			return JAUS_TRUE;
		}
		else
		{
			return JAUS_FALSE; // headerToUpdateEventBuffer failed
		}
	}
}

UpdateEventMessage updateEventMessageFromJausMessage(JausMessage jausMessage)
{
	UpdateEventMessage message;

	if(jausMessage->commandCode != commandCode)
	{
		return NULL; // Wrong message type
	}
	else
	{
		message = (UpdateEventMessage)malloc( sizeof(UpdateEventMessageStruct) );
		if(message == NULL)
		{
			return NULL;
		}
		
		message->properties.priority = jausMessage->properties.priority;
		message->properties.ackNak = jausMessage->properties.ackNak;
		message->properties.scFlag = jausMessage->properties.scFlag;
		message->properties.expFlag = jausMessage->properties.expFlag;
		message->properties.version = jausMessage->properties.version;
		message->properties.reserved = jausMessage->properties.reserved;
		message->commandCode = jausMessage->commandCode;
		message->destination = jausAddressCreate();
		*message->destination = *jausMessage->destination;
		message->source = jausAddressCreate();
		*message->source = *jausMessage->source;
		message->dataSize = jausMessage->dataSize;
		message->dataFlag = jausMessage->dataFlag;
		message->sequenceNumber = jausMessage->sequenceNumber;
		
		// Unpack jausMessage->data
		if(dataFromBuffer(message, jausMessage->data, jausMessage->dataSize))
		{
			return message;
		}
		else
		{
			return NULL;
		}
	}
}

JausMessage updateEventMessageToJausMessage(UpdateEventMessage message)
{
	JausMessage jausMessage;
	
	jausMessage = (JausMessage)malloc( sizeof(struct JausMessageStruct) );
	if(jausMessage == NULL)
	{
		return NULL;
	}	
	
	jausMessage->properties.priority = message->properties.priority;
	jausMessage->properties.ackNak = message->properties.ackNak;
	jausMessage->properties.scFlag = message->properties.scFlag;
	jausMessage->properties.expFlag = message->properties.expFlag;
	jausMessage->properties.version = message->properties.version;
	jausMessage->properties.reserved = message->properties.reserved;
	jausMessage->commandCode = message->commandCode;
	jausMessage->destination = jausAddressCreate();
	*jausMessage->destination = *message->destination;
	jausMessage->source = jausAddressCreate();
	*jausMessage->source = *message->source;
	jausMessage->dataSize = dataSize(message);
	jausMessage->dataFlag = message->dataFlag;
	jausMessage->sequenceNumber = message->sequenceNumber;
	
	jausMessage->data = (unsigned char *)malloc(jausMessage->dataSize);
	jausMessage->dataSize = dataToBuffer(message, jausMessage->data, jausMessage->dataSize);
	
	return jausMessage;
}


unsigned int updateEventMessageSize(UpdateEventMessage message)
{
	return (unsigned int)(dataSize(message) + JAUS_HEADER_SIZE_BYTES);
}

char* updateEventMessageToString(UpdateEventMessage message)
{
  if(message)
  {
    char* buf1 = NULL;
    char* buf2 = NULL;
    char* buf = NULL;
    
    int returnVal;
    
    //Print the message header to the string buffer
    returnVal = headerToString(message, &buf1);
    
    //Print the message data fields to the string buffer
    returnVal += dataToString(message, &buf2);
    
buf = (char*)malloc(strlen(buf1)+strlen(buf2)+1);
    strcpy(buf, buf1);
    strcat(buf, buf2);
    
    free(buf1);
    free(buf2);
    return buf;
  }
  else
  {
    char* buf = "Invalid UpdateEvent Message";
    char* msg = (char*)malloc(strlen(buf)+1);
    strcpy(msg, buf);
    return msg;
  }
}
//********************* PRIVATE HEADER FUNCTIONS **********************//

static JausBoolean headerFromBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	if(bufferSizeBytes < JAUS_HEADER_SIZE_BYTES)
	{ 
		return JAUS_FALSE;
	}
	else
	{
		// unpack header
		message->properties.priority = (buffer[0] & 0x0F);
		message->properties.ackNak	 = ((buffer[0] >> 4) & 0x03);
		message->properties.scFlag	 = ((buffer[0] >> 6) & 0x01);
		message->properties.expFlag	 = ((buffer[0] >> 7) & 0x01);
		message->properties.version	 = (buffer[1] & 0x3F);
		message->properties.reserved = ((buffer[1] >> 6) & 0x03);
		
		message->commandCode = buffer[2] + (buffer[3] << 8);
	
		message->destination->instance = buffer[4];
		message->destination->component = buffer[5];
		message->destination->node = buffer[6];
		message->destination->subsystem = buffer[7];
	
		message->source->instance = buffer[8];
		message->source->component = buffer[9];
		message->source->node = buffer[10];
		message->source->subsystem = buffer[11];
		
		message->dataSize = buffer[12] + ((buffer[13] & 0x0F) << 8);

		message->dataFlag = ((buffer[13] >> 4) & 0x0F);

		message->sequenceNumber = buffer[14] + (buffer[15] << 8);
		
		return JAUS_TRUE;
	}
}

static JausBoolean headerToBuffer(UpdateEventMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	JausUnsignedShort *propertiesPtr = (JausUnsignedShort*)&message->properties;
	
	if(bufferSizeBytes < JAUS_HEADER_SIZE_BYTES)
	{
		return JAUS_FALSE;
	}
	else
	{	
		buffer[0] = (unsigned char)(*propertiesPtr & 0xFF);
		buffer[1] = (unsigned char)((*propertiesPtr & 0xFF00) >> 8);

		buffer[2] = (unsigned char)(message->commandCode & 0xFF);
		buffer[3] = (unsigned char)((message->commandCode & 0xFF00) >> 8);

		buffer[4] = (unsigned char)(message->destination->instance & 0xFF);
		buffer[5] = (unsigned char)(message->destination->component & 0xFF);
		buffer[6] = (unsigned char)(message->destination->node & 0xFF);
		buffer[7] = (unsigned char)(message->destination->subsystem & 0xFF);

		buffer[8] = (unsigned char)(message->source->instance & 0xFF);
		buffer[9] = (unsigned char)(message->source->component & 0xFF);
		buffer[10] = (unsigned char)(message->source->node & 0xFF);
		buffer[11] = (unsigned char)(message->source->subsystem & 0xFF);
		
		buffer[12] = (unsigned char)(message->dataSize & 0xFF);
		buffer[13] = (unsigned char)((message->dataFlag & 0xFF) << 4) | (unsigned char)((message->dataSize & 0x0F00) >> 8);

		buffer[14] = (unsigned char)(message->sequenceNumber & 0xFF);
		buffer[15] = (unsigned char)((message->sequenceNumber & 0xFF00) >> 8);
		
		return JAUS_TRUE;
	}
}

static int headerToString(UpdateEventMessage message, char **buf)
{
  //message existance already verified 

  //Setup temporary string buffer
  
  unsigned int bufSize = 500;
  (*buf) = (char*)malloc(sizeof(char)*bufSize);
  
  strcpy((*buf), jausCommandCodeString(message->commandCode) );
  strcat((*buf), " (0x");
  sprintf((*buf)+strlen(*buf), "%04X", message->commandCode);

  strcat((*buf), ")\nReserved: ");
  jausUnsignedShortToString(message->properties.reserved, (*buf)+strlen(*buf));

  strcat((*buf), "\nVersion: ");
  switch(message->properties.version)
  {
    case 0:
      strcat((*buf), "2.0 and 2.1 compatible");
      break;
    case 1:
      strcat((*buf), "3.0 through 3.1 compatible");
      break;
    case 2:
      strcat((*buf), "3.2 and 3.3 compatible");
      break;
    default:
      strcat((*buf), "Reserved for Future: ");
      jausUnsignedShortToString(message->properties.version, (*buf)+strlen(*buf));
      break;
  }

  strcat((*buf), "\nExp. Flag: ");
  if(message->properties.expFlag == 0)
    strcat((*buf), "JAUS");
  else 
    strcat((*buf), "Experimental");
  
  strcat((*buf), "\nSC Flag: ");
  if(message->properties.scFlag == 0)
    strcat((*buf), "Service Connection");
  else
    strcat((*buf), "Not Service Connection");
  
  strcat((*buf), "\nACK/NAK: ");
  switch(message->properties.ackNak)
  {
  case 0:
    strcat((*buf), "None");
    break;
  case 1:
    strcat((*buf), "Request ack/nak");
    break;
  case 2:
    strcat((*buf), "nak response");
    break;
  case 3:
    strcat((*buf), "ack response");
    break;
  default:
    break;
  }
  
  strcat((*buf), "\nPriority: ");
  if(message->properties.priority < 12)
  {
    strcat((*buf), "Normal Priority ");
    jausUnsignedShortToString(message->properties.priority, (*buf)+strlen(*buf));
  }
  else
  {
    strcat((*buf), "Safety Critical Priority ");
    jausUnsignedShortToString(message->properties.priority, (*buf)+strlen(*buf));
  }
  
  strcat((*buf), "\nSource: ");
  jausAddressToString(message->source, (*buf)+strlen(*buf));
  
  strcat((*buf), "\nDestination: ");
  jausAddressToString(message->destination, (*buf)+strlen(*buf));
  
  strcat((*buf), "\nData Size: ");
  jausUnsignedIntegerToString(message->dataSize, (*buf)+strlen(*buf));
  
  strcat((*buf), "\nData Flag: ");
  jausUnsignedIntegerToString(message->dataFlag, (*buf)+strlen(*buf));
  switch(message->dataFlag)
  {
    case 0:
      strcat((*buf), " Only data packet in single-packet stream");
      break;
    case 1:
      strcat((*buf), " First data packet in muti-packet stream");
      break;
    case 2:
      strcat((*buf), " Normal data packet");
      break;
    case 4:
      strcat((*buf), " Retransmitted data packet");
      break;
    case 8:
      strcat((*buf), " Last data packet in stream");
      break;
    default:
      strcat((*buf), " Unrecognized data flag code");
      break;
  }
  
  strcat((*buf), "\nSequence Number: ");
  jausUnsignedShortToString(message->sequenceNumber, (*buf)+strlen(*buf));
  
  return (int)strlen(*buf);
  
}
