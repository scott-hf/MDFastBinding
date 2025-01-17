﻿#include "MDFastBindingContainer.h"
#include "MDFastBindingInstance.h"
#include "BindingDestinations/MDFastBindingDestinationBase.h"

void UMDFastBindingContainer::InitializeBindings(UObject* SourceObject)
{
	for (int32 i = 0; i < Bindings.Num(); ++i)
	{
		if (UMDFastBindingInstance* Binding = Bindings[i])
		{
			Binding->InitializeBinding(SourceObject);
			Binding->UpdateBinding(SourceObject);
		
			BindingTickPolicyLookUpMap.Add(i, Binding->ShouldBindingTick());
		}
	}
}

void UMDFastBindingContainer::UpdateBindings(UObject* SourceObject)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*GetNameSafe(SourceObject));
	for (int32 i = 0; i < Bindings.Num(); ++i)
	{
		if (bool& bShouldBindingUpdate = BindingTickPolicyLookUpMap.FindOrAdd(i))
		{
			if (UMDFastBindingInstance* Binding = Bindings[i])
			{
				Binding->UpdateBinding(SourceObject);
				bShouldBindingUpdate = Binding->ShouldBindingTick();
			}
		}
	}
}

void UMDFastBindingContainer::TerminateBindings(UObject* SourceObject)
{
	for (UMDFastBindingInstance* Binding : Bindings)
	{
		Binding->TerminateBinding(SourceObject);
	}

	BindingTickPolicyLookUpMap.Empty();
}

void UMDFastBindingContainer::SetBindingTickPolicy(UMDFastBindingInstance* Binding, bool bShouldTick)
{
	const int32 BindingIndex = Bindings.IndexOfByKey(Binding);

	if (BindingIndex != INDEX_NONE)
	{
		BindingTickPolicyLookUpMap.FindOrAdd(BindingIndex, bShouldTick);
	}
}

#if WITH_EDITORONLY_DATA
UMDFastBindingInstance* UMDFastBindingContainer::AddBinding()
{
	UMDFastBindingInstance* Binding = NewObject<UMDFastBindingInstance>(this, NAME_None, RF_Public | RF_Transactional);
	Bindings.Add(Binding);
	return Binding;
}

UMDFastBindingInstance* UMDFastBindingContainer::DuplicateBinding(UMDFastBindingInstance* InBinding)
{
	const int32 CurrentIdx = Bindings.IndexOfByKey(InBinding);
	if (CurrentIdx != INDEX_NONE)
	{
		if (UMDFastBindingInstance* NewBinding = DuplicateObject<UMDFastBindingInstance>(InBinding, this, NAME_None))
		{
			Bindings.Insert(NewBinding, CurrentIdx + 1);
			return NewBinding;
		}
	}

	return nullptr;
}

bool UMDFastBindingContainer::DeleteBinding(UMDFastBindingInstance* InBinding)
{
	return Bindings.Remove(InBinding) > 0;
}
#endif

#if WITH_EDITOR
EDataValidationResult UMDFastBindingContainer::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	for (UMDFastBindingInstance* Binding : Bindings)
	{
		if (Binding != nullptr)
		{
			if (Binding->IsDataValid(ValidationErrors) == EDataValidationResult::Invalid)
			{
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	return Result;
}
#endif
