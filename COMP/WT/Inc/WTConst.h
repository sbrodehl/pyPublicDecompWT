#ifndef WTConst_included
#define WTConst_included

namespace COMP 
{

const unsigned int c_ACNbBits = 31;		// size in bits of the arithmetic coding range

// Marker values
const unsigned __int16 c_MarkerHEADER	= 0xFF01;
const unsigned __int16 c_MarkerDATA		= 0xFF02;
const unsigned __int16 c_MarkerFOOTER	= 0xFF03;
const unsigned __int16 c_MarkerRESTART	= 0xFFE0;

} // end namespace

#endif
