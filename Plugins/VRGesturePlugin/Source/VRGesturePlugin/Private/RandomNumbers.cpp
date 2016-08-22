// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "VRGesturePluginPrivatePCH.h"
#include "RandomNumbers.h"
#include <random>


// random number generator
std::random_device                      rd;
std::mt19937                            normgen;
std::normal_distribution<float>         *rndnorm;
std::default_random_engine              unifgen;
std::uniform_real_distribution<float>   *rndunif;

RandomNumbers::RandomNumbers()
{
	normgen = std::mt19937(rd());
	rndnorm = new std::normal_distribution<float>(0.0, 1.0);
	unifgen = std::default_random_engine(rd());
	rndunif = new std::uniform_real_distribution<float>(0.0, 1.0);
}

float RandomNumbers::GetRandomUniform()
{
	return (*rndunif)(unifgen); 
}

float RandomNumbers::GetRandomNormal()
{
	return (*rndnorm)(normgen); 
}