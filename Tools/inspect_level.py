import unreal
w = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/Level1")
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    unreal.log_warning("[gen] %s %s %s" % (a.get_class().get_name(), a.get_actor_label(), a.get_actor_location()))
