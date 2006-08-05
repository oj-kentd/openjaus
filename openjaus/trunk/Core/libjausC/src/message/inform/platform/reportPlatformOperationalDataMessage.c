/*****************************************************************************
 *  Copyright (c) 2006, University of Florida.
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
// File Name: reportPlatformOperationalDataMessage.c
//
// Written By: Danny Kent (jaus AT dannykent DOT com), Tom Galluzzo (galluzzo AT gmail DOT com)
//
// Version: 3.2
//
// Date: 08/04/06
//
// Description: This file defines the functionality of a ReportPlatformOperationalDataMessage



#include <stdlib.h>
#include <string.h>
#include "cimar/jaus.h"

static const int commandCode = JAUS_REPORT_PLATFORM_OPERATIONAL_DATA;
static const int maxDataSizeBytes = 10;

static JausBoolean headerFromBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static JausBoolean headerToBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);

static JausBoolean dataFromBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static int dataToBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes);
static void dataInitialize(ReportPlatformOperationalDataMessage message);

// ************************************************************************************************************** //
//                                    USER CONFIGURED FUNCTIONS
// ************************************************************************************************************** //

// Initializes the message-specific fields
static void dataInitialize(ReportPlatformOperationalDataMessage message)
{
	// Set initial values of message fields
	message->presenceVector = newJausBytePresenceVector();
	message->engineTemperatureCelsius = newJausDouble(0);	// Scaled Short (-75, 180)
	message->odometerMeters  = newJausUnsignedInteger(0);
	message->batteryVoltagePercent = newJausDouble(0);		// Scaled Byte (0, 127)
	message->fuelLevelPercent = newJausDouble(0);			// Scaled Byte (0, 100)
	message->oilPressurePercent = newJausDouble(0);			// Scaled Byte (0, 127)
}

// Return boolean of success
static JausBoolean dataFromBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	int index = 0;
	JausShort tempShort;
	JausByte tempByte;
	
	if(bufferSizeBytes == message->dataSize)
	{
		// Unpack Message Fields from Buffer
		// Use Presence Vector
		if(!jausBytePresenceVectorFromBuffer(&message->presenceVector, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_PRESENCE_VECTOR_SIZE_BYTES;
		
		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_ENGINE_BIT))
		{
			// unpack
			if(!jausShortFromBuffer(&tempShort, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_SHORT_SIZE_BYTES;
			
			// Scaled Short (-75, 180)
			message->engineTemperatureCelsius = jausShortToDouble(tempShort, -75, 180);
		}

		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_ODOMETER_BIT))
		{
			// unpack
			if(!jausUnsignedIntegerFromBuffer(&message->odometerMeters, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_INTEGER_SIZE_BYTES;
		}

		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_BATTERY_BIT))
		{
			// unpack
			if(!jausByteFromBuffer(&tempByte, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
			
			// Scaled Byte (0, 127)
			message->batteryVoltagePercent = jausByteToDouble(tempShort, 0, 127);
		}

		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_FUEL_BIT))
		{
			// unpack
			if(!jausByteFromBuffer(&tempByte, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
			
			// Scaled Byte (0, 100)
			message->fuelLevelPercent = jausByteToDouble(tempShort, 0, 100);
		}

		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_OIL_BIT))
		{
			// unpack
			if(!jausByteFromBuffer(&tempByte, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
			
			// Scaled Byte (0, 127)
			message->oilPressurePercent = jausByteToDouble(tempShort, 0, 127);
		}
		return JAUS_TRUE;
	}
	else
	{
		return JAUS_FALSE;
	}
}

// Returns number of bytes put into the buffer
static int dataToBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	int index = 0;
	JausShort tempShort;
	JausByte tempByte;

	if(bufferSizeBytes >= message->dataSize)
	{
		// Pack Message Fields to Buffer
		// Use Presence Vector
		if(!jausBytePresenceVectorToBuffer(message->presenceVector, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
		index += JAUS_BYTE_PRESENCE_VECTOR_SIZE_BYTES;
		
		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_ENGINE_BIT))
		{
			// pack
			// Scaled Short (-75, 180)
			tempShort = jausShortFromDouble(message->engineTemperatureCelsius, -75, 180);
			
			if(!jausShortToBuffer(tempShort, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_SHORT_SIZE_BYTES;
		}

		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_ODOMETER_BIT))
		{
			if(!jausUnsignedIntegerToBuffer(message->odometerMeters, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_UNSIGNED_INTEGER_SIZE_BYTES;
		}

		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_BATTERY_BIT))
		{
			// pack
			// Scaled Byte (0, 127)
			tempByte = jausByteFromDouble(message->batteryVoltagePercent, 0, 127);
			
			if(!jausByteToBuffer(tempByte, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}
		
		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_FUEL_BIT))
		{
			// pack
			// Scaled Byte (0, 100)
			tempByte = jausByteFromDouble(message->fuelLevelPercent, 0, 100);
			
			if(!jausByteToBuffer(tempByte, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}
		
		if(jausBytePresenceVectorIsBitSet(message->presenceVector, JAUS_OPERATIONAL_PV_OIL_BIT))
		{
			// pack
			// Scaled Byte (0, 127)
			tempByte = jausByteFromDouble(message->oilPressurePercent, 0, 127);
			
			if(!jausByteToBuffer(tempByte, buffer+index, bufferSizeBytes-index)) return JAUS_FALSE;
			index += JAUS_BYTE_SIZE_BYTES;
		}
	}

	return index;
}

// ************************************************************************************************************** //
//                                    NON-USER CONFIGURED FUNCTIONS
// ************************************************************************************************************** //

ReportPlatformOperationalDataMessage reportPlatformOperationalDataMessageCreate(void)
{
	ReportPlatformOperationalDataMessage message;

	message = (ReportPlatformOperationalDataMessage)malloc( sizeof(ReportPlatformOperationalDataMessageStruct) );
	if(message == NULL)
	{
		return NULL;
	}
	
	// Initialize Values
	message->priority = JAUS_DEFAULT_PRIORITY;
	message->ackNak = JAUS_ACK_NAK_NOT_REQUIRED;
	message->scFlag = JAUS_NOT_SERVICE_CONNECTION_MESSAGE;
	message->expFlag = JAUS_NOT_EXPERIMENTAL_MESSAGE;
	message->version = JAUS_VERSION_3_2;
	message->reserved = 0;
	message->commandCode = commandCode;
	message->destination = jausAddressCreate();
	message->source = jausAddressCreate();
	message->dataFlag = JAUS_SINGLE_DATA_PACKET;
	message->dataSize = maxDataSizeBytes;
	message->sequenceNumber = 0;
	
	dataInitialize(message);
	
	return message;	
}

void reportPlatformOperationalDataMessageDestroy(ReportPlatformOperationalDataMessage message)
{
	jausAddressDestroy(message->source);
	jausAddressDestroy(message->destination);
	free(message);
}

JausBoolean reportPlatformOperationalDataMessageFromBuffer(ReportPlatformOperationalDataMessage message, unsigned char* buffer, unsigned int bufferSizeBytes)
{
	int index = 0;
	
	if(!strncmp((char *)buffer, JAUS_UDP_HEADER, JAUS_UDP_HEADER_SIZE_BYTES)) // equals 1 if same
	{
		index += JAUS_UDP_HEADER_SIZE_BYTES;
	}

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

JausBoolean reportPlatformOperationalDataMessageToBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	if(bufferSizeBytes < reportPlatformOperationalDataMessageSize(message))
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
			return JAUS_FALSE; // headerToReportPlatformOperationalDataBuffer failed
		}
	}
}

JausBoolean reportPlatformOperationalDataMessageToUdpBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	if(bufferSizeBytes < reportPlatformOperationalDataMessageUdpSize(message))
	{
		return JAUS_FALSE; // improper size
	}
	else
	{
		strncpy((char *)buffer, JAUS_UDP_HEADER, JAUS_UDP_HEADER_SIZE_BYTES); //copies the UDP header into the buffer
		return reportPlatformOperationalDataMessageToBuffer(message, buffer+JAUS_UDP_HEADER_SIZE_BYTES, bufferSizeBytes - JAUS_UDP_HEADER_SIZE_BYTES);
	}
}

ReportPlatformOperationalDataMessage reportPlatformOperationalDataMessageFromJausMessage(JausMessage jausMessage)
{
	ReportPlatformOperationalDataMessage message;
	
	if(jausMessage->commandCode != commandCode)
	{
		return NULL; // Wrong message type
	}
	else
	{
		message = (ReportPlatformOperationalDataMessage)malloc( sizeof(ReportPlatformOperationalDataMessageStruct) );
		if(message == NULL)
		{
			return NULL;
		}
		
		message->properties = jausMessage->properties;
		message->commandCode = jausMessage->commandCode;
		message->destination = jausAddressCreate();
		message->destination->id = jausMessage->destination->id;
		message->source = jausAddressCreate();
		message->source->id = jausMessage->source->id;
		message->dataControl = jausMessage->dataControl;
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

JausMessage reportPlatformOperationalDataMessageToJausMessage(ReportPlatformOperationalDataMessage message)
{
	JausMessage jausMessage;
	
	jausMessage = (JausMessage)malloc( sizeof(JausMessageStruct) );
	if(jausMessage == NULL)
	{
		return NULL;
	}	
	
	jausMessage->properties = message->properties;
	jausMessage->commandCode = message->commandCode;
	jausMessage->destination = jausAddressCreate();
	jausMessage->destination->id = message->destination->id;
	jausMessage->source = jausAddressCreate();
	jausMessage->source->id = message->source->id;
	jausMessage->dataControl = message->dataControl;
	jausMessage->sequenceNumber = message->sequenceNumber;
	
	jausMessage->data = (unsigned char *)malloc(message->dataSize);
	jausMessage->dataSize = dataToBuffer(message, jausMessage->data, message->dataSize);
	
	return jausMessage;
}

unsigned int reportPlatformOperationalDataMessageUdpSize(ReportPlatformOperationalDataMessage message)
{
	return (unsigned int)(message->dataSize + JAUS_HEADER_SIZE_BYTES + JAUS_UDP_HEADER_SIZE_BYTES);
}

unsigned int reportPlatformOperationalDataMessageSize(ReportPlatformOperationalDataMessage message)
{
	return (unsigned int)(message->dataSize + JAUS_HEADER_SIZE_BYTES);
}

//********************* PRIVATE HEADER FUNCTIONS **********************//

