// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgtreeitem.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   PkgTree Items - Tree::Item classes for displaying items in the package 
                   tree
   
   This class renders each package item directly from the actual storage
   for the package item. This simplifies the managment of the tree as
   stringified versions of package items don't have to be constantly
   resynced with the tree.
   
   Two sorted lists of package pointers are kept, one is sorted by package
   name the other is sorted by section then by package name. This allows
   the display to be inorder.
   
   ##################################################################### */
									/*}}}*/
// Include files							/*{{{*/
#include "pkgtreeitem.h"
#include "policy.h"

#include <deity/xpm.h>
									/*}}}*/
// Images								/*{{{*/
#include "minus.xpm"
#include "plus.xpm"
#include "package.xpm"
#include "section.xpm"
#include "upgrade.xpm"
#include "downgrade.xpm"
#include "checkon.xpm"
#include "checkoff.xpm"
#include "radioon.xpm"
#include "radiooff.xpm"
#include "conflicts.xpm"
#include "depends.xpm"
#include "suggests.xpm"
#include "recommends.xpm"
static XPMImage MinusImg(minus_xpm);
static XPMImage PlusImg(plus_xpm);
static XPMImage PackageImg(package_xpm);
static XPMImage SectionImg(section_xpm);
static XPMImage UpgradeImg(upgrade_xpm);
static XPMImage DowngradeImg(downgrade_xpm);
static XPMImage CheckOnImg(checkon_xpm);
static XPMImage CheckOffImg(checkoff_xpm);
static XPMImage RadioOnImg(radioon_xpm);
static XPMImage RadioOffImg(radiooff_xpm);
static XPMImage ConflictsImg(conflicts_xpm);
static XPMImage DependsImg(depends_xpm);
static XPMImage SuggestsImg(suggests_xpm);
static XPMImage RecommendsImg(recommends_xpm);
									/*}}}*/

// Drawer::Init - Constructor helper					/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Drawer::Init()
{
   // Determine the spacing unit
   if (GC.IsGraphic() == true)
      Space = PlusImg.Dim().x;
   else
      Space = 1;
   
   // Column layout
   ((PkgTree *)Owner)->GetLocs(GC,Pos,Locs,Fields);
   
   // Color selection
   Selected = Item->IsFlag(Tree::Item::Selected);
   Item->SetColors(GC,Owner,GC.IsText() == true && Selected == true);
}
									/*}}}*/
// Drawer::RenderCheck - Render a single checkbox in the list		/*{{{*/
// ---------------------------------------------------------------------
/* This renders one checkbox out of the 3 possible columns in the list. */
void PkgTree::Drawer::RenderCheck(bool D,bool K,bool I,bool State)
{
   int Loc = Delete;
   
   // Background fill the unused entries
   if (D == true)
      Loc = Delete;
   else
      GC->BFill(AbsRect(Locs[Delete],0,Locs[Delete + 1],Height));

   if (I == true)
      Loc = Install;
   else
      GC->BFill(AbsRect(Locs[Install],0,Locs[Install + 1],Height));
   
   if (K == true)
      Loc = Keep;
   else
      GC->BFill(AbsRect(Locs[Keep],0,Locs[Keep+1],Height));
      
   // Render the check box
   if (State == true)
      GC.gGC->DrawBitmap(AbsRect(Locs[Loc],0,Locs[Loc+1],Height),CheckOnImg,
			 GenGC::YCenter | GenGC::XCenter);
   else
      GC.gGC->DrawBitmap(AbsRect(Locs[Loc],0,Locs[Loc+1],Height),CheckOffImg,
			 GenGC::YCenter | GenGC::XCenter);
}
									/*}}}*/
