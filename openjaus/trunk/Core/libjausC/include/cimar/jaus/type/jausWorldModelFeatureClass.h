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
// File Name: jausWorldModelFeatureClass.h
//
// Written By: Danny Kent (jaus AT dannykent DOT com)
//
// Version: 3.2
//
// Date: 08/04/06
//
// Description: This file describes all the functionality associated with a JausWorldModelFeatureClass. 
// FeatureClasses are used to support metadata and attributes associated with objects in the JausWorldModel message set.

#ifndef JAUS_WM_FEATURE_CLASS_H
#define JAUS_WM_FEATURE_CLASS_H

#define JAUS_WM_FC_METADATA_STRING_LENGTH 65535

#include "cimar/jaus.h"

// ************************************************************************************************************************************
//			JausWorldModelFeatureClassAttribute
// ************************************************************************************************************************************
typedef struct
{
	JausByte dataType;					// Enumeration, see above
	union								// Union of possible type values
	{
		JausByte byteValue;
		JausShort shortValue;
		JausInteger integerValue;
		JausLong longValue;
		JausUnsignedShort unsignedShortValue;
		JausUnsignedInteger unsignedIntegerValue;
		JausUnsignedLong unsignedLongValue;
		JausFloat floatValue;
		JausDouble doubleValue;
		struct
		{
			JausByte redValue;
			JausByte greenValue;
			JausByte blueValue;
		};
	};
}JausWorldModelFeatureClassAttributeStruct;
typedef JausWorldModelFeatureClassAttributeStruct *JausWorldModelFeatureClassAttribute;

// JausWorldModelFeatureClassAttribute Constructor
JausWorldModelFeatureClassAttribute featureClassAttributeCreate(void);

// JausWorldModelFeatureClassAttribute Constructor (from Buffer)
JausWorldModelFeatureClassAttribute featureClassAttributeFromBuffer(unsigned char *buffer, unsigned int bufferSizeBytes);

// JausWorldModelFeatureClassAttribute Destructor
void featureClassAttributeDestroy(JausWorldModelFeatureClassAttribute attribute);

// JausWorldModelFeatureClassAttribute To Buffer
JausBoolean featureClassAttributeToBuffer(JausWorldModelFeatureClassAttribute attribute, unsigned char *buffer, unsigned int bufferSizeBytes);

// JausWorldModelFeatureClassAttribute Buffer Size
unsigned int featureClassAttributeSizeBytes(JausWorldModelFeatureClassAttribute attribute);

// JausWorldModelFeatureClassAttribute Copy
JausWorldModelFeatureClassAttribute featureClassAttributeCopy(JausWorldModelFeatureClassAttribute attribute);

// JausWorldModelFeatureClassAttribute ToString
int featureClassAttributeToString(JausWorldModelFeatureClassAttribute attribute, char *string);

// JausWorldModelFeatureClassAttribute GetValue
JausUnsignedLong featureClassAttributeGetValue(JausWorldModelFeatureClassAttribute attribute);

// ************************************************************************************************************************************
//			FeatureClass
// ************************************************************************************************************************************
typedef struct
{
	JausUnsignedShort id;										// Enumeration, defined by system
	char metaData[JAUS_WM_FC_METADATA_STRING_LENGTH];			// String of Metadata information defined for this Feature Class
	JausWorldModelFeatureClassAttribute attribute;				// Feature Class Attribute data type and value
}JausWorldModelFeatureClassStruct;
typedef JausWorldModelFeatureClassStruct *JausWorldModelFeatureClass;

// JausWorldModelFeatureClass Constructor
JausWorldModelFeatureClass featureClassCreate(void);

// JausWorldModelFeatureClass Constructor (from Buffer)
JausWorldModelFeatureClass featureClassFromBuffer(unsigned char *buffer, unsigned int bufferSizeBytes);

// JausWorldModelFeatureClass Destructor
void featureClassDestroy(JausWorldModelFeatureClass fcClass);

// JausWorldModelFeatureClass To Buffer
JausBoolean featureClassToBuffer(JausWorldModelFeatureClass fcClass, unsigned char *buffer, unsigned int bufferSizeBytes);

// JausWorldModelFeatureClass Buffer Size
unsigned int featureClassSizeBytes(JausWorldModelFeatureClass fcClass);

// JausWorldModelFeatureClass Copy
JausWorldModelFeatureClass featureClassCopy(JausWorldModelFeatureClass fcClass);

#endif
