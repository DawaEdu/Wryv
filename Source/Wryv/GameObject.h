#pragma once

#include <vector>
#include <set>
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
  static AGameObject* Nothing;
  
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
  // Cached collections of followers & attackers
  vector<AGameObject*> Followers, Attackers;
  FVector AttackTargetOffset;  // Ground position of spell attacks
  Types NextAction;

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
  float centroidDistance( AGameObject *go );
  float outsideDistance( AGameObject *go );
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool isAttackTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float distanceToAttackTarget();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float hpPercent();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float speedPercent();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasAttackTarget() { return AttackTarget != 0; }
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasFollowTarget() { return FollowTarget != 0; }


  // Invokes queued action
  void Action();
  // Sets object to use indexed action
  void Action( Types type, AGameObject *target );
  void Action( Types type, FVector where );
  void ApplyEffect( Types item );
  void UpdateStats();
  bool UseAbility( int index );
  bool Make( Types type );
  
  // 
  // Movement functions.
  void SetRot( const FRotator & ro );
  bool Reached( FVector& v, float dist );
  void CheckWaypoint();
  void Walk( float t );
  void SetDestination( FVector d );
  void StopMoving();
  void Stop();

  // 
  // Unit relationship functions
  bool isAllyTo( AGameObject* go );
  bool isEnemyTo( AGameObject* go );

  void Follow( AGameObject* go );
  // Stops following my target
  void StopFollowing();
  void LoseFollower( AGameObject* formerFollower );
  void LoseAllFollowers();

  // sets an object as the attack target
  void Attack( AGameObject* go );
  void StopAttacking();
  void LoseAttacker( AGameObject* formerAttacker );
  void LoseAllAttackers();

  // time-stepped attack of current target (if any)
  virtual void Move( float t );
  virtual void ai( float t );
  void DoAttack( float t );
  
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
  bool isPeasant() { return Stats.Type == Types::UNITPEASANT; }
  bool isUnit(){ return IsUnit( Stats.Type ); }
  bool isBuilding(){ return IsBuilding( Stats.Type ); }
  bool isResource(){ return IsResource( Stats.Type ); }
  bool isItem(){ return IsItem( Stats.Type ); }
  virtual void Die();
  virtual void BeginDestroy() override;

};
