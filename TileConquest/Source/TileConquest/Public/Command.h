// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class TILECONQUEST_API Command
{
public:
	Command();
	~Command();
	virtual void execute() = 0;
	virtual void undo() = 0;
};
