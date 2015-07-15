#pragma once

#include <vector>
#include <functional>
#include <string>
#include "GlobalFunctions.h"
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
  FBox2DU()
  {
    Min.X = Min.Y = 1e6f;
    Max.X = Max.Y = -1e6f;
  }

  float left(){ return Min.X; }
	float right(){ return Max.X; }
	float top(){ return Min.Y; }
	float bottom(){ return Max.Y; }

  FVector2D TL() { return Min; }
  FVector2D TR() { return FVector2D( Max.X, Min.Y ); }
  FVector2D BR() { return Max; }
  FVector2D BL() { return FVector2D( Min.X, Max.Y ); }
  
  void print(FString msg) {
    UE_LOG( LogTemp, Warning, TEXT( "%s fbox2du bounds min=(%f %f) max=(%f %f) meas=(%f %f)"),
      *msg, Min.X, Min.Y, Max.X, Max.Y, Size().X, Size().Y );
  }
  FVector2D Size() { 
    return Max - Min;
  }
};

class AGameObject;

struct HotSpot
{
  string Name; //the name of the widget
  static AHUD* hud;
  enum HAlign { Left=1<<0, HCenter=1<<1, Right =1<<2, ToLeftOfParent=1<<3, ToRightOfParent=1<<4,   ToHCenterInParent=1<<5 };
  enum VAlign { Top =1<<6, VCenter=1<<7, Bottom=1<<8, ToTopOfParent=1<<9,  ToBottomOfParent=1<<10, ToVCenterInParent=1<<11 };
  int align;
  bool hidden;
  float displayTime; // Amount of time to display remaining
  FVector2D Pos, Size;
  FLinearColor Color;
  // Associations with AGameObject's contained in this hotspot. Empty if no objects
  // are associated with the spot (used for item slots)
  vector<AGameObject*> Objects;
protected:
  HotSpot* Parent;
  vector<HotSpot*> children;  // Children of this hotspot.
  bool dirty;  // Set if the widget's bounds need to be remeasured
  bool bubbleUp; // When set, the event can bubbles up through the widget
public:
  // Returns true (1) if event should be consumed.
  function< int (FVector2D mouse) > OnClicked;
  function< int (FVector2D mouse) > OnRightClicked;
  function< int (FVector2D mouse) > OnHover;
  function< int (FVector2D mouse) > OnDrag;
  // A function that runs when the widget is dropped @ certain location
  function< int (FVector2D mouse) > OnDrop;
  void defaults()
  {
    align = Top | Left;
    hidden = 0;
    displayTime = 0.f;
    Pos = FVector2D(0,0);
    Size = FVector2D(32,32);
    Color = FLinearColor::White;
    Parent = 0;
    dirty = 1;
    bubbleUp = 1;
  }
  HotSpot() {
    defaults();
  }
  HotSpot( FVector2D pos ) {
    defaults();
    Pos = pos;
  }
  HotSpot( FVector2D pos, FVector2D size ) {
    defaults();
    Pos = pos;
    Size = size;
  }
  virtual ~HotSpot(){ Clear(); }
protected:
  virtual void render( FVector2D offset )
  {
    if( hidden ) return;
    // When the base class renders, it calls render on all the children.
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->render( offset + Pos );
  }
public:
  void Add( HotSpot* w ) {
    if( w->Parent )  w->Orphan();
    w->Parent = this;
    children.push_back( w );
  }
  // Orphans the child from its parent
  void Orphan()
  {
    if( Parent )
    {
      remove( Parent->children, this );
      Parent = 0;
    }
  }
  // all calls to render a widget externally route through here
  // they all kick-off to subclass::render( FVector2D(0,0) )
  virtual void render(){ if( hidden ) return; render( FVector2D(0,0) ); }
  void repad( FVector2D pad ) {
    // repad's the widget to enclose everything,
    // including the children.
    // *** widget grows infinitely if you push a child to the edge of the bounds
    // each frame, unless you keep it inside within an epsilon. ***
    Size = GetChildBounds().Size() + pad;
  }

