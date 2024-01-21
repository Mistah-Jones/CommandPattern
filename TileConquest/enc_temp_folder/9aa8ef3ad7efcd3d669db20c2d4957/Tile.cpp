// Fill out your copyright notice in the Description page of Project Settings.


#include "Tile.h"

// Sets default values
ATile::ATile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tile Mesh"));

	DeactivatedMaterial = CreateDefaultSubobject<UMaterialInterface>("Deactivated Material");
	ActivatedMaterial = CreateDefaultSubobject<UMaterialInterface>("Activated Material");

}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
	TileMesh->SetMaterial(0, SteppedOn ? ActivatedMaterial : DeactivatedMaterial);
}

// Called every frame
void ATile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATile::StepOn()
{
	SteppedOn = !SteppedOn;
	TileMesh->SetMaterial(0, SteppedOn ? ActivatedMaterial : DeactivatedMaterial);
}

