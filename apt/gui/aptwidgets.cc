// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: aptwidgets.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   DeityWidgets - Constructors for the APT widget tree.
   
   These routines construct and layout all of the widgets used in 
   APT.
      
   ##################################################################### */
									/*}}}*/
// Include files							/*{{{*/
#include <config.h>

#include <iostream.h>
#include <sys/vfs.h>
#include <stdio.h>
#include <time.h>

#include "aptwidgets.h"
#include "pkgtree.h"
#include "policy.h"
#include "extracache.h"
#include "statuswidgets.h"

#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>

#include <deity/menubar.h>
#include <deity/textwidg.h>
#include <deity/anchor.h>
#include <deity/button.h>
#include <deity/window.h>
#include <deity/tree.h>
#include <deity/columnbar.h>
#include <deity/tabdialog.h>
#include <deity/utils.h>
									/*}}}*/

MainWindow *Base = 0;
#define _(x) x

// ListToggle - Notifyer class to toggle a check item			/*{{{*/
// ---------------------------------------------------------------------
/* This is attached to all of the toggles in the list menu and inverts
   the associated field in the policy class then rebuilds the list */
class ListToggle : public Notifyer
{
   public:
   typedef void (*TFunc)(Widget *From,bool &State);

   private:
   bool &Target;
   TFunc CallFunc;
   public:
   
   // Toggle the attached boolean and rebuild the tree
   virtual bool Trigger(Widget *From,Notifyer::Tag,void *)
   {
      Target = !Target;
      if (Target == true)
	 From->Flag(Widget::Set,Widget::Set);
      else
	 From->Flag(0,Widget::Set);
      if (CallFunc != 0)
	 CallFunc(From,Target);
      return true;
   }
   
   // Create a CheckMenu item, attach this notification and set the state
   ListToggle(const char *I,bool &Target,TFunc Call = 0) :  Notifyer(Nt_Action),
              Target(Target), CallFunc(Call)
   {
      Widget *W = new CheckMenuItem(I);
      W->Add(this);
      if (Target == true)
	 W->Flag(Widget::Set,Widget::Set);
      else
	 W->Flag(0,Widget::Set);
   }
};
									/*}}}*/
// RegenList - Callback for list regeneration				/*{{{*/
// ---------------------------------------------------------------------
/* */
void RegenList(Widget *,bool &)
{
   Base->PkgView.PTree->MakeList(Base->CurCache);
}
									/*}}}*/
// SetColumns - Callback to change the column layout			/*{{{*/
// ---------------------------------------------------------------------
/* */
void SetColumns(Widget *,bool &)
{  
   // This is the size transform table, order is CV, IV IW
   long GPos[][7] = {{16,16,16,-100,-200,-200,-200},  // 000
                     {16,16,16,190,-200,-200,-100},   // 001
                     {16,16,16,190,-200,-100,-200},   // 010
                     {16,16,16,190,-200,-40,-60},     // 011
      	             {16,16,16,190,-100,-200,-200},   // 100
                     {16,16,16,190,-40,-200,-60},     // 101
                     {16,16,16,190,-50,-50,-200},     // 110
                     {16,16,16,190,85,85,-100}};      // 111

   // Compute the table index for the current state
   unsigned long Index = 0;
   if (Policy::Cur->Columns.CurVersion == true)
      Index |= (1 << 2);
   if (Policy::Cur->Columns.InstVersion == true)
      Index |= (1 << 1);
   if (Policy::Cur->Columns.InstAsWell == true)
      Index |= (1 << 0);

   if (GraphicGC::GC != 0)
      Base->PkgView.Columns->Position(GPos[Index],7);
   Base->PkgView.PTree->Damage();
}
									/*}}}*/