  // reflushes to parent widget
  void reflushToParent( FVector2D pad ) {
    if( !Parent )
    {
      UE_LOG( LogTemp, Warning, TEXT( "Cannot reflush to null parent" ) );
      return; // cannot reflush
    }
    FVector2D PSize = Parent->Size;

    if( align & Left )  Pos.X = pad.X;
    else if( align & Right )  Pos.X = PSize.X - Size.X - pad.X;
    else if( align & HCenter )  Pos.X = (PSize.X - Size.X)/2; // centering
    else if( align & ToLeftOfParent )  Pos.X = -Size.X;
    else if( align & ToRightOfParent )  Pos.X = PSize.X;
    else if( align & ToHCenterInParent )  Pos.X = (PSize.X - Size.X)/2;

    if( align & Top )  Pos.Y = pad.Y;
    else if( align & Bottom )  Pos.Y = PSize.Y + Size.Y + pad.Y;
    else if( align & VCenter ) Pos.Y = (PSize.Y - Size.Y)/2;
    else if( align & ToBottomOfParent )  Pos.Y = PSize.Y;
    else if( align & ToTopOfParent )  Pos.Y = -Size.Y;
    else if( align & ToVCenterInParent )  Pos.Y = -Size.Y;
  }

  float left(){ return Pos.X; }
	float right(){ return Pos.X + Size.X; }
	float top(){ return Pos.Y; }
	float bottom(){ return Pos.Y + Size.Y; }
  FVector2D GetAbsPos(){
    // traces back up to the parent and finds the absolute position of this widget
    HotSpot *h = this;
    FVector2D p = Pos;
    while( h->Parent ) { h = h->Parent ; p += h->Pos ; }
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
  
  FBox2DU GetBounds( FVector2D offset ){
    FBox2DU bounds;

    // bound this and all children
    bounds += offset + Pos;
    bounds += offset + Pos + Size;
    for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( offset + Pos );
    }
    return bounds;
  }
  FBox2DU GetBounds()
  {
    return GetBounds( FVector2D(0,0) );
  }
  // Get bounds of children, excluding this
  FBox2DU GetChildBounds()
  {
    FBox2DU bounds;
    for( int i = 0; i < children.size(); i++ ) {
      bounds += children[i]->GetBounds( Pos );
    }
    return bounds;
  }
  // Get the bounds of just this widget, excluding the children
  // used for when flushing position of something inside a widget
  // towards the right or left. if you get bounds including the
  // children it will tend accumulate an overflow right/left if it overflows.
  FBox2DU GetBoundsExcludingChildren()
  {
    FBox2DU bounds;
    bounds += Pos;
    bounds += Pos + Size;
    return bounds;
  }
  void Clear(){
    for( int i = 0; i < children.size(); i++ )
      delete children[i];
    children.clear();
  }

  bool Act( FVector2D v, FVector2D offset, function< int (FVector2D) > HotSpot::* f )
  {
    // Look into deepest child first.
    for( int i = 0; i < children.size(); i++ )
    {
      // check if a child is hit by the click.
      HotSpot *child = children[i];
      bool h = child->Act( v, offset+Pos, f );  // offset accumulates absolute position offset of widget
      if( h )  return 1; // If the hit was intercepted, return.
    }

    // Actually perform hit checking, if this provides the function f.
    if( this->*f && hit( v - offset ) )
    {
      (this->*f)( v - offset - Pos ); // pass in relative coordinates
      // If the widget doesn't bubble events, then you must return that it hit here

      // If the widget consumes events, then halt propagation
      // Deepest child gets searched, then we return from here
      if( !bubbleUp ) return 1;
    }
    return 0; //not hit or bubbles are allowed.
  }

