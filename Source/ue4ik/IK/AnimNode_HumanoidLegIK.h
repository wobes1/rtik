// Copyright (c) Henry Cooney 2017

#pragma once

#include "CoreMinimal.h"
#include "HumanoidIK.h"
#include "BoneControllers/AnimNode_Fabrik.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_HumanoidLegIK.generated.h"


/*
* How leg IK should behave
*/
UENUM(BlueprintType)
enum class EHumanoidLegIKMode : uint8
{	
	// IK for normal locomotion -- will prevent feet from clipping or floating above the ground during normal movement.
	IK_Human_Leg_Locomotion UMETA(DisplayName = "Normal Locomotion"),
	
	// IK onto an arbitrary world location
	IK_Human_Leg_WorldLocation UMETA(DisplayName = "IK Onto World Location")
};

/*
  * IKs a humanoid biped leg onto a target location. Should be preceeded by hip adjustment to ensure the legs can reach. 
  * Uses FABRIK IK solver.  
  * 
  * Knee rotation is not enforced in this node.
*/
USTRUCT()
struct UE4IK_API FAnimNode_HumanoidLegIK : public FAnimNode_SkeletalControlBase
{

	GENERATED_USTRUCT_BODY()

public:

	// Pose before any IK or IK pre-processing (e.g., pelvis adjustment) is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FComponentSpacePoseLink BaseComponentPose;

	// The leg on which IK is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidLegChain_Wrapper* Leg;

	// Trace data for this leg (use IKHumanoidLegTrace to update it)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	UHumanoidIKTraceData_Wrapper* TraceData;
	
	// Target location for the foot; IK will attempt to move the tip of the shin here. In world space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bones, meta = (PinShownByDefault))
	FVector FootTargetWorld;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnableDebugDraw;

	// How precise the FABRIK solver should be. Iteration will cease when effector is within this distance of 
    // the target. Set lower for more accurate IK, but potentially greater cost.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	float Precision;
	
	// Max number of FABRIK iterations. After this many iterations, FABRIK will always stop. Increase for more accurate IK,
    // but potentially greater cost.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	int32 MaxIterations;

	// If set to false, will return to base pose instead of attempting to IK
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
	bool bEnable;	

	// How to handle an unreachable IK target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	EIKUnreachableRule UnreachableRule;

	// Set to 'locomotion' for normal movement; 'world location' to manually IK the leg onto a world location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	EHumanoidLegIKMode Mode;
	
	// How to handle rotation of the effector (the foot). If set to No Change, the foot will maintain the same
	// rotation as before IK. If set to Maintain Local, it will maintain the same rotation relative to the parent
	// as before IK. Copy Target Rotation is the same as No Change for now.	
	//
	// For leg IK, this should usually be set to No Change.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
	TEnumAsByte<EBoneRotationSource> EffectorRotationSource;
   
public:

	FAnimNode_HumanoidLegIK()
		:
		bEnableDebugDraw(false),
		DeltaTime(0.0f),
		FootTargetWorld(0.0f, 0.0f, 0.0f),
		Precision(0.001f),
		MaxIterations(10),
		bEnable(true),
		UnreachableRule(EIKUnreachableRule::IK_Abort),
		Mode(EHumanoidLegIKMode::IK_Human_Leg_Locomotion),
		EffectorRotationSource(EBoneRotationSource::BRS_KeepComponentSpaceRotation)
	{ }

	// FAnimNode_SkeletalControlBase Interface
	virtual void Initialize(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones(const FAnimationCacheBonesContext& Context) override;


	virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	//virtual void EvaluateComponentSpaceInternal(FComponentSpacePoseContext& Output) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
	// End FAnimNode_SkeletalControlBase Interface

protected:
	float DeltaTime;
	FAnimNode_Fabrik FabrikSolver;
};