// MainWindow::MainWindow - The main APT window				/*{{{*/
// ---------------------------------------------------------------------
/* */
MainWindow::MainWindow(Widget *Root) : PkgView(*this)
{
   CurCache = 0;
   CtrlCache = 0;
   ::Base = this;
   
   // Construct the top level window when in graphics mode
   if (GraphicGC::GC != 0)
   {
      Main = new GUIWindow("APT",Root);
      Base = Main;
   }
   else
      Base = Root;
   
   PkgView.Create();
   PkgView.Layout();
}
									/*}}}*/
// PackageView::Create - Create all of the widgets			/*{{{*/
// ---------------------------------------------------------------------
/* */
void PackageView::Create()
{
   Widget *Base = Owner.Base;
   
   // Construct the Main Menu
   Menu = new MenuBar(Base);
   
   // The file menu
   Widget::LastWidget = new MenuItem(_("&File"));
   new MenuPopup();
   new MenuItem(_("&Import List of Installed Packages"));
   new MenuItem(_("&Export List of Installed Packages"));
   new MenuItem(_("&Go Back to Main Screen"));
   new Separator();
   new MenuItem(_("&Quit"));
   
   // The Edit menu
   Widget::LastWidget = new MenuItem(_("&Edit"),Menu);
   new MenuPopup();
   new MenuItem(_("&Find\t/"));
   new MenuItem(_("Find &Again\tn"));
   
   // The List menu
   Widget::LastWidget = new MenuItem(_("&List"),Menu);
   new MenuPopup();
   new ListToggle(_("Packages in &Profile"),Policy::Cur->Displayable.InProfile,RegenList);
   new ListToggle(_("&Installed Packages"),Policy::Cur->Displayable.Installed,RegenList);
   new ListToggle(_("Packages to &Upgrade"),Policy::Cur->Displayable.ToUpgrade,RegenList);
   new ListToggle(_("Packages to &Downgrade"),Policy::Cur->Displayable.ToDowngrade,RegenList);
   new ListToggle(_("I&gnored Up/Downgrades"),Policy::Cur->Displayable.HeldUpDn,RegenList);
   new ListToggle(_("B&roken Packages"),Policy::Cur->Displayable.Broken,RegenList);
   new ListToggle(_("&New Packages"),Policy::Cur->Displayable.New);
   new ListToggle(_("&Obsolete Packages"),Policy::Cur->Displayable.Obsolete,RegenList);
   new ListToggle(_("No&t Installed Packages"),Policy::Cur->Displayable.NotInstalled,RegenList);
   new Separator();
   new MenuItem(_("&Choose Profile"));
   new MenuItem(_("&Modify Profile"));
   
   // The Package menu
   Widget::LastWidget = new MenuItem(_("&Package"),Menu);
   new MenuPopup();
   new MenuItem(_("Check for &Errors"));
   new MenuItem(_("&Configure Packages"));
   new MenuItem(_("&Install/Upgrade/Remove"));
   new Separator();
   Widget *MarkInst = new MenuItem(_("Mark for I&nstallation\tI"));
   Widget *MarkHold = new MenuItem(_("Mark for &Keep\tK"));
   Widget *MarkRemove = new MenuItem(_("Mark for &Deletion\tD"));
   
   // The Options menu
   Widget::LastWidget = new MenuItem(_("&Options"),Menu);
   new MenuPopup();
   new MenuItem(_("&Preferences"));
   new Separator();
   new CheckMenuItem(_("Show &Toolbar"));
   new ListToggle(_("Show &Sections"),Policy::Cur->ShowSections,RegenList);
   new Separator();
   new ListToggle(_("&Current Version"),Policy::Cur->Columns.CurVersion,SetColumns);
   new ListToggle(_("&Install Version"),Policy::Cur->Columns.InstVersion,SetColumns);
   new ListToggle(_("Install as &Well"),Policy::Cur->Columns.InstAsWell,SetColumns);
   
   // Help menu item
   new HelpMenuItem(_("&Help"),Menu);
   Menu->Extent();

   // The Column bar
   if (GraphicGC::GC != 0)
      Columns = new ColumnBar(_("\eD\eK\eI\tPackage\tCur-Version\tInst-Version\tInstall as well"),Base);
   else
      Columns = new ColumnBar(_("DKI\tPackage\tCur-Version\tInst-Version\tInstall as well"),Base);
   new Anchor(Columns,Anchor::RightToRight | Anchor::TopToTop | Anchor::BotToTop);

   // The status bar
   Status = new TextWidget(Base);
   Status->Flag(Widget::Region);
   new Anchor(Status,Anchor::RightToRight | Anchor::TopToBot | Anchor::BotToBot);
   Status->Add(new ClassNotifyer<PackageView>(this,Nt_Sync,&SyncStatus));
   Usr = new TextWidget(Status);
   Usr->DrawFlags(GenGC::YCenter);
   Debs = new TextWidget(Status);
   Debs->DrawFlags(GenGC::YCenter);
   new Anchor(Debs,Anchor::RightToRight | Anchor::TopToBot | Anchor::BotToBot);
   
   // The tree widget
   PTree = new PkgTree(this,Columns,Base);
   PTree->Flag(Widget::Region);
   new Anchor(PTree,Anchor::RightToRight | Anchor::TopToTop | Anchor::BotToBot);
   PTree->Add(new ClassNotifyer<PackageView>(this,Nt_SelectChange,&TreeChange));

   // The tab dialog
   StatusBox = new TabDialog(Base);
   new TabPage(_("Info"),StatusBox);
   InfoPage = new InfoWidget;
   new TabPage(_("Stat"),StatusBox);
   new TabPage(_("Words"),StatusBox);
   StatusBox->Flag(Widget::Region);
   StatusBox->Location(TabDialog::Bottom);
   new Anchor(StatusBox,Anchor::RightToRight | Anchor::TopToTop | Anchor::BotToTop | Anchor::LeftToRight);
   
   // The description widget
   Description = new MultiLineWidget(Base);
   Description->Flag(Widget::Region);
   new Anchor(Description,Anchor::LeftToRight | Anchor::TopToTop | Anchor::BotToBot);
   
   // Connect some methods to the d/k/i commands
   MarkInst->Add(new ClassNotifyer<PkgTree>(PTree,Nt_Action,&PkgTree::InstallPkg));
   MarkHold->Add(new ClassNotifyer<PkgTree>(PTree,Nt_Action,&PkgTree::KeepPkg));
   MarkRemove->Add(new ClassNotifyer<PkgTree>(PTree,Nt_Action,&PkgTree::DeletePkg));
}
									/*}}}*/