  // returns true when the point hits the widget (or a child)
  bool Click( FVector2D v, FVector2D offset )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnClicked );
  }
  bool RightClick( FVector2D v, FVector2D offset )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnRightClicked );
  }
  bool Hover( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnHover );
  }
  bool Drag( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnDrag );
  }
  bool Drop( FVector2D v )
  {
    return Act( v, FVector2D(0,0), &HotSpot::OnDrop );
  }

};

struct TextWidget : public HotSpot
{
private:
  FString Text;

public:
  TextWidget( FString fs ) : Text( fs )
  {
    // Measures size in Text using HUD object (must be init & ready)
    Name = "TextWidget";
    SetText( fs );
  }
  TextWidget( FString fs, FVector2D pos ) : HotSpot( pos ), Text( fs )
  {
    Name = "TextWidget";
    SetText( fs );
  }
  virtual ~TextWidget(){}

  // Rendered size of text
  void SetText( FString text )
  {
    Text = text;
    dirty = 1;
    //Measure(); // call may fail here, but put it here anyway.
    // [[ cannot call measure here, since may happen
    //    during callback due to mouse motion etc. ]]
  }

  void Measure()
  {
    hud->GetTextSize( Text, Size.X, Size.Y );
    // ERROR IF THE HUD is not currently ready
    // to draw (ie canvas size won't be available)
  }
  
protected:
  // We call Measure() each call to render() since text cannot be measured except when
  // canvas is ready (ie during render() calls)
  virtual void render( FVector2D offset ) override
  {
    if( hidden ) return;
    if( dirty ) { // It seems in first calls to render(), Measure() does not properly measure text width
      Measure();
      dirty = 0;
    }
    hud->DrawText( Text, Color, Pos.X + offset.X, Pos.Y + offset.Y );
    HotSpot::render( offset );
  }
};

struct ImageWidget : public HotSpot
{
  UTexture* Icon;
  FVector2D uv; // The maximum coordinates of the UV texturing
  FVector2D hotpoint; // Usually top left corner (0,0), meaning will render from topleft corner.
  // if its half size, then it will render from the center (such as when an imageWidget is being
  // click-dragged
  FLinearColor Color;
  
  ImageWidget( UTexture* pic ) : Icon( pic ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White)
  {
    Name = "ImageWidget";
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D pos ) :
    Icon( pic ), HotSpot( pos ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White)
  {
    Name = "ImageWidget";
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D pos, FVector2D size ) :
    Icon( pic ), HotSpot( pos, size ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White)
  {
    Name = "ImageWidget";
  }
  virtual ~ImageWidget(){}

protected:
  virtual void render( FVector2D offset ) override 
  {
    if( hidden ) return;
    FVector2D renderPos = Pos - hotpoint;
    if( !Icon )
    {
      // We have to remove this comment for normal ops because
      // sometimes we want to have null texes eg in slotpalette items when
      // no item is present
      UE_LOG( LogTemp, Warning, TEXT( "Texture not set for ImageWidget" ) );
      // render should not be called when the texture is hidden
    }

    // If hidden, do not draw
    hud->DrawTexture( Icon, renderPos.X + offset.X, renderPos.Y + offset.Y, Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color );
    HotSpot::render( offset );
  }
};

struct SolidWidget : public ImageWidget
{
  static UTexture* SolidWhiteTexture;
  SolidWidget( FLinearColor color ) : ImageWidget( SolidWhiteTexture )
  {
    Color = color;
  }
  SolidWidget( FVector2D pos, FVector2D size, FLinearColor color ) : ImageWidget( SolidWhiteTexture, pos, size )
  {
    Color = color;
  }
  virtual ~SolidWidget(){}
};

struct CooldownPie : public HotSpot
{
  float Time, TotalTime; // time to cooldown
  // Creates a cooldownpie whose animation progresses over a period
  CooldownPie( FVector2D pos, FVector2D size, float t ) :
    HotSpot( pos, size ), Time(0.f), TotalTime(t)
  {
  }
  void update( float t ) { Time -= t; }
  void render()
  {
    float p = Time/TotalTime;

    // Display the interpolate animation
    // Don't have an animation for this yet.. but it should
    //hud->DrawLine( 
  }
  virtual ~CooldownPie(){}
};

