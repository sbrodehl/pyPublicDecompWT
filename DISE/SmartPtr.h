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

#ifndef _SMARTPTR_H_
#define _SMARTPTR_H_


#include "ErrorHandling.h"
namespace Util
{

/**
 * thread safe pointer class
**/
template<class T> class CSmartPtr
{

private:
/**
 * Holds address of heap memory and maintains counter of references to it.
**/
    class CCounted
	{
	private:
	    T*	m_Ptr;	/** Start address of heap memory.**/
        unsigned long m_NumReferences;
	public:
/**
 * Stores address of heap memory, initializes number of references.
**/
	    CCounted(T* i_Ptr)
		{
		    m_Ptr = i_Ptr;
            m_NumReferences = 0;
		}

/**
 * Increments number of references.
 **/
	    void Use()
		{
                    m_NumReferences++;
		}

/**
 * Decrements number of references,deletes itself when last reference dismisses.
**/
	    void Dismiss()
		{
		    if (0 == --m_NumReferences)
		    {
			delete m_Ptr;
			delete this;
		    }
		}

/**
 * Returns heap memory address and set sets own copy of the address to NULL to prevent deletion.
 * @returns 
 * Heap memory address
**/
		T* Release()
		    {
/** Return the heap memory address.**/
			T* t = m_Ptr;
			m_Ptr = NULL;
			return t;
		}


/**
 * Operators to access the heap memory.
 * @returns 
 * Address or value of dynamic object
**/
		operator T&() const { return *m_Ptr; }
		operator T*() const { return  m_Ptr; }
		T* operator-> () const { return  m_Ptr; }

	};

    CCounted*	m_Counted;/** Start address of heap memory and reference counter.**/

public:

/**
 * constructor
**/
    explicit CSmartPtr
	(
	    T* i_Ptr = NULL	/** Start address of heap memory.**/
	)
	{
	    m_Counted = new CCounted(i_Ptr);
	    m_Counted->Use();
	}

/**
 * copy constructor
**/	CSmartPtr
	(
	    const CSmartPtr<T>& i_Src	/** Reference CSmartPtr.**/
	)
	{
		m_Counted = i_Src.m_Counted;
		m_Counted->Use();
	}

/**
 * destructor
**/
	~CSmartPtr()
	{
		m_Counted->Dismiss();
	}


/**
 * Assignment operator.
 * @returns 
 * Reference to itself.
**/	
	CSmartPtr<T>& operator=
	(
	    T* i_Ptr	/** Start address of heap memory.**/
	)
	{
		m_Counted->Dismiss();
		m_Counted = new CCounted(i_Ptr);
		m_Counted->Use();
		return *this;
	}

/**
 * Assignment operator.
 * @returns 
 * Reference to itself.
**/	CSmartPtr<T>& operator=
	(
	    const CSmartPtr<T>& i_Src	/** Reference CSmartPtr.**/
	)
	{
		m_Counted->Dismiss();
		m_Counted = i_Src.m_Counted;
		m_Counted->Use();
		return *this;
	}
/**
 *compare < operator. Will call the operator for the data itself.
 * @returns 
 * true if this object is'less than' (in terms of the defined data-operator)
**/
	bool operator <
	(
	    const CSmartPtr<T>& i_Src	/** Reference CSmartPtr.**/
	)
	{
		return *get() < *(i_Src.get());
	}

/**
 * Operators and functions to access the heap memory.
 * @returns 
 * Address or value of dynamic object.
**/
	   operator T&() const	{ return m_Counted->operator T&();	}
	   operator T*() const	{ return m_Counted->operator T*();	}
	T* operator-> () const	{ return m_Counted->operator->();	}
	T* Get()         const	{ return m_Counted->operator->();	}

/**
 *Only for easier conversion of former auto_ptr's.
 *
**/

	T* get()         const	{ return m_Counted->operator->();	}


/**
 * Returns heap memory address and set sets own copy of the address to NULL to prevent deletion.
 * @returns 
 * 	Heap memory address.
**/

	T* Release
	(
	)
	{
		return m_Counted->Release();
	}

/**
 * Casts a Util::CSmartPtr<F> object to a Util:CSmartPtr<T> object.
 * Note that this will actually release the original and set it to NULL.
 * @returns 
 * Util::CSmartPtr<T>.
 * - the cast operation is not possible because the classes are not suitably related, or 
 * - the original CSmartPtr contains a NULL pointer.
**/
	template<class F> inline static Util::CSmartPtr<T> Cast
	(
		Util::CSmartPtr<F> i_From
	)
	{
		T* to = NULL;
		if (dynamic_cast<T*>(i_From.Get()))
			to = dynamic_cast<T*>(i_From.Release());
		return Util::CSmartPtr<T>(to);
	}

};



/**
 * Casts a Util::CSmartPtr<CFrom> object to a Util:CSmartPtr<CTo> object.
 * Note that this will actually release the original and set it to NULL.
 * @returns 
 * Util::CSmartPtr<T>.
 * - the cast operation is not possible because the classes are not suitably related, or 
 * - the original CSmartPtr contains a NULL pointer.
 * NOTE:
 * Function exists only for backward compatibility. 
 * Rather use the CSmartPtr<CTo>::Cast() function directly!
**/
template<class CTo, class CFrom> inline Util::CSmartPtr<CTo> SmartPtrCast
(
	Util::CSmartPtr<CFrom> i_From
)
{
	try
	{
		return Util::CSmartPtr<CTo>::Cast(i_From);
	}
	catch (...)
	{
		LOGCATCHANDTHROW;
	}
}




} 


#endif
