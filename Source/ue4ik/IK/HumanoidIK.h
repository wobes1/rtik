// Copyright (c) Henry Cooney 2017

#pragma once

#include "IK.h"
#include "BonePose.h"
#include "HumanoidIK.generated.h"


/*
* Basic structs, etc for humanoid biped IK.
*/

/*
* Represents a humanoid leg, with a hip bone, thigh bone, and shin bone.
*/
USTRUCT(BlueprintType)
struct UE4IK_API FHumanoidLegChain : public FIKModChain
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FHumanoidLegChain()
		:
		FootRadius(10.0f),
		ToeRadius(5.0f),
		TotalChainLength(0.0f),
		bInitOk(false)
	{ }
	
	// Distance between the bottom of the shin bone and the bottom surface of the foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float FootRadius;
	
	// Distance between
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float ToeRadius;
	
	// Connects from pelvis to upper leg bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone HipBone;
	
	// Connects from end of hip to top of knee
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone ThighBone;
	
	// Connects from bottom of knee to top of foot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone ShinBone;
	
	// Connects from bottom of shin to start of the toe
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FIKBone FootBone;
	
	float GetTotalChainLength() const;
	
protected:
	bool bInitOk;
	virtual bool IsValidInternal(const FBoneContainer& RequiredBones) override;
	virtual bool InitAndAssignBones(const FBoneContainer& RequiredBones) override;
	
	// Total length of all bones in the chain (thigh, shin, and foot bones).
    // Does not include foot or toe radius.
	float TotalChainLength;
};

/*
* Holds trace data used in leg IK
*/
USTRUCT(BlueprintType)
struct UE4IK_API FHumanoidIKTraceData
{
	GENERATED_USTRUCT_BODY()
		
public:
	
	FHitResult FootHitResult;
	FHitResult ToeHitResult;
};

/*
* Wrapper for passing trace data around in BP. The trace node may write into the struct contained within!
*/
UCLASS(BlueprintType, EditInlineNew)
class UE4IK_API UHumanoidIKTraceData_Wrapper : public UObject
{
	GENERATED_BODY()

public: 
	
	UHumanoidIKTraceData_Wrapper(const FObjectInitializer& ObjectInitializer)
		:
		Super(ObjectInitializer),
		bUpdatedThisTick(false)
	{ }

	// Data in this class should be updated each frame before use. This is handled
	// by the IKLegTrace class, which will ensure that this wrapper is marked as stale
	// until it is updated.

	// Gets trace data stored in this wrapper. Trace data should be updated by
	// before this is called by placing a trace node earlier in the animgraph. 
	// If an update function is not called, and the data is stale, the stale data is returned
	// and a warning is printed to console.	
	UFUNCTION(BlueprintCallable, Category = IK)
	FHumanoidIKTraceData& GetTraceData()
	{
#if ENABLE_IK_DEBUG
		if (!bUpdatedThisTick)
		{
			UE_LOG(LogIK, Warning, TEXT("Warning -- Trace data was used before it was updated and may be stale. Use a trace node (e.g., IK Humanoid Leg Trace) to update your trace data early in the animgraph, before it is used!"));
		}
#endif // ENABLE_IK_DEBUG
		return TraceData;
	}

	// Trace classes using this wrapper are declared as friends so they can directly update data and set bUpdatedThisTick
	friend struct FAnimNode_IKHumanoidLegTrace;

protected:
	bool bUpdatedThisTick;
	FHumanoidIKTraceData TraceData;
};

/*
* Wrapper class for passing around in BP
*/
UCLASS(BlueprintType, EditInlineNew)
class UE4IK_API UHumanoidLegChain_Wrapper : public UIKChainWrapper
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	FHumanoidLegChain Chain;

	UFUNCTION(BlueprintCallable, Category = IK)
	void Initialize(FHumanoidLegChain InChain) 
	{		
		Chain = InChain;
		bInitialized = true;
	}

	virtual bool InitIfInvalid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("Humanoid IK Leg Chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
#endif // ENABLE_IK_DEBUG
			return false;
		}
		
		return Chain.InitIfInvalid(RequiredBones);
	}
	
	// Initialize all bones used in this chain. Must be called before use.
	virtual bool InitBoneReferences(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
#if ENABLE_IK_DEBUG
			UE_LOG(LogIK, Warning, TEXT("Humanoid IK Leg Chain wrapper was not initialized -- make sure you call Initialize function in blueprint before use"));
#endif // ENABLE_IK_DEBUG
			return false;
		}

		return Chain.InitIfInvalid(RequiredBones);
	}
	
	// Check whether this chain is valid to use. Should be called in the IsValid method of your animnode.
	virtual bool IsValid(const FBoneContainer& RequiredBones)
	{
		if (!bInitialized)
		{
			return false;
		}

		return Chain.IsValid(RequiredBones);
	}
};

/*
* Contains Humanoid IK utility functions
*/
USTRUCT()
struct FHumanoidIK
{
	GENERATED_USTRUCT_BODY()
		
/*
* Does traces from foot and toe to the floor
*/
static void HumanoidIKLegTrace(ACharacter* Character,
	FCSPose<FCompactPose>& MeshBases,
	FHumanoidLegChain& LegChain,
	FIKBone& PelvisBone,
	float MaxPelvisAdjustHeight,
	FHumanoidIKTraceData& OutTraceData,
	bool bEnableDebugDraw = false);
};