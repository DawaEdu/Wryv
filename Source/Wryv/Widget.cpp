#include "Wryv.h"

#include "Widget.h"
#include "GameFramework/HUD.h"
#include "TheHUD.h"
#include "GameObject.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

ATheHUD* HotSpot::hud = 0;

UTexture* ResourcesWidget::GoldTexture=0;
UTexture* ResourcesWidget::LumberTexture=0;
UTexture* ResourcesWidget::StoneTexture=0;

UTexture* SolidWidget::SolidWhiteTexture = 0;
UTexture* SlotPalette::SlotPaletteTexture = 0;
UTexture* StackPanel::StackPanelTexture = 0;
UTexture* AbilitiesPanel::BuildButtonTexture = 0;
UTexture* ImageWidget::NullTexture = 0;
TextWidget* HotSpot::TooltipWidget = 0;

void HotSpot::HotSpotDefaults()
{
  SetName( "HotSpot" );
  //TooltipText = "Tip";

  Align = None; // Absolute positioning as default
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

  OnHover = [this](FVector2D mouse){
    if( !TooltipText.IsEmpty() )
      TooltipWidget->Set( TooltipText );
    return NotConsumed;
  };
}

void ImageWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( !Tex )
  {
    // We have to remove this comment for normal ops because
    // sometimes we want to have null texes eg in slotpalette items when
    // no item is present
    LOG( "Texture not set for ImageWidget `%s`, setting to NULL texture", *Name );
    // render should not be called when the texture is hidden
    Tex = NullTexture;
  }

  // The renderPosition is just the computed position minus center hotpoint
  FVector2D renderPos = Pos() - hotpoint + offset;

  // If hidden, do not draw
  ((AHUD*)hud)->DrawTexture( Tex, renderPos.X, renderPos.Y, 
    Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
    EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
  
  HotSpot::render( offset );
}

void TextWidget::Measure()
{
  // ERROR IF THE HUD is not currently ready
  // to draw (ie canvas size won't be available)
  if( hud->Valid() ){
    dirty = 0;
  }
  else{
    LOG(  "The Canvas is not ready");
  }

  hud->GetTextSize( Text, Size.X, Size.Y, Font, Scale );
}

// We call Measure() each call to render() since text cannot be measured except when
// canvas is ready (ie during render() calls)
void TextWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( dirty )
    Measure();  // when measure succeeds dirty=0
  FVector2D pos = Pos() + offset;
  hud->DrawText( Text, Color, pos.X, pos.Y, Font, Scale );
  HotSpot::render( offset );
}


Clock::Clock( FString name, FVector2D size, CooldownCounter o, FLinearColor pieColor ) : 
  ITextWidget( name, Game->GetPortrait( o.Type ), size, "", Alignment::BottomRight ),
  counter( o ), clockColor( pieColor )
{
  // Spawn the clock material instance to apply to this widget
  clockMaterial = CreateClockMaterial( pieColor );
}

Clock::~Clock()
{
  // Deallocate the clockmaterial
  //delete clockMaterial;
}

UMaterialInstanceDynamic* Clock::CreateClockMaterial( FLinearColor pieColor )
{
  UMaterialInstanceDynamic* clock = UMaterialInstanceDynamic::Create(
    Game->hud->ClockMaterialInstance, Game->hud );
  // Reset parameter values
  clock->SetScalarParameterValue( FName( "Percent" ), 0.f );
  clock->SetVectorParameterValue( FName( "Color" ), pieColor );
  return clock;
}

void Clock::Move( float t )
{
  // Update the Material's parameter
  counter.Time += t;
  if( counter.Done() ) counter.Reset();

  // Progress animations on clock faces
  if( !clockMaterial->IsValidLowLevel() )
    clockMaterial = CreateClockMaterial( clockColor );
  else if( clockMaterial->IsValidLowLevel() )
    clockMaterial->SetScalarParameterValue( FName( "Percent" ), counter.Percent() );

  // Print the time remaining into the widget
  ImageWidget::Move( t );
}