// Drawer::DrawState - Draw the 3 state indicators			/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Drawer::DrawState()
{
   Rect Pos = AbsRect(Locs[0],0,Locs[3],Height);

   // Purely virtual pacakge
   if (Pkg->VersionList == 0)
   {
      BFill(AllStates);
      return;
   }
   
   // Render the 3 state indicators
   if (GC.IsGraphic() == true)
   {
      // Render only the Install checkbox (not installed)
      if (Pkg->CurrentVer == 0)
	 RenderCheck(false,false,true,State->Mode == 2);
      
      // Render only the remove checkbox (installed, no upgrade)
      if (State->Upgradable() == false)
	 RenderCheck(true,false,false,State->Mode == 0);
     
      // Render all three as radio buttons, installed and upgradeable
      if (Pkg->CurrentVer != 0 && State->Upgradable() == true)
      {
	 for (int I = 0; I != 3; I++)
	 {
	    if (State->Mode == I)
	       GC.gGC->DrawBitmap(AbsRect(Locs[I],0,Locs[I+1],Pos.h),
				  RadioOnImg,GenGC::YCenter | GenGC::XCenter);
	    else
	       GC.gGC->DrawBitmap(AbsRect(Locs[I],0,Locs[I+1],Pos.h),
				  RadioOffImg,GenGC::YCenter | GenGC::XCenter);
	 }	 
      }   
   }
   else
   {
      char S[4] = {'-','-','-',0};
      S[State->Mode] = '*';

      if (Pkg->CurrentVer == 0 || State->Upgradable() == false)
	 S[1] = ' ';
      
      // Render only the install checkbox (not installed)
      if (Pkg->CurrentVer == 0)
	 S[0] = ' ';

      // Render only the remove checkbox (installed, no upgrade)
      if (State->Upgradable() == false)
	 S[2] = ' ';
      
      GC->DrawString(Point(0,0),S);
   }
}
									/*}}}*/
// Drawer::DrawName - Draw the Name feild				/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Drawer::DrawName(const char *Text,XPMImage &Image, 
			       Color Clr)
{
   if (Fields[Name] == false)
      return;

   if (Text == 0)
      Text = Pkg.Name();

   // Determine if there are any children that are expandable
   bool Expandy = false;
   if (Item->Parent != 0)
   {
      for (Tree::Item *I = Item->Parent->Child; I != 0; I = I->Next)
      {
	 if (I->IsExpandable() == true)
	 {
	    Expandy = true;
	    break;
	 }
      }
   }   
   else
      Expandy = true;
      
   // Draw the tree lines
   AbsRect TextBox(Locs[Name],0,Locs[Name+1],Height);
   long Width = Locs[Name+1] - Locs[Name];
   if (GC.IsGraphic() == true)
   {
      GC->SetColor(Wc_LightGray);
      Item->DrawLines(GC,Space,Depth,AbsRect(Locs[Name],0,Width,Height));
      
      // Draw the item graphic
      TextBox.x1 = Locs[Name] + Space*Depth;
      if (Expandy == true)
      {
	 if (Item->IsExpandable() == true)
	 {
	    if (Item->IsFlag(Tree::Item::Expanded) == true)
	       GC.gGC->DrawBitmap(Rect(TextBox.x1,0,PlusImg.Dim().x,Height),
				  MinusImg,GenGC::YCenter | GenGC::XCenter);
	    else
	       GC.gGC->DrawBitmap(Rect(TextBox.x1,0,PlusImg.Dim().x,Height),
				  PlusImg,GenGC::YCenter);
	 }
	 else
	 {
	    GC.gGC->BFill(Rect(TextBox.x1,0,PlusImg.Dim().x,Height));
	    GC.gGC->Line(Point(TextBox.x1,Height/2),Point(TextBox.x1+PlusImg.Dim().x,Height/2));
	 }
      
	 TextBox.x1 += PlusImg.Dim().x;
      }
      
      GC.gGC->DrawBitmap(Rect(TextBox.x1,0,Image.Dim().x,Height),
			 Image,GenGC::YCenter | GenGC::XCenter);
      TextBox.x1 += Image.Dim().x;
   }
   else
   {
      Item->DrawLines(GC,1,Depth,AbsRect(Locs[Name],0,Width,Height));
    
      // Indent the text region and remove 1 space from the end
      TextBox.x1 += Depth;
      TextBox.x2--;
      
      if (Expandy == true)
      {
	 if (Item->IsExpandable() == true)
	 {
	    if (Item->IsFlag(Tree::Item::Expanded) == true)
	       GC->DrawString(TextBox,"-");
	    else
	       GC->DrawString(TextBox,"+");
	 }
	 else
	    GC.tGC->DrawLineChar(Point(TextBox.x1,TextBox.y1),
				 TextGC::HLineChar);
	 TextBox.x1++;
      }
      
      GC->BFill(Rect(TextBox.x2,0,1,Height));
   }

   if (GC.IsGraphic() == true)
      Item->SetColors(GC,Owner,Selected,Clr);

   if (GC.IsText() == true && Clr != Wc_None)
      GC->SetColor(Clr);
   
   GC->DrawString(TextBox,Point(Space/3,Height/2),Text,GenGC::YCenter);

   if (GC.IsGraphic() == true && Selected == true)
      Item->SetColors(GC,Owner,false);

   if (GC.IsText() == true && Clr != Wc_None)
      Item->SetColors(GC,Owner,Selected);      
}
									/*}}}*/