// starting offset point and group of widgets to draw
// Make a custom widget for the overlays etc
struct ResourcesWidget : public HotSpot
{
  TextWidget *gold, *lumber, *stone;
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;
  int Px; // size of the icons
  int Spacing; // spacing between widgets

  // Add in all the subwidgets
  ResourcesWidget(int pxSize, int spacing, FVector2D pos) : HotSpot(pos), Px(pxSize), Spacing(spacing)
  {
    // +-----------------+
    // |G1000 W1000 S1000|
    // +-----------------+
    // The 3 resource types
    ImageWidget* i = new ImageWidget( GoldTexture, FVector2D(0,0), FVector2D(Px,Px) );
    gold = new TextWidget( "1000", FVector2D(Px,0) );
    i->Add( gold );
    Add( i );
    
    FBox2DU b = i->GetBounds();
    i = new ImageWidget( LumberTexture, FVector2D(b.right()+Spacing,0), FVector2D(Px,Px) );
    lumber = new TextWidget( "1000", FVector2D(Px,0) );
    i->Add( lumber );
    Add( i );
    
    b = i->GetBounds();
    i = new ImageWidget( StoneTexture, FVector2D(b.right()+Spacing,0), FVector2D(Px,Px) );
    stone = new TextWidget( "1000", FVector2D(Px,0) );
    i->Add( stone );
    Add( i );

    Size = GetChildBounds().Size(); // Size of the entire resources widget
  }
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost )
  {
    gold->SetText( FString::Printf( TEXT("%d"), goldCost ) );
    lumber->SetText( FString::Printf( TEXT("%d"), lumberCost ) );
    stone->SetText( FString::Printf( TEXT("%d"), stoneCost ) );
  }
};

// a widget that shows usage costs
struct UseWidget : public HotSpot
{
  ImageWidget *mana;
  UseWidget(){}
  virtual ~UseWidget(){}
};

// +-------------------------------------+
// | Barracks                            |
// | [] gold [] lumber [] stone          |
// | Barracks are used to build militia. |
// +-------------------------------------+
struct CostWidget : public ImageWidget
{
  TextWidget* topText;
  ResourcesWidget* cost;
  TextWidget* bottomText;
  FVector2D Pad; // padding on the background image

  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostWidget( UTexture* bkg, FVector2D pad, float vSpacing ) : ImageWidget( bkg ), Pad(pad)
  {
    topText = new TextWidget( "TopText", pad/2 );
    Add(topText);

    //UE_LOG( LogTemp, Warning, TEXT( "topText->GetBounds()" ) );
    FBox2DU bounds = topText->GetBounds();
    bounds.print("toptext");
    cost = new ResourcesWidget(16, 4, FVector2D( pad.X/2, topText->GetBounds().bottom() + vSpacing ) );
    Add(cost);

    //UE_LOG( LogTemp, Warning, TEXT( "cost pos: %f %f" ), cost->Pos.X, cost->Pos.Y );
    //UE_LOG( LogTemp, Warning, TEXT( "cost->GetBounds()" ) );
    bounds = cost->GetBounds();
    bounds.print("costbounds");
    bottomText = new TextWidget( "BottomText", FVector2D( pad.X/2, cost->GetBounds().bottom() + vSpacing ) );
    Add(bottomText);

    //UE_LOG( LogTemp, Warning, TEXT( "bottomText->GetBounds()" ) );
    bounds = bottomText->GetBounds();
    bounds.print("bottomText");
    
    repad( Pad );
  }
  virtual ~CostWidget(){}
  void Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
  {
    topText->SetText( top );
    cost->SetValues( goldCost, lumberCost, stoneCost );
    bottomText->SetText( bottom );
    dirty = 1;
  }

