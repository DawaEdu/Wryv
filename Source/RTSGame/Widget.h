#pragma once

#include <vector>
#include <functional>
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
  static AHUD* hud;

  bool hidden;
  FVector2D Pos, Size;
  FLinearColor Color;
  // Associations with AGameObject's contained in this hotspot. Empty if no objects
  // are associated with the spot (used for item slots)
  vector<AGameObject*> Objects;
protected:
  HotSpot* Parent;
  vector<HotSpot*> children;  // Children of this hotspot.
  bool dirty; // set if the widget's bounds need to be remeasured
public:
  function< void (FVector2D mouse) > OnClicked;
  function< void (FVector2D mouse) > OnHover;
  function< void (FVector2D mouse) > OnDrag;
  // A function that runs when the widget is dropped @ certain location
  function< void (FVector2D mouse) > OnDrop;

  HotSpot() : hidden(0), Pos(0,0), Size(32,32), Color( FLinearColor::White ), Parent(0), dirty(1) {}
  HotSpot( FVector2D pos ) : hidden(0), Pos(pos), Size(32,32), Color( FLinearColor::White ), Parent(0), dirty(1) {}
  HotSpot( FVector2D pos, FVector2D size ) : hidden(0), Pos( pos ), Size( size ), Color( FLinearColor::White ), Parent(0), dirty(1)
  {
  }
  virtual ~HotSpot(){ Clear(); }
protected:
  virtual void render( FVector2D offset )
  {
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
    Size = GetChildBounds().Size() + pad;
  }
  float left(){ return Pos.X; }
	float right(){ return Pos.X + Size.X; }
	float top(){ return Pos.Y; }
	float bottom(){ return Pos.Y + Size.Y; }
	//bool hit( FVector2D v )
	//{
	//	// +---+ top (0)
	//	// |   |
	//	// +---+ bottom (2) (bottom > top)
	//	// L   R
	//	return left() < v.X   &&   v.X < right() &&
  //         top()  < v.Y   &&   v.Y < bottom();
	//}
  // Check main bounds
  bool hit( FVector2D v ) {
    FBox2DU bounds = GetBounds();
    return bounds.IsInside( v ); // check if v is inside the box (<-)
  }
  FVector2D getHitPercent( FVector2D v ) { return (v - Pos)/Size; }
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
  void Clear(){
    for( int i = 0; i < children.size(); i++ )
      delete children[i];
    children.clear();
  }
  void Click( FVector2D v ) {
    // Check if hit parent. sometimes children are outside parent, which
    // is why
    if( OnClicked && hit( v ) )  OnClicked(v);
    for( int i = 0; i < children.size(); i++ )
      children[i]->Click( v - Pos ); // relatively position click point to parent
  }
  void Hover( FVector2D v ) {
    if( OnHover && hit( v ) )  OnHover(v);
    for( int i = 0; i < children.size(); i++ )
      children[i]->Hover( v - Pos );
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
    SetText( fs );
  }
  TextWidget( FString fs, FVector2D pos ) : HotSpot( pos ), Text( fs )
  {
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
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D pos ) :
    Icon( pic ), HotSpot( pos ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White)
  {
    if( Icon ){
      Size.X = Icon->GetSurfaceWidth();
      Size.Y = Icon->GetSurfaceHeight();
    }
  }
  ImageWidget( UTexture* pic, FVector2D pos, FVector2D size ) :
    Icon( pic ), HotSpot( pos, size ), uv( 1, 1 ), hotpoint(0,0), Color(FLinearColor::White)
  {
  }
  virtual ~ImageWidget(){}

