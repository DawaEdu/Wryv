#pragma once

#include <vector>
#include <map>
#include <List.h>
#include <Array.h>
using namespace std;

#include "Types.h"
#include "UnitsData.h"
#include "WidgetData.h"
#include "GameFramework/Actor.h"
#include "GameObject.generated.h"

class AItem;
class AGameObject;
struct Team;
struct FWidgetData;

struct PowerUpTimeOut
{
  float time;
  FUnitsDataRow traits;
  PowerUpTimeOut():time(0.f){}
  PowerUpTimeOut( float t, FUnitsDataRow &iTraits ) :
    time( t ), traits( iTraits ) { }
  void Tick( float t ) { time += t; }
};

struct SpawnTime
{
  float time;
  AGameObject *spawning;

  SpawnTime() : time(0.f), spawning(0) { }
  SpawnTime( AGameObject *spawn, float iTime ) : 
    spawning( spawn ), time( iTime )
  {
    
  }
};

UCLASS()
class RTSGAME_API AGameObject : public AActor
{
  GENERATED_UCLASS_BODY()
public:
  // The full set of units data.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  FUnitsDataRow UnitsData;

  // Cooldown on this unit since last attack
  float attackCooldown;

  // The HP that this unit currently has. Floating point qty so
  // heal/damage effects can be continuous (fractions of a hitpoint
  // can be added per-frame).
  float hp;

  // If the unit is repairing itself
  bool repairing;

  // When a unit is building, this is the % progress it is to completion.
  float buildProgress;

  // List set of traits that gets applied due to powerups
  //   0.025 => FUnitsDataRow()
  //   0.150 => FUnitsDataRow()
  //   0.257 => FUnitsDataRow()
  vector< PowerUpTimeOut > BonusTraits;

  // The widget for this object type. Specified in the blueprint for this object type
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitWidget)
  FWidgetData Widget;

  // Do we use the datatable to fill this unit with data.
  bool LoadsFromTable;
  vector<FVector> waypoints;
  FVector dest; // The movement destination
  
  // The unit that this unit is currently attacking
  // When casting a spell, if there is an attack target,
  // the spell goes at the attack target.
  // If the unit is casting a spell and the spell targets ground,
  AGameObject *followTarget;
  AGameObject *attackTarget;

  // When the attacktarget is the ground, we set attackTargetOffset to
  // nonzero value to make the spell go towards that direction.
  FVector attackTargetOffset;

  // Next spell to be cast by this target,
  // This variable is here because the unit may not be in range
  // @ time spell is queued.
  Types NextSpell;
  
  // Contains the team logic.
  Team *team;

  // The queue of objects being spawned
  vector<AGameObject*> spawnQueue;
  
  template <typename T> static vector<T*> GetComponentsOfType( AActor* a )
  {
    TArray<UActorComponent*> comps = a->GetComponents();
    //UE_LOG( LogTemp, Warning, TEXT("# components %d"), comps.Num() );
    vector<T*> coll;
    for( int i = 0; i < comps.Num(); i++ )
      if( T* s = Cast<T>( comps[i] ) )
        coll.push_back( s );
    return coll;
  }
  virtual void BeginPlay() override;
  virtual void BeginDestroy() override;
  void SetTeam( int teamId );
  bool ally( AGameObject* go );
  bool enemy( AGameObject* go );
  void removeAsTarget();
  float centroidDistance( AGameObject *go );
  float outsideDistance( AGameObject *go );
  FVector Pos();
  void SetPos(const FVector& pos);
  void SetRot( FRotator & ro );
  void CastSpell( Types type, AGameObject *target );
  bool Reached( FVector& v, float dist );
  void UpdateDestination();
  void MoveTowards( float t );
  virtual void SetTarget( AGameObject* go );
  void StopMoving();
  void Stop();
  virtual void Tick( float t ) override;
  virtual void ai( float t );
  void fight( float t );
  float hpPercent();
  AGameObject* GetClosestEnemyUnit();
	map<float, AGameObject*> FindEnemyUnitsInSightRange();
	AGameObject* GetClosestObjectOfType( Types type );

  void ApplyEffect( Types type );
  bool LOS( FVector p );
  // Set the color of the actor
  void SetDestination( FVector d );
  FUnitsDataRow GetTraits();
  
  bool isUnit(){ return IsUnit( UnitsData.Type ); }
  bool isBuilding(){ return IsBuilding( UnitsData.Type ); }
  bool isResource(){ return IsResource( UnitsData.Type ); }
  bool isItem(){ return IsItem( UnitsData.Type ); }

};
