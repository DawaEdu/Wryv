#pragma once

#include <functional>
#include <set>
#include <string>
#include <vector>
using namespace std;
#include "Util/Box2DU.h"
#include "Util/CooldownCounter.h"
#include "GameObjects/GameObject.h"
#include "Util/GlobalFunctions.h"
#include "GameObjects/Units/Unit.h"
#include "UE4/WryvGameMode.h"

class TextWidget;
class AGameObject;
class AHUD;
class ATheHUD;

// None (null) alignment means positioned absolutely
enum HAlign { Left=1<<0, HCenter=1<<1, Right=1<<2, 
  ToLeftOfParent=1<<3, ToRightOfParent=1<<4, HFull=1<<5 };
enum VAlign { Top=1<<10, VCenter=1<<11, Bottom=1<<12,
  OnTopOfParent=1<<13, BelowBottomOfParent=1<<14, VFull=1<<15 };
enum Alignment {
  None=0,
  TopLeft=Top|Left,        TopRight=Top|Right,           TopCenter=Top|HCenter,
  CenterLeft=Left|VCenter, CenterCenter=VCenter|HCenter, CenterRight=VCenter|Right,
  BottomLeft=Left|Bottom,  BottomCenter=Bottom|HCenter,  BottomRight=Bottom|Right,
  Full=HFull|VFull
};
enum LayoutMode { Pixels, Percentages };
enum EventCode { NotConsumed=0, Consumed=1 };

inline FString GetAlignmentString( int Align )
{
  FString A = "";
  if( Align & Left )  A += "Left";
  else if( Align & Right )  A += "Right";
  else if( Align & HCenter )  A += "HCenter";
  else if( Align & ToLeftOfParent )  A += "ToLeftOfParent";
  else if( Align & ToRightOfParent )  A += "ToRightOfParent";
  else if( Align & HFull )  A += "HFull";
  else  A += "Left";

  if( Align & Top )  A += "Top";
  else if( Align & Bottom )  A += "Bottom";
  else if( Align & VCenter )  A += "VCenter";
  else if( Align & BelowBottomOfParent )  A += "BelowBottomOfParent";
  else if( Align & OnTopOfParent )  A += "OnTopOfParent";
  else if( Align & VFull )  A += "VFull";
  else A += "Top";
  return A;
}

class HotSpot
{
public:
  static TextWidget *TooltipWidget; // Reference to the widget to send tooltip to on flyover

  string CName; //the name of the widget, visible in C++ debugger
  FString Name; //name of the widget
  FString TooltipText; // tooltip that appears on flyover
  
  int Align;
  int Layout;
  bool hidden;
  bool eternal;  // does the widget get deleted after time
  float displayTime; // Amount of time to display remaining
  //FVector2D Pos;
  FVector2D Size;
  FVector2D Pad, Margin; // Amount of padding (space inside)
  // and Margin (space outside) this widget.
  // Margin is used for locating a widget inside the parent
  // while Pad is used for locating children inside this widget.
  bool Dead;

  FLinearColor Color;
  // Associations with AGameObject's contained in this hotspot. Empty if no objects
  // are associated with the spot (used for item slots)
  //vector<AGameObject*> Objects;
  bool dirty;  // Set if the widget's bounds need to be remeasured
  bool AbsorbsMouseUp; // If this widget does absorb mouseup
  bool XLimits, YLimits; // RecomputeSize doesn't affect X,Y

private:
  HotSpot* Parent;
  vector<HotSpot*> children;  // Children of this hotspot.
  //bool bubbleUp; // When set, the event can bubbles up through the widget
public:
  inline int GetNumChildren() { return children.size(); }
  // Inlined accessor for retrieving ith child (bounds checking)
  inline HotSpot* GetChild( int i )
  {
    if( i < 0 || i >= children.size() )
    {
      error( FS( "HotSpot %s: Index %d / %d OOB children", *Name, i, children.size() ) );
      return 0;
    }
    return children[i];
  }
  // Rarely used function to clear the array of child references, for re-stacking.
  inline void ClearChildren(){ children.clear(); }
  // Returns true (1) if event should be consumed.
  function< EventCode (FVector2D mouse) > OnMouseDownLeft;
  // A function that runs when the widget is dropped @ certain location used for "drops"
  function< EventCode (FVector2D mouse) > OnMouseUpLeft;
  function< EventCode (FVector2D mouse) > OnMouseDownRight;
  function< EventCode (FVector2D mouse) > OnMouseUpRight;
  function< EventCode (FVector2D mouse) > OnHover;
  function< EventCode (FVector2D mouse) > OnMouseDragLeft;
  
