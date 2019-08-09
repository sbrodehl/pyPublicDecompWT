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

#ifndef _ERRORHANDLING_H_
#define _ERRORHANDLING_H_

#include <string>
#include <errno.h>


namespace Util
{
/**
 * Base class for DaSS exception class
**/
class CBaseException
{
 public:
 /**
 * constructor
 * @param 
 * ErrorCode the error code
**/
   CBaseException(int ErrorCode, std::string additionalInformation=""):
   	m_ErrorCode(ErrorCode), m_ErrorMessage(""),m_AdditionalInformation(additionalInformation)
	{};
 /**
 * destructor
**/
    virtual ~CBaseException(){};
/**
 * returns the Errorcode
 * @returns 
 * the errorcode
**/
    int GetErrorCode() const {return m_ErrorCode;};
/**
 * returns the corresponding Error-message
 * @returns 
 * corresponding error-message
**/
    std::string GetErrorMessage() const {return ( m_ErrorMessage + m_AdditionalInformation );};
 protected:
    int m_ErrorCode;
    std::string m_ErrorMessage;
    /**
    If additional information should be provided, it can be filled into this 
    member
    */
    std::string m_AdditionalInformation;
};

/**
 * Base class for DaSS exception class
**/
class CCLibException: public CBaseException
{
public:
 /**
 * constructor reads errno and 
 * sets the ErrroMessage to the corresponding error string
**/
    CCLibException();
 /**
 * destructor
**/
    virtual ~CCLibException(){};
};

/**
 * Exception class for user defined error messages
 *
**/
class CNamedException : public CBaseException
{
public:
/**
 * This constructor takes the text which should appear in case of an error.
 * @param				
 * name message that should appear
 **/	
    CNamedException(const char*        i_Name) throw() : CBaseException(0) {m_ErrorMessage=i_Name;}
    CNamedException(const std::string& i_Name) throw() : CBaseException(0) {m_ErrorMessage=i_Name;}
};


/**
 *  Error-handling for published DaSS-C/S library (TBD)
 **/
#ifdef DASS_C_S
/**
 * logs the exception to (TBD)
 * @param
 * file the source file name 
 * line the line number
 **/
    inline void LogException(const char* file, int line){};

/**
 * logs the logs the error message to (TBD)
 * @param
 * excp the excp class object
 **/
    inline void LogError(const CBaseException& excp){};

/**
 * Error-handling for internal DaSS C++ software
 **/    
#else
/**
 * logs the exception to log4cpp
 * @param
 * file the source file name 
 * line the line number
 **/
    void LogException(const char* file, int line);

/**
 * logs the logs the error message to log4cpp
 * @param
 * excp the excp class object
 **/
    void LogError(const CBaseException& excp);
#endif


}

/**
 * This macro logs an exception and raises it again.
 **/
#define LOGCATCHANDTHROW Util::LogException(__FILE__, __LINE__); throw;
/**
 * This macro logs an exception 
 **/
#define LOGCATCH Util::LogException(__FILE__, __LINE__);


/** 
 * This macro raises an exception if the condition a is false.
 * @param a condition
 * @param e exception
 **/
#define Assert( a, e ) if (!(a)) { Util::LogException(__FILE__, __LINE__); Util::LogError(e); throw e;}
#define AssertNamed(a,name) Assert( a, Util::CNamedException(name))
#define AssertCLib(a) Assert(a, Util::CCLibException())
/** 
 *This condition should have been true before the method was called
 **/
#define PRECONDITION(p) Assert(p, Util::CNamedException( #p ));

#endif
