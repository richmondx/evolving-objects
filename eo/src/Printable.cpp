#ifdef _MSC_VER
// to avoid long name warnings
#pragma warning(disable:4786)
#endif 

//-----------------------------------------------------------------------------
// eoPrintable.cpp
//-----------------------------------------------------------------------------

#include <eoPrintable.h>

namespace eo
{

    //-----------------------------------------------------------------------------
    //Implementation of these objects
    //-----------------------------------------------------------------------------

    std::ostream & operator << ( std::ostream& _os, const eoPrintable& _o ) {
	_o.printOn(_os);
	return _os;
    }

    //-----------------------------------------------------------------------------

}
