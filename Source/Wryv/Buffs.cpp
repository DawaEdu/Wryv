#include "Wryv.h"
#include "Buffs.h"

void Buffs::Set( AGameObject* go )
{
  selected = go; // save for tick
  Clear();
  if( !go ) return;
  
  for( int i = 0; i < go->BonusTraits.size(); i++ )
  {
    Types buff = go->BonusTraits[i].traits.Type;
    StackRight( new ImageWidget( "a buff", Game->GetPortrait( buff ) ), Top );
  }
}

void Buffs::Move( float t )
{
  Set( selected );
}
