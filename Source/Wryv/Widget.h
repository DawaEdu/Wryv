#pragma once

#include <vector>
#include <functional>
#include <string>
#include "GlobalFunctions.h"
#include "WryvGameMode.h"
#include "Ability.h"
#include "Types.h"

using namespace std;
class AHUD;

struct FBox2DU : public FBox2D
{
  // +---------->
  // | min
  // |  TL----TR
  // |   |    |
  // |   |    |
  // v  BL----BR
  //          max
  FBox2DU( FVector2D min, FVector2D max )
  {
    Min = min;
    Max = max;
  }
  FBox2DU()
  {
    Min.X = Min.Y = 1e6f;
    Max.X = Max.Y = -1e6f;
  }
  // Translates the box by V
  FBox2DU operator+( FVector2D v ){ return FBox2DU( Min+v, Max+v ); }
  FBox2DU operator-( FVector2D v ){ return FBox2DU( Min-v, Max-v ); }
  float left(){ return Min.X; }
	float right(){ return Max.X; }
	float top(){ return Min.Y; }
	float bottom(){ return Max.Y; }

  FVector2D TL() { return Min; }
  FVector2D TR() { return FVector2D( Max.X, Min.Y ); }
  FVector2D BR() { return Max; }
  FVector2D BL() { return FVector2D( Min.X, Max.Y ); }
  
  void print(FString msg) {
    LOG(  "%s fbox2du bounds min=(%f %f) max=(%f %f) meas=(%f %f)",
      *msg, Min.X, Min.Y, Max.X, Max.Y, Size().X, Size().Y );
  }
  FVector2D Size() { 
    return Max - Min;
  }
};

class AGameObject;
class TextWidget;
class ATheHUD;

// None (null) alignment means positioned absolutely
enum HAlign { None=0, Left=1<<0, HCenter=1<<1, Right=1<<2, 
  ToLeftOfParent=1<<3, ToRightOfParent=1<<4, HFull=1<<5 };
enum VAlign { Top=1<<10, VCenter=1<<11, Bottom=1<<12,
  OnTopOfParent=1<<13, BelowBottomOfParent=1<<14, VFull=1<<15 };
enum Alignment {
  TopLeft=Top|Left, TopRight=Top|Right, TopCenter=Top|HCenter,
  CenterLeft=Left|VCenter, CenterCenter=VCenter|HCenter, CenterRight=VCenter|Right,
  BottomLeft=Left|Bottom, BottomCenter=Bottom|HCenter, BottomRight=Bottom|Right,
  Full=HFull|VFull
};
enum LayoutMode { Pixels, Percentages };
enum EventCode { NotConsumed=0, Consumed=1 };

class HotSpot
{
public:
  static TextWidget *TooltipWidget; // Reference to the widget to send tooltip to on flyover
  static ATheHUD* hud;

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
  vector<AGameObject*> Objects;

protected:
  HotSpot* Parent;
  vector<HotSpot*> children;  // Children of this hotspot.
  bool dirty;  // Set if the widget's bounds need to be remeasured
  //bool bubbleUp; // When set, the event can bubbles up through the widget
public:
  
  // Returns true (1) if event should be consumed.
  function< EventCode (FVector2D mouse) > OnMouseDownLeft;
  // A function that runs when the widget is dropped @ certain location
  // used for "drops"
  function< EventCode (FVector2D mouse) > OnMouseUpLeft;
  function< EventCode (FVector2D mouse) > OnMouseDownRight;
  function< EventCode (FVector2D mouse) > OnMouseUpRight;
  function< EventCode (FVector2D mouse) > OnHover;
  function< EventCode (FVector2D mouse) > OnMouseDragLeft;
  function< void (float t) > OnTick; // a function to run at each tick.

