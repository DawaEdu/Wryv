#pragma once

#include <deque>
#include <map>
#include <set>
#include <vector>
#include <Array.h>
#include <List.h>
using namespace std;

#include "Command.h"
#include "CooldownCounter.h"
#include "SoundEffect.h"
#include "Types.h"
#include "UnitsData.h"
#include "GameFramework/Actor.h"

#include "GameObject.generated.h"

class AItem;
class AGameObject;
class AResource;
class AShape;
struct Team;

UCLASS()
class WRYV_API AGameObject : public AActor
{
  GENERATED_UCLASS_BODY()
  const static float WaypointAngleTolerance; // 
  const static float WaypointReachedToleranceDistance; // The distance to consider waypoint as "reached"
  static AGameObject* Nothing;
  static float CapDeadTime; // Dead units get cleaned up after this many seconds (time given for dead anim to play out)
  Command CurrentCommand;

  // 
  // Stats.
  int64 ID; // Unique Identity of the object.
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
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = UnitProperties)  bool Dead;            // Whether unit is dead or not.
  float DeadTime, MaxDeadTime; // how long the object has been dead for
  FLinearColor vizColor;
  float vizSize;
  bool Recovering;
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
  vector<AShape*> NavFlags; // Debug info for navigation flags

  // Series of commands.
  deque<Command> commands;

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
  
  // Net
  // Hashes the object (checking for network state desync)
  float Hash();
  void EnqueueCommand( Command task );
  void SetCommand( Command task );

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
  float outerDistance( AGameObject *go );
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool isAttackTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool isFollowTargetWithinRange();
  UFUNCTION(BlueprintCallable, Category = Fighting)  float HpFraction();
  UFUNCTION(BlueprintCallable, Category = Fighting)  float SpeedPercent();
  // Pass thru to stats structure property
  UFUNCTION(BlueprintCallable, Category = Fighting)  float AttackSpeedMultiplier() { return Stats.AttackSpeedMultiplier; }
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool hasAttackTarget() { return AttackTarget != 0; }
  UFUNCTION(BlueprintCallable, Category = Fighting)  bool hasFollowTarget() { return FollowTarget != 0; }
  // Called by blueprints (AttackAnimation) when attack is launched (for ranged weapons)
  // or strikes (for melee weapons).
  UFUNCTION(BlueprintCallable, Category = Fighting)  virtual void AttackCycle();
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
  // Walks towards Dest.
  void Walk( float t );
  // Points Actor to face particular 3-space point
  void Face( FVector point );
  // time-stepped attack of current target (if any)
  void MoveWithinDistanceOf( AGameObject* target, float fallbackDistance );
  void DisplayWaypoints();
  void exec( const Command& cmd );
  // Base Movement function. Called each frame.
  //   * Fixed time step
  //   * Predictable call order
  // This function is provided INSTEAD of ::Tick(), which has variable timestep value 
  // and unpredicatable call order.
  virtual void Move( float t );
  bool Idling();
  virtual void ai( float t );
  // The hit volumes overlapped
  virtual void Hit( AGameObject* other );
  float Radius();

  // Sets a new ground destination, dropping Follow/Attack targets.
  virtual void GoToGroundPosition( FVector groundPos );
  // Moves towards d using pathfinder, WITHOUT losing Follow/Attack targets.
  void SetDestination( FVector d );
  // Stops unit from moving, WITHOUT losing Follow/Attack targets.
  void StopMoving();
  
  // 
  // Unit relationship functions
  bool isAllyTo( AGameObject* go );
  bool isEnemyTo( AGameObject* go );

  // Sets a particular object as a Target (due to generic right click).
  // Resolves if target is ally (follow) or opponent (attack).
  virtual void Target( AGameObject* target );
  void Follow( AGameObject* go );
  void Attack( AGameObject* go );

  // Stop my attack and follow.
  virtual void StopAttackingAndFollowing();
  void LoseFollower( AGameObject* formerFollower );
  void LoseAttacker( AGameObject* formerAttacker );
  void LoseAttackersAndFollowers();

  // 
  // AI
  AGameObject* GetClosestEnemyUnit();
  map<float, AGameObject*> FindEnemyUnitsInSightRange();
  // Searches for an object of specific type on my TEAM
  AGameObject* GetClosestObjectOfType( Types type );
	
  // 
  // Utility
  void Viz( FVector pt );
  void OnSelected();
  void SetMaterialColors( FName parameterName, FLinearColor color );
  void SetTeam( Team* newTeam );
  void PlaySound( USoundBase* sound ){ UGameplayStatics::SpawnSoundAttached( sound, RootComponent ); }
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
  UFUNCTION(BlueprintCallable, Category = Fighting) 
  void Cleanup();
  virtual void BeginDestroy() override;

  UFUNCTION(BlueprintCallable, Category = Display) FString ToString();
  UFUNCTION(BlueprintCallable, Category = Display) FString FollowersToString();
  
};