protected:
  virtual void render( FVector2D offset ) override 
  {
    FVector2D renderPos = Pos - hotpoint;
    if( !Icon )
    {
      // We have to remove this comment for normal ops because
      // sometimes we want to have null texes eg in slotpalette items when
      // no item is present
      UE_LOG( LogTemp, Warning, TEXT( "Texture not set for ImageWidget" ) );
      // render should not be called when the texture is hidden
    }
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

    UE_LOG( LogTemp, Warning, TEXT( "topText->GetBounds()" ) );
    FBox2DU bounds = topText->GetBounds();
    bounds.print("toptext");
    cost = new ResourcesWidget(16, 4, FVector2D( pad.X/2, topText->GetBounds().bottom() + vSpacing ) );
    Add(cost);

    UE_LOG( LogTemp, Warning, TEXT( "cost pos: %f %f" ), cost->Pos.X, cost->Pos.Y );
    UE_LOG( LogTemp, Warning, TEXT( "cost->GetBounds()" ) );
    bounds = cost->GetBounds();
    bounds.print("costbounds");
    bottomText = new TextWidget( "BottomText", FVector2D( pad.X/2, cost->GetBounds().bottom() + vSpacing ) );
    Add(bottomText);

    UE_LOG( LogTemp, Warning, TEXT( "bottomText->GetBounds()" ) );
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
    //re-measure the background's size if the text has changed
    //if( dirty )
    {
      repad( Pad );
      dirty = 0;
    }
    ImageWidget::render( offset );
  }
};

struct SlotEntry : public ImageWidget
{
  TextWidget *qty;   // The quantity of the item
  SlotEntry( UTexture* tex, FVector2D pos, FVector2D size ) : ImageWidget( tex, pos, size )
  {
    qty = new TextWidget( "1" );
    Add( qty );
    FBox2DU bounds = GetBounds();
    qty->Pos.X = bounds.right() - qty->Size.X;

    hidden = 1; // defaults hidden with NULL texture.
  }

  void SetQuantity( int quantity )
  {
    qty->SetText( FString::Printf( TEXT("%d"), quantity ) );
    FBox2DU bounds = GetBounds();
    qty->Pos.X = bounds.right() - qty->Size.X;
  }
};

struct SlotPalette : public ImageWidget
{
  FVector2D SlotSize;
  ImageWidget* Drag;
  static UTexture* SlotPaletteTexture;
  int Rows, Cols;

  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( UTexture* bkg, FVector2D pos, FVector2D size, int rows, int cols ) : 
    Rows(rows), Cols(cols), ImageWidget( bkg, pos, size )
  {
    // Init w/ # slots used in this palette
    // The stock size of the width is 100px/slot. We re-calculate the slotsize though
    // based on # slots used.
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

  ImageWidget* SetSlot( int i, UTexture* icon )
  {
    ImageWidget* slot = GetSlot(i);
    slot->hidden = 0; // unhide it if hidden when set with a texture
    slot->Icon = icon;

    // center the child image a bit
    FVector2D originalPos = GetSlotPosition( i );

    return slot;
  }

  // Widget's by themselves are just hotspots
  vector<ImageWidget*> SetNumSlots( int rows, int cols )
  {
    Rows = rows;
    Cols = cols;

    //SlotSize = Size / FVector2D( cols, rows );
    SlotSize.X = Size.X / cols; // = FVector2D(100,100);//Size / FVector2D( cols, rows );
    SlotSize.Y = Size.Y / rows;
    int numSlots = rows*cols;

    // The uv's are equal to 
    // Measure the UV coordinates used since the texture is 6x6
    uv = FVector2D( cols/6., rows/6. ); // The texture is 6x6 blocks
    Clear();    // Remove the old ImageWidgets.
    
    // The size of this widget set here.
    vector<ImageWidget*> slots;
    for( int i = 0; i < numSlots; i++ )
    {
      ImageWidget *iw = new ImageWidget( Icon, GetSlotPosition(i), SlotSize );
      iw->hidden = 1; // initialize as hidden, until texture is set.
      slots.push_back( iw );
      Add( iw );
    }
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

    abilities = new SlotPalette( SlotPalette::SlotPaletteTexture, FVector2D( margin, s+margin ), FVector2D(s,s), 2, 3 );
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

  ~UserInterface(){
    //delete resources;
    //delete rightPanel;
    //delete costWidget;
    //delete tooltip;
    //delete itemBelt;
    //delete buffs;
    //delete building;
    //delete statusBar;
    //delete mouseCursor;
  }

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

