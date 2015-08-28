#pragma once

#include <vector>
#include <set>
#include <map>
#include <List.h>
#include <Array.h>
using namespace std;

#include "Types.h"
#include "UnitsData.h"
#include "CooldownCounter.h"
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
  Team *team;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  FUnitsDataRow BaseStats;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FUnitsDataRow Stats;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float MaxRepulsionForce;
  UShapeComponent*      bounds;

  vector< PowerUpTimeOut > BonusTraits;
  float Hp;             // Current Hp. float, so heal/dmg can be continuous (fractions of Hp)
  float Mana;           // Current Mana.
  float AttackCooldown; // Cooldown on this unit since last attack
  bool Repairing;       // If the building/unit is Repairing
  vector< CooldownCounter > Abilities;
  vector< CooldownCounter > BuildQueueCounters;  // The queue of objects being built
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
  void SetSize( FVector size );

  // 
  // Gameplay.
  float centroidDistance( AGameObject *go );
  float outsideDistance( AGameObject *go );
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool isAttackTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float distanceToAttackTarget();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float HpPercent();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float SpeedPercent();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasAttackTarget() { return AttackTarget != 0; }
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasFollowTarget() { return FollowTarget != 0; }

  // Invokes next queued action
  void Action();
  // Sets object to use indexed action
  void Action( Types type, AGameObject *target );
  void Action( Types type, FVector where );
  void ApplyEffect( FUnitsDataRow item );
  void AddBuff( Types item );
  void UpdateStats( float t );
  bool UseAbility( int index );
  bool Make( Types type );
  
  // 
  // Movement functions.
  void SetRot( const FRotator & ro );
  bool Reached( FVector& v, float dist );
  void CheckWaypoint();
  void SetPosition( FVector v );
  void Walk( float t );
  void SetDestination( FVector d );
  void StopMoving();
  void Stop();
  // time-stepped attack of current target (if any)
  virtual void Move( float t );
  virtual void ai( float t );
  void DoAttack( float t );

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
  
  // 
  // AI
  AGameObject* GetClosestEnemyUnit();
	map<float, AGameObject*> FindEnemyUnitsInSightRange();
	AGameObject* GetClosestObjectOfType( Types type );

  // 
  // Utility
  void OnSelected();
  float GetBoundingRadius();
  void SetTeam( int32 teamId );
  void PlaySound( USoundBase* sound ){ UGameplayStatics::PlaySoundAttached( sound, RootComponent ); }
  void SetMaterial( UMaterialInterface* mat );
  void SetColor( FLinearColor color );

  // Shouldn't have to reflect unit type often, but we use these
  // to select a building for example from the team's `units` collection.
  // Another way to do this is orthogonalize collections [buildings, units.. etc]
  bool isPeasant() { return Stats.Type == Types::UNITPEASANT; }
  bool isUnit(){ return IsUnit( Stats.Type ); }
  bool isBuilding(){ return IsBuilding( Stats.Type ); }
  bool isResource(){ return IsResource( Stats.Type ); }
  bool isItem(){ return IsItem( Stats.Type ); }
  bool isShape(){ return IsShape( Stats.Type ); }
  virtual void Die();
  virtual void BeginDestroy() override;

};
