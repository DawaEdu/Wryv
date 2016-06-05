#include "Wryv.h"

#include "UI/HotSpot/GamePanels/BuffsPanel.h"
#include "GameObjects/Things/Items/Item.h"
#include "UI/HotSpot/Elements/IText.h"

BuffsPanel::BuffsPanel( FString name, UTexture* bkg ) : StackPanel( name, bkg )
{
  Pad = FVector2D( 4, 4 );
  BuffSize = FVector2D( 32, 32 );
  Align = Alignment::TopRight;
}

void BuffsPanel::Set( AGameObject* go )
{
  Clear();
  if( !go ) return;
  
  for( int i = 0; i < go->BonusTraits.size(); i++ )
  {
    UTexture* tex = go->BonusTraits[i].BonusStats.Portrait;
    //IText* widget = new IText( go->BonusTraits[i].BonusStats.Name,
    //  tex, BuffSize, FS( "%.1f", go->BonusTraits[i].timeRemaining ),
    //  Alignment::CenterCenter );
    ImageHS* widget = new ImageHS( go->BonusTraits[i].BonusStats.Name, tex, BuffSize );
    StackRight( widget, Top );
  }

  recomputeSizeToContainChildren();
}


