#ifndef STAR_TAttributes
#define STAR_TAttributes

// $Id: TAttributes.h,v 1.1 2014/10/29 17:24:39 perev Exp $
// *-- Author :    Valery Fine(fine@bnl.gov)   10/06/2009


///////////////////////////////////////////////////////////////////////
///
/*! \brief  Class TAttributes - to a container if the float/double/int/string 
     attributes that one access by the unique attribute name 
 */
/// \author Valery Fine (fine@bnl.gov)
/// \date 10/06/2009
///
///  Class TAttributes provides a container of the 
///  template<class T> pair<string,T>
///  the simple way to visualize the StMuDst event 
///  primitivies in 3D quickly against of the STAR detector 
///  geometry.
///  One instance of the class is instantiated as soon as the class shared library
///  is loaded.
///  This allows to use the class object (invoke the class methods) with one C++ statement. 
///  This  is to allow creating the 3D views "on fly", 
///  for example, from the GNU debugger (gdb) command prompt 
/// \n Try:
/// \code
///  > star.dev	
///  > ln -s  $STAR/QtRoot/qtExamples/QtGBrowser/.rootrc
///  > root.exe $STAR/StRoot/macros/mudst/draw3DTracks.C
/// \endcode
///  to  read some MuDst ROOT file and get the pictire below:
///  \htmlonly
///  <P>You need to install the <a href="http://get.adobe.com/reader/?promoid=BUIGO">Adobe Reader version 9 or higher
///  <img src="http://www.adobe.com/images/shared/download_buttons/get_adobe_reader.png"></a>
///  to be able to "click and see" the interactive ( zoom, pan, select / highlight the pieces, etc )  3D image also
///  <center>
///  <a href="http://www.star.bnl.gov/public/comp/vis/StDraw3D/TAttributes.pdf">
///  <img src="http://www.star.bnl.gov/public/comp/vis/StDraw3D/examples/StMuTracks.png"></a>
///  </center><p>
///  \endhtmlonly
///
///////////////////////////////////////////////////////////////////////
class TAttributes 
{
};

#endif
