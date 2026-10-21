#pragma once

class UWorld;
class ACEnemy;

// Read-only visualization of the existing Investigate blackboard and movement.
struct FInvestigateDebug
{
	static void Draw(UWorld* World, const ACEnemy* Enemy);
	static void Clear(UWorld* World);
};