  // Resets functions etc.
  virtual void Reset();
  virtual void HotSpotDefaults() {
    Reset();
    SetName( "HotSpot" );
    //TooltipText = "Tip";

    Align = Alignment::TopLeft;
    Layout = Pixels; // pixel positioning (can also use percentages of parent widths)
    hidden = 0;
    eternal = 1;
    displayTime = FLT_MAX; // Amount of time remaining before removal
    // assumes extremely large number (1e37 seconds which is practically infinite)
    Margin = Pad = FVector2D(0,0);
    Size = FVector2D(32,32);
    Dead = 0;
    Color = FLinearColor::White;
    Parent = 0;
    dirty = 1;
    //bubbleUp = 1; // events by default bubble up thru to the next widget
    AbsorbsMouseUp = 0;
  }
  HotSpot( FString name ) {
    HotSpotDefaults();
    SetName( name );
  }
  HotSpot( FString name, FVector2D size ) {
    HotSpotDefaults();
    Size = size;
    SetName( name );
  }
  virtual ~HotSpot(){ Clear(); }
  void SetName( FString name ){
    Name = name;
    CName = TCHAR_TO_UTF8( *Name );
  }
protected:
  virtual void render( FVector2D offset )
  {
    if( hidden ) return;
    // When the base class renders, it calls render on all the children.
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->render( offset + Pos() );
  }
public:
  // Updates a UI widget
  virtual void Update( float t )
  {
    displayTime -= t;

    // Hide if display time drops below 0, or delete it if it is not 'eternal'
    if( displayTime <= 0 ) {
      if( eternal )  Hide();
      else Dead = 1; // removes on next tick
    }
    else if( displayTime <= 1 ) {
      // display time waning, start to fade
      Color.A = displayTime;
    }

    // tick the children
    for( int i = 0; i < children.size(); i++ )
      children[i]->Update( t );

    Clean();
  }

  void Clean()
  {
    // Remove dead ones (recursively)
    for( int i = children.size()-1; i >= 0; i-- )
    {
      // if the child should die, delete it
      if( children[i]->Dead ) {
        delete children[i];
        removeIndex( children, i );
      }
    }
  }

  // Base reflow doesn't change anything, but passes on reflow call to all children
  virtual void Reflow()
  {
    if( Parent )
    {
      FVector2D PSize = Parent->Size;
      if( Align & HFull )  Size.X = PSize.X - 2.f*Parent->Pad.X;
      if( Align & VFull )  Size.Y = PSize.Y - 2.f*Parent->Pad.Y;
    }

    for( int i = 0; i < children.size(); i++ )
      children[i]->Reflow();
  }

  // Runs after Add, overrideable. Since Add is templated,
  // it is not an overrideable function.
  virtual void PostAdd()
  {
    Reflow();
  }

  virtual HotSpot* Add( HotSpot* w ) {
    if( w->Parent )  w->Orphan();
    w->Parent = this;
    children.push_back( w );
    PostAdd();
    return w;
  }
  // Positions a widget in current parent (with TopLeft alignment)
  // as if child of some other hotspot
  void PositionAsChild( HotSpot* parent, int alignment )
  {
    Align = alignment;
    HotSpot* oldParent = Parent;
    parent->Add( this ); // loses old parent
    FVector2D posInParent = Pos();
    oldParent->Add( this );
    Margin = posInParent;
  }
  void SetByCorners( FVector2D TL, FVector2D BR )
  {
    Margin = TL;
    Size = BR - TL;
  }
  // Orphans the child from its parent
  void Orphan()
  {
    if( Parent )
    {
      removeElement( Parent->children, this );
      Parent = 0;
    }
  }



