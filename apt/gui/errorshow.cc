// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: errorshow.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

 Error Show - Show errors from the apt-pkg error class.

   ##################################################################### */
									/*}}}*/
// Include								/*{{{*/
#include "errorshow.h"

#include <deity/window.h>
#include <deity/textwidg.h>
#include <deity/widget-thread.h>
#include <deity/button.h>
#include <deity/utils.h>

#include <apt-pkg/error.h>
									/*}}}*/

// ShowErrors - Show any errors that may have come up			/*{{{*/
// ---------------------------------------------------------------------
/* If this returns true then a fatal error has occured and things should
   be aborted. */
bool ShowErrors(string Reason,bool Fatal)
{
   // Nothing to show.
   if (_error->empty() == true)
      return false;
   
   Widget::Lock Lock;

   // Create the dialog window
   GUIWindow *Win;
   if (Fatal == true)
      Win = new GUIWindow("Fatal Error");
   else
      Win = new GUIWindow("Error");
      
   // Create the OK button
   TextButton *OkButton;
   if (Fatal == true)
      OkButton = new TextButton("Exit",Win);
   else
      OkButton = new TextButton("Ok",Win);
   
   SNotify *Trigger;
   OkButton->Add(Trigger = new SNotify(Nt_Action));
   OkButton->Extent();
   
   // Add each string element to the dialog
   long Spacer = OkButton->IdealSize().y;
   long H = Spacer/2;
   if (TextGC::GC != 0)
      H = 1;
   long Width = (long)(Widget::Root->Size().w*0.90) - 2*Spacer;

   // Generate widgets for each of the error codes
   int Narrow = -1;
   while (true)
   {      
      WrapedText *String = new WrapedText(Reason,Win);
      
      // Narrow the positions of the raw error texts a bit
      if (Narrow == 1)
	 String->Resize(Rect(Spacer + Spacer/2,H,Width - Spacer/2,1));
      else
	 String->Resize(Rect(Spacer,H,Width,1));
	 	 
      /* This extenting mode makes sure the widget takes the correct amount
         of space in all cases. */
      String->ExtentMode(Widget::ExtentAlways,Widget::ExtentAlways);
      
      // We are now showing error texts not the title
      if (Narrow == -1 && Reason.empty() == false)
      {
	 /* We also change the colour and use a bigger font for the
	    main error message */
	 String->Foreground(Wc_Blue);
	 String->Font(SimpleFont("helvetica",140));
	 Width = String->IdealSize().x;
	 Narrow = 1;
      }      
      else 
	 Narrow = 0;      

      H += String->IdealSize().y;
      if (_error->empty() == true)
	 break;
      _error->PopMessage(Reason);
   }   

   // Resize the window and position the OK button
   Point WinSize = ChildrenExtent(Win);
   OkButton->Resize(Rect((WinSize.x + Spacer - OkButton->Size().w)/2,
			 H + Spacer/2,OkButton->Size().w,OkButton->Size().h));		    
   Win->Resize(Rect(0,0,WinSize.x + Spacer,WinSize.y + 2*Spacer));
   CenterWidget(Win,Widget::Root->Size());   
   Win->RealizeFamily();
   
   Trigger->Wait();

   delete Win;
   
   if (Fatal == true)
      exit(100);

   return false;
}
									/*}}}*/
