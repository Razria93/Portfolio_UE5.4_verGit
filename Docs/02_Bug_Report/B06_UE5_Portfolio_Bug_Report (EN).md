# UE5 Portfolio Bug Report (EN)

## Title

**M4-B06: Existing Weapon Blueprint parent class load failure after C++ class rename**

### Date

- **2026.04.20**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/action-orchestration`


---

## Summary

- After renaming `ACAttachment` to `ACWeaponActor`, the existing `BP_CAttachment_Sword` Blueprint asset failed to load.

- The existing Blueprint had stored its parent class path as `/Script/Portfolio.CAttachment`, but after the C++ class rename, that class no longer existed. As a result, `BlueprintGeneratedClass` and child exports failed to load.

- This was fixed by adding a `CoreRedirects` **class redirect** to `Config/DefaultEngine.ini`, allowing the existing Blueprint to resolve its parent class to the new C++ class `ACWeaponActor`.

- After restarting the editor and verifying Blueprint loading, the existing Weapon Blueprint asset was confirmed to be restored successfully.


---

## Environment

- Engine: Unreal Engine 5.4

- Related Commit:

  - `c0504d6`

- Related Code:

  - `Source/Portfolio/Weapon/CAttachment.h`

  - `Source/Portfolio/Weapon/CWeaponActor.h`

  - `Config/DefaultEngine.ini`

- Related Asset:

  - `/Game/06_Weapon/BP_CAttachment_Sword`


---

## Reproduction Steps

1. Rename the `ACAttachment` class to `ACWeaponActor`.

2. Open the existing `BP_CAttachment_Sword` asset or load the project in the editor.

3. Check the editor log for existing Blueprint class export load failure messages.

```text
CreateExport: Failed to load Outer for resource 'Capsule_GEN_VARIABLE' BlueprintGeneratedClass /Game/06_Weapon/BP_CAttachment_Sword.BP_CAttachment_Sword_C.

CreateExport: Failed to load Outer for resource 'Trail' BP_CAttachment_Sword_C /Game/06_Weapon/BP_CAttachment_Sword.Default__BP_CAttachment_Sword_C.

CreateExport: Failed to load Outer for resource 'DefaultSceneRoot_GEN_VARIABLE' BlueprintGeneratedClass /Game/06_Weapon/BP_CAttachment_Sword.BP_CAttachment_Sword_C.

CreateExport: Failed to load Outer for resource 'RootScene' BP_CAttachment_Sword_C /Game/06_Weapon/BP_CAttachment_Sword.Default__BP_CAttachment_Sword_C.
```


---

## Expected Result

- The existing `BP_CAttachment_Sword` should correctly reference the new C++ parent class `ACWeaponActor`.

- The existing Blueprint components and configured values should be preserved while the asset loads normally.

- The migration should be possible through the C++ class rename without recreating the asset.


---

## Actual Result

- The existing Blueprint referenced the old `CAttachment` class, which no longer existed.

- The Blueprint class export and child component exports failed to load.

- As a result, the existing Weapon Blueprint asset could not be opened normally.


---

## Cause

- Unreal Blueprint assets serialize and store the script path of their C++ parent class.

- `ACAttachment` was renamed to `ACWeaponActor`, but there was no redirect from the old class path to the new class path.

- As a result, the existing asset that referenced `/Script/Portfolio.CAttachment` could not resolve the new class `/Script/Portfolio.CWeaponActor`.


---

## Fix

Added a class redirect to `Config/DefaultEngine.ini`.

```ini
[CoreRedirects]
+ClassRedirects=(OldName="/Script/Portfolio.CAttachment",NewName="/Script/Portfolio.CWeaponActor")
```

- Redirected the previous C++ UClass name `CAttachment` to the new UClass name `CWeaponActor`.

- This allows the existing Blueprint to resolve the new parent class when the editor restarts.


---

## Verification Result

- Completely closed the editor, rebuilt the project, and restarted the editor.

- Confirmed that `/Game/06_Weapon/BP_CAttachment_Sword` loads correctly with the new C++ parent class `ACWeaponActor`.

- Confirmed that the existing Blueprint could be restored without recreating it.

- Saved the Blueprint again so that its reference state reflects the applied redirect.

- Confirmed that the original `BlueprintGeneratedClass` and component export load failure logs no longer reproduce.


---

## Follow-Up Cleanup

- Rename the asset to `BP_CWeaponActor_Sword` in the editor if needed.

- Keep the redirect for now after the Blueprint asset has been successfully saved following the C++ class rename.

- Remove the redirect only after confirming that all references have been saved with the new name and no old class references remain.

- Confirm that the Blueprint continues to work normally afterward.


---

## Notes

- Even if only a C++ class rename is performed, Blueprint assets may continue referencing the previous parent class path.

- In Unreal, UCLASS names are registered without the `A` / `U` prefix. Therefore, the UClass name of `ACAttachment` is `CAttachment`, and the UClass name of `ACWeaponActor` is `CWeaponActor`.

- When a C++ class rename affects a Blueprint parent class, it is safer to try migration through `CoreRedirects` before recreating the asset.


---
