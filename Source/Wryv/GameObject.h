#pragma once

#include <vector>
#include <map>
#include <List.h>
#include <Array.h>
using namespace std;

#include "Types.h"
#include "UnitsData.h"
#include "Ability.h"
#include "SoundEffect.h"
#include "GameFramework/Actor.h"
#include "GameObject.generated.h"

class AItem;
class AGameObject;
struct Team;

UCLASS()
class WRYV_API AGameObject : public AActor
{
  GENERATED_UCLASS_BODY()
  const static float WaypointAngleTolerance; // 
  const static float WaypointReachedToleranceDistance; // The distance to consider waypoint as "reached"
public:
  // 
  // Stats.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  FUnitsDataRow BaseStats;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FUnitsDataRow Stats;
  vector< PowerUpTimeOut > BonusTraits;
  vector< Ability > Abilities;
  Team *team;
  float Hp;             // Current Hp. float, so heal/dmg can be continuous (fractions of Hp)
  float AttackCooldown; // Cooldown on this unit since last attack
  bool Repairing;       // If the building/unit is Repairing
  vector<CooldownCounter> BuildQueueCounters;  // The queue of objects being spawned
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  TArray<FSoundEffect> Greets;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  TArray<FSoundEffect> Oks;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  TArray<FSoundEffect> Attacks;

  // 
  // Movement & Attack.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector Pos;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector Dir;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  float Speed;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector Vel;
  FVector Dest;
  vector<FVector> Waypoints;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject* FollowTarget;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject* AttackTarget;
  FVector AttackTargetOffset;  // Ground position of spell attacks
  Types NextSpell;

  // 
  // UE4 & Utility
  template <typename T> vector<T*> GetComponentsByType() {
    return ::GetComponentsByType<T>( this );
  }
  virtual void PostInitializeComponents();
  virtual void BeginPlay() override;
  virtual void OnMapLoaded();
  
  // 
  // Scenegraph management.
  AGameObject* SetParent( AGameObject* newParent );
  AGameObject* AddChild( AGameObject* newChild );
  bool isParentOf( AGameObject* go );
  bool isChildOf( AGameObject* parent );
  AGameObject* MakeChild( Types type );

  // 
  // Gameplay.
  bool isAlly( AGameObject* go );
  bool isEnemy( AGameObject* go );
  void removeAsTarget();
  float centroidDistance( AGameObject *go );
  float outsideDistance( AGameObject *go );
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool isAttackTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float distanceToAttackTarget();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float hpPercent();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float speedPercent();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasAttackTarget() { return AttackTarget != 0; }
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasFollowTarget() { return FollowTarget != 0; }
  void CastSpell();
  void CastSpell( Types type, AGameObject *target );
  void CastSpell( Types type, FVector where );
  void ApplyEffect( FUnitsDataRow item );
  void UseAbility( Ability& ability );
  void UseAbility( Ability& ability, AGameObject *target );
  void UpdateStats();
  bool Build( Types type );
  
  // 
  // Movement functions.
  void SetRot( const FRotator & ro );
  bool Reached( FVector& v, float dist );
  void CheckWaypoint();
  void Walk( float t );
  virtual void SetTarget( AGameObject* go );
  void StopMoving();
  void Stop();
  FVector SetOnGround( FVector v );
  void SetDestination( FVector d );
  virtual void Move( float t );
  virtual void ai( float t );
  void fight( float t );

  // 
  // AI
  AGameObject* GetClosestEnemyUnit();
	map<float, AGameObject*> FindEnemyUnitsInSightRange();
	AGameObject* GetClosestObjectOfType( Types type );

  // 
  // Utility
  bool LOS( FVector p );
  void OnSelected();
  float GetBoundingRadius();
  void SetTeam( int32 teamId );
  void PlaySound( USoundBase* sound ){ UGameplayStatics::PlaySoundAttached( sound, RootComponent ); }
  
  // Shouldn't have to reflect unit type often, but we use these
  // to select a building for example from the team's `units` collection.
  // Another way to do this is orthogonalize collections [buildings, units.. etc]
  bool isUnit(){ return IsUnit( Stats.Type ); }
  bool isBuilding(){ return IsBuilding( Stats.Type ); }
  bool isResource(){ return IsResource( Stats.Type ); }
  bool isItem(){ return IsItem( Stats.Type ); }

  virtual void BeginDestroy() override;

};
