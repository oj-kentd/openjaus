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
//File Name: ReleaseComponentControlMessage.java
//
// Written By: Tom Galluzzo (galluzzo@gmail.com)
//
// Version: 3.2  
//
// Date: 07/04/06
//

package jaus.message.command;
import jaus.message.*;

public class ReleaseComponentControlMessage extends JausMessage
{    
	private static final int COMMAND_CODE = RELEASE_COMPONENT_CONTROL;
	private static final int MAX_DATA_SIZE_BYTES = 0;
   
	public ReleaseComponentControlMessage()
	{
		super();
	}
	
	public ReleaseComponentControlMessage(byte[] buffer)
	{
		super(buffer, 0);
	}
	
	public ReleaseComponentControlMessage(byte[] buffer, int index)
	{
	    super(buffer, index);
	}

	public ReleaseComponentControlMessage(JausMessage jausMessage)
	{
	    super(jausMessage);
	}
	
	public void setCommandCode(int commandCode)
	{
	    this.commandCode = COMMAND_CODE;
	}	
	
}