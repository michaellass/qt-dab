#
//
//	This LUT implementation of atan2 is a C++ translation of
//	a Java discussion on the net
//	http://www.java-gaming.org/index.php?topic=14647.0

#pragma once

#include	<math.h>
#include	<stdio.h>
#include	<stdint.h>
#include	<cstdlib>
#include	<limits>
#include	"dab-constants.h"
#
class	compAtan {
public:
		compAtan	(void);
		~compAtan	(void);
	float	atan2		(float, float);
	float	argX		(Complex);
private:
	float	*ATAN2_TABLE_PPY;
	float	*ATAN2_TABLE_PPX;
	float	*ATAN2_TABLE_PNY;
	float	*ATAN2_TABLE_PNX;
	float	*ATAN2_TABLE_NPY;
	float	*ATAN2_TABLE_NPX;
	float	*ATAN2_TABLE_NNY;
	float	*ATAN2_TABLE_NNX;
	float	Stretch;
};