  virtual void render( FVector2D offset ) override
  {
    if( hidden ) return;
    if( dirty )
    {
      repad( Pad ) ;
      dirty = 0 ;
    }
    ImageWidget::render( offset );
  }
};

struct Tooltip : public ImageWidget
{
private:
  TextWidget* Text;

public:
  FVector2D Pad;
  Tooltip( UTexture* bkg, FString txt, FVector2D pad ) : ImageWidget( bkg ), Pad(pad)
  {
    Text = new TextWidget( txt );
    Add( Text );
    Set( txt );
  }
  virtual ~Tooltip(){}
  void Set( FString txt )
  {
    Text->SetText( txt );
    Size = Text->Size + Pad; /// cannot re-measure size here.
    Text->Pos = Pos + Pad/2;
    hidden = 0; // unhide the tooltip if it was hidden before
    dirty = 1;
  }

  virtual void render( FVector2D offset ) override {
    if( hidden ) return;
    //re-measure the background's size if the text has changed
    if( dirty )
    {
      repad( Pad );
      dirty = 0;
    }
    ImageWidget::render( offset );
  }
};

struct SlotPalette;

struct SlotEntry : public ImageWidget
{
  SlotPalette* Parent; // hides base member
  int Quantity; // the numeric quantity remaining
  TextWidget* TextQuantity;   // The quantity of the item
  
  SlotEntry( SlotPalette *palette, UTexture* tex, FVector2D pos, FVector2D size, int qty ) :
    Parent(palette), ImageWidget( tex, pos, size ), Quantity( qty )
  {
    TextQuantity = new TextWidget( "A" );
    TextQuantity->align = Right|Bottom;
    Add( TextQuantity );
    dirty = 1;
    //SetQuantity( qty );
    //hidden = 1; // defaults hidden with NULL texture.
  }

  void SetTexture( UTexture* tex, FVector2D originalPos );

  void SetQuantity( int quantity )
  {
    Quantity = quantity;
    TextQuantity->SetText( FString::Printf( TEXT("%d"), quantity ) );
    dirty = 1;
    // 0 qty means qty widget disappears.
    //if( !quantity )  TextQuantity->hidden = 1;
    //else  TextQuantity->hidden = 0;
  }

  virtual void render( FVector2D offset ) override;
};

struct SlotPalette : public ImageWidget
{
  FVector2D SlotSize;
  ImageWidget* Drag;
  static UTexture* SlotPaletteTexture;
  FVector2D Pad;
  int Rows, Cols;

  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( UTexture* bkg, FVector2D pos, FVector2D size,
    int rows, int cols, FVector2D pad ) : 
    Rows(rows), Cols(cols), Pad(pad), ImageWidget( bkg, pos, size )
  {
    // Init w/ # slots used in this palette
    // The stock size of the width is 100px/slot.
    // We re-calculate the slotsize though based on # slots used.
    Drag = 0;
    SetNumSlots( rows, cols );
  }
  virtual ~SlotPalette(){}

  FVector2D GetSlotPosition( int i )
  {
    FVector2D v;
    int row = i / Cols;
    int col = i % Cols;
    v.X = col * SlotSize.X;
    v.Y = row * SlotSize.Y;
    return v;
  }

  // Direct-children of SlotPalette must be ImageWidget*.
  // Guaranteed to be ImageWidget* because Add is protected.
  // External users of class cannot add a textwidget for example to the slotpanel.
  // (make sure you don't add children to slotpanel that are not imageWidgets)
  // the children of an imagewidget CAN be textnodes etc.
  SlotEntry* GetSlot( int i )
  {
    return (SlotEntry*)children[i];
  }

  void ResetSlots()
  {
    // reset slot texture entries to nulls
    for( int i = 0; i < children.size(); i++ )
      children[i]->hidden = 1;
  }

  SlotEntry* SetSlotTexture( int i, UTexture* tex )
  {
    FVector2D texSize( tex->GetSurfaceWidth(), tex->GetSurfaceHeight() );
    SlotEntry* slot = GetSlot( i );
    slot->SetTexture( tex, GetSlotPosition(i) );
    return slot;
  }

