#pragma once

#include <vector>
#include <map>
#include <List.h>
#include <Array.h>
using namespace std;

#include "Types.h"
#include "UnitsData.h"
#include "Ability.h"
#include "GameFramework/Actor.h"
#include "GameObject.generated.h"

class AItem;
class AGameObject;
struct Team;

UCLASS()
class WRYV_API AGameObject : public AActor
{
  GENERATED_UCLASS_BODY()
public:
  // The position of the gameobject. This is actually a mirror of RootComponent->GetActorLocation()
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector Pos;
  // The current direction the game object is facing.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector Dir;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector Vel;
  FVector OriginalScale; // The original scale set by the actor in 3D space
  // Scalar speed for the unit at the present time. Velocity comes from speed
  // and refreshed each frame.
  // Current speed. modified from Stats.Speed (depending on buffs applied).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  float Speed;
  float Hp;             // Current Hp. float, so heal/dmg can be continuous (fractions of Hp)
  float AttackCooldown; // Cooldown on this unit since last attack
  bool Repairing;       // If the building/unit is Repairing
  
  // The full set of units data.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  FUnitsDataRow BaseStats;
  // The current frame's traits
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FUnitsDataRow Stats;
  // List set of traits that gets applied due to powerups
  //   0.025 => FUnitsDataRow(),  0.150 => FUnitsDataRow(),  0.257 => FUnitsDataRow()
  vector< PowerUpTimeOut > BonusTraits;
  
  //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UCapsuleComponent* bounds;
  // Instances of units abilities and their cooldown (if any)
  vector<Ability> Abilities;
  
  // Do we use the datatable to fill this unit with data.
  bool LoadsFromTable;
  vector<FVector> Waypoints;
  FVector Dest; // The movement destination
  
  // The unit that this unit is currently attacking
  // When casting a spell, if there is an attack target,
  // the spell goes at the attack target.
  // If the unit is casting a spell and the spell targets ground,
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject *FollowTarget;
  // Readable in blueprints for animation
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject *AttackTarget;

  FVector AttackTargetOffset;
  Types NextSpell;
  Team *team;   // Contains the team logic.

  // The queue of objects being spawned. Each has a time before it is spawned.
  vector<CooldownCounter> buildQueue;
  template <typename T> vector<T*> GetComponentsByType() {
    return ::GetComponentsByType<T>( this );
  }
  virtual void PostInitializeComponents();
  virtual void BeginPlay() override;
  virtual void OnMapLoaded();

  // Functions to change parenting of the object, in Actor class
  AGameObject* SetParent( AGameObject* newParent );
  AGameObject* AddChild( AGameObject* newChild );
  bool isParentOf( AGameObject* go );
  bool isChildOf( AGameObject* parent );

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

  FRotator GetRot();
  void SetRot( FRotator & ro );

  void CastSpell( Types type, AGameObject *target );
  void ApplyEffect( FUnitsDataRow item );
  // Use an ability. Abilities with a target
  // will switch the game's UI mode into a "select target" mode.
  void UseAbility( Ability& ability );
  // Invokation of an ability with a target.
  void UseAbility( Ability& ability, AGameObject *target );
  void UpdateStats();
  bool Build( Types type );

  bool Reached( FVector& v, float dist );
  void UpdateDestination();
  void MoveTowards( float t );
  virtual void SetTarget( AGameObject* go );
  void StopMoving();
  void Stop();
  FVector SetOnGround( FVector v );
  // Game's Move function. This is separate from UE4's ::Tick() function,
  // to create deterministic call order.
  virtual void Move( float t );
  virtual void ai( float t );
  void fight( float t );
  AGameObject* GetClosestEnemyUnit();
	map<float, AGameObject*> FindEnemyUnitsInSightRange();
	AGameObject* GetClosestObjectOfType( Types type );

  bool LOS( FVector p );
  // Set the color of the actor
  void SetDestination( FVector d );
  void RefreshBuildingQueue();
  void OnSelected();
  FString PrintStats();
  float GetBoundingRadius();

  // Shouldn't have to reflect unit type often, but we use these
  // to select a building for example from the team's `units` collection.
  // Another way to do this is orthogonalize collections [buildings, units.. etc]
  bool isUnit(){ return IsUnit( Stats.Type ); }
  bool isBuilding(){ return IsBuilding( Stats.Type ); }
  bool isResource(){ return IsResource( Stats.Type ); }
  bool isItem(){ return IsItem( Stats.Type ); }

  virtual void BeginDestroy() override;

};
