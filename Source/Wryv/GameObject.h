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
  // The amount that this object multiplies incoming repulsion forces by.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float RepelMultiplier;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  USceneComponent* DummyRoot;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UCapsuleComponent* hitBounds;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  USphereComponent* repulsionBounds;
  
  vector< PowerUpTimeOut > BonusTraits;
  float Hp;             // Current Hp. float, so heal/dmg can be continuous (fractions of Hp)
  float Mana;           // Current Mana.
  bool Repairing;       // If the building/unit is Repairing
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UnitProperties)  bool Dead;            // Whether unit is dead or not.
  vector< CooldownCounter > AbilityCooldowns;
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
  function <void ()> OnReachDestination; // Something to do when reach destination.

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject* FollowTarget;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UnitProperties)  AGameObject* AttackTarget;
  // Cached collections of followers & attackers
  vector<AGameObject*> Followers, Attackers, Overlaps;
  FVector AttackTargetOffset;  // Ground position of spell attacks

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
  template <typename T> T* MakeChild( Types type )
  {
    T* child = Game->Make<T>( type, team );
    AddChild( child );
    return child;
  }

  void SetSize( FVector size );

  // 
  // Gameplay.
  FVector GetTip();
  FVector GetCentroid();
  float GetHeight();
  float centroidDistance( AGameObject *go );
  float outsideDistance( AGameObject *go );
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool isAttackTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = Fighting)  float HpPercent();
  UFUNCTION(BlueprintCallable, Category = Fighting)  float SpeedPercent();
  // Pass thru to stats structure property
  UFUNCTION(BlueprintCallable, Category = Fighting)  float AttackSpeedMultiplier() { return Stats.AttackSpeedMultiplier; }
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool hasAttackTarget() { return AttackTarget != 0; }
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool hasFollowTarget() { return FollowTarget != 0; }
  // Called by blueprints (AttackAnimation) when attack is launched (for ranged weapons)
  // or strikes (for melee weapons).
  UFUNCTION(BlueprintCallable, Category = Fighting)  void AttackCycle();
  inline float DamageRoll() { return Stats.BaseAttackDamage + randFloat( Stats.BonusAttackDamage ); }
  void Shoot();
  void SendDamageTo( AGameObject* other );

  // Sets object to use indexed action
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
  void FlushPosition();
  FVector Repel( AGameObject* go );

  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void OnHitContactBegin( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult );
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void OnHitContactEnd( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );
  
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void OnRepulsionContactBegin( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult );
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void OnRepulsionContactEnd( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );
  
  void AddRepulsionForcesFromOverlappedUnits();
  void Walk( float t );
  void SetGroundPosition( FVector groundPos );
  void SetDestination( FVector d );
  void StopMoving();
  void Stop();
  void Face( FVector point );
  // time-stepped attack of current target (if any)
  void MoveWithinDistanceOf( AGameObject* target, float distance );
  virtual void Move( float t );
  virtual void ai( float t );

  // 
  // Unit relationship functions
  bool isAllyTo( AGameObject* go );
  bool isEnemyTo( AGameObject* go );

  void Follow( AGameObject* go );
  void Attack( AGameObject* go );
  void StopAttackAndFollow();

  void LoseFollower( AGameObject* formerFollower );
  void LoseAttacker( AGameObject* formerAttacker );
  void LoseAttackersAndFollowers();

  // 
  // AI
  AGameObject* GetClosestEnemyUnit();
	map<float, AGameObject*> FindEnemyUnitsInSightRange();
	AGameObject* GetClosestObjectOfType( Types type );

  // 
  // Utility
  void OnSelected();
  float GetBoundingRadius();
  FBox GetBoundingBox();
  FCollisionShape GetBoundingCylinder();
  void SetMaterialColors( FName parameterName, FLinearColor color );
  void SetTeam( Team* newTeam );
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