// Drawer::DrawCurrentVersion - Draw the current version		/*{{{*/
// ---------------------------------------------------------------------
/* Draw the current version field */
void PkgTree::Drawer::DrawCurrentVersion()
{
   if (Fields[CurrentVer] == false)
      return;
   
   // Draw the current version field
   AbsRect TextBox = AbsRect(Locs[CurrentVer],0,Locs[CurrentVer+1],Height);
   if (Pkg->CurrentVer != 0)
   {
      // Check if the package has an error with its current version
      if ((State->DepState & ExtraCache::DepNowMin) != ExtraCache::DepNowMin)
      {
	 if (Selected == true && GC.IsText() == true)
	    GC->SetColor(Color(239,44,40,Color::Brown));
	 else
	    GC->SetColor(Color(239,44,40,Color::BrightRed));
      }
      
      GC->DrawString(TextBox,Point(Space/3,Height/2),State->CurVersion,
		     GenGC::YCenter);
      
      // Set the colouring back
      if ((State->DepState &  ExtraCache::DepNowMin) != ExtraCache::DepNowMin)
	     Item->SetColors(GC,Owner,GC.IsText() == true && Selected == true);
   }
   else
      GC->BFill(TextBox);
}
									/*}}}*/
// Drawer::DrawCandidateVersion - Draw the candidate version		/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Drawer::DrawCandidateVersion()
{
   if (Fields[CandidateVer] == false)
      return;
   
   /* Draw the Install Version. The version is only drawn if something is going
      to happen to the package */
   AbsRect TextBox = AbsRect(Locs[CandidateVer],0,Locs[CandidateVer+1],Height);

   // We have to draw a status icon..
   if (State->Status != 0)
   {
      // In graphic mode
      if (GC.IsGraphic() == true)
      {
	 // Draw the upgrade image, downgrade image or a blank
	 TextBox.x2 = TextBox.x1 + UpgradeImg.Dim().x;
	 if (State->Status == -1)
	    GC.gGC->DrawBitmap(TextBox,Point(0,Height/2),
			       DowngradeImg,GenGC::YCenter);
	 if (State->Status == 1)
	    GC.gGC->DrawBitmap(TextBox,Point(0,Height/2),
			       UpgradeImg,GenGC::YCenter);
	 
	 if (State->Status != -1 && State->Status != 1)
	    GC->BFill(TextBox);	 
      }
      else
      {
	 // Perform the image lookup in a table for text mode..
	 TextBox.x2 = TextBox.x1 + Space;
	 char *StatusChar[] =  {"v","=","^"," "};
	 GC->DrawString(TextBox,StatusChar[State->Status + 1],
			GenGC::YCenter | GenGC::XCenter);
      }
      
      // Redner the version text
      TextBox.x1 = TextBox.x2;
      TextBox.x2 = Locs[CandidateVer+1];

      GC->DrawString(TextBox,Point(Space/3,Height/2),State->CandVersion,
		     GenGC::YCenter);
   }
   else
   {
      // Just fill.
      TextBox.x2 = Locs[CandidateVer+1];
      GC->BFill(TextBox);
   }
}
									/*}}}*/
// Drawer::DrawAsWell - Draw the Install as well field			/*{{{*/
// ---------------------------------------------------------------------
/* The install as well field has a number of special requirements to make 
   it work properly these are all handled by the extra cache dep generator. */
