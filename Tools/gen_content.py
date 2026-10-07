# エディタの Python で BP 子クラス・マテリアル・サウンド・入力アセット・レベルを作り直すスクリプト（最終版）。
# 実行: powershell/pwsh -File Tools/run_python.ps1   （中で UnrealEditor-Cmd.exe -run=pythonscript を呼ぶ）
# 何度実行しても同じ結果になる（既存のアセットとレベルは消して作り直す）。
import os
import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary


def log(msg):
    unreal.log_warning("[gen] " + str(msg))


def native(name):
    cls = unreal.load_class(None, "/Script/BreakoutGame." + name)
    if cls is None:
        raise RuntimeError("native class not found: " + name)
    return cls


def recreate(path):
    if eal.does_asset_exist(path):
        eal.delete_asset(path)


def make_bp(name, folder, parent):
    path = folder + "/" + name
    recreate(path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    bp = asset_tools.create_asset(name, folder, unreal.Blueprint, factory)
    log("created BP " + path)
    return bp


def cdo(bp):
    return unreal.get_default_object(bp.generated_class())


def compile_save(bp):
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    eal.save_loaded_asset(bp)


# ---------------------------------------------------------------- 入力
def key(name):
    k = unreal.Key()
    k.set_editor_property("key_name", name)
    return k


def make_input_assets():
    """IA_Move / IA_Action / IMC_InGame（A キーは Negate、Space = IA_Action）"""
    folder = "/Game/Input"
    for n in ("IA_Move", "IA_Action", "IMC_InGame"):
        recreate(folder + "/" + n)
    ia_move = asset_tools.create_asset("IA_Move", folder, unreal.InputAction, unreal.InputAction_Factory())
    ia_move.set_editor_property("value_type", unreal.InputActionValueType.AXIS1D)
    ia_action = asset_tools.create_asset("IA_Action", folder, unreal.InputAction, unreal.InputAction_Factory())
    ia_action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
    imc = asset_tools.create_asset("IMC_InGame", folder, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    imc.map_key(ia_move, key("D"))
    imc.map_key(ia_move, key("A"))
    imc.map_key(ia_action, key("SpaceBar"))
    data = imc.get_editor_property("default_key_mappings")
    mappings = list(data.get_editor_property("mappings"))
    for m in mappings:
        if str(m.get_editor_property("key").get_editor_property("key_name")) == "A":
            m.set_editor_property("modifiers", [unreal.new_object(unreal.InputModifierNegate, outer=imc)])
    data.set_editor_property("mappings", mappings)
    imc.set_editor_property("default_key_mappings", data)
    for a in (ia_move, ia_action, imc):
        eal.save_loaded_asset(a)
    log("input assets ok")
    return {"IA_Move": ia_move, "IA_Action": ia_action}, imc


# ---------------------------------------------------------------- マテリアル・サウンド
def make_block_material():
    """VectorParameter「BaseColor」を BaseColor と Emissive につないだ BlockMaterial。
    シェーディングは Unlit（ライトの影響を受けず、色がそのまま出るネオン調）"""
    path = "/Game/Materials/BlockMaterial"
    recreate(path)
    mat = asset_tools.create_asset("BlockMaterial", "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
    p = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, 0)
    p.set_editor_property("parameter_name", "BaseColor")
    p.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mel.connect_material_property(p, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 200)
    mul.set_editor_property("const_b", 1.0)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mel.connect_material_expressions(p, "", mul, "A")
    mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    eal.save_loaded_asset(mat)
    log("created material " + path)
    return mat


def make_material_instance(name, parent, color):
    """BlockMaterial の色違い（背景・壁用）"""
    path = "/Game/Materials/" + name
    recreate(path)
    mic = asset_tools.create_asset(name, "/Game/Materials", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mic.set_editor_property("parent", parent)
    mel.set_material_instance_vector_parameter_value(mic, "BaseColor", unreal.LinearColor(*color))
    mel.update_material_instance(mic)
    eal.save_loaded_asset(mic)
    log("created material instance " + path)
    return mic


def import_sound(name, filename, looping=False):
    path = "/Game/Sounds/" + name
    recreate(path)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", os.path.join(unreal.Paths.project_dir(), "Tools", "SourceAudio", filename))
    task.set_editor_property("destination_path", "/Game/Sounds")
    task.set_editor_property("destination_name", name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    asset_tools.import_asset_tasks([task])
    snd = unreal.load_asset(path)
    if snd is None:
        raise RuntimeError("sound import failed: " + filename)
    if looping:
        snd.set_editor_property("looping", True)
    eal.save_loaded_asset(snd)
    log("imported sound " + path)
    return snd


# ---------------------------------------------------------------- ステージ定義
# 6 列（Y = -1250, -750, ... , 1250）。数字 = Hp、# = 壊れないブロック、. = なし。上の行から並べる
STAGES = {
    "Level1": dict(
        stage="STAGE 1", speed=1000.0, next="Level2", z_top=3900,
        rows=["..11..",
              ".1221.",
              "111111"]),
    "Level2": dict(
        stage="STAGE 2", speed=1150.0, next="Level3", z_top=4300,
        rows=["#2222#",
              "1.##.1",
              "212212"]),
    "Level3": dict(
        stage="STAGE 3", speed=1300.0, next="TitleLevel", z_top=4500,
        rows=["..44..",
              ".3##3.",
              "225522"]),
}
EXPOSURE_LIGHT = 6.0
EXPOSURE_FIXED = 1.0
ROW_STEP = 220
COL_STEP = 500


def layout_of(stage):
    """ステージ定義から (y, z, hp) の一覧を作る。hp = -1 は壊れないブロック"""
    out = []
    for r, row in enumerate(stage["rows"]):
        for c, ch in enumerate(row):
            if ch == ".":
                continue
            out.append((-1250 + COL_STEP * c, stage["z_top"] - ROW_STEP * r, -1 if ch == "#" else int(ch)))
    return out


# ---------------------------------------------------------------- レベル
CUBE = "/Engine/BasicShapes/Cube.Cube"


def spawn(cls, loc, rot=(0, 0, 0), label=None, folder=None):
    # rot は (pitch, yaw, roll)
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(cls, unreal.Vector(*loc), unreal.Rotator(roll=rot[2], pitch=rot[0], yaw=rot[1]))
    if label:
        a.set_actor_label(label)
    if folder:
        a.set_folder_path(folder)
    return a


def spawn_cube(label, loc, scale, folder="Walls", material=None, collision="BlockAll"):
    a = spawn(unreal.StaticMeshActor, loc, label=label, folder=folder)
    smc = a.static_mesh_component
    smc.set_static_mesh(unreal.load_asset(CUBE))
    a.set_actor_scale3d(unreal.Vector(*scale))
    smc.set_collision_profile_name(collision)
    if material is not None:
        smc.set_material(0, material)
    return a


def build_level(name, assets, setup_extra=None, game_mode="GameMode"):
    path = "/Game/Maps/" + name
    recreate(path)
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    # 明るさ・空
    light = spawn(unreal.DirectionalLight, (0, 0, 3000), (-30, 10, 0), "DirectionalLight", "Lighting")
    light.light_component.set_editor_property("intensity", EXPOSURE_LIGHT)
    # 自動露出だと暗い背景で画面全体が白っぽくなるので、明るさを固定する
    ppv = spawn(unreal.PostProcessVolume, (0, 0, 2550), label="PostProcessVolume", folder="Lighting")
    ppv.set_editor_property("unbound", True)
    pps = ppv.get_editor_property("settings")
    pps.set_editor_property("override_auto_exposure_min_brightness", True)
    pps.set_editor_property("auto_exposure_min_brightness", EXPOSURE_FIXED)
    pps.set_editor_property("override_auto_exposure_max_brightness", True)
    pps.set_editor_property("auto_exposure_max_brightness", EXPOSURE_FIXED)
    pps.set_editor_property("override_bloom_intensity", True)
    pps.set_editor_property("bloom_intensity", 0.35)
    ppv.set_editor_property("settings", pps)
    spawn(unreal.SkyAtmosphere, (0, 0, 0), label="SkyAtmosphere", folder="Lighting")
    sky = spawn(unreal.SkyLight, (0, 0, 2000), label="SkyLight", folder="Lighting")
    sky.light_component.set_editor_property("real_time_capture", True)
    sky.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

    # 暗い背景（フィールドの奥。当たり判定なし）
    spawn_cube("Backdrop", (300, 0, 2550), (1, 120, 70), folder="Backdrop", material=assets["backdrop"], collision="NoCollision")

    # 外壁（内側は Y -1500..1500、Z 50..5050）
    wall = assets["wall"]
    spawn_cube("Wall_Left", (0, -1550, 2550), (1, 1, 52), material=wall)
    spawn_cube("Wall_Right", (0, 1550, 2550), (1, 1, 52), material=wall)
    spawn_cube("Wall_Top", (0, 0, 5100), (1, 32, 1), material=wall)
    spawn_cube("Wall_Bottom", (0, 0, 0), (1, 32, 1), material=wall)

    cam = spawn(unreal.CameraActor, (-4750, 0, 2550), (0, 0, 0), "CameraActor")
    cam.set_editor_property("auto_activate_for_player", unreal.AutoReceiveInput.PLAYER0)

    spawn(unreal.PlayerStart, (0, 0, 600), label="PlayerStart")

    if setup_extra:
        setup_extra(world)

    ws = unreal.EditorLevelLibrary.get_editor_world().get_world_settings()
    ws.set_editor_property("default_game_mode", assets[game_mode])
    unreal.EditorLoadingAndSavingUtils.save_map(unreal.EditorLevelLibrary.get_editor_world(), path)
    log("saved level " + path)


def main():
    for lv in ("Level1", "Level2", "Level3", "TitleLevel"):
        recreate("/Game/Maps/" + lv)
    bp_folder = "/Game/Blueprints"

    # マテリアルとサウンド
    block_mat = make_block_material()
    backdrop_mat = make_material_instance("BackdropMaterial", block_mat, (0.012, 0.016, 0.05, 1.0))
    wall_mat = make_material_instance("WallMaterial", block_mat, (0.08, 0.35, 0.9, 1.0))
    snd = {
        "knock": import_sound("Breakout_SE_Knock", "Breakout_SE_Knock.wav"),
        "bgm": import_sound("Breakout_BGM", "Breakout_BGM.wav", looping=True),
        "break": import_sound("Breakout_SE_Break", "Breakout_SE_Break.wav"),
        "item": import_sound("Breakout_SE_Item", "Breakout_SE_Item.wav"),
        "gameover": import_sound("Breakout_SE_GameOver", "Breakout_SE_GameOver.wav"),
        "clear": import_sound("Breakout_SE_Clear", "Breakout_SE_Clear.wav"),
    }

    # Blueprint（C++ クラスの子）。GameInstance は DefaultEngine.ini で起動時に読まれるため、あるときは作り直さない
    if not eal.does_asset_exist(bp_folder + "/BreakoutGameInstance"):
        make_bp("BreakoutGameInstance", bp_folder, native("BreakoutGameInstance"))
    compile_save(unreal.load_asset(bp_folder + "/BreakoutGameInstance"))
    paddle = make_bp("Paddle", bp_folder, native("BreakoutPaddle"))
    ball = make_bp("Ball", bp_folder, native("BreakoutBall"))
    block = make_bp("Block", bp_folder, native("BreakoutBlock"))
    item = make_bp("AddBallItem", bp_folder, native("BreakoutAddBallItem"))
    gm = make_bp("GameManager", bp_folder, native("BreakoutGameManager"))
    gamemode = make_bp("BreakoutGame", bp_folder, native("BreakoutGameModeBase"))

    actions, imc = make_input_assets()
    cdo(paddle).set_editor_property("input_mapping_context", imc)
    cdo(paddle).set_editor_property("move_action", actions["IA_Move"])
    cdo(paddle).set_editor_property("action_action", actions["IA_Action"])
    cdo(paddle).set_editor_property("paddle_material", block_mat)
    cdo(ball).set_editor_property("knock_sound", snd["knock"])
    cdo(ball).set_editor_property("ball_material", block_mat)
    cdo(block).set_editor_property("block_material", block_mat)
    cdo(block).set_editor_property("break_sound", snd["break"])
    cdo(block).set_editor_property("item_class", item.generated_class())
    cdo(item).set_editor_property("item_material", block_mat)
    cdo(item).set_editor_property("pickup_sound", snd["item"])
    cdo(gm).set_editor_property("ball_class", ball.generated_class())
    cdo(gm).set_editor_property("bgm_sound", snd["bgm"])
    cdo(gm).set_editor_property("clear_sound", snd["clear"])
    cdo(gm).set_editor_property("game_over_sound", snd["gameover"])
    for bp in (paddle, ball, block, item, gm):
        compile_save(bp)
    cdo(gamemode).set_editor_property("default_pawn_class", paddle.generated_class())
    compile_save(gamemode)

    assets = {
        "GameMode": gamemode.generated_class(),
        "TitleGameMode": native("BreakoutTitleGameMode"),
        "backdrop": backdrop_mat,
        "wall": wall_mat,
    }
    block_cls = block.generated_class()
    gm_cls = gm.generated_class()

    def spawn_blocks(layout):
        for i, (y, z, hp) in enumerate(layout):
            b = spawn(block_cls, (0, y, z), label="Block_%d" % i, folder="Blocks")
            if hp < 0:
                b.set_editor_property("unbreakable", True)
            else:
                b.set_editor_property("hp", hp)

    def stage_extra(stage):
        def extra(world):
            gm_actor = spawn(gm_cls, (0, 0, 0), label="GameManager")
            # ボールの発射位置（スライドの RespawnLocationActor）
            respawn = spawn(unreal.TargetPoint, (0, 0, 1000), label="RespawnLocationActor")
            gm_actor.set_editor_property("spawn_location_actor", respawn)
            gm_actor.set_editor_property("next_level_name", stage["next"])
            gm_actor.set_editor_property("stage_name", stage["stage"])
            gm_actor.set_editor_property("ball_speed", stage["speed"])
            spawn(native("MissArea"), (0, 0, 120), label="MissArea")
            spawn_blocks(layout_of(stage))
        return extra

    for lv, stage in STAGES.items():
        build_level(lv, assets, stage_extra(stage))

    def title_extra(world):
        # 背景にブロックを並べる（数えるだけで GameManager はいない）
        spawn_blocks(layout_of(STAGES["Level3"]))
        tm = spawn(native("BreakoutTitleManager"), (0, 0, 0), label="TitleManager")
        tm.set_editor_property("next_level_name", "Level1")
        tm.set_editor_property("input_mapping_context", imc)
        tm.set_editor_property("action_action", actions["IA_Action"])

    build_level("TitleLevel", assets, title_extra, game_mode="TitleGameMode")
    log("done")


main()
