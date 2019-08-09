#define NOMINMAX

#include "ErrorHandling.h"
#include "CompressT4.h"
#include "T4Coder.h"
#include "T4Decoder.h"

namespace COMP
{

Util::CDataFieldCompressedImage CCompressT4::Compress( 
			const Util::CDataFieldUncompressedImage & i_uncompressedImage
			)
			const
{
	COMP_TRYTHIS
	CT4Coder c( i_uncompressedImage );
	c.CodeBuffer();
	return c.GetCompressedImage();
	COMP_CATCHTHIS
}

void DecompressT4( const Util::CDataFieldCompressedImage   & i_compressedImage, 
	 				     Util::CDataFieldUncompressedImage & o_decompressedImage,
						 std::vector<short>           & o_QualityInfo)
{	
	COMP_TRYTHIS
	CT4Decoder d( i_compressedImage);  // initialize the beast
		                               // and autosize if needed
	d.DecodeBuffer();                  // decode & fill QualityInfo array
	o_decompressedImage = d.GetDecompressedImage(); 
	o_QualityInfo       = d.GetQualityInfo(); 
	COMP_CATCHTHIS
}

} // end namespace

