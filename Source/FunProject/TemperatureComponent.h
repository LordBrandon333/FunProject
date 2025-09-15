#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TemperatureComponent.generated.h"

class AWorldTimeManager;
class UHealthComponent;
class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBodyTempChanged, float, OldC, float, NewC);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmbientTempChanged, float, OldC, float, NewC);

/**
 * Einfaches, robustes Temperaturmodell:
 * - AmbientTempC = (Wetter/Tag-Nacht/Höhe) + HeatSources (für UI spürbar)
 * - Körpertemperatur ändert sich pro Tick gemäß:
 *     dT = EnvFlux + MetabolicFlux + HomeostasisFlux
 *   mit Dämpfung durch Kleidung (Insulation), Verstärkung durch Wind/Nässe.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FUNPROJECT_API UTemperatureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTemperatureComponent();

	// === Reads ===
	UFUNCTION(BlueprintPure, Category = "Temperature")
	float GetBodyTempC() const { return BodyTempC; }

	UFUNCTION(BlueprintPure, Category = "Temperature")
	float GetAmbientTempC() const { return AmbientTempC; } // enthält HeatSources (bewusst für UI)

	// === Events ===
	UPROPERTY(BlueprintAssignable, Category = "Temperature")
	FOnBodyTempChanged OnBodyTempChanged;

	UPROPERTY(BlueprintAssignable, Category = "Temperature")
	FOnAmbientTempChanged OnAmbientTempChanged;

	// === Tuning / Balance ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance")
	float ComfortBodyTempC = 37.0f;

	// „Sicherer“ Bereich – außerhalb gibt's leichten Schaden über Zeit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance")
	FVector2D SafeBodyTempRangeC = FVector2D(35.5f, 38.5f);

	// Kleidung/Dämmung 0..1 (0 = keine, 1 = perfekt)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Insulation = 0.30f;

	// 0..1: draußen/regengeschützt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindExposure = 0.20f;

	// 0..1: trocken/nass (Nässe verstärkt Kälte)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Wetness = 0.0f;

	// Basalmetabolismus in °C pro Sekunde (im Stand)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0"))
	float MetabolicIdleCPerSec = 0.0030f;

	// Zusatz durch Bewegung (bei Sprint ~ volle Skala)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0"))
	float MetabolicMoveBonusMaxCPerSec = 0.0040f;

	// Thermoregulation („Homeostasis“) zieht T zum Komfort – stützt Stabilität in moderatem Klima
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0"))
	float HomeostasisCPerSec = 0.0025f;

	// Umgebungskopplung (Newton-Kühlung) – kleine Werte = langsame Anpassung
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0"))
	float EnvBaseCouplingCPerSec = 0.00020f;

	// Verstärker für Wind/Nässe auf den Kopplungskoeffizienten
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0"))
	float WindCouplingFactor = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0"))
	float WetnessCouplingFactor = 2.0f;

	// Wie stark die Thermoregulation bei sehr rauem Klima abnimmt (0..1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HomeostasisFalloff = 0.6f;

	// „Moderates Klima“-Band um Comfort, volle Thermoregulation innerhalb (°C)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance", meta = (ClampMin = "1.0"))
	float HomeostasisFullRangeC = 12.0f;

	// Höhenkorrektur (°C pro Meter; UE-Einheiten ≈ cm → Z/100 = m)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance")
	float ElevationLapseRateCPerMeter = -0.0065f; // ~ -0.65 °C / 100 m

	// Basis-Wetter-Mittelwert (ohne Tagesgang, ohne Höhe, ohne HeatSources)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Balance")
	float BaseAmbientC = 15.0f;

	// Tagesgang als Kurve (t in [0..1] → Δ°C). Wenn leer, verwenden wir Sinus.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperature|Curves")
	TObjectPtr<UCurveFloat> DiurnalCurve = nullptr;

	// Fallback: Taglänge in Sekunden, falls kein WorldTimeManager gefunden wird
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Time", meta = (ClampMin = "10.0"))
	float FallbackDayLengthSeconds = 600.0f;

	// Tickabstand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature|Tick", meta = (ClampMin = "0.05"))
	float UpdateInterval = 0.5f;

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle TickHandle;

	// Laufende Werte
	UPROPERTY(VisibleAnywhere, Category = "Temperature|State")
	float BodyTempC = 37.0f;

	UPROPERTY(VisibleAnywhere, Category = "Temperature|State")
	float AmbientTempC = 15.0f; // enthält HeatSources (für „gefühlte Umgebung“ im UI)

	TWeakObjectPtr<AWorldTimeManager> TimeManager;
	TWeakObjectPtr<UHealthComponent>  Health;

	// === Kern-Update ===
	void UpdateOnce();

	// === Teilberechnungen ===
	float ComputeAmbientBaseAt(const FVector& Loc, float& OutDiurnalDelta) const;
	float SumHeatSourcesAt(const FVector& Loc) const;
	float ComputeMetabolicCPerSec() const;
	float ComputeHomeostasisCPerSec(float EffectiveAmbientC) const;
	float ComputeEnvCoupling() const;

	// Health-Folgen bei Extremwerten
	void ApplyHealthEffects(float OldC, float NewC);
};
