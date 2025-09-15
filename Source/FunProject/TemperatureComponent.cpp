#include "TemperatureComponent.h"

#include "WorldTimeManager.h"
#include "HeatSourceInterface.h"
#include "HealthComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UTemperatureComponent::UTemperatureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTemperatureComponent::BeginPlay()
{
	Super::BeginPlay();

	// Optional: Health referenzieren
	if (AActor* Owner = GetOwner())
	{
		Health = Owner->FindComponentByClass<UHealthComponent>();
	}

	// WorldTimeManager (falls in der Map platziert)
	if (AWorldTimeManager* Mgr = Cast<AWorldTimeManager>(UGameplayStatics::GetActorOfClass(this, AWorldTimeManager::StaticClass())))
	{
		TimeManager = Mgr;
	}

	// Initiale Ambient setzen
	float Dummy = 0.f;
	AmbientTempC = ComputeAmbientBaseAt(GetOwner()->GetActorLocation(), Dummy) + SumHeatSourcesAt(GetOwner()->GetActorLocation());

	// Regelmäßiges Update
	GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &UTemperatureComponent::UpdateOnce, UpdateInterval, true, 0.1f);
}

void UTemperatureComponent::UpdateOnce()
{
	const FVector Loc = GetOwner()->GetActorLocation();

	const float OldAmbient = AmbientTempC;
	const float OldBody = BodyTempC;

	// 1) Ambient (inkl. HeatSources für „gefühlte“ Umgebung)
	float DiurnalDelta = 0.f;
	const float AmbientBase = ComputeAmbientBaseAt(Loc, DiurnalDelta);        // Wetter/Tag/Nacht/Höhe
	const float HeatSources = SumHeatSourcesAt(Loc);                           // lokale Wärme-/Kältequellen
	AmbientTempC = AmbientBase + HeatSources;

	// 2) Flüsse (°C/s) berechnen
	const float EnvCoupling = ComputeEnvCoupling();                           // Kleidung/Wind/Nässe
	const float EnvFlux = (AmbientTempC - BodyTempC) * EnvCoupling;      // Newton-Kühlung
	const float MetabolicFlux = ComputeMetabolicCPerSec();                      // + Wärme
	const float HomeoFlux = ComputeHomeostasisCPerSec(AmbientBase + HeatSources);

	// 3) Integration
	const float Dt = UpdateInterval;
	BodyTempC += (EnvFlux + MetabolicFlux + HomeoFlux) * Dt;

	// 4) Events/Health
	if (!FMath::IsNearlyEqual(OldAmbient, AmbientTempC, 0.05f))
	{
		OnAmbientTempChanged.Broadcast(OldAmbient, AmbientTempC);
	}
	if (!FMath::IsNearlyEqual(OldBody, BodyTempC, 0.01f))
	{
		OnBodyTempChanged.Broadcast(OldBody, BodyTempC);
		ApplyHealthEffects(OldBody, BodyTempC);
	}
}

float UTemperatureComponent::ComputeAmbientBaseAt(const FVector& Loc, float& OutDiurnalDelta) const
{
	// Basis
	float Ambient = BaseAmbientC;

	// Tagesgang t in [0..1]
	float T = 0.f;
	if (TimeManager.IsValid())
	{
		T = TimeManager->GetTimeOfDayNormalized();
	}
	else
	{
		// Fallback: Eigene Uhr
		const float L = FMath::Max(10.f, FallbackDayLengthSeconds);
		T = FMath::Fmod(GetWorld()->GetTimeSeconds() / L, 1.f);
	}

	// Diurnaler Anteil
	if (DiurnalCurve)
	{
		OutDiurnalDelta = DiurnalCurve->GetFloatValue(T);
	}
	else
	{
		// Sinus: Tmin ~ 04:00 (t≈0.167), Tmax ~ 14:00 (t≈0.583)
		OutDiurnalDelta = 8.f * FMath::Sin(2.f * PI * (T - 0.08f));
	}
	Ambient += OutDiurnalDelta;

	// Höhe (UE: cm → m)
	Ambient += ElevationLapseRateCPerMeter * (Loc.Z / 100.f);

	// (Wetter/Biome kannst du später als weitere Offsets hinzufügen)

	return Ambient;
}

float UTemperatureComponent::SumHeatSourcesAt(const FVector& Loc) const
{
	TArray<AActor*> Candidates;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UHeatSourceInterface::StaticClass(), Candidates);

	float Sum = 0.f;
	for (AActor* A : Candidates)
	{
		if (!A) continue;
		const float C = IHeatSourceInterface::Execute_GetHeatContributionAt(A, Loc);
		Sum += C;
	}
	return Sum;
}

float UTemperatureComponent::ComputeMetabolicCPerSec() const
{
	const ACharacter* C = Cast<ACharacter>(GetOwner());
	if (!C) return MetabolicIdleCPerSec;

	const UCharacterMovementComponent* Move = C->GetCharacterMovement();
	const float Speed = Move ? C->GetVelocity().Size() : 0.f;

	// Normalisiere auf „schnelles Laufen/Sprint“ ~600 uu/s
	const float Alpha = FMath::Clamp(Speed / 600.f, 0.f, 1.f);
	return MetabolicIdleCPerSec + Alpha * MetabolicMoveBonusMaxCPerSec;
}

float UTemperatureComponent::ComputeHomeostasisCPerSec(float EffectiveAmbientC) const
{
	// In moderatem Klima hält der Körper aktiv ~37°C (Stellgröße zu ComfortBodyTempC)
	float Strength = HomeostasisCPerSec;

	// Wenn Umgebung sehr weit weg vom Komfort ist, nimmt die Regelstärke ab
	const float Dist = FMath::Abs(EffectiveAmbientC - ComfortBodyTempC);
	if (Dist > HomeostasisFullRangeC)
	{
		const float Excess = (Dist - HomeostasisFullRangeC) / HomeostasisFullRangeC; // 0..∞
		const float Falloff = FMath::Clamp(1.f - Excess, 0.f, 1.f); // linear bis 0
		Strength = FMath::Lerp(Strength * HomeostasisFalloff, Strength, Falloff);
	}

	// Richtung: zurück zur Komforttemperatur
	return (ComfortBodyTempC - BodyTempC) * Strength > 0.f
		? Strength
		: -Strength; // kleines symmetrisches Ziehen – genügt für Spiel-Feedback
}

float UTemperatureComponent::ComputeEnvCoupling() const
{
	// Kleidung reduziert Kopplung, Wind/Nässe erhöhen sie
	const float K = EnvBaseCouplingCPerSec
		* (1.f - Insulation)
		* (1.f + WindCouplingFactor * WindExposure)
		* (1.f + WetnessCouplingFactor * Wetness);
	return K;
}

void UTemperatureComponent::ApplyHealthEffects(float OldC, float NewC)
{
	if (!Health.IsValid()) return;

	// Leichter Schaden, wenn außerhalb des sicheren Bereichs
	if (NewC < SafeBodyTempRangeC.X)
	{
		// skaliere mit Distanz vom SafeMin (optionales Feintuning)
		const float D = SafeBodyTempRangeC.X - NewC;           // °C
		const float PerSecond = 0.02f * (1.f + D * 0.5f);      // 0.02..+
		//Health->ApplyDamage(PerSecond * UpdateInterval, GetOwner());
	}
	else if (NewC > SafeBodyTempRangeC.Y)
	{
		const float D = NewC - SafeBodyTempRangeC.Y;
		const float PerSecond = 0.02f * (1.f + D * 0.5f);
		//Health->ApplyDamage(PerSecond * UpdateInterval, GetOwner());
	}
}
