// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: progressmeter.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Progress Meter - Graphical progress meter from the package library's 
                    progress meter class

   This is split into two halfs, one is a meter dialog window the 
   other is an interfacing module for the libapt-pkg progress class.
   It was designed to either run from a thread or directly within the
   main task.
   
   ##################################################################### */
									/*}}}*/
// Include								/*{{{*/
#include "progressmeter.h"
#include <deity/utils.h>
#include <deity/widget-thread.h>
									/*}}}*/

// ProgressWindow::ProgressWindow - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
ProgressWindow::ProgressWindow(string Title,Widget *Parent) :
                 GUIWindow(Title,Parent)
{
   Meter = new Progress(this);
   Operation = new TextWidget(this);
   SubOp = new TextWidget(this);
   Divider = new Separator(this);
   
   Operation->Text("Initializing...");
 }
									/*}}}*/
// ProgressWindow::~ProgressWindow - Destructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
ProgressWindow::~ProgressWindow()
{
   delete Meter;
   delete Operation;
   delete SubOp;
   delete Divider;
}
									/*}}}*/
// ProgressWindow::Realize - Size the window				/*{{{*/
// ---------------------------------------------------------------------
/* */
void ProgressWindow::Realize()
{
   ExtentFamily();
   if (TextGC::GC != 0)
   {
      Operation->Resize(Rect(1,0,58,1));
      SubOp->Resize(Rect(1,1,58,1));
      Meter->Resize(Rect(0,3,58,1));
      Divider->Resize(Rect(0,2,58,1));
      Resize(Rect(0,0,60,6));

      Meter->Foreground(Wc_Magenta);
      Meter->BorderWidth(0);
      Meter->Background(Wc_Black);
   }

   if (GraphicGC::GC != 0)
   {
      Operation->Font(SimpleFont("helvetica",140,SimpleFont::Bold));
      
      unsigned int UnitHeight = SubOp->Size().h;
      Resize(Rect(0,0,300,100));
      Operation->Resize(Rect(0.2*UnitHeight,0.6*UnitHeight,
			     Size().w - UnitHeight,Operation->IdealSize().y));
      SubOp->Resize(Rect(UnitHeight,Operation->Loc().y2 + 0.3*UnitHeight,
			 Size().w - UnitHeight,SubOp->Size().h));
      Divider->Resize(Rect(0,SubOp->Loc().y2 + 0.5*UnitHeight,
			   Size().w,Divider->Size().h));
      Meter->Resize(Rect(0.7*UnitHeight,Divider->Loc().y2 + 0.5*UnitHeight,
			 Size().w - 2*0.7*UnitHeight,Meter->Size().h));

      Meter->Foreground(Wc_Blue);
      Meter->Background(Wc_White);
   }

   CenterWidget(this,Rect(0,0,Parent->Pos.w,Parent->Pos.h));
   
   GUIWindow::Realize();
}
									/*}}}*/

// ProgressMeter::ProgressMeter - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
ProgressMeter::ProgressMeter(string Title,Widget *Parent)
{
   Win = new ProgressWindow(Title,Parent);
}
									/*}}}*/
// ProgressMeter::~ProgressMeter - Destructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
ProgressMeter::~ProgressMeter()
{
   delete Win;
}
									/*}}}*/
// ProgressMeter::Update - Update the display				/*{{{*/
// ---------------------------------------------------------------------
/* */
void ProgressMeter::Update()
{
   if (CheckChange(0.5) == false)
      return;

   Widget::Lock Lock;
   
   Win->SetOp(Op);
   Win->SetSubOp(SubOp);
   Win->Percent(Percent);
}
									/*}}}*/
// ProgressMeter::Done - Finalize the display				/*{{{*/
// ---------------------------------------------------------------------
/* */
void ProgressMeter::Done()
{
   Widget::Lock Lock;
   
   Win->SetOp(Op);
   Win->SetSubOp(SubOp);
   Win->Percent(Percent);
}
									/*}}}*/