  // Widget's by themselves are just hotspots
  vector<SlotEntry*> SetNumSlots( int rows, int cols )
  {
    Rows = rows;
    Cols = cols;
    vector<SlotEntry*> slots;

    hidden = !rows && !cols;
    if( hidden ) return slots; // don't change the size vars when 0 size because
    // it will corrupt the variables

    //SlotSize = Size / FVector2D( cols, rows );
    SlotSize.X = Size.X / cols;
    SlotSize.Y = Size.Y / rows;
    int numSlots = rows*cols;

    // The uv's are equal to 
    // Measure the UV coordinates used since the texture is 6x6
    uv = FVector2D( cols/6., rows/6. ); // The texture is 6x6 blocks
    Clear();    // Remove the old ImageWidgets.
    
    // The size of this widget set here.
    for( int i = 0; i < numSlots; i++ )
    {
      /// initialize with 0 of item
      SlotEntry *iw = new SlotEntry( this, Icon, GetSlotPosition(i), SlotSize, 0 );
      
      //iw->hidden = 1; // initialize as hidden, until texture is set.
      slots.push_back( iw );
      Add( iw );
    }

    // Change the size of the panel to being 
    Size = SlotSize * FVector2D( cols, rows );

    return slots;
  }
};

// Supports stacking-in of widgets from left/right or top/bottom
struct StackPanel : public HotSpot
{
  FVector2D IconSize;
  FVector2D Margins;  // Padding between entries
  StackPanel( FVector2D iconSize, FVector2D margins ) :
    HotSpot( FVector2D(0,0), FVector2D(0,0) ), IconSize( iconSize ), Margins( margins )
  {
  }
  StackPanel( FVector2D pos, FVector2D size, FVector2D iconSize, FVector2D margins ) :
    HotSpot( pos, size ), IconSize( iconSize ), Margins( margins )
  {
  }
  virtual ~StackPanel(){}
  
  //    _ _
  // > |_|_|
  void StackLeft( ImageWidget* w )
  {
    w->Pos.X = GetBounds().left() - w->Size.X - Margins.X;
    w->Size = IconSize;
    HotSpot::Add( w );
  }

  //  _ _
  // |_|_| <
  void StackRight( ImageWidget* w )
  {
    w->Pos.X = GetBounds().right() + Margins.X;
    w->Size = IconSize;
    HotSpot::Add( w );
  }

  //  v
  //  _
  // |_|
  // |_|
  void StackTop( ImageWidget* w )
  {
    w->Pos.Y = GetBounds().top() - Margins.Y - w->Size.Y;
    w->Size = IconSize;
    HotSpot::Add( w );
  }

  //  _
  // |_|
  // |_|
  //  ^
  void StackBottom( ImageWidget* w )
  {
    w->Pos.Y = GetBounds().bottom() + Margins.Y;
    w->Size = IconSize;
    HotSpot::Add( w );
  }
};

struct Minimap : public ImageWidget
{
  Minimap( UTexture* icon, FVector2D pos, FVector2D size,
    float borderSize, FLinearColor borderColor ) : 
    ImageWidget( icon, pos, size )
  {
    FBox2DU bounds = GetBounds();

    // tl -> tr
    Add( new SolidWidget( bounds.TL() - borderSize/2,
      FVector2D( size.X + borderSize, borderSize ), borderColor ) );
    
    // tr -> br
    Add( new SolidWidget( bounds.TR() - borderSize/2,
      FVector2D( borderSize, size.Y + borderSize ), borderColor ) );

    // bl -> br
    Add( new SolidWidget( bounds.BL() - borderSize/2,
      FVector2D( borderSize, size.Y + borderSize ), borderColor ) );

    // tl -> bl
    Add( new SolidWidget( bounds.TL() - borderSize/2,
      FVector2D( borderSize, size.Y + borderSize ), borderColor ) );
  }
  virtual ~Minimap(){}
};

