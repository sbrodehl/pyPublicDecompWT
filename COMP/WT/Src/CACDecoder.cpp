#include "CACDecoder.h"


void COMP::CACDecoder::UpdateInterval()
{
	COMP_TRYTHIS_SPEED
	unsigned int nbBit = 0U;
	do
	{
		m_Range += m_Range;
		nbBit++;
	}
	while (m_Range <= c_FirstQtr);
	m_Value <<= nbBit;
	m_Value += InputBits (nbBit);
	COMP_CATCHTHIS_SPEED
}


unsigned int COMP::CACDecoder::DecodeSymbol
(
	CACModel& i_Mod	// the multi-symbols probability model
)
{
	COMP_TRYTHIS_SPEED
	unsigned int index = 1U;
	const unsigned __int32 r = m_Range / i_Mod.GetCumFreq(0);
	unsigned __int32 rLps = i_Mod.GetCumFreq(index) * r;
	while (rLps > m_Value)
		rLps = i_Mod.GetCumFreq(++index) * r;
	const unsigned int symbol = i_Mod.GetSymbol(index);
	m_Value -= rLps;
	if (index == 1U)
	{
		m_Range -= rLps;
		i_Mod.UpdateMps ();
	}
	else
	{
		m_Range = i_Mod.GetFreq(index) * r;
		i_Mod.UpdateLps (index);
	}
	if (m_Range <= c_FirstQtr) UpdateInterval ();
	return symbol;
	COMP_CATCHTHIS_SPEED
}