void PkgTree::Drawer::DrawAsWell()
{
   if (Fields[InstAsWell] == false)
      return;
   
   AbsRect Pos(Locs[InstAsWell],0,Locs[InstAsWell + 1],Height);

   if (State->CandidateVer == 0)
   {
      GC->BFill(Pos);
      return;
   }

   // Run over the deps of the candidate version
   pkgCache::DepIterator Dep = State->CandidateVerIter(Cache).DependsList();
   long Space = GC->ExtentText(" ").w;
   for (; Dep.end() != true;)
   {
      pkgCache::DepIterator Start = Dep;
      bool Result = true;
      for (bool LastOR = true; Dep.end() == false && LastOR == true; Dep++)
      {
	 LastOR = (Dep->CompareOp & pkgCache::Dep::Or) == pkgCache::Dep::Or;

	 // Figure out some of the flags
	 unsigned char DepState = Cache[Dep];
	 ExtraCache::StateCache &Parent = Cache[Dep.TargetPkg()];
	 bool Auto = (Parent.Flags & pkgCache::Flag::Auto) == pkgCache::Flag::Auto;
	 bool Current = (DepState & ExtraCache::DepNow) ==
	                ExtraCache::DepNow;
	 bool Install = (DepState & ExtraCache::DepInstall) == 
	                ExtraCache::DepInstall;

	 /* The currently installed version and target install version are 
	    both okay, there is no need to show the dep */
	 if (Current == true && Install == true)
	    Result = false;
	 
	 /* The install version is okay and the target package is not in auto
	    mode */
	 if (Install == true && Auto == false)
	    Result = false;
      }
      
      // Dep is satisfied okay.
      if (Result == false)
	 continue;
      
      /* Now, decide if we should show user deps (suggest/rec). This is done by
         checking if the package was installed, if so then user deps are 
       	 ignored otherwise they are shown. */
      if (Policy::Cur->IsImportantDep(Start) == false)
	 continue;
      if (Pkg->CurrentVer != 0 && Start.IsCritical() == false)
	 continue;
	  
      // Ah, now we show the dep.
      const char *Name = Start.TargetPkg().Name();
      Rect Size = GC->ExtentText(Name);
      Size.w += Space;
      GC->DrawString(Rect(Pos.x1,Pos.y1,Size.w,Pos.y2 - Pos.y1),
		     Point(Space,(Pos.y2 - Pos.y1)/2),Name,GenGC::YCenter);
      Pos.x1 += Size.w;
      
      if (Pos.x1 >= Pos.x2)
	 return;
   }
   GC->BFill(Pos);
}
									/*}}}*/
// Drawer::BFill - Fills one of the sections				/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Drawer::BFill(unsigned int Region)
{
   if (Region == AllStates)
      GC->BFill(AbsRect(Locs[0],0,Locs[3],Height));
   else
      GC->BFill(AbsRect(Locs[Region],0,Locs[Region+1],Height));
}
									/*}}}*/

// Item::Item - Constructor						/*{{{*/
// ---------------------------------------------------------------------
/* */
PkgTree::Item::Item(pkgCache::Package *Pkg,PkgTree *Owner,Tree::Item *Parent) :
                    Tree::Item(Parent), Pkg(Pkg)
{
   pkgCache::PkgIterator Pkg(*Owner->Cache,this->Pkg);
   ExtraCache::StateCache &State = (*Owner->Cache)[Pkg];

   // Pick a version iterator
   if (State.CandidateVer == 0)
      return;
   pkgCache::VerIterator Ver = State.CandidateVerIter(*Owner->Cache);
   
   if (Ver->DependsList != 0)
      Flag(Expandable);
}
									/*}}}*/
// Item::Render - Draw the item						/*{{{*/
// ---------------------------------------------------------------------
/* This does the complex redering of each package item, including all the
   columns and the images that are required. */
void PkgTree::Item::Render(CombinedGC &GC,unsigned long Depth,Rect Pos,
			   Tree *Owner)
{
   GC->AddClipping(Pos);
   
   Drawer Draw(GC,Depth,Pos,Owner,Pkg,this);

   // Check if the packages install state is broken, draw in red.
   pkgCache::PkgIterator P(Draw,Pkg);
   if ((Draw.GetCache()[P].DepState & ExtraCache::DepInstMin) !=
       ExtraCache::DepInstMin)
   {
      if (Draw.IsSelected() == true && GC.IsText() == true)
	 Draw.DrawName(0,PackageImg,Color(239,44,40,Color::Brown));
      else
	 Draw.DrawName(0,PackageImg,Color(239,44,40,Color::BrightRed));
   }		       
   else
      Draw.DrawName(0,PackageImg);
		    
   Draw.DrawState();
   Draw.DrawCurrentVersion();
   Draw.DrawCandidateVersion();
   Draw.DrawAsWell();

   GC->PopClipping();
}
									/*}}}*/
