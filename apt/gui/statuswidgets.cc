// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: statuswidgets.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   StatusWidgets - The right hand status display widgets
  
   These show various bits of information derived from the package
   control block.
   
   ##################################################################### */
									/*}}}*/
#include "statuswidgets.h"
#include <system.h>

// MultiLineWidget::Render - Draw the description text with wrapping	/*{{{*/
// ---------------------------------------------------------------------
/* */
void MultiLineWidget::Render(CombinedGC &GC)
{
   /*
   BasicRender(GC,true);
   
   if (Elm == 0)
      return;
   
   vector<string> Lines;
   Elm->GetLines(Lines);

   AbsRect Loc(BorderX,BorderY,Pos.w - BorderX,Pos.h - BorderY);
   GC->SetColor(Wc_Blue);
   Loc.y1 = GC->DrawWrappedText(Loc,Elm->Value(),true);
   GC->SetColor(iColor);
   for (vector<string>::iterator I = Lines.begin(); I < Lines.end();)
   {
      string Tmp = *I;
      for (I++; I != Lines.end(); I++)
      {
	 const char *J = (*I).begin();
	 for (;J != (*I).end() && *J == ' '; J++);
	 if (J == (*I).end())
	 {
	    I++;
	    break;
	 }
	 
	 // * indicates a list
	 if (*J == '*')
	    break;
	 
	 Tmp += ' ';
	 Tmp += string(J,(*I).end() - J);
      }
      
      Loc.y1 = GC->DrawWrappedText(Loc,Tmp,true);
   }*/
}
									/*}}}*/

// InfoWidget::InfoWidget - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
InfoWidget::InfoWidget(Widget *Parent) : BasicWidget(Parent) 
{
   if (GraphicGC::GC != 0)
      Margins(Point(1,1));
   BorderWidth(0);
};
									/*}}}*/
// InfoWidget::Render - Draw all the single fields			/*{{{*/
// ---------------------------------------------------------------------
/* */
void InfoWidget::Render(CombinedGC &GC)
{
/*   BasicRender(GC,true);
  
   if (Info.isNull() == true)
      return;

   long Height = GC->ExtentText(" ").h;
   Point Pos(BorderX + iMargins.x,BorderY + iMargins.y);
   for (const pkgElement **I = List; I != List + _count(List); I++)
   {
      if (*I == 0)
	 continue;
      
      string Tag = (*I)->Tag() + ": ";
      string Value = (*I)->Value();
      long Width = GC->ExtentText(Tag).w;
      GC->SetColor(Wc_Blue);
      GC->DrawString(Pos,Tag);
      GC->SetColor(iColor);
      
      if (Width + GC->ExtentText(Value).w < this->Pos.w)
      {
	 GC->DrawString(Point(Pos.x + Width,Pos.y),Value);
	 Pos.y += Height;
      }
      else
      {
	 Pos.y += Height;
	 Pos.y = GC->DrawWrappedText(AbsRect(Pos.x+Height,Pos.y,
					     this->Pos.w,this->Pos.h),
				     Value,false);
      }      
   }*/
}
									/*}}}*/
// InfoWidget::IdealSize - Returns the proper size for the info widget	/*{{{*/
// ---------------------------------------------------------------------
/* */
Point InfoWidget::IdealSize()
{
   return Point(Parent->Pos.w,Parent->Pos.h);
}
									/*}}}*/
