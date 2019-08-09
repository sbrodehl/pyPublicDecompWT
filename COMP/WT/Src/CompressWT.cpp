#include "ErrorHandling.h"
#include "CompressWT.h"
#include "CWTCoder.h"
#include "CWTDecoder.h"

namespace COMP 
{
	
Util::CDataFieldCompressedImage CCompressWT::Compress (const Util::CDataFieldUncompressedImage &i_uncompressedImage)
const
{
	COMP_TRYTHIS
	CWTCoder c (i_uncompressedImage, (CWTParams)(*this));
	c.CodeBuffer ();
	return c.GetCompressedImage ();
	COMP_CATCHTHIS
}

void DecompressWT (const Util::CDataFieldCompressedImage &i_compressedImage,
				   const unsigned char &i_NR,
				   Util::CDataFieldUncompressedImage &o_decompressedImage,
				   std::vector<short> &o_QualityInfo)
{	
	COMP_TRYTHIS
	CWTDecoder d (i_compressedImage);	// initialize the beast
	d.DecodeBuffer ();	// decode & fill QualityInfo array
	o_decompressedImage = d.GetDecompressedImage (i_NR);
	o_QualityInfo = d.GetQualityInfo ();
	COMP_CATCHTHIS
}

} // end namespace

