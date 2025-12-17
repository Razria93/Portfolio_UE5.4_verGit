# Implement Character / Contorller / Camera Core & TestRoom Level Setup

## Title

✨ feat: implement character / controller / camera core and TestRoom level setup (#4)

## Summary

- This PR implements the basic PlayerCharacter, PlayerController, camera setup, and the basic TestRoom level for initial gameplay testing.


---

## Completed Tasks

### 1. Test Room Level

- [x] Created TestRoom level

- [x] Added floor, walls, and simple blockout props (boxes, stairs)

### 2. Character & Controller

- [x] Added APortfolioPlayerController C++ class

- [x] Added APortfolioCharacter C++ class

- [x] Configured capsule, skeletal mesh, and movement component defaults

### 3. Camera System

- [x] Created SpringArm + CameraComponent setup

- [x] Configured default camera height, shoulder offset, and distance

- [x] Added mouse-driven camera rotation (LookUp / LookRight bindings)


---

## How to Test

1. Launch the project

2. TestRoom will load as the startup map

3. Move with WASD

4. Rotate the camera with mouse movement

5. Check character pivot & mesh orientation

6. Check collision and capsule height feel correct


--

## Related Issue / Branch

- Branch: feature/character-camera-core

- Issue: #4


--

## Notes


- The basic character and camera setup is now complete

- The current input system is temporary, and I plan to migrate to EnhancedInput after the core milestone


---

## Following PRs will focus on

- [ ] The basic movement (WASD) and jump features are now implemented

- [ ] The initial animation blueprint setup (Idle/Walk/Run) is in place
