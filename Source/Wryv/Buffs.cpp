#include "Wryv.h"

#include "Buffs.h"
#include "Item.h"
#include "ITextWidget.h"

Buffs::Buffs( FString name, UTexture* bkg ) : StackPanel( name, bkg )
{
  Pad = FVector2D( 4, 4 );
  BuffSize = FVector2D( 32, 32 );
  Align = Alignment::TopRight;
}

void Buffs::Set( AGameObject* go )
{
  Clear();
  if( !go ) return;
  
  info( FS( "Buffs::Set( %s ) has %d traits", *go->GetName(), go->BonusTraits.size() ) );
  for( int i = 0; i < go->BonusTraits.size(); i++ )
  {
    UTexture* tex = go->BonusTraits[i].BonusStats.Portrait;
    //ITextWidget* widget = new ITextWidget( go->BonusTraits[i].BonusStats.Name,
    //  tex, BuffSize, FS( "%.1f", go->BonusTraits[i].timeRemaining ),
    //  Alignment::CenterCenter );
    ImageWidget* widget = new ImageWidget( go->BonusTraits[i].BonusStats.Name, tex, BuffSize );
    StackRight( widget, Top );
  }

  recomputeSizeToContainChildren();
}


