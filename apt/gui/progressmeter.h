// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: progressmeter.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Progress Meter - Graphical progress meter from the package library's 
                    progress meter class
   
   ##################################################################### */
									/*}}}*/
#ifndef PROGRESSMETER_H
#define PROGRESSMETER_H

#include <apt-pkg/progress.h>
#include <deity/window.h>
#include <deity/progress.h>
#include <deity/textwidg.h>

class ProgressWindow : public GUIWindow
{
   protected:
   
   Progress *Meter;
   TextWidget *Operation;
   TextWidget *SubOp;
   Separator *Divider;
   
   virtual void Realize();
   
   public:
   
   void SetOp(string Op) {Operation->Text(Op);};
   void SetSubOp(string Op) {SubOp->Text(Op);};
   void Percent(double Per) {Meter->Percent(Per);};
   
   ProgressWindow(string Title,Widget *Parent);
   virtual ~ProgressWindow();
};

class ProgressMeter : public OpProgress
{
   virtual void Update();
   ProgressWindow *Win;
   
   public:

   virtual void Done();
   
   ProgressMeter(string Title,Widget *Parent);
   virtual ~ProgressMeter();
};

#endif
