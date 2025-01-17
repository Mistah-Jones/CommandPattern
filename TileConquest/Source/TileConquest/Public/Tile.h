// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UCLASS()
class TILECONQUEST_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Indicates if the tile has been stepped on by the player
	UPROPERTY(EditAnywhere, Category = "Functionality")
	bool SteppedOn;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* TileMesh;

	UPROPERTY(EditAnywhere, Category = "Materials")
		UMaterialInterface* DeactivatedMaterial;
	UPROPERTY(EditAnywhere, Category = "Materials")
		UMaterialInterface* ActivatedMaterial;

	void StepOn();
	void StepOff();

};
