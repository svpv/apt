// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: statuswidgets.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   StatusWidgets - The right hand status display widgets
  
   These show various bits of information derived from the package
   control block.
   
   ##################################################################### */
									/*}}}*/
#ifndef STATUSWIDGETS_H
#define STATUSWIDGETS_H

#include <deity/basic.h>

class MultiLineWidget : public BasicWidget
{
   virtual void Render(CombinedGC &GC);
   
   public:

   MultiLineWidget(Widget *Parent = 0) : BasicWidget(Parent) {};
};

class InfoWidget : public BasicWidget
{
   virtual void Render(CombinedGC &GC);
   
   public:

   virtual Point IdealSize();
   InfoWidget(Widget *Parent = 0);
};

#endif
