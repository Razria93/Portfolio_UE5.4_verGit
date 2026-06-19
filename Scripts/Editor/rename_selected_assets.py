import unreal
import re

# Content Browser에서 선택한 에셋 이름을 프로젝트 규칙으로 일괄 변경한다.
# False면 미리보기만 출력하고, True면 실제 Rename을 실행한다.
APPLY_CHANGES = True

# === 사용자 설정 =========================================
# 최종 이름 기본 형식: PF_[ASSET_NAME]_[ASSET_TYPE]_[Direction]_[Index]_[Variant]
NAME_PREFIX = "PF"
ASSET_NAME = "Parry"
ASSET_TYPE = "Sword"

# 방향 토큰을 이름에 포함할지 / 방향을 못 찾으면 실패로 볼지 설정한다.
INCLUDE_DIRECTION_TOKEN = True
REQUIRE_DIRECTION_TOKEN = True

# 파일명에서 숫자를 찾아 이름에 포함할지 / 숫자를 못 찾으면 실패로 볼지 설정한다.
INCLUDE_INDEX_TOKEN = False
REQUIRE_INDEX_TOKEN = False
INDEX_DIGITS = 2

# 원본 이름에 Start / Loop / End 같은 구분자가 있으면 마지막에 유지한다.
KEEP_VARIANT_TOKEN = True
VARIANT_TOKENS = {
    "Start": "Start",
    "Loop": "Loop",
    "End": "End",
    "In": "In",
    "Out": "Out",
    "Fail": "ParryBreak",
}

# 원본 방향 이름을 프로젝트 방향 토큰으로 바꾼다.
# Front_Left는 좌측 이동의 전방 버전이므로 LF로 둔다.
# Front_Center_Left가 일반적인 전방좌측 이동이므로 FL로 둔다.
DIRECTION_TOKENS = {
    # 전방 / 전방좌측 / 전방우측
    "Front_Center": "F",
    "Front_Center_Left": "FL",
    "Front_Center_Right": "FR",

    # 후방 / 후방좌측 / 후방우측
    "Back_Center": "B",
    "Back_Center_Left": "BL",
    "Back_Center_Right": "BR",

    # 좌측 (전방 / 후방)
    "Front_Left": "L_F",
    "Back_Left": "L_B",

    # 우측 (전방 / 후방)
    "Front_Right": "R_F",
    "Back_Right": "R_B",

    # 상/하 + 좌/우
    "Lower_Left": "Low_L",
    "Lower_Right": "Low_R",
    "Upper_Left": "Up_L",
    "Upper_Right": "Up_R",

    "_F_R_": "FR",
    "_F_L_": "FL",
    "_B_R_": "BR",
    "_B_L_": "BL",

    "_F_": "F",
    "_B_": "B",
    "_L_": "L",
    "_R_": "R",
}

# 방향 / 인덱스 / variant를 찾기 전에 무시할 원본 suffix.
SOURCE_SUFFIXES_TO_IGNORE = [
    "_UE5",
    "_Anim",
    "_Montage",
]


def strip_source_suffixes(name):
    # 원본 에셋 suffix를 제거해서 토큰 탐지 오차를 줄인다.
    stripped = name
    for suffix in SOURCE_SUFFIXES_TO_IGNORE:
        if stripped.endswith(suffix):
            stripped = stripped[: -len(suffix)]
    return stripped


def resolve_direction_token(source_name):
    # 긴 토큰부터 검사해서 Front_Center_Left가 Front로 먼저 잡히지 않게 한다.
    normalized = strip_source_suffixes(source_name)

    for source_token, direction_token in sorted(DIRECTION_TOKENS.items(), key=lambda item: len(item[0]), reverse=True):
        # _F_R_ 같은 축약형은 단어 경계를 맞춰 검사한다.
        if source_token.startswith("_") and source_token.endswith("_"):
            if source_token in f"_{normalized}_":
                return direction_token
            continue

        if source_token in normalized:
            return direction_token

    return None


def resolve_variant_token(source_name):
    # Start / Loop / End 같은 lifecycle 토큰을 선택적으로 보존한다.
    if not KEEP_VARIANT_TOKEN:
        return None

    normalized = strip_source_suffixes(source_name)
    for source_token, variant_token in sorted(VARIANT_TOKENS.items(), key=lambda item: len(item[0]), reverse=True):
        if source_token in normalized:
            return variant_token

    return None


