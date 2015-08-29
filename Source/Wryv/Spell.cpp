#include "Wryv.h"
#include "Spell.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

ASpell::ASpell(const FObjectInitializer& PCIP) : AGameObject(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  AttackTarget = 0;
  init = 0;
  caster = 0;
}

void ASpell::Init()
{
  // Get the bounding volume and prox based on it
  vector<UShapeComponent*> bounds = GetComponentsByType<UShapeComponent>();
  
  for( int i = 0; i < bounds.size(); i++ )
  {
    // Attach function when spell comes near target.
    bounds[i]->OnComponentBeginOverlap.AddDynamic( this, &ASpell::ProxSpell );
  }

  init = 1;
}

void ASpell::BeginPlay()
{
  Super::BeginPlay();
}

void ASpell::ProxSpell_Implementation( AActor* OtherActor,
  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
  bool bFromSweep, const FHitResult & SweepResult )
{
  LOG( "SPELL: %s collided with %s", *Stats.Name, *OtherActor->GetName() );
  // Deal damage to otheractor (which should be a GameObject)
  AGameObject* go = Cast<AGameObject>( OtherActor );
  if( go   &&   go != caster )
  {
    go->Hp -= Stats.BaseAttackDamage;
    // spawn the OnContact spell
    team->AddUnit( Game->Make<ASpell>( OnContact.GetValue(), Pos ) );
    Destroy();
  }
}

void ASpell::ai( float t )
{
  // The AI for the spell is basically nothing at this point,
  // though we might want to add spin/curl to some spells
}