// PackageView::Layout - Position all of the widgets			/*{{{*/
// ---------------------------------------------------------------------
/* Once positions the anchors will ensure that they remain in the correct
   location. */
void PackageView::Layout()
{
   Widget *Base = Owner.Base;
   
   // Perform graphic mode widget layout (400x100)
   if (GraphicGC::GC != 0)
   {
      Menu->Extent();
      
      // Set the column positions and the location of the column headr
      Columns->Extent();
      Columns->Resize(Rect(0,Menu->Loc().y2,250,Columns->Size().h));
      SetColumns(0,Policy::Cur->Columns.CurVersion);

      // Setup the status bar
      Usr->BorderWidth(1);
      Usr->Margins(Point(2,2));
      Usr->SwapBorderColors();
      Usr->Extent();
      
      Debs->BorderWidth(1);
      Debs->Margins(Point(2,2));
      Debs->SwapBorderColors();
      Debs->Extent();
      
      Status->BorderWidth(1);
      Status->Margins(Point(2,2));
      Status->Extent();
      
      Status->Resize(Rect(0,100 - Status->Size().h,400,Status->Size().h));
      Usr->Resize(Rect(2,1,196,Status->Size().h-4));
      Debs->Resize(Rect(202,1,194,Status->Size().h-4));
      
      Status->Sync();
      
      // Position the main list
      PTree->Resize(AbsRect(Columns->Loc().x1,Columns->Loc().y2,
			      Columns->Loc().x2,Status->Loc().y1));

      // Set the initial size.
      Owner.Base->Resize(Rect(Owner.Base->Size().x,Owner.Base->Size().y,
			      600,370));
      
      // Position the tabbed widget
      StatusBox->Resize(AbsRect(PTree->Loc().x2,Menu->Loc().y2,
			 Base->Size().w,Menu->Loc().y2 + 200));

      // Position the description
      Description->Resize(AbsRect(StatusBox->Loc().x1,StatusBox->Loc().y2,
				  StatusBox->Loc().x2,Status->Loc().y1));
      Description->BorderWidth(1);
   }

   // Perform text mode widget layout (80x25)
   if (TextGC::GC != 0)
   {
      // Set the column positions and the location of the column header
      long Poses[4] = {4,19,17,-100};
      Columns->Position(Poses,4);
      Columns->Resize(Rect(0,1,55,1));
      Columns->Margins(Point(1,0));
      
      // Position the status bar
      Status->Resize(Rect(0,24,80,1));
      Usr->Resize(Rect(0,0,30,1));
      Debs->Resize(Rect(30,0,50,1));
      Status->Sync();
      
      // Construct the main list
      PTree->Resize(AbsRect(Columns->Loc().x1,Columns->Loc().y2,
			    Columns->Loc().x2,Status->Loc().y1));
      
      // Position the tabbed widget
      StatusBox->Resize(AbsRect(PTree->Loc().x2,Menu->Loc().y2,
			 Base->Size().w,Menu->Loc().y2 + 13));

      // Position the description widget
      Description->Resize(AbsRect(StatusBox->Loc().x1,StatusBox->Loc().y2,
				  StatusBox->Loc().x2,Status->Loc().y1));
      Description->BorderWidth(0);
   }
   
   PTree->GiveFocus();
}
									/*}}}*/
