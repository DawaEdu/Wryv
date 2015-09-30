#include "Wryv.h"
#include "Buffs.h"
#include "Item.h"

void Buffs::Set( AGameObject* go )
{
  selected = go; // save for tick
  Clear();
  if( !go ) return;
  
  for( int i = 0; i < go->BonusTraits.size(); i++ )
  {
    AItem* item = go->BonusTraits[i].Powerup;
    StackRight( new ImageWidget( "a buff", item->BuffPortrait ), Top );
  }
}

void Buffs::Move( float t )
{
  Set( selected );
}
