// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgtree.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   PkgTree - Manages a tree of packages. 
   
   The Package Tree class manages a list of packages.
   
   ##################################################################### */
									/*}}}*/
// Include files							/*{{{*/
#include "pkgtreeitem.h"
#include "policy.h"
#include "aptwidgets.h"

#include <deity/columnbar.h>
									/*}}}*/

// PkgTree::PkgTree - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
PkgTree::PkgTree(PackageView *Owner,ColumnBar *Bar, Widget *Parent) : 
                 Tree(Parent), Bar(Bar), Owner(Owner)
{
}
									/*}}}*/
// PkgTree::MakeList - Generate the package list based on the policy	/*{{{*/
// ---------------------------------------------------------------------
/* This runs over either the section sorted list or the sorted list and
   generates the tree. */
void PkgTree::MakeList(ExtraCache *Cache)
{
   // Remove all items
   EraseAll();
   this->Cache = Cache;

   // Generate a new list
   Tree::Item *Root = 0;
   if (Policy::Cur->ShowSections == true)
   {
      Tree::Item *Sec = 0;
      Tree::Item *Last = 0;
      unsigned long LastSect = 0;
      for (pkgCache::Package **I = Cache->SecSortedPkgs; *I != 0; I++)
      {
	 pkgCache::PkgIterator P = pkgCache::PkgIterator(*Cache,*I);
	 if (Policy::Cur->ShouldDisplay(*Cache,P) == false)
	    continue;
	 
	 // Create a new section
	 if (Sec == 0 || LastSect != (*I)->Section)
	 {
	    (new SecItem(P.Section()))->AfterStep(Root,Sec);
	    Sec->Flag(Tree::Item::Expanded);
	    Last = 0;
	    LastSect = P->Section;
	 }

	 (new Item(*I,this,Sec))->AfterStep(Sec->Child,Last); 	 
	 Last->Parent = Sec;
      }
   }
   else
   {
      Tree::Item *Last = 0;
      for (pkgCache::Package **I = Cache->SortedPkgs; *I != 0; I++)
      {
	 pkgCache::PkgIterator P = pkgCache::PkgIterator(*Cache,*I);
	 if (Policy::Cur->ShouldDisplay(*Cache,P) == false)
	    continue;
	 
	 (new Item(*I,this))->AfterStep(Root,Last);
      }
   }
   
   this->Root(Root);      
}
									/*}}}*/
// PkgTree::GetLocs - Returns the location of the 8 dividers		/*{{{*/
// ---------------------------------------------------------------------
/* Locs should be an array of 8 longs. The function will fill the array
   with the 8 positions of the vertical lines. Fields is a boolean
   array indicating if the field is activated */
void PkgTree::GetLocs(CombinedGC &GC,Rect Pos,long *Locs,bool *Fields)
{
   if (RegenLocs == false)
   {
      memcpy(Locs,ColLocs,sizeof(ColLocs));
      memcpy(Fields,ColFields,sizeof(ColFields));
      return;
   }
   
   for (int I = 0; I != 8; I++)
      Fields[I] = true;

   // Put a fake end loc for simplicity
   Locs[7] = Pos.w - Pos.x + 1;
   
   // Convert the smaller list from the text widget
   if (GC.IsText() == true)
   {
      // Get column alignment from the columns widget
      long Alignment[5] = {1,4,-1,20,40};
      Bar->PixelPositions(Alignment,5);
      
      // Deal with inactive fields
      int J = 3;
      for (int I = 1; I != 5; I++)
      {
	 if (Alignment[I] == -1)
	 {
	    Fields[I+2] = false;
	    continue;
	 }
	 
	 Alignment[I] -= Pos.x;
	 for (;J <= I+2; J++)
	    Locs[J] = Alignment[I];
      }
      for (;J <= 6; J++)
	 Locs[J] = Pos.w;
      Locs[0] = 0;
      Locs[1] = 1;
      Locs[2] = 2;
   }

   // Use the full list from the graphic column widget
   if (GC.IsGraphic() == true)
   {
      // Get column alignment from the columns widget
      long Alignment[7] = {4,8,12,16,-1,70,90};
      Bar->PixelPositions(Alignment,7);

      // Deal with inactive fields
      int J = 0;
      for (int I = 1; I != 7; I++)
      {
	 if (Alignment[I] == -1)
	 {	    
	    Fields[I] = false;
	    continue;
	 }
	 
	 if (Alignment[I] - 1 >= Pos.w)
	    break;
	 Alignment[I] -= 1 + Pos.x;
	 for (;J <= I; J++)
	    Locs[J] = Alignment[I];
      }
      for (;J <= 6; J++)
	 Locs[J] = Pos.w;
      Locs[0] = -1;
   }   

   memcpy(ColLocs,Locs,sizeof(ColLocs));
   memcpy(ColFields,Fields,sizeof(ColFields));
   RegenLocs = false;
}
									/*}}}*/
// PkgTree::DeletePkg - Mark the current package for deletion		/*{{{*/
// ---------------------------------------------------------------------
/* This is hooked to the proper menu item */
bool PkgTree::DeletePkg(Widget *,Notifyer::Tag,void *)
{
   Cache->MarkDelete(CurrentPkg());
   Damage();
   Owner->Sync();
   return true;
}
									/*}}}*/
