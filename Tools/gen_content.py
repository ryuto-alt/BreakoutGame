# エディタの Python で BP 子クラス・入力アセット・レベルを作り直すスクリプト。
# 実行: UnrealEditor-Cmd.exe BreakoutGame.uproject -run=pythonscript -script="Tools/gen_content.py"
import os
import unreal

STEP = int(os.environ.get("BREAKOUT_STEP", "1"))

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


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
    folder = "/Game/Input"
    try:
        recreate(folder + "/IA_Move")
        recreate(folder + "/IMC_InGame")
        ia_move = asset_tools.create_asset("IA_Move", folder, unreal.InputAction, unreal.InputAction_Factory())
        ia_move.set_editor_property("value_type", unreal.InputActionValueType.AXIS1D)
        actions = {"IA_Move": ia_move}
        if STEP >= 3:
            recreate(folder + "/IA_Action")
            ia_action = asset_tools.create_asset("IA_Action", folder, unreal.InputAction, unreal.InputAction_Factory())
            ia_action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
            actions["IA_Action"] = ia_action
        imc = asset_tools.create_asset("IMC_InGame", folder, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
        imc.map_key(ia_move, key("D"))
        imc.map_key(ia_move, key("A"))
        if "IA_Action" in actions:
            imc.map_key(actions["IA_Action"], key("SpaceBar"))
        # A キーに Negate を付ける
        data = imc.get_editor_property("default_key_mappings")
        mappings = list(data.get_editor_property("mappings"))
        for m in mappings:
            if str(m.get_editor_property("key").get_editor_property("key_name")) == "A":
                m.set_editor_property("modifiers", [unreal.new_object(unreal.InputModifierNegate, outer=imc)])
        data.set_editor_property("mappings", mappings)
        imc.set_editor_property("default_key_mappings", data)
        for a in list(actions.values()) + [imc]:
            eal.save_loaded_asset(a)
        log("input assets ok")
        return actions, imc
    except Exception as e:  # 失敗しても C++ 側で同じものを作るので続行
        import traceback
        log("input assets failed: %s" % traceback.format_exc().replace(chr(10), " | "))
        return None, None


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


def spawn_cube(label, loc, scale, folder="Walls", color=None):
    a = spawn(unreal.StaticMeshActor, loc, label=label, folder=folder)
    smc = a.static_mesh_component
    smc.set_static_mesh(unreal.load_asset(CUBE))
    a.set_actor_scale3d(unreal.Vector(*scale))
    smc.set_collision_profile_name("BlockAll")
    if color is not None:
        mat = unreal.load_asset("/Game/Materials/WallMaterial")
        if mat:
            smc.set_material(0, mat)
    return a


# ---------------------------------------------------------------- アセット
def make_block_material():
    """VectorParameter「BaseColor」を BaseColor につないだ BlockMaterial"""
    path = "/Game/Materials/BlockMaterial"
    recreate(path)
    mat = asset_tools.create_asset("BlockMaterial", "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew())
    mel = unreal.MaterialEditingLibrary
    p = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -400, 0)
    p.set_editor_property("parameter_name", "BaseColor")
    p.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mel.connect_material_property(p, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.recompile_material(mat)
    eal.save_loaded_asset(mat)
    log("created material " + path)
    return mat


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


# ブロック配置（y, z, hp）
LAYOUT_LEVEL1 = [(-750, 3000, 1), (0, 3000, 3), (750, 3000, 2)]
LAYOUT_LEVEL2 = [(y, 4500, hp) for y, hp in zip((-900, -300, 300, 900), (4, 2, 2, 4))] +                 [(y, 4200, hp) for y, hp in zip((-600, 0, 600), (3, 5, 3))]


def build_level(name, classes, setup_extra=None):
    path = "/Game/Maps/" + name
    recreate(path)
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    # 明るさ・空
    light = spawn(unreal.DirectionalLight, (0, 0, 3000), (-50, 40, 0), "DirectionalLight", "Lighting")
    light.light_component.set_editor_property("intensity", 8.0)
    spawn(unreal.SkyAtmosphere, (0, 0, 0), label="SkyAtmosphere", folder="Lighting")
    spawn(unreal.ExponentialHeightFog, (0, 0, -500), label="ExponentialHeightFog", folder="Lighting")
    sky = spawn(unreal.SkyLight, (0, 0, 2000), label="SkyLight", folder="Lighting")
    sky.light_component.set_editor_property("real_time_capture", True)
    sky.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

    # 外壁（内側は Y -1500..1500、Z 50..5050）
    spawn_cube("Wall_Left", (0, -1550, 2550), (1, 1, 52))
    spawn_cube("Wall_Right", (0, 1550, 2550), (1, 1, 52))
    spawn_cube("Wall_Top", (0, 0, 5100), (1, 32, 1))
    spawn_cube("Wall_Bottom", (0, 0, 0), (1, 32, 1))

    cam = spawn(unreal.CameraActor, (-4750, 0, 2550), (0, 0, 0), "CameraActor")
    cam.set_editor_property("auto_activate_for_player", unreal.AutoReceiveInput.PLAYER0)

    spawn(unreal.PlayerStart, (0, 0, 600), label="PlayerStart")

    if setup_extra:
        setup_extra(world)

    ws = unreal.EditorLevelLibrary.get_editor_world().get_world_settings()
    ws.set_editor_property("default_game_mode", classes["GameMode"])
    unreal.EditorLoadingAndSavingUtils.save_map(unreal.EditorLevelLibrary.get_editor_world(), path)
    log("saved level " + path)


def main():
    for lv in ("Level1", "Level2", "Level3", "TitleLevel"):
        recreate("/Game/Maps/" + lv)
    bp_folder = "/Game/Blueprints"
    paddle = make_bp("Paddle", bp_folder, native("BreakoutPaddle"))
    if STEP >= 4:
        make_bp("BreakoutGameInstance", bp_folder, native("BreakoutGameInstance"))
        compile_save(unreal.load_asset(bp_folder + "/BreakoutGameInstance"))
        block_mat = make_block_material()
        knock = import_sound("Breakout_SE_Knock", "Breakout_SE_Knock.wav")
        bgm = import_sound("Breakout_BGM", "Breakout_BGM.wav", looping=True)
    ball = make_bp("Ball", bp_folder, native("BreakoutBall"))
    block = gm = None
    if STEP >= 2:
        block = make_bp("Block", bp_folder, native("BreakoutBlock"))
        gm = make_bp("GameManager", bp_folder, native("BreakoutGameManager"))
    gamemode = make_bp("BreakoutGame", bp_folder, native("BreakoutGameModeBase"))

    actions, imc = make_input_assets()
    if imc:
        cdo(paddle).set_editor_property("input_mapping_context", imc)
        cdo(paddle).set_editor_property("move_action", actions["IA_Move"])
        if "IA_Action" in actions:
            cdo(paddle).set_editor_property("action_action", actions["IA_Action"])
    compile_save(paddle)
    compile_save(ball)
    if gm:
        # GameManager が生成するボールは BP の Ball
        cdo(gm).set_editor_property("ball_class", ball.generated_class())
    if STEP >= 4:
        cdo(block).set_editor_property("block_material", block_mat)
        cdo(ball).set_editor_property("knock_sound", knock)
        cdo(gm).set_editor_property("bgm_sound", bgm)
    for bp in (block, gm):
        if bp:
            compile_save(bp)

    cdo(gamemode).set_editor_property("default_pawn_class", paddle.generated_class())
    compile_save(gamemode)

    classes = {"Paddle": paddle.generated_class(), "Ball": ball.generated_class(), "GameMode": gamemode.generated_class()}
    if STEP >= 2:
        classes["Block"] = block.generated_class()
        classes["GameManager"] = gm.generated_class()

    def spawn_blocks(block_cls, layout):
        for i, (y, z, hp) in enumerate(layout):
            b = spawn(block_cls, (0, y, z), label="Block_%d" % i, folder="Blocks")
            b.set_editor_property("hp", hp)

    def level_extra(layout, next_level):
        return lambda world: level1_extra(world, layout, next_level)

    def level1_extra(world, layout=None, next_level=None):
        if STEP <= 2:
            spawn(classes["Ball"], (0, 0, 1000), label="Ball")
        if STEP >= 2:
            gm_actor = spawn(classes["GameManager"], (0, 0, 0), label="GameManager")
            if STEP >= 3:
                # ボールの発射位置（スライドの RespawnLocationActor）
                respawn = spawn(unreal.TargetPoint, (0, 0, 1000), label="RespawnLocationActor")
                gm_actor.set_editor_property("spawn_location_actor", respawn)
            spawn(native("MissArea"), (0, 0, 120), label="MissArea")
            if STEP >= 4:
                spawn_blocks(classes["Block"], layout)
                gm_actor.set_editor_property("next_level_name", next_level)
            else:
                # 2段 x 3個（自動プレイで30秒ほどでクリアできる配置）
                for row, z in enumerate((4500, 4200)):
                    for col, y in enumerate((-750, 0, 750)):
                        spawn(classes["Block"], (0, y, z), label="Block_%d_%d" % (row, col), folder="Blocks")

    if STEP >= 4:
        build_level("Level1", classes, level_extra(LAYOUT_LEVEL1, "Level2"))
        build_level("Level2", classes, level_extra(LAYOUT_LEVEL2, "Level1"))
    else:
        build_level("Level1", classes, level1_extra)
    log("done step %d" % STEP)


main()