struct Panel : public ImageWidget
{
  ImageWidget* portrait;  // pictoral representation of selected unit
  TextWidget* unitStats;  // The stats of the last selected unit
  SlotPalette* abilities; // 
  Minimap* minimap;       // the minimap widget for displaying the world map

  // For the group of selected units.
  Panel( UTexture* texPanelBkg, UTexture* texMinimap, FVector2D size, float margin ) :
    ImageWidget( texPanelBkg, FVector2D( 0, 0 ), size )
  {
    float s = size.X - 2*margin;
    portrait = new ImageWidget( 0, FVector2D( margin, margin ), FVector2D(s,s) );
    Add( portrait );

    unitStats = new TextWidget( "Stats:", FVector2D( margin, portrait->GetBounds().bottom()+margin ) );
    Add( unitStats );

    SolidWidget *leftBorder = new SolidWidget( FVector2D( -4, 0 ), FVector2D( margin, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    Add( leftBorder );

    abilities = new SlotPalette( SlotPalette::SlotPaletteTexture, FVector2D( margin, s+margin ),
      FVector2D(s,s), 2, 3, FVector2D(8,8) );
    Add( abilities );

    minimap = new Minimap( texMinimap, FVector2D(0, abilities->GetBounds().bottom()+margin),
      FVector2D( texMinimap->GetSurfaceWidth(), texMinimap->GetSurfaceHeight() ),
      20.f, FLinearColor( 1.0f, 0.1f, 0.1f, 1.f ) );
    Add( minimap );
  }
  virtual ~Panel(){}
};

struct StatusBar : public SolidWidget
{
  TextWidget* Text;
  StatusBar( FVector2D canvasSize, float height, FLinearColor bkgColor ) :
    SolidWidget( FVector2D( 0, canvasSize.Y - height ),
    FVector2D( canvasSize.X, height ), bkgColor )
  {
    Text = new TextWidget( "status text" );
    Add( Text );
  }
};

// The root UI component
struct UserInterface : public HotSpot
{
  // +-----------------------------+
  // | resourcesW     | rightPanel  |
  // |                |  Portrait   |
  // |                |  unitStats  |
  // |                |  abilities  |
  // |                | +---------+ |
  // |  +----------+  | | minimap | |
  // |  | itemBelt |  | +---------+ |
  // +--buffs-----------------------+
  //
  // 
  ResourcesWidget* resources; // The resources widget in the top left
  Panel* rightPanel;      // containing: portrait, unitStats, abilities, minimap
  CostWidget* costWidget; // a flyover cost of the hovered item from 
  Tooltip* tooltip;
  SlotPalette* itemBelt;
  StackPanel* buffs;
  StackPanel* building; // Queue of things we are building (in order)
  StatusBar* statusBar; // the bottom status bar has 
  ImageWidget* mouseCursor;

  UserInterface( FVector2D pos, FVector2D size ) : HotSpot( pos, size ) {
    resources = 0, rightPanel = 0, costWidget = 0, tooltip = 0,
    itemBelt = 0, buffs = 0, building = 0, statusBar = 0, mouseCursor = 0;
  }

  // don't need to explicitly delete children of the UI object
  virtual ~UserInterface(){ }

  // layout all widgets based on canvas (screen) size
  void layout( FVector2D canvasSize )
  {
    Size = canvasSize;
    rightPanel->Pos.X = Size.X - rightPanel->Size.X;
    // buffs appear listed in the 
    buffs->Pos = FVector2D( 0, Size.Y / 2 );
    building->Pos = FVector2D( 0, 2*Size.Y / 3 );

    statusBar->Pos = FVector2D( 0, Size.Y - statusBar->Size.Y );
    statusBar->Size.X = Size.X;
  }

};
// Covers entries with a cooldownpie each
//struct SpawnQueue : public StackPanel
//{
//  
//};