  // all calls to render a widget externally route through here
  // they all kick-off to subclass::render( FVector2D(0,0) )
  virtual void render(){ if( hidden ) return; render( FVector2D(0,0) ); }

  // reflows the contained hotspot to parent widget,
  // using ALIGNMENT settings.
  // ** TODO: can add a margin parameter for flushing outside of widget border.
  // The DRAW() function always lays out from the top left, so this just dynamically
  // computes Position based on alignment, size & parent size parameters
  FVector2D Pos()
  {
    FVector2D P(0,0);
    if( !Parent )
    {
      //LOG(  "Cannot realign to null parent, %s",
      //  *Name );
      // Can reflush position based on Margins, or just leave it
      //Pos = Margin;
      return P; // cannot reflush
    }
    FVector2D PSize = Parent->Size;
    FVector2D PM = Margin + Parent->Pad;

    // Null alignment (0) means absolutely positioned (does not reflow in parent, Pos is
    // just set from Margins)
    if( Align & Left )  P.X = PM.X;
    else if( Align & Right )  P.X = PSize.X - Size.X - PM.X;
    else if( Align & HCenter )  P.X = (PSize.X - Size.X)/2; // centering
    else if( Align & ToLeftOfParent )  P.X = -Size.X - Parent->Margin.X - Margin.X;
    else if( Align & ToRightOfParent )  P.X = PSize.X + 2*PM.X + Parent->Margin.X;
    else if( Align & HFull )  P.X = PM.X;  // The size changes in Reflow()
    else P.X = PM.X; // When absolutely positioned (None) values, the X position 
    // is just Left aligned

    //info( FS("elt %s's [%d/%s] has P.X=%f Size.X=%f, Parent->Size.X=%f",
    //  *Name, Align, *GetAlignmentString(Align), P.X, Size.X, Parent->Size.X) );

    if( Align & Top )  P.Y = PM.Y;
    else if( Align & Bottom )  P.Y = PSize.Y - Size.Y - PM.Y;
    else if( Align & VCenter )  P.Y = (PSize.Y - Size.Y)/2;
    else if( Align & BelowBottomOfParent )  P.Y = PSize.Y + 2*PM.Y + Parent->Margin.Y;
    else if( Align & OnTopOfParent )  P.Y = -Size.Y - Parent->Margin.Y - Margin.Y;
    else if( Align & VFull )  P.Y = PM.Y; // The size changes in Reflow()
    else P.Y = PM.Y; // default Top aligned

    return P;
  }

  float left(){ return Pos().X; }
	float right(){ return Pos().X + Size.X; }
	float top(){ return Pos().Y; }
	float bottom(){ return Pos().Y + Size.Y; }
  FVector2D GetAbsPos(){
    // traces back up to the parent and finds the absolute position of this widget
    HotSpot *h = this;
    FVector2D p = Pos();
    while( h->Parent ) { h = h->Parent ; p += h->Pos() ; }
    return p;
  }
  //bool hit( FVector2D v ) {
  //  FBox2DU bounds = GetAbsBounds();
  //  return bounds.IsInside( v ); // check if v is inside the box (<-)
  //}
  // This function checks absolute coordinates
	// Check main bounds
  bool hit( FVector2D v ) {
    FBox2DU bounds = GetAbsBounds();
    return bounds.IsInside( v ); // check if v is inside the box (<-)
  }
  
