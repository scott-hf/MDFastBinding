﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "MDFastBindingHelpers.h"
#include "UObject/UnrealType.h"

#include "MDFastBinding.h"

void FMDFastBindingHelpers::GetFunctionParamProps(const UFunction* Func, TArray<const FProperty*>& OutParams)
{
	OutParams.Empty();
	
	if (Func != nullptr)
	{
		for (TFieldIterator<const FProperty> It(Func); It; ++It)
		{
			if (const FProperty* Param = *It)
			{
				if (Param->HasAnyPropertyFlags(CPF_Parm))
				{
					OutParams.Add(Param);
				}
			}
		}
	}
}

void FMDFastBindingHelpers::SplitFunctionParamsAndReturnProp(const UFunction* Func, TArray<const FProperty*>& OutParams,
                                                             const FProperty*& OutReturnProp)
{
	OutParams.Empty();
	OutReturnProp = nullptr;
	
	if (Func != nullptr)
	{
		OutReturnProp = Func->GetReturnProperty();
		
		for (TFieldIterator<const FProperty> It(Func); It; ++It)
		{
			if (const FProperty* Param = *It)
			{
				if (Param->HasAnyPropertyFlags(CPF_Parm))
				{
					const bool bIsReturnParam = Param->HasAnyPropertyFlags(CPF_ReturnParm)
						|| (Param->HasAnyPropertyFlags(CPF_OutParm) && !Param->HasAnyPropertyFlags(CPF_ReferenceParm));
					if (Param != OutReturnProp && !bIsReturnParam)
					{
						OutParams.Add(Param);
					}
					else if (OutReturnProp == nullptr && bIsReturnParam)
					{
						OutReturnProp = Param;
					}
				}
			}
		}
	}
}

FString FMDFastBindingHelpers::PropertyToString(const FProperty& Prop)
{
	if (const FArrayProperty* ArrayProp = CastField<const FArrayProperty>(&Prop))
	{
		return FString::Printf(TEXT("%s<%s>"), *Prop.GetCPPType(), *PropertyToString(*ArrayProp->Inner));
	}
	else if (const FSetProperty* SetProp = CastField<const FSetProperty>(&Prop))
	{
		return FString::Printf(TEXT("%s<%s>"), *Prop.GetCPPType(), *PropertyToString(*SetProp->ElementProp));
	}
	else if (const FMapProperty* MapProp = CastField<const FMapProperty>(&Prop))
	{
		return FString::Printf(TEXT("%s<%s, %s>"), *Prop.GetCPPType(), *PropertyToString(*MapProp->KeyProp), *PropertyToString(*MapProp->ValueProp));
	}

	return Prop.GetCPPType();
}

bool FMDFastBindingHelpers::ArePropertyValuesEqual(const FProperty* PropA, const void* ValuePtrA, const FProperty* PropB, const void* ValuePtrB)
{
	if (PropA == nullptr || ValuePtrA == nullptr || PropB == nullptr || ValuePtrB == nullptr)
	{
		return false;
	}

	bool bResult = false;
	if (PropA->GetClass() == PropB->GetClass())
	{
		return PropA->Identical(ValuePtrA, ValuePtrB);
	}
	else if (FMDFastBindingModule::CanSetProperty(PropA, PropB))
	{
		void* AllocatedValue = FMemory::Malloc(PropA->GetSize(), PropA->GetMinAlignment());
		PropA->InitializeValue(AllocatedValue);
		FMDFastBindingModule::SetProperty(PropA, AllocatedValue, PropB, ValuePtrB);
		bResult = PropA->Identical(ValuePtrA, AllocatedValue);
		FMemory::Free(AllocatedValue);
	}
	else if (FMDFastBindingModule::CanSetProperty(PropB, PropA))
	{
		void* AllocatedValue = FMemory::Malloc(PropB->GetSize(), PropB->GetMinAlignment());
		PropB->InitializeValue(AllocatedValue);
		FMDFastBindingModule::SetProperty(PropB, AllocatedValue, PropA, ValuePtrA);
		bResult = PropB->Identical(AllocatedValue, ValuePtrB);
		FMemory::Free(AllocatedValue);
	}

	return bResult;
}
