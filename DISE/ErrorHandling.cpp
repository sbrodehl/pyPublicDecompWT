/*------------------------------------------------------------------------------------------
* Columbus Control Center
*
* Data Services Subsystem DaSS
*
* This software is developed by VCS AG under contract for DLR, contract no 6-057-2272.
* 
* The copyright in this document is vested VCS Nachrichtentechnik GmbH. This document may only be
* reproduced in whole or in part, or stored in a retrieval system, or transmitted in any form, or by any
* means electronic, mechanical, photocopying or otherwise, either with the prior permission of VCS AG
* or in accordance with the terms of DLR Contract no 6-057-2272.
*
*
--------------------------------------------------------------------------------------------
****  DaSS MODULE HEADER   ***

TYPE:			// to be added

PURPOSE:		// to be added

FUNCTION:		// to be added

INTERFACES:	// to be added

RESOURCES:		// to be added

PROCESSING:	// to be added

DATA:			// to be added

LOGIC:		// to be added

****  END MODULE HEADER   ***
------------------------------------------------------------------------------------------*/

#include "ErrorHandling.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>



void Util::LogException(const char* file, int line)
{

    std::ostringstream trcMsg;

    trcMsg << "Software Exception Trace (Source: "
	   << file 
	   << ", Line: "
	   << line 
	   << ")";


    std::cout << trcMsg.str() << std::endl;


};

void Util::LogError(const CBaseException& excp)
{
    std::ostringstream trcMsg;
    trcMsg << "Software Exception!  Error Code: "
	   << excp.GetErrorCode()
	   << ", Error Message: "
	   << excp.GetErrorMessage();
   
    std::cout << trcMsg.str() << std::endl;

};



Util::CCLibException::CCLibException():CBaseException(errno)
{
//    to implement
// get the system error message
    m_ErrorMessage = std::string(strerror(m_ErrorCode));
}