// PkgTree::KeepPkg - Mark the current package for keep			/*{{{*/
// ---------------------------------------------------------------------
/* This is hooked to the proper menu item */
bool PkgTree::KeepPkg(Widget *,Notifyer::Tag,void *)
{
   Cache->MarkKeep(CurrentPkg());
   Damage();
   Owner->Sync();
   return true;
}
									/*}}}*/
// PkgTree::InstallPkg - Mark the current package for installation	/*{{{*/
// ---------------------------------------------------------------------
/* This is hooked to the proper menu item */
bool PkgTree::InstallPkg(Widget *,Notifyer::Tag,void *)
{
   Cache->MarkInstall(CurrentPkg());
   Cache->PromoteAutoKeep();
   
   Damage();
   Owner->Sync();
   return true;
}
									/*}}}*/
// PkgTree::CurrentPkg - Return the currently selected package		/*{{{*/
// ---------------------------------------------------------------------
/* This will return the end iterator if no package is selected */
pkgCache::PkgIterator PkgTree::CurrentPkg()
{
   Tree::Item *Cur = CurrentItem();
   if (Cur == 0)
      return pkgCache::PkgIterator(*Cache,0);
   
   if (Cur->Type() == 100)
      return ((Item *)Cur)->Package(*Cache);
   if (Cur->Type() == 101)
      return ((DepItem *)Cur)->Package(*Cache);
   return pkgCache::PkgIterator(*Cache,0);
}
									/*}}}*/
// PkgTree::Key - Handle keyboard events				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool PkgTree::Key(const KeyEvent &Key,Widget *Source)
{
   /* We check for d/k/i keys and handle them as though the user clicked
      on the proper button */
   switch (toupper(Key.Key))
   {
      case 'D':
      DeletePkg(this,Nt_Action,0);
      return true;
      
      case 'K':
      KeepPkg(this,Nt_Action,0);
      return true;
      
      case 'I':
      InstallPkg(this,Nt_Action,0);
      return true;
   }
   return Tree::Key(Key,Source);
}
									/*}}}*/
// PkgTree::ItemMouse - Generic mouse handler for the items		/*{{{*/
// ---------------------------------------------------------------------
/* This is a generic mouse handler for all of the items. */
void PkgTree::ItemMouse(Tree::Item *Itm,pkgCache::PkgIterator Pkg,
			const MouseEvent &Event,int Spacer)
{
   // Mouse is just moving around with no pressed buttons.
   if (Event.IsMotion() == true && Event.IsDrag() == false)
      return;
   
   CurrentItem(Itm);

   // Get the division locations
   CombinedGC GC;
   long Locs[8];
   bool Fields[8];
   GetLocs(GC,Rect(BorderX,0,Pos.w - 2*BorderX,ItemHeight),Locs,Fields);

   // In the name column
   if (Event.Pos.x >= Locs[3] && Event.Pos.x < Locs[4])
   {
      // Item isnt expandable
      if (Itm->IsExpandable() == false)
	 return;
      
      if (Event.IsClick() == false)
	 return;
      
      // Check if it is in the +/- inidicator
      int Depth = 0;
      for (Tree::Item *I = Itm; I != 0; I = I->Parent)
	 Depth++;
      Depth--;
      
      // Determine the spacing
      long Space;
      if (GC.IsGraphic() == true)
	 Space = Spacer;
      else
	 Space = 1;

      // Is it in the +/- region?
      if ((Locs[3] + Space*Depth) <= Event.Pos.x && 
	  (Locs[3] + Space*(Depth + 1)) > Event.Pos.x)
      {
	 if (Itm->IsExpanded() == true)
	    Itm->ContractChildren(this);
	 else 
	    Itm->ExpandChildren(this);
      }
   }
   
   // In the dki column
   if (Event.Pos.x < Locs[3] && Pkg.end() == false)
   {
      if (Event.IsClick() == false)
	 return;
      
      // Figure out which one
      ExtraCache::StateCache &State = (*Cache)[Pkg];
      int Item = 0;
      for (;Event.Pos.x >= Locs[Item]; Item++);
      Item--;
      
      // Toggle the install column (not installed)
      if (Pkg->CurrentVer == 0 && Item == 2)
      {
	 if (State.Mode == ExtraCache::ModeInstall)
	    Cache->MarkKeep(Pkg);
	 else
	 {
	    Cache->MarkInstall(Pkg);
	    Cache->PromoteAutoKeep();
	 }
	 Damage();
	 Owner->Sync();
      }
      
      // Toggle the delete column (installed, no upgrade)
      if (State.Upgradable() == false && Item == 0)
      {
	 if (State.Mode == ExtraCache::ModeDelete)
	    Cache->MarkKeep(Pkg);
	 else
	    Cache->MarkDelete(Pkg);
	 Damage();
	 Owner->Sync();
      }
      
      // Any of the three states
      if (Pkg->CurrentVer != 0 && State.Upgradable() == true)
      {
	 if (Item == 0)
	    Cache->MarkDelete(Pkg);
	 if (Item == 1)
	    Cache->MarkKeep(Pkg);
	 if (Item == 2)
	 {
	    Cache->MarkInstall(Pkg);
	    Cache->PromoteAutoKeep();
	 }
	 
	 Damage();
	 Owner->Sync();
      }   
   }
}
									/*}}}*/