// PackageView::SyncStatus - Regenerate the status bar			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool PackageView::SyncStatus(Widget *,Notifyer::Tag,void *)
{
   struct statfs Buf;
   if (statfs("/usr",&Buf) != 0)
   {
      cerr << "Statfs failed" << endl;
      Buf.f_bfree = 0; 
   }
   
   ExtraCache &Cache = *Owner.CurCache;
   char S[300];
   sprintf(S,"/usr: %liMB free, %liMB needed",(long)Buf.f_bfree/(1024*1024/Buf.f_bsize),
	   Cache.UsrSize()/1024/1024);
   Usr->Text(S);
   
   sprintf(S,".debs: %liMB, %li broken, %li kept, %li inst, %li del",
	   Owner.CurCache->DebSize()/1024/1024,Cache.BrokenCount(),
	   Cache.KeepCount(),Cache.InstCount(),Cache.DelCount());
   Debs->Text(S);
   return true;
}
									/*}}}*/
// PackageView::TreeChange - Called when the tree selection changes	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool PackageView::TreeChange(Widget *,Notifyer::Tag,void *)
{
/*   pkgCache::PkgIterator Pkg = PTree->CurrentPkg();
   if (Pkg.end() == true)
   {
      Description->Set(0);
      InfoPage->Set(0);
      return true;
   }
   
   ExtraCache &Cache = *Owner.CurCache;
   pkgSPkgCtrlInfo Inf = (*Owner.CtrlCache)[Cache[Pkg].CandidateVerIter(Cache)];

   if (Inf.isNull() == false)
   {
      Description->Set(Inf->Find("Description"));
      InfoPage->Set(Inf);
   }
   else
   {
      Description->Set(0);
      InfoPage->Set(0);
   }   */
   
   return true;
}
									/*}}}*/
// PackageView::Sync - Calls the various syncing functions		/*{{{*/
// ---------------------------------------------------------------------
/* This is called when the tree changes something */
void PackageView::Sync()
{
   Status->Sync();
};
									/*}}}*/