def resolve_index_token(source_name):
    # 파일명 안의 숫자 토큰을 찾아 마지막 숫자를 인덱스로 사용한다.
    normalized = strip_source_suffixes(source_name)
    index_matches = re.findall(r"(?<![A-Za-z])(\d+)(?![A-Za-z])", normalized)

    if not index_matches:
        return None

    index_token = index_matches[-1]
    if INDEX_DIGITS <= 0:
        return index_token

    # 1 -> 01처럼 자리수를 맞춘다.
    return index_token.zfill(INDEX_DIGITS)


def build_new_name(old_name):
    # 최종 이름 조립 순서: Prefix / AssetName / AssetType / Direction / Index / Variant.
    name_parts = [NAME_PREFIX, ASSET_NAME, ASSET_TYPE]

    direction_token = None
    if INCLUDE_DIRECTION_TOKEN:
        direction_token = resolve_direction_token(old_name)

    if INCLUDE_DIRECTION_TOKEN and REQUIRE_DIRECTION_TOKEN and not direction_token:
        return None, "direction not found"

    if direction_token:
        name_parts.append(direction_token)

    index_token = None
    if INCLUDE_INDEX_TOKEN:
        index_token = resolve_index_token(old_name)

    if INCLUDE_INDEX_TOKEN and REQUIRE_INDEX_TOKEN and not index_token:
        return None, "index not found"

    if index_token:
        name_parts.append(index_token)

    variant_token = resolve_variant_token(old_name)
    if variant_token:
        name_parts.append(variant_token)

    return "_".join(name_parts), None


def get_package_path(asset):
    # Unreal rename API에 넘길 패키지 폴더 경로만 추출한다.
    asset_path = asset.get_path_name()
    return asset_path.split(".")[0].rsplit("/", 1)[0]


def has_conflicts(rename_plan):
    # 같은 폴더 안에서 여러 에셋이 같은 이름으로 바뀌면 rename을 막는다.
    names_by_package = {}
    b_has_conflict = False

    for asset, package_path, new_name in rename_plan:
        conflict_key = f"{package_path}/{new_name}"
        names_by_package.setdefault(conflict_key, []).append(asset.get_name())

    for conflict_key, old_names in names_by_package.items():
        if len(old_names) <= 1:
            continue

        b_has_conflict = True
        unreal.log_error(f"[RenameSelectedAssets] Conflict: {conflict_key} <= {old_names}")

    return b_has_conflict


def main():
    # Content Browser에서 선택한 에셋만 rename 대상으로 삼는다.
    assets = unreal.EditorUtilityLibrary.get_selected_assets()
    if not assets:
        unreal.log_warning("[RenameSelectedAssets] No assets selected.")
        return

    rename_plan = []
    skipped_assets = []

    for asset in assets:
        # 각 에셋의 새 이름을 만들고, 실패한 항목은 skip한다.
        old_name = asset.get_name()
        new_name, skip_reason = build_new_name(old_name)

        if not new_name:
            skipped_assets.append(old_name)
            unreal.log_warning(f"[RenameSelectedAssets] Skip: {skip_reason} | {old_name}")
            continue

        if old_name == new_name:
            unreal.log(f"[RenameSelectedAssets] Skip: already named | {old_name}")
            continue

        package_path = get_package_path(asset)
        unreal.log(f"[RenameSelectedAssets] {old_name} -> {new_name}")
        rename_plan.append((asset, package_path, new_name))

    if skipped_assets:
        unreal.log_warning("[RenameSelectedAssets] Some assets were skipped. Update settings or token maps if needed.")

    if not rename_plan:
        unreal.log("[RenameSelectedAssets] No rename targets.")
        return

    if has_conflicts(rename_plan):
        unreal.log_error("[RenameSelectedAssets] Rename canceled because target names conflict.")
        return

    if not APPLY_CHANGES:
        # 기본은 미리보기 모드다. 로그를 확인한 뒤 APPLY_CHANGES를 True로 바꾼다.
        unreal.log_warning("[RenameSelectedAssets] Preview only. Set APPLY_CHANGES = True to rename.")
        return

    # Unreal Editor의 asset rename API로 실제 rename을 실행한다.
    rename_data = [
        unreal.AssetRenameData(asset, package_path, new_name)
        for asset, package_path, new_name in rename_plan
    ]

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.rename_assets(rename_data)
    unreal.log(f"[RenameSelectedAssets] Renamed {len(rename_data)} assets.")


main()