  // ABSOLUTE bounds on the object.
  // If you need RELATIVE bounds, use the SIZE of the
  // Absolute bounds.
  FBox2DU GetAbsBounds( FVector2D offset ){
    //if( dirty ) { 
    //  warning( FS( "Getting bounds of dirty element %s", *Name ) );
    //  // to make the widget clean, we have to call render. render
    //  // will re-measure any bounds.
    //  render();
    //}
    FBox2DU bounds;
    // If this gets called on a hidden widget,
    // just return a single-point bound on the offset.
    // This won't count to the parent.
    if( hidden ) {
      bounds += offset; // ZERO SIZE @ offset (not Pos())
      return bounds;
    }
    // bound this and all children
    bounds += offset + Pos();
    bounds += offset + Pos() + Size; // *Scale;
    for( int i = 0; i < children.size(); i++ ) {
      if( children[i]->hidden )  skip;
      bounds += children[i]->GetAbsBounds( offset + Pos() );
    }
    return bounds;
  }
  FBox2DU GetAbsBounds()
  {
    return GetAbsBounds( FVector2D(0,0) );
  }

  // Get absolute bounds containing the children, excluding this
  FBox2DU GetChildAbsBounds()
  {
    FBox2DU bounds ;
    if( !children.size() || hidden )
    {
      bounds += Pos(); // ZERO SIZE bounds @ Pt
    }
    else for( int i = 0; i < children.size(); i++ ) {
      if( children[i]->hidden )  skip;
      bounds += children[i]->GetAbsBounds( Pos() );
    }
    return bounds;
  }
  
  // Recomputes the widget's bounds size based on the size of the children
  void recomputeSizeToContainChildren()
  {
    // pads around the children.
    FVector2D newSize = GetChildAbsBounds().Size() + Pad*2.f;
    if( !XLimits )
      Size.X = newSize.X;
    if( !YLimits )
      Size.Y = newSize.Y;
  }

  void Show() { hidden = 0; }
  virtual void Hide() { hidden = 1; }
  // ShowChildren without affecting parent
  virtual void ShowChildren(){
    for( int i = 0; i < children.size(); i++ )
      children[i]->Show();
  }
  // HideChildren without affecting parent
  virtual void HideChildren(){
    for( int i = 0; i < children.size(); i++ )
      children[i]->Hide();
  }
  // Get the bounds of just this widget, excluding the children
  // used for when flushing position of something inside a widget
  // towards the right or left. if you get bounds including the
  // children it will tend accumulate an overflow right/left if it overflows.
  FBox2DU GetBoundsExcludingChildren()
  {
    FBox2DU bounds;
    bounds += Pos();
    bounds += Pos() + Size;
    return bounds;
  }
  virtual void Clear()
  {
    for( int i = 0; i < children.size(); i++ )
      delete children[i];
    children.clear();
  }

  // Checks if a point v hits this widget,
  // if so, an Action (described by function f) is executed on this hotspot.
  HotSpot* Act( FVector2D v, FVector2D offset, function< EventCode (FVector2D) > HotSpot::* f )
  {
    if( hidden ) return 0;

    // Look into deepest child first.
    for( int i = 0; i < children.size(); i++ )
    {
      // check if a child is hit by the click.
      HotSpot *h = children[i]->Act( v, offset + Pos(), f );  // offset accumulates absolute position offset of widget
      if( h )  return h; // If the hit was intercepted, return object hit.
    }

    // Actually perform hit checking, if this provides the function f.
    if( this->*f && hit( v - offset ) )
    {
      // The widget returns 0 when propagation should stop.
      int result = (this->*f)( v - offset - Pos() ); // pass in relative coordinates
      // If the widget doesn't bubble events, then you must return that it hit here

      // If the widget consumes events, then halt propagation by returning this
      if( result == Consumed ) return this;
    }

    return 0; //not hit or bubbles are allowed.
  }

  // returns the child hit (or this) if event tripped, NULL if event not tripped
  HotSpot* MouseDownLeft( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseDownLeft );
  }
  HotSpot* MouseUpLeft( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseUpLeft );
  }
  HotSpot* MouseDownRight( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseDownRight );
  }
  HotSpot* MouseUpRight( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseUpRight );
  }
  HotSpot* Hover( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnHover );
  }
  HotSpot* MouseDraggedLeft( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnMouseDragLeft );
  }
};