// Item::Height - Return the Height of the item				/*{{{*/
// ---------------------------------------------------------------------
/* */
long PkgTree::Item::Height()
{
   if (TextGC::GC != 0)
      return GenGC::GC->ExtentText("").h;
   else
      return GenGC::GC->ExtentText("").h + 2;
}
									/*}}}*/
// Item::Expand - Expand the tree node					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool PkgTree::Item::Expand(Tree *Owner)
{
   PkgTree *Tree = (PkgTree *)Owner;
   pkgCache::PkgIterator Pkg(*Tree->Cache,this->Pkg);
   ExtraCache::StateCache &State = (*Tree->Cache)[Pkg];

   // Pick a version iterator
   if (State.CandidateVer == 0)
      return false;
   pkgCache::VerIterator Ver = State.CandidateVerIter(*Tree->Cache);
   
   // Generate the new child list
   Tree::Item *Root = 0;
   Tree::Item *Last = 0;
   for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
   {
      (new DepItem(D,Tree))->AfterStep(Root,Last);
      Last->Parent = this;
   }
   
   Child = Root;
   return true;
}
									/*}}}*/
// Item::Contract - Collapse the tree node				/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Item::Contract(Tree *)
{
   // Erase all the children
   for (Tree::Item *I = Child; I != 0;)
   {
      Tree::Item *Next = I->Next;
      I->Parent = 0;
      I->Next = 0;
      I->Last = 0;
      delete I;
      I = Next;
   }
   Child = 0;
}
									/*}}}*/
// Item::Mouse - Mouse handler						/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::Item::Mouse(Tree *Owner,const MouseEvent &Event)
{
   PkgTree &P = *(PkgTree *)Owner;
   P.ItemMouse(this,Package(*P.Cache),Event,PlusImg.Dim().x);
}
									/*}}}*/

// DepItem::DepItem - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
PkgTree::DepItem::DepItem(pkgCache::Dependency *Dep,PkgTree *Owner,
			  Tree::Item *Parent) : Tree::Item(Parent), Dep(Dep)
{
   // Check if the provide list only has one item 
   pkgCache::DepIterator D(*Owner->Cache,Dep);
   pkgCache::PkgIterator P(*Owner->Cache);
   if (D.SmartTargetPkg(P) == true)
      Flag(Expandable);
}
									/*}}}*/
// DepItem::Height - Return the Height of the item			/*{{{*/
// ---------------------------------------------------------------------
/* */
long PkgTree::DepItem::Height()
{
   if (TextGC::GC != 0)
      return GenGC::GC->ExtentText("").h;
   else
      return GenGC::GC->ExtentText("").h + 2;
}
									/*}}}*/
// DepItem::Expand - Expand the tree node				/*{{{*/
// ---------------------------------------------------------------------
/* This iterates over all the packages providing this name. If a package
   exists already with this name then it is shown as the parent. */
bool PkgTree::DepItem::Expand(Tree *Owner)
{
   PkgTree *Tree = (PkgTree *)Owner;
   pkgCache::DepIterator D(*Tree->Cache,Dep);
   pkgCache::PrvIterator Prv = D.TargetPkg().ProvidesList();
   
   // Generate the new child list
   Tree::Item *Root = 0;
   Tree::Item *Last = 0;
   for (; Prv.end() == false; Prv++)
   {
      // Check if this provides is duplicated in the list.
      pkgCache::PrvIterator DupCheck = Prv;
      for (DupCheck++;DupCheck.end() == false && DupCheck.OwnerPkg() != Prv.OwnerPkg(); 
	   DupCheck++);
      if (DupCheck.end() == false)
	 continue;
	 
      // Create the new item and add it to the end of the child list.
      (new PkgTree::Item(Prv.OwnerPkg(),Tree))->AfterStep(Root,Last);
      Last->Parent = this;
      Last->Flag(0,Expandable);
   }
   
   Child = Root;
   return true;
}
									/*}}}*/