  virtual void HotSpotDefaults();
  HotSpot( FString name ) {
    HotSpotDefaults();
    SetName( name );
  }
  HotSpot( FString name, FVector2D size ) {//: HotSpot( name ){
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
  virtual void Move( float t ) {
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
      children[i]->Move( t );

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

  template <typename T>
  T* Add( T* w ) {
    if( w->Parent )  w->Orphan();
    w->Parent = this;
    children.push_back( w );
    return w;
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
    else if( Align & HFull )  P.X = 0, Size.X = PSize.X;
    else P.X = PM.X; // When absolutely positioned (None) values, the X position 
    // is just Left aligned

    if( Align & Top )  P.Y = PM.Y;
    else if( Align & Bottom )  P.Y = PSize.Y - Size.Y - PM.Y;
    else if( Align & VCenter )  P.Y = (PSize.Y - Size.Y)/2;
    else if( Align & BelowBottomOfParent )  P.Y = PSize.Y + 2*PM.Y + Parent->Margin.Y;
    else if( Align & OnTopOfParent )  P.Y = -Size.Y - Parent->Margin.Y - Margin.Y;
    else if( Align & VFull )  P.Y = 0, Size.Y = PSize.Y;
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
  //  FBox2DU bounds = GetBounds();
  //  return bounds.IsInside( v ); // check if v is inside the box (<-)
  //}
  // This function checks absolute coordinates
	// Check main bounds
  bool hit( FVector2D v ) {
    FBox2DU bounds = GetBounds();
    return bounds.IsInside( v ); // check if v is inside the box (<-)
  }
  
  // ABSOLUTE bounds on the object
  FBox2DU GetBounds( FVector2D offset ){
    FBox2DU bounds;
    // bound this and all children
    bounds += offset + Pos();
    bounds += offset + Pos() + Size;
    for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( offset + Pos() );
    }
    return bounds;
  }
  FBox2DU GetBounds()
  {
    return GetBounds( FVector2D(0,0) );
  }

  // Get absolute bounds containing the children, excluding this
  FBox2DU GetChildBounds()
  {
    FBox2DU bounds ;
    if( !children.size() )
    {
      LOG(  "Widget %s had NO CHILDREN!", *Name );
      // return the bounds of the container instead
      //bounds += GetBounds();
      // give 0 size box
      bounds = FBox2DU( FVector2D(0,0), FVector2D(0,0) );
    }
    else for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( Pos() );
    }
    return bounds;
  }
  
  // Recomputes the widget's bounds size based on the size of the children
  void recomputeSizeToContainChildren()
  {
    Size = GetChildBounds().Size() + Pad*2.f;
  }

  void Show() { hidden = 0; }
  void Hide() { hidden = 1; }
  // ShowChildren without affecting parent
  void ShowChildren(){
    for( int i = 0; i < children.size(); i++ )
      children[i]->Show();
  }
  // HideChildren without affecting parent
  void HideChildren(){
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
  void Clear(){
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
      if( h )  return children[i]; // If the hit was intercepted, return object hit.
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

class TextWidget : public HotSpot
{
public:
  UFont *Font;
  FString Text;
  float Scale;

  // During construction, we assign Text directly, then call Measure.
  // Construction is assumed to happen when the HUD is available.
  TextWidget( FString fs, UFont *font=0, float scale=1.f ) : 
    HotSpot( FString( "TextWidget " ) + fs ),
    Text( fs ), Font(font), Scale( 1.f )
  {
    SetAndMeasure( fs );
  }
  virtual ~TextWidget(){}

  // Modifies text inside widget, but does not remeasure text size (until next draw call)
  void Set( FString text )
  {
    Text = text;
    dirty = 1;
    // We do not call Measure() here, instead we call it at the
    // next RENDER call, since if the HUD is not available, we cannot
    // measure the text.
    //Measure(); 
    // [[ cannot call measure here, since may happen
    //    during callback due to mouse motion etc. ]]
  }

  void Set( int v ) { Set( FString::Printf( TEXT( "%d" ), v ) ); }

  // Sets the text and re-measures the size of the text immediately afterwards
  // the HUD must be ready for this
  void SetAndMeasure( FString text )
  {
    Set( text );
    Measure();
  }

  void Measure();
  
protected:
  // We call Measure() each call to render() since text cannot be measured except when
  // canvas is ready (ie during render() calls)
  virtual void render( FVector2D offset ) override;
};

class ImageWidget : public HotSpot
{
public:
  static UTexture* NullTexture;
  UTexture* Tex;
  FVector2D uv; // The maximum coordinates of the UV texturing
  FVector2D hotpoint; // Usually top left corner (0,0), meaning will render from topleft corner.
  // if its half size, then it will render from the center (such as when an imageWidget is being
  // click-dragged
  float Rotation;
  FVector2D PivotPoint; // the pivot about which the rotation is based
  virtual void ImageWidgetDefaults(){
    Tex = 0;
    hotpoint = FVector2D(0,0);
    uv = FVector2D(1,1);
    Rotation = 0.f;
  }
  ImageWidget( FString name ) : HotSpot( name ) { 
    ImageWidgetDefaults();
  }
  ImageWidget( FString name, UTexture* pic ) : HotSpot( name )
  {
    ImageWidgetDefaults();
    Tex = pic;
    if( Tex ) {
      SetName( Tex->GetName() );
      Size.X = Tex->GetSurfaceWidth();
      Size.Y = Tex->GetSurfaceHeight();
    }
  }
  ImageWidget( FString name, UTexture* pic, FLinearColor color ) : HotSpot( name )
  {
    ImageWidgetDefaults();
    Tex = pic;
    Color = color;
    if( Tex ){ // assign size from tex
      Size.X = Tex->GetSurfaceWidth();
      Size.Y = Tex->GetSurfaceHeight();
    }
  }

  // Size specified:
  ImageWidget( UTexture* pic, FVector2D size ) : HotSpot( "ImageWidget", size )
  {
    ImageWidgetDefaults();
    Tex = pic;
    if( Tex ) SetName( Tex->GetName() );
    // Don't assign size from tex
  }
  ImageWidget( FString name, UTexture* pic, FVector2D size ) : HotSpot( name, size )
  {
    ImageWidgetDefaults();
    Tex = pic;
  }
  ImageWidget( FString name, UTexture* pic, FVector2D size, FLinearColor color ) : HotSpot( name, size )
  {
    ImageWidgetDefaults(); 
    Tex = pic;
    Color = color;
    // Don't assign size from tex
  }
  
  virtual ~ImageWidget(){}
protected:
  virtual void render( FVector2D offset ) override ;
};

class ITextWidget : public ImageWidget
{
  TextWidget* Text;
  bool FixedSize; // the background graphic can have fixed size or
  // it can wrap/house the text in the textwidget inside
public:
  FString GetText(){ return Text->Text; }
  ITextWidget( FString name, UTexture* pic, FString ftext, int textAlignment, 
    UFont* font=0, float scale=1.f ) :
    ImageWidget( name, pic )
  {
    if( pic ) Name += pic->GetName();
    SetName( Name );
    Text = new TextWidget( ftext, font, scale );
    Text->Align = textAlignment;
    Add( Text );
    Pad = FVector2D( 8,8 );
    // assumes size of texture when not fixed size
    FixedSize = 0;// when size not supplied, assumed to wrap the textwidget inside
  }

  ITextWidget( FString name, UTexture* pic, FVector2D size, FString ftext, int textAlignment, 
    UFont* font=0, float scale=1.f ) :
    ImageWidget( pic, size )
  {
    Name = name;
    if( pic ) Name += pic->GetName();
    SetName( Name );
    Text = new TextWidget( ftext, font, scale );
    Text->Align = textAlignment;
    Add( Text );
    Pad = FVector2D( 8,8 );
    FixedSize = 1;// size supplied in ctor maintained
  }

  void Set( FString text )
  {
    Text->Set( text );
    dirty = 1;
  }
  void Set( int v )
  {
    Set( FS( "%d", v ) );
  }

  // on redraw, remeasure the bounds to wrap the contained text
  virtual void render( FVector2D offset ) override
  {
    // Re-measure the text also
    if( dirty && !FixedSize )
    {
      Text->Measure();
      recomputeSizeToContainChildren();
      dirty = 0;
    }

    ImageWidget::render( offset );
  }
};

class Clock : public ITextWidget
{
public:
  CooldownCounter counter; // The actual counter
  FLinearColor clockColor;
  UMaterialInstanceDynamic *clockMaterial; //anim param
  
  static UMaterialInstanceDynamic *CreateClockMaterial(FLinearColor color);
  Clock( FString name, FVector2D size, CooldownCounter o, FLinearColor pieColor );
  ~Clock();

  virtual void Move( float t ) override;
  virtual void render( FVector2D offset ) override;
};

class SolidWidget : public ImageWidget
{
public:
  static UTexture* SolidWhiteTexture;
  SolidWidget( FString name, FLinearColor color ) :
    ImageWidget( name, SolidWhiteTexture, color )
  {
  }
  SolidWidget( FString name, FVector2D size, FLinearColor color ) : 
    ImageWidget( name, SolidWhiteTexture, size, color )
  {
  }
  virtual ~SolidWidget(){}
};

// Supports stacking-in of widgets from left/right or top/bottom
class StackPanel : public ImageWidget
{
public:
  static UTexture* StackPanelTexture;
  
  StackPanel( FString name ) :
    ImageWidget( name )
  {
  }
  StackPanel( FString name, UTexture* bkg ) :
    ImageWidget( name, bkg )
  {
  }
  StackPanel( FString name, UTexture* bkg, FLinearColor color ) :
    ImageWidget( name, bkg, color )
  {
  }
  virtual ~StackPanel(){}
  
  // Override base class function, to prevent warning when
  // texture not provided
  virtual void render( FVector2D offset ) override
  {
    // Doesn't render the texture if it isn't set,
    // so that clear background is allowed
    if( Tex )  ImageWidget::render( offset );

    // render children
    HotSpot::render( offset );
  }

  //  _ _
  // |_|_| <
  template <typename T> T* StackRight( T* widget )
  {
    widget->Align = CenterLeft;
    // Situates the bounds @ the origin,
    // which means its just a measure of the widget's total size.
    FBox2DU childBounds = GetChildBounds();
    widget->Margin.X = childBounds.right();
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //  _
  // |_|
  // |_|
  //  ^
  template <typename T> T* StackBottom( T* widget )
  {
    widget->Align = TopCenter;
    FBox2DU childBounds = GetChildBounds();
    widget->Margin.Y = childBounds.bottom();
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //    _ _
  // > |_|_|
  template <typename T> T* StackLeft( HotSpot* widget )
  {
    widget->Align = CenterLeft;
    // pull all the children already in the widget over to the left
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.X += widget->Size.X + Pad.X;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //  v
  //  _
  // |_|
  // |_|
  template <typename T> T* StackTop( T* widget )
  {
    widget->Align = TopCenter;
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.Y += widget->Size.Y + Pad.Y;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }
};

class Border : public HotSpot
{
  SolidWidget *left, *top, *bottom, *right;
public:
  FBox2DU Box;
  float Thickness;
  Border( FString name, FBox2DU box, float thickness, FLinearColor color ):
    HotSpot( name )
  {
    Thickness = thickness;

    left = new SolidWidget( "left", color );
    Add( left );
    top = new SolidWidget( "top", color );
    Add( top );
    bottom = new SolidWidget( "bot", color );
    Add( bottom );
    right = new SolidWidget( "right", color );
    Add( right );

    Set( box );
  }

  void Set( FBox2DU box )
  {
    Box = box;
    // TL                  TR
    //   +----------------+
    //   |+--------------+|
    //   ||              ||
    //   |+--------------+|
    //   +----------------+
    // BL                  BR
    // Set the widths
    // trbl
    top->SetByCorners( box.TL(), box.TR() + FVector2D(0,Thickness) );
    right->SetByCorners( box.TR() + FVector2D( -Thickness, 0 ), box.BR() );
    bottom->SetByCorners( box.BL() + FVector2D( 0, Thickness ), box.BR() );
    left->SetByCorners( box.TL(), box.BL() + FVector2D( Thickness, 0 ) );
  }
};

class MouseSelectBox : public Border
{
  FVector2D StartPt;
public:
  MouseSelectBox( FString name, FBox2DU box, float thickness, FLinearColor color ) :
    Border( name, box, thickness, color )
  {
  }

  void SetStart( FVector2D pt )
  {
    //LOG(  "box start (%f,%f)", pt.X, pt.Y );
    StartPt = pt;
    Box.Min = Box.Max = pt; // close box.
    Set( Box );
  }

  void SetEnd( FVector2D pt )
  {
    //LOG(  "box endpt (%f,%f)", pt.X, pt.Y );
    Box.Min = Box.Max = StartPt; // reset box
    Box += pt; // expand to contain new point.
    Set( Box );
  }
};

// Describes a gold, lumber, stone amount.
// Can be used for displaying player resources
// or the cost of something.
class ResourcesWidget : public StackPanel
{
  TextWidget *Gold, *Lumber, *Stone;
  int Px;       // size of the icons
  int Spacing;  // spacing between widgets
public:
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;

  // Add in all the subwidgets
  ResourcesWidget( FString name, int pxSize, int spacing ) :
    StackPanel( name, 0 ), Px( pxSize ), Spacing( spacing )
  {
    // +-----------------+
    // |G1000 W1000 S1000|
    // +-----------------+
    // The 3 resource types
    StackRight( new ImageWidget( "Gold icon", GoldTexture ) ); // icon
    Gold = StackRight( new TextWidget( "1000" ) );
    StackRight( new ImageWidget( "Lumber icon", LumberTexture ) );
    Lumber = StackRight( new TextWidget( "1000" ) );
    StackRight( new ImageWidget( "Stone icon", StoneTexture ) );
    Stone = StackRight( new TextWidget( "1000" ) );

    recomputeSizeToContainChildren();
  }
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost )
  {
    Gold->Set( goldCost );
    Lumber->Set( lumberCost );
    Stone->Set( stoneCost );
  }
};

// a widget that shows usage costs
class UseWidget : public HotSpot
{
  ImageWidget *mana;
  UseWidget( FString name ) : HotSpot( name )
  {
  }
  virtual ~UseWidget(){}
};

// Describes the cost of something
// +-------------------------------------+
// | Barracks                            |
// | [] gold [] lumber [] stone          |
// | Barracks are used to build militia. |
// +-------------------------------------+
class CostWidget : public StackPanel
{
  TextWidget* TopText;
  ResourcesWidget* Cost;
  TextWidget* BottomText;

public:
  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostWidget( UTexture* bkg ) : StackPanel( "CostWidget", bkg )
  {
    Pad = FVector2D(16,13);
    TopText = new TextWidget( "TopText" );
    TopText->Align = TopCenter;
    StackBottom( TopText );

    Cost = new ResourcesWidget( "CostWidget's ResourcesWidget", 16, 4 );
    Cost->Align = TopCenter;
    StackBottom(Cost);

    BottomText = new TextWidget( "BottomText" );
    BottomText->Align = TopCenter;
    StackBottom(BottomText);

    recomputeSizeToContainChildren();
  }
  virtual ~CostWidget(){}
  void Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
  {
    TopText->Set( top );
    Cost->SetValues( goldCost, lumberCost, stoneCost );
    BottomText->Set( bottom );
    dirty = 1;
  }
};

class Tooltip : public ITextWidget
{
public:
  Tooltip( UTexture* bkg, FString txt ) 
    : ITextWidget( "Tooltip", bkg, txt, CenterCenter )
  {
  }
  virtual ~Tooltip(){}

  // The tooltip can be set in a callback, so it may be reset
  // outside of a render which means that the text size
  // measurement will be wrong. That's why we have the dirty bit.
  void Set( FString txt, float timeout )
  {
    ITextWidget::Set( txt );
    displayTime = timeout;
  }
};

// A slotPalette typically contains 
class SlotPalette : public ImageWidget
{
  HotSpot* Drag;
  int Rows, Cols;
public:
  FVector2D EntrySize;
  static UTexture* SlotPaletteTexture;

  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( FString name, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
    ImageWidget( name, bkg ), Rows(rows), Cols(cols), EntrySize(entrySize)
  {
    Pad = pad;
    // Init w/ # slots used in this palette
    // The stock size of the width is 100px/slot.
    // We re-calculate the slotsize though based on # slots used.
    Drag = 0;
    SetNumSlots( rows, cols );
  }
  virtual ~SlotPalette(){}

  int GetNumSlots() { return Rows*Cols; }
  FVector2D GetSlotPosition( int i )
  {
    int row = i / Cols;
    int col = i % Cols;
    return FVector2D( col * EntrySize.X, row * EntrySize.Y );
  }

  // Direct-children of SlotPalette must be ImageWidget*.
  // Guaranteed to be ImageWidget* because Add is protected.
  // External users of class cannot add a textwidget for example to the slotpanel.
  // (make sure you don't add children to slotpanel that are not imageWidgets)
  // the children of an imagewidget CAN be textnodes etc.
  Clock* GetSlot( int i )
  {
    if( i < 0 || i >= children.size() )
    {
      LOG(  "SlotPalette::GetSlot(%d) oob", i );
      return 0;
    }
    return (Clock*)children[i];
  }

  // Gets you the adjusted size dimensions of a size
  // if slots are (100x100) and you send a (150x100) icon,
  // the icon resizes to (100x66)
  // Computed by:
  //   scales = 100 / 150, 100 / 100 = (2/3, 1)
  //   take the minimum scale and scale the icon down by it (mult)
  //   (150x100) * (2/3,2/3) = (100x66)
  // if the slots are (100x100) and you send a (75x60) texture
  // then 
  //   scales = 100 / 75, 100 / 60 = (4/3, 5/3)
  // scale up:
  //   (75,60) * (4/3,4/3) = (100x80) [min scale is correct value to use]
  // (200x300) 
  //   scales = 100 / 200, 100 / 300 = (1/2,1/3)
  FVector2D GetAdjustedSize( FVector2D size )
  {
    // Calculate the % scale to scale up/down by
    FVector2D scales = EntrySize / size;
    // icons are always square so no stretch occurs to src icon images
    float minScale = scales.GetMin();
    // minScale works b/c if should scale up then its the min scale up to make it fit
    // if scale down, then its the min scale to make the larger side fit
    FVector2D adjSize = size * minScale;  // multiply by smaller of two scales to shrink to fit.
    return adjSize;
  }

  void AdjustPosition( int i )
  {
    if( i < 0 || i >= children.size() )  return;

    // get the size of entry i
    ITextWidget *tw = GetSlot( i );
    FVector2D adjSize = GetAdjustedSize( tw->Size );
    FVector2D diff = EntrySize - adjSize; // Move the position from stock pos to adj pos
    tw->Margin = GetSlotPosition( i ) + diff/2;
  }

  ITextWidget* SetSlotTexture( int i, UTexture* tex )
  {
    if( i < 0 || i >= children.size() ) {
      LOG( "SlotPalette::SetSlotTexture() invalid slot" );
      return 0;
    }
    if( !tex ) {
      LOG( "Texture not set in slotpalette %s", *Name );
      return 0;
    }
    FVector2D texSize( tex->GetSurfaceWidth(), tex->GetSurfaceHeight() );
    ITextWidget* slot = GetSlot( i );
    slot->Tex = tex;
    // actual position of the icon in the slot is a bit different than just std layout pos
    AdjustPosition(i);
    // adjust it a little bit based on the slot entry
    return slot;
  }

  // Widget's by themselves are just hotspots
  vector<ITextWidget*> SetNumSlots( int rows, int cols )
  {
    Clear();    // Remove the old SlotEntries
    vector<ITextWidget*> slots;
    Rows = rows;
    Cols = cols;
    
    // If there are NO slots, then return here
    if( !rows && !cols ) {
      Hide(); // hides the SlotPalette when empty, this is the usual side effect of emptying all slots
      LOG(  "SlotPalette %s has no entries", *Name );
      return slots; // don't change the size vars when 0 size because
      // it will corrupt the Size variables
    }

    int numSlots = rows*cols;
    // Measure the UV coordinates used since the texture is 6x6
    uv = FVector2D( cols/6., rows/6. ); // The texture is 6x6 blocks
    
    // The size of this widget set here.
    for( int i = 0; i < numSlots; i++ )
    {
      // initialize a bunch of cooldown counters
      FString name = FString::Printf( TEXT("SP `%s`'s Clock %d"), *Name, i+1 );
      Clock *cooldown = new Clock( name, EntrySize, 
        CooldownCounter(), FLinearColor( 0.15, 0.15, 0.15, 0.15 ) );
      Add( cooldown );
      AdjustPosition( i );
      slots.push_back( cooldown );
    }

    // Change the size of the panel to being correct size to bound the children
    recomputeSizeToContainChildren();
    return slots;
  }
};

class FlowPanel : public ImageWidget
{
public:
  int Rows, Cols;
  FlowPanel( FString name, UTexture* tex, int rows, int cols, FVector2D size ) :
    ImageWidget( name, tex, size ), Rows( rows ), Cols( cols )
  {
  }
  void reflow(){ reflow( Size ); }
  void reflow( FVector2D size )
  {
    Size = size;
    FVector2D ColsRows( Cols, Rows );
    FVector2D tileDims = ( size - (Pad*ColsRows) ) / ColsRows;
    // +---+---+---+
    // |   |   |   |
    // +---+---+---+
    // |   |   |   |
    // +---+---+---+
    //   0   1   2
    // Set new bounds for container
    for( int i = 0; i < children.size(); i++ )
    {
      int row = i / Cols;
      int col = i % Cols;
      FVector2D pos = Pad/2 + ( Pad + tileDims ) * FVector2D( col, row );
      children[i]->Margin = pos;
      children[i]->Size = tileDims;
    }
  }

  virtual void Move( float t )
  {
    reflow( Size );
  }

  void Set( AGameObject* go );
  void Set( vector<AGameObject*> objects, int rows, int cols );
};

class StatsPanel : public StackPanel
{
public:
  StatsPanel( FString name, UTexture* tex, FLinearColor color ) :
    StackPanel( name, tex, color ) { }
  void Set( AGameObject* go );
};

class Actions;

// An abilities panel is a slotpalette but with the ability to populate
// from a game object's capabiltiies
class AbilitiesPanel : public SlotPalette
{
  Actions* actions;
public:
  // The Build button appears for Peasant-class units that build buildings.
  static UTexture* BuildButtonTexture;
  ITextWidget* buildButton;

  // parent container is the actions panel
  AbilitiesPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( AGameObject *go );
};

class BuildPanel : public SlotPalette
{
  Actions *actions;
public:
  BuildPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( AGameObject *go );
};

// set of slots for items.
class ItemBelt : public SlotPalette
{
public:
  ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( AUnit* go );
};

class Buffs : public StackPanel
{
public:
  AGameObject* selected;
  Buffs( FString name, UTexture* bkg ) : StackPanel( name, bkg ), selected( 0 ) { }
  void Set( AGameObject* go );
  virtual void Move( float t );
};

class Minimap : public ImageWidget
{
  Border* borders;
public:
  Minimap( UTexture* icon, float borderSize, FLinearColor borderColor ) : 
    ImageWidget( "minimap", icon )
  {
    FBox2DU box( FVector2D(0,0), Size );
    borders = new Border( "Minimap border", box, borderSize, borderColor );
    Add( borders );
  }
  virtual ~Minimap(){}
};

class Actions : public HotSpot
{
public:
  AbilitiesPanel* abilities; // the group of abilities this unit is capable of
  BuildPanel* buildPanel; // the group of buildPanel this unit can build
  Actions( FString name, FVector2D entrySize ) : HotSpot( name )
  {
    abilities = new AbilitiesPanel( this, SlotPalette::SlotPaletteTexture, 2, 3,
      entrySize, FVector2D(8,8) );
    Add( abilities );

    buildPanel = new BuildPanel( this, SlotPalette::SlotPaletteTexture, 2, 3,
      entrySize, FVector2D(8,8) );
    buildPanel->Hide();
    Add( buildPanel );

    recomputeSizeToContainChildren();
  }

  void ShowAbilitiesPanel(){ abilities->Show(); buildPanel->Hide(); }
  void ShowBuildingsPanel(){ abilities->Hide(); buildPanel->Show(); }
};

// The right-side panel
class SidePanel : public StackPanel
{
public:
  // +-----------+
  // | portrait  |
  // +-----------+
  // | unitStats |
  // +-----------+
  // | abilities |
  // +-----------+
  // | minimap   |
  // +-----------+
  FlowPanel* portraits;    // pictoral representation of selected unit
  StatsPanel* stats;  // The stats of the last selected unit
  Actions* actions; // contains both the abilities & buildings pages
  Minimap* minimap;       // the minimap widget for displaying the world map

  // For the group of selected units.
  SidePanel( UTexture* texPanelBkg, UTexture* PortraitTexture, UTexture* MinimapTexture,
    FVector2D size, FVector2D spacing ) :
    StackPanel( "Sidepanel", texPanelBkg, FLinearColor::White )
  {
    Pad = spacing;

    portraits = new FlowPanel( "FlowPanel", PortraitTexture, 1, 1, FVector2D( size.X, size.Y / 4 ) );
    portraits->Align = TopCenter;
    StackBottom( portraits );

    stats = new StatsPanel( "Stats", SolidWidget::SolidWhiteTexture, FLinearColor(0.15,0.15,0.15,0.2) );
    StackBottom( stats );
    
    actions = new Actions( "Actions", FVector2D(size.X/3,size.X/3) );
    StackBottom( actions );

    minimap = new Minimap( MinimapTexture, 4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    StackBottom( minimap );
    
    // Add the leftBorder in as last child, because it takes up full height,
    // and stackpanel will stack it in below the border
    SolidWidget *leftBorder = new SolidWidget( "panel leftborder",
      FVector2D( 4, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    leftBorder->Margin = - Pad + FVector2D( -4, 0 );
    Add( leftBorder );

    recomputeSizeToContainChildren();
  }
  virtual ~SidePanel(){}
};

class StatusBar : public SolidWidget
{
public:
  TextWidget* Text;
  StatusBar( FLinearColor bkgColor ) : SolidWidget( "status bar", bkgColor )
  {
    Align = Bottom | HFull;
    Text = new TextWidget( "status text" );
    Add( Text );
  }

  void Set( FString text )
  {
    Text->Set( text );
  }
};

class Controls : public StackPanel
{
public:
  ImageWidget *Pause;

  Controls( UTexture* texPause ) : StackPanel( "Controls", 0 )
  {
    Pause = new ImageWidget( "pause", texPause );
    StackRight( Pause );
  }
};

// This is the object that describes the units we are building
class BuildQueue : public StackPanel
{
public:
  FVector2D EntrySize;
  // A BuildQueue contains a list of things that are being built
  BuildQueue( FString name, UTexture* bkgTex, FVector2D entrySize ) : 
    StackPanel( name, bkgTex ), EntrySize( entrySize )
  {
  }
  void Set( AGameObject* go );
};

#pragma region screens and layers
class Screen : public HotSpot
{
public:
  Screen( FString name ) : HotSpot( name ){
    Align = Full;
  }
};

class MapSelectionScreen : public Screen
{
public:
  ImageWidget *Logo;
  StackPanel *MapFiles;
  FVector2D MapFileEntrySize;
  ImageWidget *Thumbnail;
  ITextWidget *OKButton;
  
  UTexture* MapSlotEntryBkg;
  ITextWidget* Selected;
  UFont* Font;

  // Constructs the MapSelection Screen.
  MapSelectionScreen( UTexture* logoTex,
    UTexture* mapFilesBkg, UTexture* mapSlotEntryBkg,
    UTexture* thumbnailTex, FVector2D entrySize, UFont* font ) :
    Screen( "MapSelectScreen" ),
    MapFileEntrySize( entrySize ),
    MapSlotEntryBkg( mapSlotEntryBkg ),
    Selected( 0 ), Font( font )
  {
    // Throw in the title
    Logo = new ImageWidget( "Logo", logoTex );
    Logo->Align = TopLeft;
    Logo->Margin = FVector2D( 75, 50 );
    Add( Logo );
    
    // The stack of menu items.
    MapFiles = new StackPanel( "Map files", mapFilesBkg );
    MapFiles->Align = TopLeft;
    MapFiles->Margin = FVector2D( 12, 12 );
    MapFiles->Pad = FVector2D( 8,8 );
    Add( MapFiles );

    Thumbnail = new ImageWidget( "Thumbnail", thumbnailTex );
    Thumbnail->Align = CenterRight;
    Thumbnail->Margin = FVector2D( 80, 0 );
    Add( Thumbnail );

    OKButton = new ITextWidget( "Okbutton", mapSlotEntryBkg, entrySize, "OK", CenterCenter );
    OKButton->Align = BottomRight;
    OKButton->Margin = FVector2D( 20, 10 );
    
    Add( OKButton );
  }

  // Adds a slot
  ITextWidget* AddText( FString ftext, int textAlignment )
  {
    ITextWidget* text = new ITextWidget( FS("mss text %s",*ftext),
      MapSlotEntryBkg, MapFileEntrySize, ftext, textAlignment );
    text->OnMouseDownLeft = [this,text](FVector2D mouse){
      Select( text );
      return Consumed;
    };
    MapFiles->StackBottom( text );
    return text;
  }

  // Make widget selected
  void Select( ITextWidget *widget )
  {
    LOG(  "Selected %s", *widget->GetText() ) ;

    // turn prev selected back to white
    if( Selected )  Selected->Color = FColor::White;
    Selected = widget;
    Selected->Color = FColor::Yellow;
  }
};

/// Displays mission objectives on screen
class MissionObjectivesScreen : public Screen
{
  UTexture *SlotBkg;
  StackPanel *Objectives;
public:
  MissionObjectivesScreen( UTexture *bkg, UTexture *slotBkg,
    FVector2D slotSize, FVector2D pad ) :
    Screen( "MissionObjectives" )
  {
    Objectives = new StackPanel( "Objectives stackpanel", bkg );
  }

  // Add a series of mission objectives here
  ITextWidget* AddText( FString text, int align )
  {
    ITextWidget *tw = new ITextWidget( FS("MBText `%s`",*text), SlotBkg, FVector2D( 100, 40 ), text, CenterCenter );
    tw->Margin = FVector2D( 10, 0 );
    tw->Align = CenterLeft;
    return Add( tw );
  }
};
#pragma endregion

// The in-game chrome object, consisting of many subpanels etc
// +---------------------------------+
// | resourcesW        | rightPanel  |
// |                   |  Portrait   |
// |                   |  unitStats  |
// |                   |  abilities  |
// |                   | +---------+ |
// |  +-------------+  | | minimap | |
// |  | itemBelt/bq |  | +---------+ |
// +--buffs--------------------------+
class GameChrome : public Screen
{
public:
  ResourcesWidget* resources; // The resources widget in the top left
  SidePanel* rightPanel;      // containing: portrait, unitStats, abilities, minimap
  CostWidget* costWidget;     // a flyover cost of the hovered item from 
  Tooltip* tooltip;

  ItemBelt* itemBelt;  // things this unit is carrying, at the bottom of the screen
  Buffs* buffs;      // list of buffs applied to this unit
  BuildQueue* buildQueue; // Queue of things we are building (in order)
  Controls* controls;     // 
  
  GameChrome( FString name ) : Screen( name )
  {
    resources = new ResourcesWidget( "Main ResourcesWidget", 16, 4 );
    Add( resources );

    rightPanel = 0, costWidget = 0, tooltip = 0, itemBelt = 0,
    buffs = 0, buildQueue = 0, controls = 0;
  }

  // When an object is selected
  void Select( AGameObject *go );
  void Select( vector<AGameObject*> objects, int rows, int cols );
};

class Title : public Screen
{
public:
  ImageWidget *graphic;
  Title( UTexture* titleTex ) : Screen( "Title screen" )
  {
    graphic = new ImageWidget( "title graphic", titleTex );
    graphic->Align = Full;
    Add( graphic );
  }
};

// The root UI component
class UserInterface : public Screen
{
public:
  Title* titleScreen; // the title screen.
  GameChrome* gameChrome; // in-game chrome.
  MapSelectionScreen* mapSelectionScreen;
  MissionObjectivesScreen* missionObjectivesScreen;

  // widgets/selectors
  StatusBar* statusBar; // the bottom status bar
  MouseSelectBox* selectBox;
  ImageWidget* mouseCursor;

  UserInterface( FVector2D size ) : Screen( "UI-root" )
  {
    Size = size;
    titleScreen = 0;
    gameChrome = new GameChrome( "GameChrome" );
    Add( gameChrome );
    mapSelectionScreen = 0, missionObjectivesScreen = 0;
    
    // Initialize the status bar
    statusBar = new StatusBar( FLinearColor::Black );
    Add( statusBar );
    HotSpot::TooltipWidget = statusBar->Text; // Setup the default tooltip location

    // connect the mouse drag functions 
    selectBox = new MouseSelectBox( "MouseSelectBox border",
      FBox2DU( FVector2D(100,100), FVector2D(50,50) ),
      8.f, FLinearColor::Green);
    Add( selectBox );
    selectBox->Hide();
    
    mouseCursor = 0;
  }

  // don't need to expli  citly delete children of the UI object,
  // they get deleted in the base dtor.
  virtual ~UserInterface(){ /* no delete */ }

  void SetScreen( int mode )
  {
    // This hides all direct children
    //HideChildren(); // hide all widgets, then show just the ones we want
    // The above line was dangerous because if you're adding more elts
    // because will have to explicitly show each below

    // explicitly hide elts not to be shown
    titleScreen->Hide(); // the title screen.
    gameChrome->Hide(); // in-game chrome.
    mapSelectionScreen->Hide();
    missionObjectivesScreen->Hide();
    
    // Then go ahead and show elts depending on mode we're in
    if( mode == TitleScreen )  titleScreen->Show();
    else if( mode == MapSelect )  mapSelectionScreen->Show();
    else if( mode == Running )  gameChrome->Show();

    statusBar->Show(); // Show the status bar at all times
    mouseCursor->Show();
  }

  void Select( AGameObject* go );
};