void Clock::render( FVector2D offset )
{
  ImageWidget::render( offset );
  // Put the overlay on top.
  FVector2D pos = Pos() + offset;
  if( clockMaterial->IsValidLowLevel() )
  {
    ((AHUD*)hud)->DrawMaterial( clockMaterial, pos.X, pos.Y, Size.X, Size.Y, 0, 0, 1, 1 );
  }
  else
  {
    LOG( "Clock::render(): The MID became invalid" );
  }
}

void BuildQueue::Set( AGameObject* go )
{
  // Things that are spawning.
  for( int i = 0; i < go->buildQueue.size(); i++ )
  {
    Types type = go->buildQueue[i].Type;
    Clock *clock = new Clock( 
      Game->unitsData[ type ].Name + FString( "'s cooldown" ),
      EntrySize, CooldownCounter( type ),
      FLinearColor( 0.15, 0.15, 0.15, 0.15 ) );
    //cooldown->Hide();
    StackRight( clock );

    // Register a function to remove the widget from this queue when clicked.
    clock->OnMouseDownLeft = [this,go,clock,i](FVector2D mouse)
    {
      // Remove this entry from the buildQueue then refresh the spawn queue
      // This works perfectly fine, the index to remove is actually
      // correct since a call to Set() follows removal from the queue.
      removeIndex( go->buildQueue, i );
      Set( go ); // reset the buildQueue's contents.
      return Consumed;
    };
  }
}

void StatsPanel::Set( AGameObject* go )
{
  // set the text inside with gameobject
  LOG( "Set text with gameobject" );

}

AbilitiesPanel::AbilitiesPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
    SlotPalette( "abilities", bkg, rows, cols, entrySize, pad )
{
  actions = iActions;

  if( GetNumSlots() )
  {
    buildButton = GetSlot( GetNumSlots() - 1 );
    buildButton->Tex = BuildButtonTexture;
    buildButton->OnMouseDownLeft = [this]( FVector2D mouse ) {
      actions->ShowAbilitiesPanel();
      return Consumed;
    };
  }
}

void FlowPanel::Set( AGameObject* go )
{
  Clear();
  Rows = Cols = 1;
  Add( new ImageWidget( go->Stats.Name, go->Stats.Portrait ) );
  reflow();
}

void FlowPanel::Set( vector<AGameObject*> objects, int rows, int cols )
{
  Rows = rows, Cols = cols;
  Clear();
  for( int i = 0; i < objects.size(); i++ )
    Add( new ImageWidget( objects[i]->Stats.Name, objects[i]->Stats.Portrait ) ) ;
  if( Rows && Cols ) reflow();
}

void AbilitiesPanel::Set( AGameObject *go )
{
  // Set abilities. Abilities panel always has [2 rows, 3 cols]
  // We line up each ability with the slot number.
  for( int i = 0; i < go->Abilities.size() && i < GetNumSlots()-1; i++ )
  {
    Ability ability = go->Abilities[i];
    FUnitsDataRow abilityData = Game->unitsData[ability.Type];
    
    // Want to change texture, it recenters the texture in case it is
    // the incorrect size for the slot.
    SetSlotTexture( i, abilityData.Portrait );
    ITextWidget* button = GetSlot( i );
    button->Show();

    // Attach button with invokation of i'th ability
    button->OnMouseDownLeft = [go,i,abilityData]( FVector2D mouse ) {
      LOG( "%s used ability %s", *go->Stats.Name, *abilityData.Name );
      go->UseAbility( go->Abilities[i] );
      return Consumed;
    };
  }

  // the rest of the abilities can be set to null
  for( int i = go->Abilities.size(); i < GetNumSlots(); i++ )
  {
    // Turn off the function object, jsut in case
    GetSlot(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
    // hide slot 
    GetSlot(i)->Hide();
  }
  
  // Last slot will contain the build button.
  int lastSlot = GetNumSlots()-1;
  if( lastSlot >= 0 )
  {
    ITextWidget *buildSlot = GetSlot( lastSlot );
    buildSlot->Show();
  }
}

BuildPanel::BuildPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "BuildPanel", bkg, rows, cols, entrySize, pad )
{
  actions = iActions;
}