// DepItem::Render - Draw the item					/*{{{*/
// ---------------------------------------------------------------------
/* This does the complex redering of each package item, including all the
   columns and the images that are required. */
void PkgTree::DepItem::Render(CombinedGC &GC,unsigned long Depth,Rect Pos,
			      Tree *Owner)
{
   GC->AddClipping(Pos);
   
   Drawer Draw(GC,Depth,Pos,Owner,this);
   pkgCache::DepIterator Dep(Draw,this->Dep);
   
   // Smart locate the target package
   pkgCache::PkgIterator TargetPkg(Draw);
   Dep.SmartTargetPkg(TargetPkg);
   Draw.SetPkg(TargetPkg);

   // Determine if the item should be drawn in red (firebrick2 to be exact)
   Color Clr = Wc_None;   
   pkgCache::DepIterator D = Dep;
   for (;(D->CompareOp & pkgCache::Dep::Or) == pkgCache::Dep::Or; D++);
   if ((Draw.GetCache()[D] & ExtraCache::DepGInstall) == 0)
   {
      if (Draw.IsSelected() == true && GC.IsText() == true)
	 Clr = Color(239,44,40,Color::Brown);
      else
	 Clr = Color(239,44,40,Color::BrightRed);
   }		       
   
   // Draw the name with the proper image
   switch (Dep->Type)
   {
      case pkgCache::Dep::PreDepends:
      case pkgCache::Dep::Depends:
      Draw.DrawName(0,DependsImg,Clr);
      break;
      
      case pkgCache::Dep::Suggests:
      Draw.DrawName(0,SuggestsImg,Clr);
      break;
      
      case pkgCache::Dep::Recommends:
      Draw.DrawName(0,RecommendsImg,Clr);
      break;
      
      case pkgCache::Dep::Conflicts:
      Draw.DrawName(0,ConflictsImg,Clr);
      break;
      
      default:
      Draw.DrawName(0,PackageImg,Clr);
      break;
   }
   
   Draw.DrawState();
   Draw.DrawCurrentVersion();
   Draw.DrawCandidateVersion();
   Draw.DrawAsWell();

   GC->PopClipping();
}
									/*}}}*/
// DepItem::Mouse - Mouse handler					/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::DepItem::Mouse(Tree *Owner,const MouseEvent &Event)
{
   PkgTree &P = *(PkgTree *)Owner;
   P.ItemMouse(this,Package(*P.Cache),Event,PlusImg.Dim().x);
}
									/*}}}*/

// SecItem::SecItem - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
PkgTree::SecItem::SecItem(const char *Sec,Tree::Item *Parent) : 
                    Tree::Item(Parent), Section(Sec)
{
}
									/*}}}*/
// SecItem::Render - Draw the item					/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::SecItem::Render(CombinedGC &GC,unsigned long Depth,
			      Rect Pos,Tree *Owner)
{
   GC->AddClipping(Pos);
   
   Drawer Draw(GC,Depth,Pos,Owner,this);
   Draw.DrawName(Section,SectionImg);
   Draw.BFill(Drawer::AllStates);
   Draw.BFill(Drawer::CurrentVer);
   Draw.BFill(Drawer::CandidateVer);
   Draw.BFill(Drawer::InstAsWell);

   GC->PopClipping();
}
									/*}}}*/
// SecItem::Height - Return the Height of the item			/*{{{*/
// ---------------------------------------------------------------------
/* */
long PkgTree::SecItem::Height()
{
   if (TextGC::GC != 0)
      return GenGC::GC->ExtentText("").h;
   else
      return GenGC::GC->ExtentText("").h + 2;
}
									/*}}}*/
// SecItem::Mouse - Mouse handler					/*{{{*/
// ---------------------------------------------------------------------
/* */
void PkgTree::SecItem::Mouse(Tree *Owner,const MouseEvent &Event)
{
   PkgTree &P = *(PkgTree *)Owner;
   P.ItemMouse(this,pkgCache::PkgIterator(*P.Cache,0),Event,PlusImg.Dim().x);
}
									/*}}}*/
