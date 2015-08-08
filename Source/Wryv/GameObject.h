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
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector pos;
  // The velocity of the game object
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  FVector vel;
  // The full set of units data.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  FUnitsDataRow UnitsData;

  // Instances of units abilities and their cooldown (if any)
  vector<Ability> abilities;
  
  float hp;             // Current hp. float, so heal/dmg can be continuous (fractions of hp)
  float speed;          // Current speed. modified from UnitsData.Speed (depending on buffs applied).
  float attackCooldown; // Cooldown on this unit since last attack
  bool repairing;       // If the building/unit is repairing
  
  // Do we use the datatable to fill this unit with data.
  bool LoadsFromTable;
  vector<FVector> waypoints;
  FVector dest; // The movement destination
  
  // The unit that this unit is currently attacking
  // When casting a spell, if there is an attack target,
  // the spell goes at the attack target.
  // If the unit is casting a spell and the spell targets ground,
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject *followTarget;
  // Readable in blueprints for animation
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject *attackTarget;

  // When the attacktarget is the ground, we set attackTargetOffset to
  // nonzero value to make the spell go towards that direction.
  FVector attackTargetOffset;

  // Next spell to be cast by this target,
  // This variable is here because the unit may not be in range
  // @ time spell is queued.
  Types NextSpell;
  Team *team;   // Contains the team logic.

  // List set of traits that gets applied due to powerups
  //   0.025 => FUnitsDataRow(),  0.150 => FUnitsDataRow(),  0.257 => FUnitsDataRow()
  vector< PowerUpTimeOut > BonusTraits;

  // The queue of objects being spawned. Each has a time before it is spawned.
  vector<CooldownCounter> buildQueue;
  template <typename T> vector<T*> GetComponentsByType() {
    return ::GetComponentsByType<T>( this );
  }
  virtual void BeginPlay() override;
  void SetTeam( int teamId );
  bool ally( AGameObject* go );
  bool enemy( AGameObject* go );
  void removeAsTarget();
  float centroidDistance( AGameObject *go );
  float outsideDistance( AGameObject *go );
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool isAttackTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  float distanceToAttackTarget();
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasAttackTarget() { return attackTarget != 0; }
  UFUNCTION(BlueprintCallable, Category = UnitProperties)  bool hasFollowTarget() { return followTarget != 0; }
  
  FVector GetPos();
  void SetPos(const FVector& pos);
  FRotator GetRot();
  void SetRot( FRotator & ro );

  void CastSpell( Types type, AGameObject *target );
  void ApplyEffect( FUnitsDataRow item );
  // Use an ability. Abilities with a target
  // will switch the game's UI mode into a "select target" mode.
  void UseAbility( Ability& ability );
  // Invokation of an ability with a target.
  void UseAbility( Ability& ability, AGameObject *target );
  FUnitsDataRow GetTraits();
  bool Build( Types type );

  bool Reached( FVector& v, float dist );
  void UpdateDestination();
  void MoveTowards( float t );
  virtual void SetTarget( AGameObject* go );
  void StopMoving();
  void Stop();
  // Game's Move function. This is separate from UE4's ::Tick() function,
  // to create deterministic call order.
  virtual void Move( float t );
  virtual void ai( float t );
  void fight( float t );
  float hpPercent();
  AGameObject* GetClosestEnemyUnit();
	map<float, AGameObject*> FindEnemyUnitsInSightRange();
	AGameObject* GetClosestObjectOfType( Types type );

  bool LOS( FVector p );
  // Set the color of the actor
  void SetDestination( FVector d );
  void RefreshBuildingQueue();
  virtual void OnSelected();
  FString PrintStats();
  float GetBoundingRadius();

  bool isUnit(){ return IsUnit( UnitsData.Type ); }
  bool isBuilding(){ return IsBuilding( UnitsData.Type ); }
  bool isResource(){ return IsResource( UnitsData.Type ); }
  bool isItem(){ return IsItem( UnitsData.Type ); }

  virtual void BeginDestroy() override;

};