void BuildPanel::Set( AGameObject *go )
{
  for( int i = 0; i < go->Stats.Spawns.Num(); i++ )
  {
    Types spawn = go->Stats.Spawns[i];

    // Construct buttons that run abilities of the object.
    SetSlotTexture( i, Game->GetPortrait( spawn ) );
    ITextWidget* button = GetSlot( i );
    button->OnMouseDownLeft = [go,i]( FVector2D mouse ){
      // try spawn the object type listed
      go->Build( go->Stats.Spawns[i] );
      return Consumed;
    };
  }
}

ItemBelt::ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) :
  SlotPalette( "itembelt", bkg, rows, cols, entrySize, pad )
{
  
}

void ItemBelt::Set( AUnit *unit )
{
  ItemBelt* itemBelt = Game->hud->ui->gameChrome->itemBelt;

  // repopulate the # grid slots according to # items unit has
  if( unit->Items.Num() )
  {
    // Populate the toolbelt, etc
    int itemRows = 1 + ((unit->Items.Num() - 1) / 4); // 1 + ( 5 - 1 )/4
    int itemCols = 4;
    itemCols = unit->Items.Num() % 4;
    if( !itemCols )  itemCols = 4;

    vector<ITextWidget*> slots = itemBelt->SetNumSlots( itemRows, itemCols );
    
    // The function associated with the Item is hooked up here.
    // Inventory size dictates #items.
    for( int i = 0; i < slots.size(); i++ )
    {
      ITextWidget* slot = slots[i];
      FUnitsDataRow item = unit->Items[i];
      itemBelt->SetSlotTexture( i, item.Portrait );

      // Trigger the gameobject to consume i'th item.
      itemBelt->GetSlot( i )->OnMouseDownLeft = [this,i,unit](FVector2D mouse){
        unit->ConsumeItem( i );
        return Consumed;
      };

      Tooltip* tooltip = Game->hud->ui->gameChrome->tooltip;
      itemBelt->GetSlot(i)->OnHover = [slot,item,tooltip](FVector2D mouse)
      {
        // display a tooltip describing the current item.
        // or could add as a child of the img widget
        tooltip->Set( item.Name + FString(": ") + item.Description, 5.f );
        tooltip->Align = HCenter | OnTopOfParent;
        // put the tooltip as a child of the slot
        slot->Add( tooltip );
        return Consumed;
      };
    }
  }
  else
  {
    // No items in the belt. Empty the belt.
    itemBelt->SetNumSlots(0, 0);
  }
}

void Buffs::Set( AGameObject* go )
{
  Clear(); // Clear any existing/previous buffs.
  // Go through the applied buffs

  for( int i = 0; i < go->BonusTraits.size(); i++ )
  {
    Types buff = go->BonusTraits[i].traits.Type;
    StackRight( new ImageWidget( "a buff", Game->GetPortrait( buff ) ) );
  }
}

void Buffs::Move( float t )
{
  if( selected )  Set( selected );
}

void GameChrome::Select( AGameObject *go )
{
  rightPanel->portraits->Set( go );

  rightPanel->stats->Set( go );
  
  // Set this unit as selected in the game chrome.
  // Ensure the build button inside the UI is showing, only if the unit is a peasant type
  if( go->Stats.Type == Types::UNITPEASANT )
  {
    rightPanel->actions->abilities->buildButton->Show();
  }

  // on selection, populate the itemBelt and other widgets
  // Clear the items palette
  if( go->isUnit() )
  {
    AUnit* unit = Cast<AUnit>( go );
    itemBelt->Set( unit );
  }

  
  
  
  // Set with the abilities of the object
  rightPanel->actions->abilities->Set( go );

  // Set with the spawns of the object
  rightPanel->actions->buildPanel->Set( go );
  
  // Building types have a build queue
  buildQueue->Set( go );

  // buffs. PER-FRAME: Clear & then re-draw the buffs
  buffs->Set( go );
}

void GameChrome::Select( vector<AGameObject*> objects, int rows, int cols )
{
  // Select all units in the array, but put the front item's options
  // This is for the picture of the last clicked object. Generate a widget for the picture of the unit.
  // Print unit's stats into the stats panel
  Select( objects[0] );

  rightPanel->portraits->Set( objects, rows, cols );
}