static JausBoolean headerFromBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	if(bufferSizeBytes < JAUS_HEADER_SIZE_BYTES)
	{
		return JAUS_FALSE;
	}
	else
	{
		// unpack header
		message->properties = buffer[0] + (buffer[1] << 8);
		message->commandCode = buffer[2] + (buffer[3] << 8);
	
		message->destination->instance = buffer[4];
		message->destination->component = buffer[5];
		message->destination->node = buffer[6];
		message->destination->subsystem = buffer[7];
	
		message->source->instance = buffer[8];
		message->source->component = buffer[9];
		message->source->node = buffer[10];
		message->source->subsystem = buffer[11];
		
		message->dataControl = buffer[12] + (buffer[13] << 8);
		message->sequenceNumber = buffer[14] + (buffer[15] << 8);
		
		return JAUS_TRUE;
	}
}

static JausBoolean headerToBuffer(ReportPlatformOperationalDataMessage message, unsigned char *buffer, unsigned int bufferSizeBytes)
{
	if(bufferSizeBytes < JAUS_HEADER_SIZE_BYTES)
	{
		return JAUS_FALSE;
	}
	else
	{	
		buffer[0] = (unsigned char)(message->properties & 0xFF);
		buffer[1] = (unsigned char)((message->properties & 0xFF00) >> 8);

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
		
		buffer[12] = (unsigned char)(message->dataControl & 0xFF);
		buffer[13] = (unsigned char)((message->dataControl & 0xFF00) >> 8);

		buffer[14] = (unsigned char)(message->sequenceNumber & 0xFF);
		buffer[15] = (unsigned char)((message->sequenceNumber & 0xFF00) >> 8);
		
		return JAUS_TRUE;
	}
}

