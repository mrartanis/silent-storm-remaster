bool UpstreamTests::RunLegacyTests(const char* mode, int argc, char** argv)
{
    if(!std::strcmp(mode,"legacy-directional-ambient")) {LegacyDirectionalAmbient();return true;}
    if(!std::strcmp(mode,"convenience-tooltip-ui")) {Require(argc==4,"tooltip requires db/res");ConvenienceTooltipUI(argv[2],argv[3]);return true;}
    if(!std::strcmp(mode,"convenience-mission-clock")) {Require(argc==4,"clock requires db/res");ConvenienceMissionClock(argv[2],argv[3]);return true;}
    if(!std::strcmp(mode,"objectives-row-compatibility")) {ObjectivesRowCompatibility();return true;}
    if(!std::strcmp(mode,"convenience-defaults-input")) {ConvenienceDefaultsAndInput();return true;}
    if(!std::strcmp(mode,"weapon-comparison-state")) {Require(argc==4,"comparison requires db/res");WeaponComparisonState(argv[2],argv[3]);return true;}
    if(!std::strcmp(mode,"convenience-settings-ui")) {Require(argc==4,"settings require db/res");ConvenienceSettingsUI(argv[2],argv[3]);return true;}
    if(!std::strcmp(mode,"default-melee-lifetime")) {Require(argc==3,"melee lifetime requires db");DefaultMeleeLifetime(argv[2]);return true;}
    if(!std::strcmp(mode,"objectives-value-roundtrip")) {Require(argc==3,"objectives require db");ObjectivesValueRoundTrip(argv[2]);return true;}
    if (!std::strcmp(mode,"native-knife-solver")) { Require(argc==5,"knife solver requires db/resources/mode"); NativeKnifeLaunchTarget(argv[2],argv[3],std::atoi(argv[4]),true); return true; }
    if (!std::strcmp(mode,"native-knife-flight-intent")) { Require(argc==5,"knife flight intent requires db/resources/mode"); NativeKnifeFlightIntent(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode,"native-knife-frame")) { Require(argc==5,"knife frame requires db/resources/mode"); NativeKnifeDoorFrame(argv[2],argv[3],std::atoi(argv[4])); return true; }

    if (!std::strcmp(mode, "heard-icon-anchor-height")) { Require(argc == 4, "heard icon requires db and res"); EnemyIconVisibility(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "owned-menu-sound-pump") && argc==4) { OwnedMenuSoundPump(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "missing-ui-sound-fallback")) { Require(argc == 4, "missing UI sound requires db and res"); MissingUISoundFallback(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "selected-attack-target-rejection")) { Require(argc == 4, "selected target requires db and res"); SelectedAttackTargetRejection(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "melee-current-pose-caption")) { Require(argc == 4, "melee caption requires db and res"); MeleeReachCaption(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "cancelled-inventory-hidden-item-pose")) { Require(argc == 4, "inventory cancellation requires db and res"); CancelledInventoryHiddenItemPose(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "initial-hidden-item-pose")) { Require(argc == 4, "initial item pose requires db and res"); CancelledInventoryHiddenItemPose(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "cancelled-equipment-hidden-item-pose")) { Require(argc == 4, "equipment cancellation requires db and res"); CancelledInventoryHiddenItemPose(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "spent-medkit-pose-cleanup")) { Require(argc == 4, "spent medkit requires db and res"); SpentMedkitPoseCleanup(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "spent-picklock-pose-cleanup")) { Require(argc == 4, "spent picklock requires db and res"); SpentPicklockPoseCleanup(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "object-shot-lifetime-pocket") && argc == 4) { NativeObjectShot(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "object-shot-lifetime-restored-pocket") && argc == 4) { NativeObjectShot(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "object-preparation-status") && argc == 4) { NativeObjectShot(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "object-preparation-holstered") && argc == 4) { NativeObjectShot(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "object-shot-identity") && argc == 4) { NativeObjectShot(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "object-shot-extra-ap") && argc == 4) { NativeObjectShot(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "object-shot-saved-target") && argc == 4) { NativeObjectShot(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "object-shot-point-control") && argc == 4) { NativeObjectShot(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "object-shot-missing-target") && argc == 4) { NativeObjectShot(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "object-shot-ignore-target") && argc == 4) { NativeObjectShot(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "object-shot-reload") && argc == 4) { NativeObjectShot(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "mine-late-reach-near") && argc == 4) { MineLateReach(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "mine-late-reach-boundary") && argc == 4) { MineLateReach(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "mine-late-reach-far") && argc == 4) { MineLateReach(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "mine-late-reach-missing-tool") && argc == 4) { MineLateReach(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "mine-late-reach-unknown") && argc == 4) { MineLateReach(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "mine-late-reach-missing-target") && argc == 4) { MineLateReach(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "mine-late-reach-ignore-target") && argc == 4) { MineLateReach(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "mine-late-reach-relocated") && argc == 4) { MineLateReach(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "mine-approach-status-off-grid") && argc == 4) { MineApproachStatus(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "mine-approach-status-reachable") && argc == 4) { MineApproachStatus(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "mine-approach-status-missing-target") && argc == 4) { MineApproachStatus(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "mine-approach-status-unsupported") && argc == 4) { MineApproachStatus(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "mine-placement-successor-handoff")) { Require(argc == 4, "mine handoff requires db and res"); MinePlacementSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "trap-placement-successor-handoff")) { Require(argc == 4, "trap handoff requires db and res"); TrapPlacementSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "engineer-trap-placement-successor-handoff")) { Require(argc == 4, "engineer trap handoff requires db and res"); TrapPlacementSuccessorHandoff(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "thrown-knife-successor-handoff")) { Require(argc == 4, "knife handoff requires db and res"); ThrownKnifeSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "thrown-knife-feedback")) { Require(argc == 4, "knife feedback requires db and res"); ThrownKnifeSuccessorHandoff(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "restored-grenade-throw")) { Require(argc == 4, "saved grenade requires db and res"); RestoredGrenadeThrow(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "restored-engineer-grenade-throw")) { Require(argc == 4, "saved engineer grenade requires db and res"); RestoredGrenadeThrow(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "grenade-successor-handoff")) { Require(argc == 4, "grenade handoff requires db and res"); GrenadeSuccessorHandoff(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "engineer-grenade-successor-handoff")) { Require(argc == 4, "engineer grenade handoff requires db and res"); GrenadeSuccessorHandoff(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "default-item-detachment")) { Require(argc == 4, "item detachment requires db and res"); DefaultItemDetachment(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "unit-hit-roll-equality-log")) { Require(argc == 4, "unit hit log requires db and res"); HitRollEqualityLog(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "tile-hit-roll-equality-log")) { Require(argc == 4, "tile hit log requires db and res"); HitRollEqualityLog(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "native-recruitment-idle")) { Require(argc==5,"recruitment idle requires db/resources/mode"); NativeRecruitmentIdle(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "dead-head-idle") && argc == 4) { DownedHeadIdle(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "unconscious-head-idle") && argc == 4) { DownedHeadIdle(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "active-door-hull-lookup") && argc == 4) { ActiveDoorHullLookup(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "projective-ray-unprojection")) { ProjectiveRayUnprojection(); return true; }
    if (!std::strcmp(mode, "shooting-torso-turn-limit")) { Require(argc == 4, "shooting torso limit requires db and res"); ShootingTorsoTurnLimit(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "grenade-throw-animation-refresh")) { Require(argc == 4, "grenade animation requires db and res"); GrenadeThrowAnimationRefresh(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "invalidated-grenade-trajectory")) { Require(argc == 4, "grenade trajectory requires db and res"); InvalidatedGrenadeTrajectory(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "grenade-completion-visibility")) { Require(argc == 4, "grenade visibility requires db and res"); GrenadeCompletionVisibility(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "unavailable-attack-shortcut")) { UnavailableAttackShortcut(); return true; }
    if (!std::strcmp(mode, "shared-action-shortcut-priority")) { SharedActionShortcutPriority(); return true; }
    if (!std::strcmp(mode, "hide-shortcut-eligibility")) { HideShortcutEligibility(); return true; }
    if (!std::strcmp(mode, "nonattack-tile-orders")) { Require(argc == 4, "non-attack tile orders require db and res"); EquippedToolActionAvailability(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "equipped-tool-action-availability")) { Require(argc == 4, "tool availability requires db and res"); EquippedToolActionAvailability(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "assigned-level-experience")) { Require(argc == 3, "assigned level experience requires db"); AssignedLevelExperience(argv[2]); return true; }
    if (!std::strcmp(mode, "action-skill-acknowledgment") && argc == 3) { ActionSkillAcknowledgment(argv[2]); return true; }
    if (!std::strcmp(mode, "interrupt-skill-acknowledgment") && argc == 3) { ActionSkillAcknowledgment(argv[2],true); return true; }
    if (!std::strcmp(mode, "snipe-skill-acknowledgment") && argc == 3) { SnipeSkillAcknowledgment(argv[2]); return true; }
    if (!std::strcmp(mode, "vitality-skill-acknowledgment") && argc == 3) { VitalitySkillAcknowledgment(argv[2]); return true; }
    if (!std::strcmp(mode, "persona-full-display-name")) { Require(argc == 3, "full display name requires db"); PersonaFullDisplayName(argv[2]); return true; }
    if (!std::strcmp(mode, "character-panel-full-name")) { Require(argc == 4, "character panel name requires db and res"); CharacterPanelFullName(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "temporary-groups")) { TemporaryGroups(); return true; }
    if (!std::strcmp(mode, "scroll-initialization")) { ScrollInitialization(); return true; }
    if (!std::strcmp(mode, "attack-location")) { AttackLocationRefresh(); return true; }
    if (!std::strcmp(mode, "bind-initialization")) { BindInitialization(); return true; }
    if (!std::strcmp(mode, "flying-restoration")) { FlyingRestoration(); return true; }
    if (!std::strcmp(mode, "sniping-aim")) { SnipingAim(); return true; }
    if (!std::strcmp(mode, "geometry-unused-vertices")) { GeometryUnusedVertices(false); return true; }
    if (!std::strcmp(mode, "geometry-unused-vertices-weighted")) { GeometryUnusedVertices(true); return true; }
    if (!std::strcmp(mode, "decal-rebuild")) { DecalRebuild(); return true; }
    if (!std::strcmp(mode, "lightmapped-decal-eligibility")) { LightmappedDecalEligibility(); return true; }
    if (!std::strcmp(mode, "decal-authored-offset")) { DecalSurfaceOffset(false); return true; }
    if (!std::strcmp(mode, "decal-impact-layer")) { DecalSurfaceOffset(true); return true; }
    if (!std::strcmp(mode, "memory-mesh-decal-projection")) { MemoryMeshDecalProjection(); return true; }
    if (!std::strcmp(mode, "decal-projection")) { DecalProjection(); return true; }
    if (!std::strcmp(mode, "empty-model-fallback") && argc==3) { EmptyModelFallback(argv[2]); return true; }
    if (!std::strcmp(mode, "partial-reference-import") && argc == 3) { PartialReferenceImport(argv[2]); return true; }
    if (!std::strcmp(mode, "permanent-clue-single-map")) { PermanentClueSingleMap(); return true; }
    if (!std::strcmp(mode, "deferred-click-world")) { DeferredClickWorld(); return true; }
    if (!std::strcmp(mode, "point-light-without-scene")) { PointLightWithoutScene(); return true; }
    if (!std::strcmp(mode, "dynamic-ambient-fallback")) { DynamicAmbientFallback(); return true; }
    if (!std::strcmp(mode, "corpse-backpack-drop") && argc == 4) { CorpseBackpackDrop(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "scripted-dialogue-priority") && argc == 4) { ScriptedDialoguePriority(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "point-light-save-layout")) { PointLightSaveLayout(); return true; }
    if (!std::strcmp(mode, "legacy-point-light-save-layout")) { PointLightSaveLayout(true); return true; }
    if (!std::strcmp(mode, "point-light-shadow-flag")) { PointLightShadowFlag(); return true; }
    if (!std::strcmp(mode, "global-point-light-shadow-flag")) { GlobalPointLightShadowFlag(); return true; }
    if (!std::strcmp(mode, "point-light-radiance")) { PointLightRadiance(); return true; }
    if (!std::strcmp(mode, "tiny-point-light-cache")) { TinyPointLightCache(); return true; }
    if (!std::strcmp(mode, "inactive-animated-light")) { AnimatedPointLightGuard(true); return true; }
    if (!std::strcmp(mode, "tiny-animated-light")) { AnimatedPointLightGuard(false); return true; }
    if (!std::strcmp(mode, "fresh-ambient-graph")) { FreshAmbientGraph(); return true; }
    if (!std::strcmp(mode, "compressed-save-writing")) { CompressedSaveWriting(); return true; }
    if (!std::strcmp(mode, "save-success-feedback") && argc == 3) { SaveSuccessFeedback(argv[2]); return true; }
    if (!std::strcmp(mode, "encoded-save-command") && argc == 3) { SaveSuccessFeedback(argv[2],true); return true; }
    if (!std::strcmp(mode, "load-success-feedback") && argc == 4) { LoadSuccessFeedback(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "encoded-load-command") && argc == 4) { LoadSuccessFeedback(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "zone-snapshot-excludes-hud") && argc == 4) { ZoneSnapshotExcludesHUD(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "window-interface-propagation")) { WindowInterfacePropagation(); return true; }
    if (!std::strcmp(mode, "vision-destination")) { Require(argc==5,"vision destination requires db/resources/mode"); VisionDestinationVoxel(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "point-vision-cache")) { Require(argc==3 || argc==5,"point vision cache requires mode and optional db/resources"); PointVisionCaching(std::atoi(argv[2]),argc==5?argv[3]:nullptr,argc==5?argv[4]:nullptr); return true; }
    if (!std::strcmp(mode, "vision-voxel-query")) { Require(argc==3 || argc==5,"vision voxel query requires mode and optional db/resources"); VisionVoxelQuery(std::atoi(argv[2]),argc==5?argv[3]:nullptr,argc==5?argv[4]:nullptr); return true; }
    if (!std::strcmp(mode, "script-misc-weak-cleanup")) { Require(argc==3,"script miscellaneous cleanup requires mode"); ScriptMiscWeakCleanup(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "window-activation-flags")) { Require(argc==3,"window activation requires mode"); WindowActivationFlags(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "window-child-registration")) { WindowChildRegistration(); return true; }
    if (!std::strcmp(mode, "lua-implicit-end")) { Require(argc==3,"implicit end requires mode"); LuaImplicitEndYield(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "lua-argument-state")) { LuaArgumentState(); return true; }
    if (!std::strcmp(mode, "worldless-ui-script")) { WorldlessUIScript(); return true; }
    if (!std::strcmp(mode, "lua-window-userdata")) { LuaWindowUserdata(); return true; }
    if (!std::strcmp(mode, "lua-window-setter-stack")) { LuaWindowSetterStack(); return true; }
    if (!std::strcmp(mode, "lua-window-state-context")) { LuaWindowStateContext(); return true; }
    if (!std::strcmp(mode, "window-message-callbacks")) { WindowMessageCallbacks(); return true; }
    if (!std::strcmp(mode, "window-callback-lifetime")) { WindowCallbackLifetime(); return true; }
    if (!std::strcmp(mode, "window-lazy-update")) { WindowLazyUpdate(); return true; }
    if (!std::strcmp(mode, "combo-popup-clipping")) { ComboPopupClipping(); return true; }
    if (!std::strcmp(mode, "combo-popup-item-width")) { ComboPopupClipping(true); return true; }
    if (!std::strcmp(mode, "chapter-clue-heading") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "medal-cursor-visible") && argc == 4) { MedalCursorVisible(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-medal-screenshot") && argc == 4) { ChapterMedalScreenshot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "medal-panel-caption") && argc == 4) { MedalPanelCaption(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "medal-preview-camera") && argc == 4) { MedalPanelCaption(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "window-nested-wakeup")) { WindowNestedWakeup(); return true; }
    if (!std::strcmp(mode, "window-template-script")) { Require(argc == 3, "window template script requires db"); WindowTemplateScript(argv[2]); return true; }
    if (!std::strcmp(mode, "console-visibility-toggle")) { ConsoleVisibilityToggle(); return true; }
    if (!std::strcmp(mode, "ai-reaction-reset-contacts")) { AIReactionResetContacts(); return true; }
    if (!std::strcmp(mode, "ai-reaction-value-latch") && argc == 4) { AIReactionValueLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "script-ai-activation") && argc == 4) { ScriptAIActivation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-event-collection-locks") && argc == 4) { AIEventCollectionLocks(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-immediate-event-dispatch") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-reaction-contact-edits") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-world-sighting-dispatch") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-immediate-event-guards") && argc == 4) { AIContactEventDispatch(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-contact-cache-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 0); return true; }
    if (!std::strcmp(mode, "ai-contact-cache-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 0); return true; }
    if (!std::strcmp(mode, "ai-contact-cache-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 0); return true; }
    if (!std::strcmp(mode, "ai-contact-refresh-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 1); return true; }
    if (!std::strcmp(mode, "ai-contact-refresh-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 1); return true; }
    if (!std::strcmp(mode, "ai-contact-refresh-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 1); return true; }
    if (!std::strcmp(mode, "ai-contact-remove-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 2); return true; }
    if (!std::strcmp(mode, "ai-contact-remove-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 2); return true; }
    if (!std::strcmp(mode, "ai-contact-remove-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 2); return true; }
    if (!std::strcmp(mode, "ai-contact-eligibility-enemy") && argc == 4) { AIContactMembership(argv[2], argv[3], 0, 3); return true; }
    if (!std::strcmp(mode, "ai-contact-eligibility-possible") && argc == 4) { AIContactMembership(argv[2], argv[3], 1, 3); return true; }
    if (!std::strcmp(mode, "ai-contact-eligibility-ally") && argc == 4) { AIContactMembership(argv[2], argv[3], 2, 3); return true; }
    if (!std::strcmp(mode, "ai-snapshot-active") && argc == 4) { AICombatSnapshot(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-snapshot-unconscious") && argc == 4) { AICombatSnapshot(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-snapshot-dead") && argc == 4) { AICombatSnapshot(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-snapshot-counters") && argc == 4) { AICombatSnapshot(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-snapshot-contacts") && argc == 4) { AICombatSnapshot(argv[2], argv[3], 4); return true; }
    if (!std::strcmp(mode, "ai-snapshot-human") && argc == 4) { AICombatSnapshot(argv[2], argv[3], 5); return true; }
    if (!std::strcmp(mode, "ai-reaction-guard-suspect") && argc == 4) { AIReactionContactDelivery(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-reaction-guard-ally") && argc == 4) { AIReactionContactDelivery(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-reaction-retreat-suspect") && argc == 4) { AIReactionContactDelivery(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-reaction-fear-suspect") && argc == 4) { AIReactionContactDelivery(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-reaction-normal-ally") && argc == 4) { AIReactionContactDelivery(argv[2], argv[3], 4); return true; }
    if (!std::strcmp(mode, "ai-reaction-normal-alarm") && argc == 4) { AIReactionContactDelivery(argv[2], argv[3], 5); return true; }
    if (!std::strcmp(mode, "ai-fear-route-calm") && argc == 4) { AIFearRouteRetention(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-fear-route-suspect") && argc == 4) { AIFearRouteRetention(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-fear-route-enemy") && argc == 4) { AIFearRouteRetention(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-fear-route-empty") && argc == 4) { AIFearRouteRetention(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-pass-control-memo-own") && argc == 4) { AIPassControlEndMemo(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-pass-control-memo-foreign") && argc == 4) { AIPassControlEndMemo(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-pass-control-memo-unbound") && argc == 4) { AIPassControlEndMemo(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-pass-control-memo-restoring") && argc == 4) { AIPassControlEndMemo(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-pass-control-ai") && argc == 4) { AIPassControlSnapshots(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "player-tbs-dispatch-start") && argc == 4) { PlayerTBSTurnOwnership(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "player-tbs-dispatch-finish") && argc == 4) { PlayerTBSTurnOwnership(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "player-tbs-dispatch-realtime") && argc == 4) { PlayerTBSTurnOwnership(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "player-tbs-dispatch-cancel") && argc == 4) { PlayerTBSTurnOwnership(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "player-tbs-dispatch-null-front") && argc == 4) { PlayerTBSTurnOwnership(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "player-tbs-dispatch-null-middle") && argc == 4) { PlayerTBSTurnOwnership(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-pass-control-human") && argc == 4) { AIPassControlSnapshots(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-pass-control-foreign") && argc == 4) { AIPassControlSnapshots(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-pass-control-repeat") && argc == 4) { AIPassControlSnapshots(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-own-empty") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],0,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-cached") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],1,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-neutral") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],2,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-human") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],3,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-ko") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],4,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-hostile") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],5,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-no-saved-owner") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],7,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-no-world") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],8,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-no-commander") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],9,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-reowned") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],11,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-transition") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],12,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-support-roll-decline") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],13,true); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-own-empty") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-cached") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-neutral") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-human") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-ko") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-hostile") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-none") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-no-saved-owner") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-no-world") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-no-commander") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-tie") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-reowned") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "ai-alarm-target-transition") && argc == 4) { AIAlarmNearestTarget(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-own-missing") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-cached") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-neutral") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-allied") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-hostile") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-far") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-ko") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-human") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-transition") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-no-saved-owner") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-other-saved-owner") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-no-commander") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-no-world") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "ai-alarm-world-reowned") && argc == 4) { AIAlarmWorldQuery(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-live") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-caller-plan-hp") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-enemy-plan-hp") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-recipient-plan-hp") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-caller-ko") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 4); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-enemy-ko") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 5); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-recipient-ko") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 6); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-human-recipient") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 7); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-human-caller") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 8); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-locked") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 9); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-dirty") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 10); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-far") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 11); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-no-enemy") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 12); return true; }
    if (!std::strcmp(mode, "ai-alarm-delivery-no-caller") && argc == 4) { AIAlarmDelivery(argv[2], argv[3], 13); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-stand") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-crouch") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-prone") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-equal") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-below") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 4); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-zero") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 5); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-ap-cheat") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 6); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-sequence-cheat") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 7); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-calm") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 8); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-enemy") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 9); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-perk") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 10); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-perk-zero") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 11); return true; }
    if (!std::strcmp(mode, "ai-fear-ap-run") && argc == 4) { AIFearAPReservation(argv[2], argv[3], 12); return true; }
    if (!std::strcmp(mode, "mission-overlay-readiness")) { MissionOverlayReadiness(); return true; }
    if (!std::strcmp(mode, "ai-player-mission-readiness")) { AIPlayerMissionReadiness(); return true; }
    if (!std::strcmp(mode, "sound-icon-visibility")) { OverlayIconVisibility(false); return true; }
    if (!std::strcmp(mode, "trap-icon-visibility")) { OverlayIconVisibility(true); return true; }
    if (!std::strcmp(mode, "enemy-icon-visibility")) { Require(argc == 4, "enemy icons require db and res"); EnemyIconVisibility(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "finished-music-silence")) { Require(argc == 3, "finished music requires db"); FinishedMusicSilence(argv[2]); return true; }
    if (!std::strcmp(mode, "holstered-heavy-weapon-mesh")) { Require(argc == 4, "heavy mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-active-item-mesh")) { Require(argc == 4, "hidden active mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "explicit-item-activation-visibility") && argc == 4) { ExplicitItemActivationVisibility(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "movement-item-activation-visibility") && argc == 4) { ExplicitItemActivationVisibility(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "hidden-carried-heavy-mesh")) { Require(argc == 4, "hidden carried mesh requires db and res"); HolsteredHeavyWeaponMesh(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "script-holster-persistence")) { Require(argc == 4, "script holster requires db and res"); ScriptHolsterPersistence(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-holster-policy")) { Require(argc == 4, "movement holster requires db and res"); MovementHolsterPolicy(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "short-inactive-movement")) { Require(argc == 4, "inactive movement requires db and res"); MovementHolsterPolicy(argv[2], argv[3], MovementHolsterCheck::ShortInactive); return true; }
    if (!std::strcmp(mode, "hidden-active-movement")) { Require(argc == 4, "hidden active movement requires db and res"); MovementHolsterPolicy(argv[2], argv[3], MovementHolsterCheck::HiddenActive); return true; }
    if (!std::strcmp(mode, "equipment-holster-reset")) { Require(argc == 4, "equipment activation requires db and res"); ExplicitWeaponActivation(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "movement-holster-reset")) { Require(argc == 4, "movement activation requires db and res"); ExplicitWeaponActivation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "movement-first-step-wait")) { Require(argc == 4, "movement wait requires db and res"); MovementFirstStepWaitTimer(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-collision-hidden-other")) { Require(argc == 4, "collision reveal requires db and res"); MovementCollisionReveal(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "movement-collision-hidden-own")) { Require(argc == 4, "collision reveal requires db and res"); MovementCollisionReveal(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "movement-collision-visible-other")) { Require(argc == 4, "collision reveal requires db and res"); MovementCollisionReveal(argv[2],argv[3],false,false); return true; }
    if (!std::strcmp(mode, "bullet-decal-scene")) { Require(argc==4,"bullet decal scene requires db and res"); BulletDecalEquipmentTarget(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "bullet-impact-positive") && argc==4) { BulletImpactEffects(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "bullet-impact-stopped") && argc==4) { BulletImpactEffects(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "bullet-impact-rejected") && argc==4) { BulletImpactEffects(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "bullet-impact-visual-exit") && argc==4) { BulletImpactEffects(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "bullet-impact-other-receiver") && argc==4) { BulletImpactEffects(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "bullet-impact-blood-off") && argc==4) { BulletImpactEffects(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "bullet-impact-nonblood-off") && argc==4) { BulletImpactEffects(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "bullet-impact-surface-exit") && argc==4) { BulletImpactEffects(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "bullet-impact-floor") && argc==4) { BulletImpactEffects(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "bullet-impact-disabled") && argc==4) { BulletImpactEffects(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "bullet-impact-unit-disabled") && argc==4) { BulletImpactEffects(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "bullet-impact-null-armor") && argc==4) { BulletImpactEffects(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "bullet-impact-dodge-order") && argc==4) { BulletImpactEffects(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "bullet-decal-human")) { Require(argc==4,"bullet decal requires db and res"); BulletDecalEquipmentTarget(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "bullet-decal-panzerklein")) { Require(argc==4,"bullet decal requires db and res"); BulletDecalEquipmentTarget(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "movement-rotation-create-wire")) { Require(argc==4,"rotation queue requires db and res"); MovementRotationQueueWire(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "movement-rotation-cancel-wire")) { Require(argc==4,"rotation queue requires db and res"); MovementRotationQueueWire(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "movement-rotation-reroute-wire")) { Require(argc==4,"rotation queue requires db and res"); MovementRotationQueueWire(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "movement-rotate-continuation")) { Require(argc==4,"rotate continuation requires db and res"); MovementRerouteContinuation(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "movement-strafe-continuation")) { Require(argc==4,"strafe continuation requires db and res"); MovementRerouteContinuation(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "movement-waiting-reroute")) { Require(argc==4,"waiting reroute requires db and res"); MovementBlockedWalkingTrajectory(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "movement-reroute-final-step")) { Require(argc==4,"reroute final step requires db and res"); MovementRerouteFinalStep(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "movement-end-rotate-wire")) { Require(argc==4,"end rotate requires db and res"); MovementEndRotateWire(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "movement-legacy-end-rotate-wire")) { Require(argc==4,"legacy end rotate requires db and res"); MovementEndRotateWire(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "movement-blocked-walking")) { Require(argc == 4, "blocked movement requires db and res"); MovementBlockedWalkingTrajectory(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "movement-terminal-finished")) { Require(argc == 4, "terminal movement requires db and res"); MovementTerminalCompletion(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "movement-terminal-failed")) { Require(argc == 4, "terminal movement requires db and res"); MovementTerminalCompletion(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "movement-action-camera")) { Require(argc == 4, "movement camera requires db and res"); MovementActionCamera(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "cannon-personal-pose-reload") && argc==4) { CannonPersonalPose(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "cannon-personal-pose-no-reload") && argc==4) { CannonPersonalPose(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "cannon-personal-pose-unconscious") && argc==4) { CannonPersonalPose(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "cannon-personal-pose-dead") && argc==4) { CannonPersonalPose(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "cannon-personal-pose-cancel") && argc==4) { CannonPersonalPose(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "cannon-personal-pose-restored") && argc==4) { CannonPersonalPose(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "cannon-dismount-live") && argc==4) { CannonDismountClip(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "cannon-dismount-unconscious") && argc==4) { CannonDismountClip(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "cannon-dismount-dead") && argc==4) { CannonDismountClip(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "cannon-dismount-restored") && argc==4) { CannonDismountClip(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ladder-door-path-entry") && argc==4) { LadderDoorPath(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ladder-door-path-aligned") && argc==4) { LadderDoorPath(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ladder-door-path-move") && argc==4) { LadderDoorPath(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ladder-door-path-turn") && argc==4) { LadderDoorPath(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "animation-reseat-write-legacy") && argc==5) { WriteLegacyAnimationWalk(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "animation-reseat-ladder-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "animation-reseat-ladder-stop") && argc==4) { AnimationReseat(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "animation-reseat-restored-ladder-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "animation-reseat-restored-ladder-stop") && argc==4) { AnimationReseat(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "animation-reseat-walk-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "animation-reseat-walk-stop") && argc==4) { AnimationReseat(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "animation-reseat-turn-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "animation-reseat-turn-stop") && argc==4) { AnimationReseat(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "animation-reseat-end-turn-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "animation-reseat-end-turn-stop") && argc==4) { AnimationReseat(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "animation-reseat-aim-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "animation-reseat-aim-stop") && argc==4) { AnimationReseat(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "animation-reseat-cannon-recalc") && argc==4) { AnimationReseat(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "animation-reseat-cannon-stop") && argc==4) { AnimationReseat(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "animation-reseat-legacy-walk-recalc") && argc==5) { AnimationReseat(argv[2],argv[3],14,argv[4]); return true; }
    if (!std::strcmp(mode, "animation-reseat-legacy-walk-stop") && argc==5) { AnimationReseat(argv[2],argv[3],15,argv[4]); return true; }
    if (!std::strcmp(mode, "cannon-exit-alive")) { Require(argc==4, "cannon exit requires db and res"); CannonExitIncapacitated(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "cannon-exit-unconscious")) { Require(argc==4, "cannon exit requires db and res"); CannonExitIncapacitated(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "cannon-exit-dead")) { Require(argc==4, "cannon exit requires db and res"); CannonExitIncapacitated(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "cannon-entry-legacy-write") && argc==5) { WriteLegacyCannonEntryFixture(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "cannon-entry-legacy") && argc==5) { LegacyCannonEntryCompletion(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "cannon-entry-completion")) { Require(argc==4, "cannon completion requires db and res"); CannonEntryCompletion(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-look") && argc == 4) { AIMountedRoute(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-path") && argc == 4) { AIMountedRoute(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-wish") && argc == 4) { AIMountedRoute(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-strafe") && argc == 4) { AIMountedRoute(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-hide") && argc == 4) { AIMountedRoute(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-unmounted") && argc == 4) { AIMountedRoute(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-shoot-mode") && argc == 4) { AIMountedRoute(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-wait") && argc == 4) { AIMountedRoute(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-mounted-route-restored") && argc == 4) { AIMountedRoute(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ai-cannon-undock-plan-live") && argc == 4) { AICannonUndockPlan(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cannon-undock-plan-null-log") && argc == 4) { AICannonUndockPlan(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cannon-undock-plan-no-cannon") && argc == 4) { AICannonUndockPlan(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cannon-undock-plan-null-server") && argc == 4) { AICannonUndockPlan(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cannon-undock-plan-invalid-server") && argc == 4) { AICannonUndockPlan(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "cannon-entry-animation")) { Require(argc == 4, "cannon animation requires db and res"); CannonEntryAnimation(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mounted-reload-weapon")) { Require(argc == 4, "mounted reload requires db and res"); MountedReloadWeapon(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "queued-aim-0") && argc==4) { QueuedAim(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "queued-aim-1") && argc==4) { QueuedAim(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "queued-aim-2") && argc==4) { QueuedAim(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "queued-aim-3") && argc==4) { QueuedAim(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "queued-aim-4") && argc==4) { QueuedAim(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "queued-aim-5") && argc==4) { QueuedAim(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "queued-aim-6") && argc==4) { QueuedAim(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "queued-aim-7") && argc==4) { QueuedAim(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "queued-aim-8") && argc==4) { QueuedAim(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "queued-aim-9") && argc==4) { QueuedAim(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "queued-aim-10") && argc==4) { QueuedAim(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "queued-aim-11") && argc==4) { QueuedAim(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "queued-aim-12") && argc==4) { QueuedAim(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "queued-aim-13") && argc==4) { QueuedAim(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "queued-aim-14") && argc==4) { QueuedAim(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "queued-aim-15") && argc==4) { QueuedAim(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "queued-aim-16") && argc==4) { QueuedAim(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "queued-aim-17") && argc==4) { QueuedAim(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "queued-aim-18") && argc==4) { QueuedAim(argv[2],argv[3],18); return true; }
    if (!std::strcmp(mode, "queued-aim-19") && argc==4) { QueuedAim(argv[2],argv[3],19); return true; }
    if (!std::strcmp(mode, "burst-preview-0") && argc==4) { BurstPreview(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "burst-preview-1") && argc==4) { BurstPreview(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "burst-preview-2") && argc==4) { BurstPreview(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "burst-preview-3") && argc==4) { BurstPreview(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "burst-preview-4") && argc==4) { BurstPreview(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "burst-preview-5") && argc==4) { BurstPreview(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "burst-preview-6") && argc==4) { BurstPreview(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "burst-preview-7") && argc==4) { BurstPreview(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "burst-preview-8") && argc==4) { BurstPreview(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "burst-preview-9") && argc==4) { BurstPreview(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "burst-preview-10") && argc==4) { BurstPreview(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "burst-preview-11") && argc==4) { BurstPreview(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "burst-preview-12") && argc==4) { BurstPreview(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "burst-preview-13") && argc==4) { BurstPreview(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "burst-preview-14") && argc==4) { BurstPreview(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "burst-preview-15") && argc==4) { BurstPreview(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "burst-preview-16") && argc==4) { BurstPreview(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "burst-preview-17") && argc==4) { BurstPreview(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "preparation-ap-0") && argc==4) { WeaponPreparation(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "preparation-ap-1") && argc==4) { WeaponPreparation(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "preparation-ap-2") && argc==4) { WeaponPreparation(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "preparation-ap-3") && argc==4) { WeaponPreparation(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "preparation-ap-4") && argc==4) { WeaponPreparation(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "preparation-ap-5") && argc==4) { WeaponPreparation(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "preparation-ap-6") && argc==4) { WeaponPreparation(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "preparation-ap-7") && argc==4) { WeaponPreparation(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "preparation-ap-8") && argc==4) { WeaponPreparation(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "preparation-ap-9") && argc==4) { WeaponPreparation(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "preparation-ap-10") && argc==4) { WeaponPreparation(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "preparation-ap-11") && argc==4) { WeaponPreparation(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "reserved-ap-0") && argc==4) { ReservedAP(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "reserved-ap-1") && argc==4) { ReservedAP(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "reserved-ap-2") && argc==4) { ReservedAP(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "reserved-ap-3") && argc==4) { ReservedAP(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "reserved-ap-4") && argc==4) { ReservedAP(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "reserved-ap-5") && argc==4) { ReservedAP(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "reserved-ap-6") && argc==4) { ReservedAP(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "reserved-ap-7") && argc==4) { ReservedAP(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "reserved-ap-8") && argc==4) { ReservedAP(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "reserved-ap-9") && argc==4) { ReservedAP(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "reserved-ap-10") && argc==4) { ReservedAP(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "reserved-ap-11") && argc==4) { ReservedAP(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "reserved-ap-12") && argc==4) { ReservedAP(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "reserved-ap-13") && argc==4) { ReservedAP(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "reserved-ap-14") && argc==4) { ReservedAP(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "reserved-ap-15") && argc==4) { ReservedAP(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "reserved-ap-16") && argc==4) { ReservedAP(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "reserved-ap-17") && argc==4) { ReservedAP(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "approach-retry-0") && argc==4) { ApproachRetry(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "approach-retry-1") && argc==4) { ApproachRetry(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "approach-retry-2") && argc==4) { ApproachRetry(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "approach-retry-3") && argc==4) { ApproachRetry(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "approach-retry-4") && argc==4) { ApproachRetry(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "approach-retry-5") && argc==4) { ApproachRetry(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "approach-retry-6") && argc==4) { ApproachRetry(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "approach-retry-7") && argc==4) { ApproachRetry(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "pickup-radii-0") && argc==4) { PickupRadii(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "pickup-radii-1") && argc==4) { PickupRadii(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "pickup-radii-2") && argc==4) { PickupRadii(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "pickup-radii-3") && argc==4) { PickupRadii(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "pickup-radii-4") && argc==4) { PickupRadii(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "pickup-radii-5") && argc==4) { PickupRadii(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "pickup-radii-6") && argc==4) { PickupRadii(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "pickup-radii-7") && argc==4) { PickupRadii(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "pickup-radii-8") && argc==4) { PickupRadii(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "pickup-radii-9") && argc==4) { PickupRadii(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "pickup-radii-10") && argc==4) { PickupRadii(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "pickup-radii-11") && argc==4) { PickupRadii(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "standing-talk-0") && argc==4) { StandingTalk(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "standing-talk-1") && argc==4) { StandingTalk(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "standing-talk-2") && argc==4) { StandingTalk(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "standing-talk-3") && argc==4) { StandingTalk(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "standing-talk-4") && argc==4) { StandingTalk(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "standing-talk-5") && argc==4) { StandingTalk(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "standing-talk-6") && argc==4) { StandingTalk(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "standing-talk-7") && argc==4) { StandingTalk(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "standing-talk-8") && argc==4) { StandingTalk(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "standing-talk-9") && argc==4) { StandingTalk(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "nearby-pickup-0") && argc==4) { NearbyPickup(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "nearby-pickup-1") && argc==4) { NearbyPickup(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "nearby-pickup-2") && argc==4) { NearbyPickup(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "nearby-pickup-3") && argc==4) { NearbyPickup(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "nearby-pickup-4") && argc==4) { NearbyPickup(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "nearby-pickup-5") && argc==4) { NearbyPickup(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "nearby-pickup-6") && argc==4) { NearbyPickup(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "nearby-pickup-7") && argc==4) { NearbyPickup(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "nearby-pickup-8") && argc==4) { NearbyPickup(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "nearby-pickup-9") && argc==4) { NearbyPickup(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "nearby-pickup-10") && argc==4) { NearbyPickup(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "nearby-pickup-11") && argc==4) { NearbyPickup(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "nearby-pickup-12") && argc==4) { NearbyPickup(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "ready-action-0") && argc==4) { ReadyAction(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ready-action-1") && argc==4) { ReadyAction(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ready-action-2") && argc==4) { ReadyAction(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ready-action-3") && argc==4) { ReadyAction(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ready-action-4") && argc==4) { ReadyAction(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ready-action-5") && argc==4) { ReadyAction(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ready-action-6") && argc==4) { ReadyAction(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ready-action-7") && argc==4) { ReadyAction(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ready-action-8") && argc==4) { ReadyAction(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ready-action-9") && argc==4) { ReadyAction(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "ready-action-10") && argc==4) { ReadyAction(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "holstered-reload-preparation")) { Require(argc == 4, "reload preparation requires db and res"); HolsteredReloadPreparation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "empty-clip-attack-entry") && argc == 4) { EmptyClipAttackEntry(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "invalid-action-cache-query") && argc == 4) { EmptyClipAttackEntry(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "object-trap-placement-cursor") && argc == 3) { PlacementCursor(argv[2]); return true; }
    if (!std::strcmp(mode, "mine-placement-cursor") && argc == 3) { PlacementCursor(argv[2],true); return true; }
    if (!std::strcmp(mode, "reload-action-wire")) { Require(argc == 4, "reload wire requires db and res"); ReloadActionWire(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "pain-voice-owner")) { Require(argc == 4, "pain voice requires db and res"); PainVoiceOwner(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "unconscious-pain-voice")) { Require(argc == 4, "pain eligibility requires db and res"); PainVoiceOwner(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "world-ui-refresh-latch")) { Require(argc == 4, "world UI refresh requires db and res"); WorldUIRefreshLatch(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "follow-camera-command-policy")) { Require(argc==3,"follow-camera command policy requires mode"); FollowCameraCommandPolicy(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "dialogue-camera-filter")) { CinematicCameraCommands(0); return true; }
    if (!std::strcmp(mode, "movie-camera-filter")) { CinematicCameraCommands(1); return true; }
    if (!std::strcmp(mode, "fade-camera-filter")) { CinematicCameraCommands(2); return true; }
    if (!std::strcmp(mode, "completed-zone-recommendation")) { CompletedZoneRecommendation(); return true; }
    if (!std::strcmp(mode, "perk-tooltip-parameters")) { PerkTooltipParameters(); return true; }
    if (!std::strcmp(mode, "ambient-variant-empty")) { Require(argc == 3, "ambient variant requires mode"); AmbientVariantEmpty(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "mechanical-step-sound")) { MechanicalStepSound(); return true; }
    if (!std::strcmp(mode, "lua-empty-default")) { EmptyLuaDefault(); return true; }
    if (!std::strcmp(mode, "missing-lua-global-warning")) { MissingLuaGlobalWarning(); return true; }
    if (!std::strcmp(mode, "smart-aimer-boundary")) { SmartAimerBoundary(); return true; }
    if (!std::strcmp(mode, "restored-portrait-selection")) { RestoredPortraitPanel(false); return true; }
    if (!std::strcmp(mode, "restored-empty-portraits")) { RestoredPortraitPanel(true); return true; }
    if (!std::strcmp(mode, "edit-escape-cancel")) { EditKeyboardRouting(true); return true; }
    if (!std::strcmp(mode, "edit-keyboard-edges")) { EditKeyboardRouting(false); return true; }
    if (!std::strcmp(mode, "native-cursor-position")) { NativeCursorPositioning(); return true; }
    if (!std::strcmp(mode, "disabled-cursor-input")) { DisabledCursorInput(); return true; }
    if (!std::strcmp(mode, "exit-menu-save-visibility") && argc == 4) { ExitMenuSavePermission(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "exit-menu-save-shortcut") && argc == 4) { ExitMenuSavePermission(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "main-menu-load-save-tab") && argc == 4) { MainMenuLoadSaveTab(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-legacy-fixture-write") && argc == 5) { WriteLegacyChapterFixture(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "script-sound-start-2d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],false,false,500); return true; }
    if (!std::strcmp(mode, "script-sound-start-3d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],true,false,500); return true; }
    if (!std::strcmp(mode, "script-sound-unassigned-2d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "script-sound-unassigned-3d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "script-sound-stop-2d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "script-sound-stop-3d") && argc == 4) { ScriptSoundStop(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "backdrop-script-audio-2d") && argc == 4) { BackdropScriptAudio(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "main-menu-map-script") && argc == 4) { MainMenuMapScript(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "main-menu-authored-script") && argc == 4) { MainMenuMapScript(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "main-menu-script-cadence") && argc == 4) { MainMenuMapScript(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "mission-loading-input") && argc == 4) { MissionLoadingInput(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-save-policy") && argc==4) { MissionSavePolicy(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-network-save-policy") && argc==4) { MissionSavePolicy(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "side-menu-tooltips")) { Require(argc==5,"side-menu tooltips require db/resources/mode"); SideMenuTooltips(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "side-menu-selected-style") && argc==4) { SideMenuSelectedStyle(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "side-menu-difficulty-mapping") && argc==4) { SideMenuDifficultyMapping(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "options-pen-cursor") && argc==4) { OptionsPenCursor(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "ai-unit-flash") && argc==4) { AIUnitFlash(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "body-hover-labels") && argc==4) { BodyHoverLabels(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "forced-state-cursor-caption") && argc==4) { ForcedAttackTooltip(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "forced-attack-tooltip") && argc==4) { ForcedAttackTooltip(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "hotseat-keyboard") && argc==4) { HotseatDialogTemplate(argv[2],argv[3],false,true,true); return true; }
    if (!std::strcmp(mode, "hotseat-keyboard-legacy") && argc==4) { HotseatDialogTemplate(argv[2],argv[3],true,true,true); return true; }
    if (!std::strcmp(mode, "hotseat-handoff") && argc==4) { HotseatDialogTemplate(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "hotseat-handoff-legacy") && argc==4) { HotseatDialogTemplate(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "hotseat-dialog-legacy") && argc==4) { HotseatDialogTemplate(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "hotseat-dialog-template") && argc==4) { HotseatDialogTemplate(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-restart-menu") && argc==4) { MissionRestartMenu(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-network-restart-menu") && argc==4) { MissionRestartMenu(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "mission-save-menu") && argc == 4) { MissionSaveMenu(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-quick-save") && argc == 4) { MissionSaveMenu(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "stored-aura-evasion")) { Require(argc==5,"stored aura requires db/resources/mode"); StoredAuraEvasion(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-attacker-ignore")) { Require(argc==5,"melee ignore requires db/resources/mode"); NativeMeleeAttackerIgnore(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-object-melee-route")) { Require(argc==5,"object melee route requires db/resources/mode"); NativeObjectMeleeRoute(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-object-melee-save")) { Require(argc==5,"object melee requires db/resources/mode"); const int selected=std::atoi(argv[4]); if(selected<6)NativeObjectMeleeSave(argv[2],argv[3],selected);else {Require(selected==6 || selected==7,"unknown object melee filter mode");NativeMeleeTargetFilter(argv[2],argv[3],selected==6?0:3,true);} return true; }
    if (!std::strcmp(mode, "native-melee-target-filter")) { Require(argc==5,"melee filter requires db/resources/mode"); NativeMeleeTargetFilter(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-world-context")) { Require(argc==5,"melee world requires db/resources/mode"); NativeMeleeWorldContext(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-plain-ray-route")) { Require(argc==5,"plain ray requires db/resources/mode"); NativePlainRayRoute(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-miss-gate")) { Require(argc==5,"melee miss requires db/resources/mode"); NativeMeleeMissGate(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-fragment-speed")) { Require(argc==5,"fragment speed requires db/resources/mode"); NativeFragmentSpeed(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "scenario-attack-knockout")) { Require(argc==5,"scenario knockout requires db/resources/mode"); ScenarioAttackKnockout(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "scenario-carrier-destroyed")) { Require(argc==5,"carrier notification requires db/resources/mode"); ScenarioCarrierDestroyed(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "terrain-grass-floor")) { Require(argc==5,"terrain grass requires db/resources/mode"); TerrainGrassFloor(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "world-victim-aura")) { Require(argc==5,"world aura requires db/resources/mode"); WorldVictimAuraScope(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "aura-player-ownership")) { Require(argc==5,"aura ownership requires db/resources/mode"); AuraPlayerOwnership(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "explosion-evasion")) { Require(argc==5,"explosion evasion requires db/resources/mode"); ExplosionEvasionPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "unsigned-ic-chance")) { Require(argc==5,"unsigned IC requires db/resources/mode"); UnsignedICChance(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "incidental-damage-base")) { Require(argc==5,"incidental damage requires db/resources/mode"); IncidentalDamageBase(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "faster-targeting")) { Require(argc==5,"targeting73 requires db/resources/mode"); FasterTargetingPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "targeting-base")) { Require(argc==5,"targeting base requires db/resources/mode"); TargetingBaseFormula(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "additive-order")) { Require(argc==5,"additive order requires db/resources/mode"); AdditiveOrderRegression(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "live-familiarity")) { Require(argc==5,"live familiarity requires db/resources/mode"); LiveWeaponFamiliarity(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "explosive-modifier-math")) { Require(argc==3,"explosion modifier requires mode"); ExplosiveModifierMath(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "cheap-grenade-ap")) { Require(argc==5,"grenade43 requires db/resources/mode"); CheapGrenadeAPPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "grenade-table-ap")) { Require(argc==5,"grenade table requires db/resources/mode"); GrenadeTableAP(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "cheap-knife-throw-ap")) { Require(argc==5,"knife43 requires db/resources/mode"); CheapKnifeThrowAPPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "throw-knife-base-ap")) { Require(argc==5,"throw base requires db/resources/mode"); ThrowKnifeBaseAP(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "better-evasion")) { Require(argc==5,"evasion40 requires db/resources/mode"); BetterEvasionPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "cheap-melee-ap")) { Require(argc==5,"melee32 requires db/resources/mode"); CheapMeleeAPPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "corpse-tracker-ap")) { Require(argc==5,"corpse42 requires db/resources/mode"); CorpseTrackerAPPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "crouch-costs-walk")) { Require(argc==5,"crouch35 requires db/resources/mode"); CrouchCostsWalkPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "prone-costs-crouch")) { Require(argc==5,"prone34 requires db/resources/mode"); ProneCostsCrouchPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "pk-unsupported-pose")) { Require(argc==5,"PK pose cost requires db/resources/mode"); PKUnsupportedPoseCost(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "observer-stealth")) { Require(argc==5,"observer75 requires db/resources/mode"); ObserverStealthPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "better-spotting")) { Require(argc==5,"spotting requires db/resources/mode"); BetterSpottingPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-xp-threshold-pc24")) { Require(argc==4,"XP threshold requires db/mode"); NativeXPThresholdPC24(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-kill-reward-quote")) { Require(argc==4,"reward quote requires db/mode"); NativeKillRewardQuote(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-d1-precision")) { Require(argc==4,"D1 precision requires db/mode"); NativeD1Precision(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-grenade-range-stance")) { Require(argc==5,"grenade range/stance requires db/resources/mode"); NativeGrenadeRangeStance(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-grenade-hide-launch")) { Require(argc==5,"grenade hide requires db/resources/mode"); NativeGrenadeHideLaunch(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-grenade-pk-launch")) { Require(argc==5,"grenade PK requires db/resources/mode"); NativeGrenadePKLaunch(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-particle-atlas-margin")) { Require(argc==5,"atlas margin requires db/resources/mode"); NativeParticleAtlasMargin(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-grass-atlas-margin")) { Require(argc==5,"atlas margin requires db/resources/mode"); NativeGrassAtlasMargin(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-floor-culling")) {Require(argc==3,"floor culling requires case");NativeFloorCulling(std::atoi(argv[2]));return true;}
    if (!std::strcmp(mode, "native-integer-shadow-depth")) { Require(argc==3,"integer shadow depth requires mode"); NativeIntegerShadowDepth(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-raster-guard-plane")) { Require(argc==3,"guard plane requires mode"); NativeRasterGuardPlane(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-ally-cover-cache")) { Require(argc==5,"ally cache requires db/res/mode"); NativeAllyCoverCache(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ally-raster-safety")) { Require(argc==5,"ally safety requires db/res/mode"); NativeAllyRasterSafety(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-wounded-ai-scared")) { Require(argc==5,"wounded AI requires db/res/mode"); NativeWoundedAIScared(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-wounded-ai-contacts")) { Require(argc==5,"fear contacts requires db/res/mode"); NativeWoundedAIContacts(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-wounded-ai-retreat")) { Require(argc==5,"fear retreat requires db/res/mode"); NativeWoundedAIRetreat(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-tooltip-delay")) { Require(argc==5,"tooltip delay requires db/res/mode"); NativeTooltipDelay(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-reach-preview") && argc==5) { MeleeReachPreview(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-preview-rocket-cover") && argc==5) { PreviewRocketCover(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-preview-object-wire") && argc==4) { PreviewObjectWire(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "native-preview-object-facing") && argc==5) { PreviewObjectFacing(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-preview-command-object") && argc==5) { PreviewCommandObject(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-preview-dispatch") && argc==5) { PreviewDispatch(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-preview-firstturn-lua") && argc==5) { PreviewFirstTurnLua(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-tile-shot-preview") && argc==5) { TileShotPreview(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-tile-cover-origin") && argc==5) { TileCoverOrigin(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ledge-retreat") && argc==4) { NativeLedgeRetreat(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "native-inactive-door-turn") && argc==5) { InactiveDoorTurn(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-selector-preview") && argc==5) { PreviewMeleeSelector(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-unit-facing-preview") && argc==5) { PreviewUnitFacing(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-directed-attack-pose")) { Require(argc==5,"directed attack pose requires db/resources/mode"); NativeDirectedAttackPose(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-preview-hull")) { Require(argc==5,"melee preview hull requires db/resources/mode"); NativeMeleePreviewHull(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-dead-pk-support")) { Require(argc==5,"PK support requires db/resources/mode"); NativeDeadPKSupport(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-object-hover")) { Require(argc==5,"object hover requires db/resources/mode"); NativeObjectHoverAttack(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-smooth-relocation")) { Require(argc==5,"smooth relocation requires db/resources/mode"); NativeSmoothRelocation(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-unit-aware-pk")) { Require(argc==5,"unit-aware PK control requires db/resources/mode"); NativeUnitAwarePKControl(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-unit-aware-waypoint")) { Require(argc==5,"unit-aware waypoint requires db/resources/mode"); NativeUnitAwareWaypointPlacement(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-unit-aware-deploy")) { Require(argc==5,"unit-aware deploy requires db/resources/mode"); NativeUnitAwareDeployment(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-explosive-clearance")) { Require(argc==5,"explosive clearance requires db/resources/mode"); NativeExplosiveClearance(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-explosive-groups")) { Require(argc==5,"explosive groups requires db/resources/mode"); NativeExplosiveGroups(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-scenario-defeat")) { Require(argc==5,"scenario defeat requires db/resources/mode"); NativeScenarioDefeat(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-match-defeat")) { Require(argc==5,"match defeat requires db/resources/mode"); NativeMatchDefeat(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "cover-legacy-ray-wire")) { CoverLegacyRayWire(); return true; }
    if (!std::strcmp(mode, "native-attack-session")) { Require(argc==5,"attack session requires db/resources/mode"); NativeAttackSession(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-hero-defeat")) { Require(argc==5,"hero defeat requires db/resources/mode"); NativeHeroDefeat(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-hotseat-equipment-flow")) { Require(argc==5,"hotseat equipment flow requires db/resources/mode"); NativeHotseatEquipmentFlow(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-hotseat-setup")) { Require(argc==5,"hotseat setup requires db/resources/mode"); NativeHotseatSetup(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ai-merc-addplayer")) { Require(argc==5,"AI merc AddPlayer requires db/resources/mode"); NativeAIMercAddPlayer(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-group-waypoint")) { Require(argc==5,"group waypoint requires db/resources/mode"); NativeGroupWaypoint(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-world-waypoint-route")) { Require(argc==5,"world waypoint route requires db/resources/mode"); NativeWorldWaypointRoute(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-placement-reservations")) { Require(argc==5,"placement reservations requires db/resources/mode"); NativePlacementReservations(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-health-knockout")) { Require(argc==5,"health knockout requires db/resources/mode"); NativeHealthKnockout(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-healed-damage")) { Require(argc==5,"healed damage requires db/resources/mode"); NativeHealedDamage(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-unaware-critical")) { Require(argc==5,"unaware critical requires db/resources/mode"); NativeUnawareCritical(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-healing-pose")) {Require(argc==5,"healing requires db/resources/case");NativeHealingPose(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-hero-seven")) {Require(argc==5,"hero requires db/resources/case");NativeHeroSeven(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-temporary-companions")) {Require(argc==5,"temporary companions require db/resources/case");NativeTemporaryCompanions(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-subzone-transition")) { Require(argc==5,"subzone transition requires db/resources/mode"); NativeSubzoneTransition(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-store-passage")) { Require(argc==5,"store passage requires db/resources/mode"); NativeStorePassage(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-authored-light")) { Require(argc==5,"authored light requires db/resources/mode"); NativeAuthoredLight(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-foot-support")) { Require(argc==5,"foot support requires db/resources/mode"); if(std::atoi(argv[4])<4)NativeFootSupport(argv[2],argv[3],std::atoi(argv[4]));else NativeFootSupportProducer(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-cinematic-floor-range")) { Require(argc==5,"cinematic range requires db/resources/mode"); NativeCinematicFloorRange(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-camera-floor-owner")) { Require(argc==5,"camera floor ownership requires db/res/mode"); NativeCameraFloorOwner(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-cached-point-shadow-flag")) { Require(argc==5,"cached point flags require db/res/mode"); NativeCachedPointShadowFlag(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-menu-idle-demo")) { Require(argc==5,"menu idle requires db/res/mode"); NativeMenuIdleDemo(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-autoplay-presentation")) { Require(argc==5,"autoplay presentation requires db/res/mode"); NativeAutoPlayPresentation(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-autoplay-lifecycle")) { Require(argc==5,"autoplay lifecycle requires db/res/mode"); NativeAutoPlayLifecycle(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-autoplay-squads")) { Require(argc==5,"autoplay squads require db/res/mode"); NativeAutoPlaySquads(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-tooltip-placement")) { Require(argc==5,"tooltip placement requires db/res/mode"); NativeTooltipPlacement(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ally-ray-classification")) { Require(argc==5,"ally ray requires db/res/mode"); NativeAllyRayClassification(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ai-cover-depth")) { Require(argc==5,"native AI policy requires db/resources/mode"); NativeAICoverDepth(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-path-wave-flags")) { Require(argc==5,"native AI policy requires db/resources/mode"); NativePathWaveFlags(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ai-flank-wave")) { Require(argc==5,"native AI policy requires db/resources/mode"); NativeAIFlankWave(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-generic-raster")) { Require(argc==4,"generic raster requires mode and fog flag"); NativeGenericRaster(std::atoi(argv[2]),std::atoi(argv[3])!=0); return true; }
    if (!std::strcmp(mode, "native-cover-whole-center")) { Require(argc==5,"cover policy requires db/resources/mode"); NativeCoverFramingPolicy(argv[2],argv[3],std::atoi(argv[4]),false); return true; }
    if (!std::strcmp(mode, "native-cover-projection-extent")) { Require(argc==5,"cover policy requires db/resources/mode"); NativeCoverFramingPolicy(argv[2],argv[3],std::atoi(argv[4]),true); return true; }
    if (!std::strcmp(mode, "native-cover-range-policy")) { Require(argc==5,"cover policy requires db/resources/mode"); NativeCoverRangePolicy(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-rpg-ownership-damage")) { Require(argc==5,"ownership damage requires db/resources/mode"); NativeRPGOwnershipDamage(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-rpg-ownership-AI")) { Require(argc==5,"heard AI requires db/resources/mode"); NativeRPGOwnershipAI(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-heard-AI-camera")) { Require(argc==5,"heard AI requires db/resources/mode"); NativeHeardAICamera(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-hearing-probability")) { Require(argc==4,"hearing/noise requires db[/resources]/mode"); NativeHearingProbability(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-hearing-practice")) { Require(argc==4,"hearing/noise requires db[/resources]/mode"); NativeHearingPractice(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-movement-noise")) { Require(argc==5,"hearing/noise requires db[/resources]/mode"); NativeMovementNoise(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-pk-collapse")) { Require(argc==5,"PK collapse requires db/resources/mode"); NativePKCollapse(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-pk-corpse-gate")) { Require(argc==5,"PK corpse gate requires db/resources/mode"); NativePKCorpseGate(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-pk-pilot-exit")) { Require(argc==5,"PK pilot exit requires db/resources/mode"); NativePKPilotExit(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-pk-hide-gate")) { Require(argc==5,"PK hide gate requires db/resources/mode"); NativePKHideGate(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-camera-framing-bridge")) { Require(argc==5,"camera bridge requires db/resources/mode"); NativeCameraFramingBridge(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-context-modlinks")) { Require(argc==5,"context modlinks requires db/resources/mode"); NativeContextModLinks(argv[2],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-context-trap")) { Require(argc==5,"context trap requires db/resources/mode"); NativeContextTrap(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-cached-lighting-pipeline")) {Require(argc==5,"cached lighting pipeline requires db/resources/case");NativeCachedLightingPipeline(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-runtime-input")) {Require(argc==5,"runtime input requires db/resources/case");NativeRuntimeInputSettings(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-deferred-turns")) {Require(argc==5,"deferred turns requires db/resources/case");NativeDeferredTurns(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-loot-hotseat")) {Require(argc==5,"hotseat requires db/resources/case");NativeLootHotseat(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-shared-globals")) {Require(argc==3,"shared globals requires case");NativeSharedGlobals(std::atoi(argv[2]));return true;}
    if (!std::strcmp(mode, "native-loot-records")) {Require(argc==5,"loot requires db/resources/case");NativeLootRecords(argv[2],argv[3],std::atoi(argv[4]));return true;}
    if (!std::strcmp(mode, "native-inactive-swap")) { Require(argc==5,"inactive swap requires db/resources/mode"); NativeInactiveSwap(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-context-swap")) { Require(argc==5,"context swap requires db/resources/mode"); NativeContextSwap(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-context-end-turn")) { Require(argc==5,"context end-turn requires db/resources/mode"); NativeContextEndTurn(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-context-clue")) { Require(argc==5,"context clue requires db/resources/mode"); NativeContextClue(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-fractional-ui-pixels")) { Require(argc==5,"fractional UI pixels requires db/resources/mode"); NativeFractionalUIPixels(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "automatic-camera-poi")) { Require(argc==5,"automatic camera requires db/resources/mode"); NativeAutomaticCameraPOI(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "automatic-camera-fov")) { Require(argc==5,"automatic camera requires db/resources/mode"); NativeAutomaticCameraFOV(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "automatic-camera-following")) { Require(argc==5,"automatic camera requires db/resources/mode"); NativeAutomaticCameraFollowing(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "automatic-camera-mission")) { Require(argc==5,"automatic camera requires db/resources/mode"); NativeAutomaticCameraMission(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-camera-candidate")) { Require(argc==3,"camera candidate requires mode"); NativeCameraCandidate(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-camera-slomo-window")) { Require(argc==5,"camera slomo requires db/resources/mode"); NativeCameraSloMoWindow(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-camera-framing")) { Require(argc==3,"camera framing requires mode"); NativeCameraFraming(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-held-grenade-priority")) { Require(argc==5,"held grenade priority requires db/resources/mode"); NativeHeldGrenadePriority(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-grenade-fuse")) { Require(argc==5,"grenade fuse requires db/resources/mode"); NativeGrenadeFuse(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-grenade-physics")) { Require(argc==5,"grenade range/stance requires db/resources/mode"); NativeGrenadePhysics(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-shared-grenade-accuracy")) { Require(argc==5,"shared grenade accuracy requires db/resources/mode"); NativeSharedGrenadeAccuracy(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-shared-melee-skill")) { Require(argc==4,"shared melee skill requires db/mode"); NativeSharedMeleeSkill(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-shared-accuracy")) { Require(argc==4,"shared accuracy requires db/mode"); NativeSharedAccuracy(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-accuracy-movement-range")) { Require(argc==4,"accuracy movement/range requires db/mode"); NativeAccuracyMovementRange(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-extra-round-ap")) { Require(argc==4,"extra-round AP requires db/mode"); NativeExtraRoundAP(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-hide-practice")) { Require(argc==5,"Hide practice requires db/resources/mode"); NativeHidePractice(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-medical-engineering-practice")) { Require(argc==4,"action practice requires db/mode"); NativeMedicalEngineeringPractice(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-learning-perk-training")) { Require(argc==4,"learning training requires db/mode"); NativeLearningPerkTraining(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-associated-attribute-training")) { Require(argc==4,"associated training requires db/mode"); NativeAssociatedAttributeTraining(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-dynamic-skill-rounding")) { Require(argc==3,"dynamic rounding requires mode"); NativeDynamicSkillRounding(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-dynamic-skill-upgrade")) { Require(argc==3,"dynamic skill upgrade requires mode"); NativeDynamicSkillUpgrade(std::atoi(argv[2])); return true; }
    if (!std::strcmp(mode, "native-fractional-skill-training")) { Require(argc==4,"fractional training requires db/mode"); NativeFractionalSkillTraining(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "unhide-base")) { Require(argc==5,"unhide base requires db/resources/mode"); UnhideBasePrecision(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "disarm-traps")) { Require(argc==5,"disarm requires db/resources/mode"); DisarmTrapsPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "classless-perk")) { Require(argc==5,"classless perk requires db/resources/mode"); ClasslessPerkQuery(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "difficult-traps")) { Require(argc==5,"trap difficulty requires db/resources/mode"); DifficultTrapsPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "zeroing-bonus")) { Require(argc==5,"zeroing bonus requires db/resources/mode"); ZeroingBonusPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "best-shooting-pose")) { Require(argc==5,"best pose requires db/resources/mode"); BestShootingPosePerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "perfect-engineering-tool")) { Require(argc==5,"perfect engineering requires db/resources/mode"); PerfectEngineeringToolEligibility(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "first-turn-perk")) { Require(argc==5,"first-turn perk requires db/resources/mode"); FirstTurnPenaltyPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "grenade-unit-first-turn")) { Require(argc==5,"grenade unit-turn requires db/resources/mode"); GrenadeUnitFirstTurnInput(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "grenade-throw-night")) { Require(argc==5,"grenade night requires db/resources/mode"); GrenadeThrowNightInput(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "night-ai-inputs")) { Require(argc==5,"AI night requires db/resources/mode"); NightAIConstructorInputs(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "night-preview-inputs")) { Require(argc==5,"night preview requires db/resources/mode"); NightPreviewInputs(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "script-grenade-chance")) { Require(argc==5,"script grenade chance requires db/resources/mode"); ScriptGrenadeExplicitChance(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "vp-duration-countdown")) { Require(argc==5,"VP countdown requires db/resources/mode"); VPBoostDurationCountdown(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "vp-immediate-overdose")) { Require(argc==5,"VP overdose requires db/resources/mode"); VPBoostImmediateOverdose(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "debug-explosion-routing")) { Require(argc==5,"debug explosion requires db/resources/mode"); DebugExplosionRouting(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "clue-death-reserve")) { Require(argc==5,"clue reserve requires db/resources/mode"); PersonClueCreation(argv[2],argv[3],false,std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "main-menu-version-caption")) { Require(argc==4,"menu version requires db/resources"); MainMenuVersionCaption(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-pk-gate")) { Require(argc==5,"chapter PK gate requires db/resources/mode"); ChapterPanzerkleinGate(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "weather-world-placement")) { Require(argc==5,"weather placement requires db/resources/mode"); WeatherWorldPlacement(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "unit-preview-shot-origin")) { Require(argc==5,"shot origin preview requires db/resources/mode"); UnitPreviewShotOrigin(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "knife-unit-preview")) { Require(argc==5,"native knife preview requires db/resources/mode"); KnifeUnitPreview(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "knife-unit-dispatch")) { Require(argc==5,"game knife dispatch requires db/resources/mode"); KnifeUnitDispatch(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "knife-time-rebase")) { Require(argc==5,"knife clock requires db/resources/mode"); KnifeTimeRebase(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "hidden-skip-budget")) { Require(argc==5,"hidden skip requires db/resources/mode"); HiddenSkipBudget(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "localized-log-style")) { Require(argc==5,"log style requires db/resources/mode"); LocalizedLogStyle(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "carrier-admission")) { Require(argc==5,"carrier admission requires db/resources/mode"); CorpseCarrierAdmission(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "object-sound-stage")) { Require(argc==5,"object sound stage requires db/resources/mode"); ObjectSoundStage(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "corpse-null-taker")) { Require(argc==5,"corpse null taker requires db/resources/mode"); CorpseNullTaker(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "corpse-cancel-state")) { Require(argc == 5, "corpse cancellation requires db, resources and mode"); CorpseCancelState(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "backpack-bone")) { Require(argc==5,"backpack binding requires db, resources and mode"); BackpackBoneBinding(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "bind-places-seed")) { Require(argc==5,"bind seed requires db, resources and mode"); BindPlacesSeed(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "item-initial-floor")) { Require(argc == 5, "item floor contact requires db, resources and mode"); ItemInitialFloorContact(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "inactive-random-encounter")) { Require(argc == 6, "inactive encounter requires db, resources, viewport and mode"); InactiveRandomEncounter(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "global-map-desktop")) { Require(argc == 5, "global desktop requires db, resources and mode"); GlobalMapDesktop(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "global-map-script-lifecycle")) { Require(argc == 6, "global map lifecycle requires db, resources, context and mode"); GlobalMapScriptLifecycle(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-ai-flash")) { Require(argc == 6, "world AI flash requires db, resources, context and mode"); WorldAIFlash(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-clue")) { Require(argc == 6, "world clue requires db, resources, context and mode"); WorldScriptClue(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-video")) { Require(argc == 6, "world video requires db, resources, context and mode"); WorldScriptVideo(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-camera-locator")) { Require(argc == 6, "world camera locator requires db, resources, context and mode"); WorldScriptCameraLocator(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-floor")) { Require(argc == 6, "world floor requires db, resources, context and mode"); WorldScriptFloor(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-camera-lock")) { Require(argc == 6, "world camera lock requires db, resources, context and mode"); WorldScriptCameraLock(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-camera-clipping")) { Require(argc == 6, "world camera clipping requires db, resources, context and mode"); WorldScriptCameraClipping(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-hints")) { Require(argc == 6, "world hints require db, resources, context and mode"); WorldScriptHints(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "world-script-lighting")) { Require(argc == 6, "world lighting requires db, resources, context and mode"); WorldScriptLighting(argv[2],argv[3],std::atoi(argv[4]),std::atoi(argv[5])); return true; }
    if (!std::strcmp(mode, "script-ambient-variants")) { Require(argc == 5, "script ambient variants requires db, resources and mode"); ScriptAmbientVariants(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "script-immediate-ambient")) { Require(argc == 5, "script immediate ambient requires db, resources and mode"); ScriptImmediateAmbient(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "script-play-effect")) { Require(argc == 5, "script play effect requires db, resources and mode"); ScriptPlayEffect(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "script-ambient-effect")) { Require(argc == 5, "script ambient effect requires db, resources and mode"); ScriptAmbientEffect(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "backdrop-ui-audio-ownership")) { Require(argc == 4, "backdrop UI ownership requires db and resources"); BackdropUIAudioOwnership(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "menu-music-pools")) { Require(argc == 5, "menu music requires db, resources and origin"); MenuMusicPools(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "audio-option-checkboxes") && argc == 4) { AudioOptionCheckboxes(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "audio-option-checkbox-notify") && argc == 4) { AudioOptionCheckboxes(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "audio-option-defaults") && argc == 4) { AudioOptionCheckboxes(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "lua-unused-stack-init")) { LuaUnusedStackInitialization(); return true; }
    if (!std::strcmp(mode, "in-game-save-button") && argc == 4) { InGameSaveButton(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "in-game-save-shortcut") && argc == 4) { InGameSaveButton(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "backdrop-script-audio-3d") && argc == 4) { BackdropScriptAudio(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "chapter-active-script") && argc == 4) { ChapterActiveScript(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-active-command") && argc == 5) { ChapterActiveScript(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "chapter-clock-step") && argc == 5) { ChapterClockStep(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "legacy-chapter-world-migration") && argc == 5) { LegacyChapterWorldMigration(argv[2],argv[3],argv[4]); return true; }
    if (!std::strcmp(mode, "chapter-focus-audio") && argc == 4) { ChapterFocusAudio(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-script-initialization") && argc == 4) { ChapterScriptInitialization(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-world-graph") && argc == 4) { ChapterWorldGraph(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "global-sector-description") && argc == 4) { GlobalSectorDescription(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "global-map-return-destination") && argc == 4) { GlobalMapReturnDestination(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "global-map-missing-record") && argc == 4) { GlobalMapReturnDestination(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "failed-global-map-chapter-return") && argc == 4) { GlobalMapReturnDestination(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "chapter-team-marker-tooltip") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "chapter-marker-tooltip-transitions") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "chapter-exit-marker-hint") && argc == 4) { ChapterTeamMarkerToolTip(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "forced-tool-targeting-entry") && argc == 3) { ForcedToolTargetingEntry(argv[2]); return true; }
    if (!std::strcmp(mode, "tool-targeting-cursor-refresh") && argc == 3) { ToolTargetingCursorRefresh(argv[2]); return true; }
    if (!std::strcmp(mode, "sound-mixer-clock-catchup")) { SoundMixerClockCatchUp(); return true; }
    if (!std::strcmp(mode, "first-render-sound-clock")) { FirstRenderSoundClock(); return true; }
    if (!std::strcmp(mode, "hotseat-opponent-visibility")) { HotseatOpponentVisibility(); return true; }
    if (!std::strcmp(mode, "restored-clue-close-notification")) { RestoredClueCloseNotification(); return true; }
    if (!std::strcmp(mode, "hit-label-colour-human") && argc==4) { RejectedHitLabelText(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "hit-label-colour-pk") && argc==4) { RejectedHitLabelText(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "rejected-hit-label-text")) { Require(argc == 4, "hit label text requires db and res"); RejectedHitLabelText(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "oversized-tooltip-position") && argc==4) { OversizedTooltipPosition(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "scaled-edit-text-layout")) { ScaledEditTextLayout(); return true; }
    if (!std::strcmp(mode, "long-burst-voice-donor") && argc == 4) { LongBurstVoiceDonor(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "fresh-clue-wait-release") && argc == 4) { FreshClueWaitRelease(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mission-sound-game-clock")) { MissionSoundGameClock(); return true; }
    if (!std::strcmp(mode, "delayed-sound-effect-age") && argc == 3) { DelayedSoundEffectAge(argv[2]); return true; }
    if (!std::strcmp(mode, "infinite-sound-effect-loop") && argc == 3) { DelayedSoundEffectAge(argv[2],true); return true; }
    if (!std::strcmp(mode, "late-looping-sound-offset")) { LateLoopingSoundOffset(); return true; }
    if (!std::strcmp(mode, "sound-2d-ending")) { Sound2DEnding(); return true; }
    if (!std::strcmp(mode, "released-sound-channel-cleanup")) { ReleasedSoundChannelCleanup(); return true; }
    if (!std::strcmp(mode, "failed-sound-channel-lifetime")) { FailedSoundChannelLifetime(); return true; }
    if (!std::strcmp(mode, "headless-music-stream")) { HeadlessMusicStream(); return true; }
    if (!std::strcmp(mode, "trackless-scene-music")) { TracklessSceneMusicContinuity(); return true; }
    if (!std::strcmp(mode, "reloaded-music-record")) { ReloadedMusicRecordContinuity(); return true; }
    if (!std::strcmp(mode, "exit-menu-confirmation") && argc == 4) { ExitMenuConfirmation(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "supplied-defeat-screenshot") && argc == 4) { SuppliedMenuScreenshot(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "supplied-options-screenshot") && argc == 4) { SuppliedMenuScreenshot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "new-layer-compact-save")) { NewLayerCompactSave(); return true; }
    if (!std::strcmp(mode, "unsupported-navigation-bake") && argc == 4) { UnsupportedNavigationBake(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "restored-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "fresh-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "legacy-ai-event-owner") && argc == 4) { AIEventOwnerBinding(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "scripted-unconscious-enemy") && argc == 4) { ScriptedEnemyDeath(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "scripted-enemy-death") && argc == 4) { ScriptedEnemyDeath(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "empty-passage-exit") && argc == 4) { ScriptedPassageTransition(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "passage-preview-party-far") && argc==4) { PassagePreview(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "passage-preview-party-near") && argc==4) { PassagePreview(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "passage-preview-actor-far") && argc==4) { PassagePreview(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "passage-preview-split-exits") && argc==4) { PassagePreview(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "passage-preview-alternate-exit") && argc==4) { PassagePreview(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "passage-preview-unconscious") && argc==4) { PassagePreview(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "passage-preview-dead") && argc==4) { PassagePreview(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "passage-preview-forced") && argc==4) { PassagePreview(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "passage-preview-other-owner") && argc==4) { PassagePreview(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "passage-preview-no-zone") && argc==4) { PassagePreview(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "passage-preview-unregistered") && argc==4) { PassagePreview(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "passage-preview-no-target") && argc==4) { PassagePreview(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "scripted-passage-transition") && argc == 4) { ScriptedPassageTransition(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "flying-attack-queue") && argc == 4) { FlyingAttackQueue(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "simple-pose-facing-preference")) { SimplePoseFacingPreference(); return true; }
    if (!std::strcmp(mode, "ground-passability-precedence")) { GroundPassabilityPrecedence(); return true; }
    if (!std::strcmp(mode, "ladder-passability-layer-bounds")) { LadderPassabilityLayerBounds(); return true; }
    if (!std::strcmp(mode, "ladder-passability-facing")) { LadderPassabilityFacing(); return true; }
    if (!std::strcmp(mode, "anchor-passability")) { AnchorPassability(); return true; }
    if (!std::strcmp(mode, "simple-cross-layer-pose-entry")) { SimpleCrossLayerPoseEntry(); return true; }
    if (!std::strcmp(mode, "isolated-source-alias-vector")) { NavigationAliasDestinations(false,true); return true; }
    if (!std::strcmp(mode, "isolated-source-alias-array")) { NavigationAliasDestinations(true,true); return true; }
    if (!std::strcmp(mode, "door-creation-flags-0") && argc==4) { DoorCreationFlags(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "door-creation-flags-1") && argc==4) { DoorCreationFlags(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "door-creation-flags-2") && argc==4) { DoorCreationFlags(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "terrain-reduction-passability") && argc==4) { TerrainReduction(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "terrain-reduction-normal-sort") && argc==4) { TerrainReduction(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "terrain-reduction-no-sort") && argc==4) { TerrainReduction(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "terrain-reduction-ladder-map") && argc==4) { TerrainReduction(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "layer-merge-empty-other")) { MovementLayerMerge(0); return true; }
    if (!std::strcmp(mode, "layer-merge-disconnected-current")) { MovementLayerMerge(1); return true; }
    if (!std::strcmp(mode, "layer-merge-disconnected-other")) { MovementLayerMerge(2); return true; }
    if (!std::strcmp(mode, "layer-merge-connected")) { MovementLayerMerge(3); return true; }
    if (!std::strcmp(mode, "layer-merge-first-h-c")) { MovementLayerMerge(4); return true; }
    if (!std::strcmp(mode, "layer-merge-other-h-c")) { MovementLayerMerge(5); return true; }
    if (!std::strcmp(mode, "layer-merge-height")) { MovementLayerMerge(6); return true; }
    if (!std::strcmp(mode, "layer-merge-displacement")) { MovementLayerMerge(7); return true; }
    if (!std::strcmp(mode, "layer-merge-empty-current")) { MovementLayerMerge(8); return true; }
    if (!std::strcmp(mode, "isolated-movement-cell")) { IsolatedMovementCell(); return true; }
    if (!std::strcmp(mode, "navigation-alias-vector")) { NavigationAliasDestinations(false); return true; }
    if (!std::strcmp(mode, "navigation-alias-array")) { NavigationAliasDestinations(true); return true; }
    if (!std::strcmp(mode, "flying-dynamic-locks")) { FlyingDynamicLocks(); return true; }
    if (!std::strcmp(mode, "arrived-retreat-reaction-lifetime")) { Require(argc == 4, "retreat reaction lifetime requires db and res"); ArrivedRetreatReactionLifetime(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "arrived-retreat-completion") && argc == 4) { ArrivedRetreatCompletion(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "finished-ai-logic-retirement") && argc == 4) { FinishedAILogicRetirement(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "removed-selected-enemy")) { RemovedSelectedAIContact(0); return true; }
    if (!std::strcmp(mode, "removed-selected-possible")) { RemovedSelectedAIContact(1); return true; }
    if (!std::strcmp(mode, "removed-selected-ally")) { RemovedSelectedAIContact(2); return true; }
    if (!std::strcmp(mode, "begin-turn-live-watcher") && argc == 4) { BeginTurnLiveContacts(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "begin-turn-live-enemy") && argc == 4) { BeginTurnLiveContacts(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "combat-chooser-context") && argc == 4) { CombatChooserContext(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "live-ai-position") && argc == 4) { LiveAIPosition(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "live-ai-unit-position") && argc == 4) { LiveAIPosition(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "owned-ai-action-enemy")) { OwnedAIEnemySelection(false); return true; }
    if (!std::strcmp(mode, "owned-ai-place-enemy")) { OwnedAIEnemySelection(true); return true; }
    if (!std::strcmp(mode, "mounted-ai-log-commands")) { MountedAILogCommands(); return true; }
    if (!std::strcmp(mode, "removed-player-ai-rosters") && argc == 4) { RemovedPlayerAIRosters(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "removed-player-acks") && argc == 4) { RemovedPlayerFootprints(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "removed-player-footprints") && argc == 4) { RemovedPlayerFootprints(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "empty-bullet-retirement") && argc == 4) { EmptyBulletConstruction(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "bullet-nearest-single") && argc==4) { BulletNearestTarget(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "bullet-nearest-multiple") && argc==4) { BulletNearestTarget(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "bullet-nearest-visual-before-hit") && argc==4) { BulletNearestTarget(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "bullet-nearest-retained") && argc==4) { BulletNearestTarget(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "bullet-nearest-visual-only") && argc==4) { BulletNearestTarget(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "empty-bullet-construction") && argc == 4) { EmptyBulletConstruction(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-unit-tint") && argc == 4) { HiddenUnitTint(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "living-unit-floor-refresh") && argc == 4) { LivingUnitFloorRefresh(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-ai-unit-sliver") && argc == 4) { AICoverProjection(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-ai-unit-near") && argc == 4) { AICoverProjection(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-human-unit-sliver") && argc == 4) { AICoverProjection(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-human-unit-near") && argc == 4) { AICoverProjection(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-tile-sliver-ally") && argc == 4) { AICoverProjection(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-tile-far-ally") && argc == 4) { AICoverProjection(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-tile-near-ally") && argc == 4) { AICoverProjection(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-cover-projection-pk-unit-near") && argc == 4) { AICoverProjection(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-ai-itemmuzzle") && argc == 4) { AICoverMask(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-ai-passonly") && argc == 4) { AICoverMask(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-ai-itemonly") && argc == 4) { AICoverMask(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-ai-fragmentonly") && argc == 4) { AICoverMask(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewhigh-itemmuzzle") && argc == 4) { AICoverMask(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewhigh-passonly") && argc == 4) { AICoverMask(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewhigh-itemonly") && argc == 4) { AICoverMask(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewhigh-fragmentonly") && argc == 4) { AICoverMask(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewlow-itemmuzzle") && argc == 4) { AICoverMask(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewlow-passonly") && argc == 4) { AICoverMask(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewlow-itemonly") && argc == 4) { AICoverMask(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-previewlow-fragmentonly") && argc == 4) { AICoverMask(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-ai-ignored") && argc == 4) { AICoverMask(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "ai-cover-mask-preview-low-far") && argc == 4) { AICoverMask(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "ai-cover-native-fragment-parity") && argc == 4) { AICoverNativeFragmentParity(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "ai-tohit-weapon-armed-empty") && argc == 4) { AIToHitWeaponOwner(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-tohit-weapon-empty-armed") && argc == 4) { AIToHitWeaponOwner(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-tohit-weapon-both-armed") && argc == 4) { AIToHitWeaponOwner(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-tohit-weapon-both-empty") && argc == 4) { AIToHitWeaponOwner(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-tohit-target-null-armed") && argc == 4) { AIToHitTargetValidity(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-tohit-target-invalid-armed") && argc == 4) { AIToHitTargetValidity(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-tohit-target-null-empty") && argc == 4) { AIToHitTargetValidity(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-tohit-target-valid-control") && argc == 4) { AIToHitTargetValidity(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-tohit-target-invalid-empty") && argc == 4) { AIToHitTargetValidity(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-tohit-server-null-armed") && argc == 4) { AIToHitServerValidity(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-tohit-server-invalid-armed") && argc == 4) { AIToHitServerValidity(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-tohit-server-valid-control") && argc == 4) { AIToHitServerValidity(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-tohit-server-null-empty") && argc == 4) { AIToHitServerValidity(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-tohit-server-invalid-empty") && argc == 4) { AIToHitServerValidity(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-snipe-proposed-pose-and-target") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-snipe-selected-empty-hand") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-snipe-selected-current-hand") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-snipe-selected-no-ammo") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-snipe-escape-four-visible") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-snipe-escape-four-blocked") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-precision-query-ratio") && argc == 4) { AICoverPrecision(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cover-precision-constructor-fractional") && argc == 4) { AICoverPrecision(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cover-precision-constructor-integer") && argc == 4) { AICoverPrecision(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cover-precision-constructor-zero") && argc == 4) { AICoverPrecision(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cover-precision-prediction") && argc == 4) { AICoverPrecision(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-precision-constructor-open") && argc == 4) { AICoverPrecision(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-death") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-unconscious") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-nonlethal") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-blocked") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-anonymous") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-repeat-dead") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-finish-unconscious") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-first-incapacitation-corpse-event") && argc == 4) { AIFirstIncapacitation(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-snipe-facing-parity-visible") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-snipe-facing-parity-blocked") && argc == 4) { AISnipeSelectedWeapon(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "scenario-discovery-carrier") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "scenario-discovery-item") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "scenario-discovery-person") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "scenario-discovery-unseen") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "scenario-discovery-latched") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "scenario-discovery-script-control") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "scenario-initial-carrier") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "scenario-initial-other-tags") && argc == 4) { ScenarioDiscovery(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-cover-range-ai-far-elevated") && argc == 4) { AICoverRange(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cover-range-ai-near-elevated") && argc == 4) { AICoverRange(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cover-range-human-far-elevated") && argc == 4) { AICoverRange(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cover-range-ai-far-flat-blocked") && argc == 4) { AICoverRange(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cover-range-human-far-flat-blocked") && argc == 4) { AICoverRange(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-range-ai-near-flat-blocked") && argc == 4) { AICoverRange(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-cover-range-human-near-flat-blocked") && argc == 4) { AICoverRange(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-cover-range-human-near-elevated") && argc == 4) { AICoverRange(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-cover-target-ignored") && argc == 4) { AICoverTargetReach(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-cover-target-ai-beyond") && argc == 4) { AICoverTargetReach(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cover-target-human-beyond") && argc == 4) { AICoverTargetReach(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cover-target-ai-reached") && argc == 4) { AICoverTargetReach(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cover-target-human-reached") && argc == 4) { AICoverTargetReach(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cover-target-tile") && argc == 4) { AICoverTargetReach(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-target-pk-beyond") && argc == 4) { AICoverTargetReach(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-cover-target-pk-reached") && argc == 4) { AICoverTargetReach(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-stand") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-crouch") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-crawl") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-turn") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-position") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-no-weapon-argument") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-no-target") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-cover-origin-human") && argc == 4) { AICoverWeaponOrigin(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-live") && argc == 4) { AINearestContact(argv[2],argv[3],0,0); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-zero-hp") && argc == 4) { AINearestContact(argv[2],argv[3],0,1); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-negative-hp") && argc == 4) { AINearestContact(argv[2],argv[3],0,2); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-tie-zero-hp") && argc == 4) { AINearestContact(argv[2],argv[3],0,3); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-corpse") && argc == 4) { AINearestContact(argv[2],argv[3],0,4); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-live-position") && argc == 4) { AINearestContact(argv[2],argv[3],0,5); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-ko") && argc == 4) { AINearestContact(argv[2],argv[3],0,6); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-ally-tie") && argc == 4) { AINearestContact(argv[2],argv[3],0,7); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-live") && argc == 4) { AINearestContact(argv[2],argv[3],1,0); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-zero-hp") && argc == 4) { AINearestContact(argv[2],argv[3],1,1); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-negative-hp") && argc == 4) { AINearestContact(argv[2],argv[3],1,2); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-tie-zero-hp") && argc == 4) { AINearestContact(argv[2],argv[3],1,3); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-corpse") && argc == 4) { AINearestContact(argv[2],argv[3],1,4); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-live-position") && argc == 4) { AINearestContact(argv[2],argv[3],1,5); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-ko") && argc == 4) { AINearestContact(argv[2],argv[3],1,6); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-suspect-tie") && argc == 4) { AINearestContact(argv[2],argv[3],1,7); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-shared-live") && argc == 4) { AINearestContact(argv[2],argv[3],2,0); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-shared-zero-hp") && argc == 4) { AINearestContact(argv[2],argv[3],2,1); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-shared-corpse") && argc == 4) { AINearestContact(argv[2],argv[3],2,4); return true; }
    if (!std::strcmp(mode, "ai-nearest-contact-shared-tie") && argc == 4) { AINearestContact(argv[2],argv[3],2,7); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-cached") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-neutral") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-allied") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-human") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-transition") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-saved-player") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-no-saved-player") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-no-commander") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-no-world") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-own-ko") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-foreign-backref") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-reowned") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "ai-tactical-views-no-fighters") && argc == 4) { AITacticalWorldViews(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "owned-ai-commander-rosters") && argc == 4) { OwnedAICommanderRosters(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "navigation-colouring-occupancy")) { NavigationColouringOccupancy(); return true; }
    if (!std::strcmp(mode, "missing-navigation-colourer")) { NavigationColouringOccupancy(1); return true; }
    if (!std::strcmp(mode, "stale-navigation-colourer")) { NavigationColouringOccupancy(2); return true; }
    if (!std::strcmp(mode, "ragdoll-restoration")) { RagdollRestoration(); return true; }
    if (!std::strcmp(mode, "fog-surface-eligibility")) { FogSurfaceEligibility(); return true; }
    if (!std::strcmp(mode, "head-render-topology") && argc==4) { HeadRenderTopology(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "negative-mesh-bounds")) { NegativeMeshBounds(); return true; }
    if (!std::strcmp(mode, "sniper-targetless-probe") && argc == 4) { SniperHeldAim(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "sniper-held-aim") && argc == 4) { SniperHeldAim(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "snipe-tile-restriction") && argc == 4) { SnipeTileRestriction(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "person-clue-creation") && argc == 4) { PersonClueCreation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "native-critical-resistance-perk")) { Require(argc==5,"critical46 requires db/resources/mode"); NativeCriticalResistancePerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-pk-resistance")) { Require(argc==5,"PK critical resistance requires db/resources/mode"); NativeCriticalPKResistance(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-severity-perk")) { Require(argc==5,"critical47 requires db/resources/mode"); NativeCriticalSeverityPerk(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-difficulty-fallback")) { Require(argc==5,"critical fallback requires db/resources/mode"); NativeCriticalDifficultyFallback(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-called-shot")) { Require(argc==5,"melee called shot requires db/resources/mode"); NativeMeleeCalledShot(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-ranged-called-shot")) { Require(argc==5,"ranged called shot requires db/resources/mode"); NativeRangedCalledShot(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-tohit-awareness")) { Require(argc==5,"tohit awareness requires db/resources/mode"); NativeToHitAwareness(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-tohit-night")) { Require(argc==5,"tohit night requires db/resources/mode"); NativeToHitNight(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-world")) { Require(argc==5,"knife world requires db/resources/mode"); NativeKnifeWorldContext(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-direction")) { Require(argc==5,"knife direction requires db/resources/mode"); NativeKnifeImpactDirection(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-launch-target")) { Require(argc==5,"knife launch target requires db/resources/mode"); NativeKnifeLaunchTarget(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-called-shot")) { Require(argc==5,"knife called shot requires db/resources/mode"); NativeKnifeCalledShot(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-chance-truncation")) { Require(argc==5,"knife truncation requires db/resources/mode"); NativeKnifeChanceTruncation(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-ballistic-range")) { Require(argc==5,"knife ballistic range requires db/resources/mode"); NativeKnifeBallisticRange(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-accuracy-range")) { Require(argc==5,"knife accuracy range requires db/resources/mode"); NativeKnifeAccuracyRange(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-launch-speed")) { Require(argc==5,"knife launch speed requires db/resources/mode"); NativeKnifeLaunchSpeed(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-fractional-range")) { Require(argc==5,"knife fractional range requires db/resources/mode"); NativeKnifeFractionalRange(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-knife-skill-context")) { Require(argc==5,"knife skill context requires db/resources/mode"); NativeKnifeSkillContext(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-db-overlay-sentinel")) { Require(argc==4,"DB sentinel requires db/mode"); NativeDBOverlaySentinel(argv[2],std::atoi(argv[3])); return true; }
    if (!std::strcmp(mode, "native-critical-probability-precision")) { Require(argc==5,"critical probability precision requires db/resources/mode"); NativeCriticalProbabilityPrecision(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-forced")) { Require(argc==5,"forced critical requires db/resources/mode"); NativeCriticalForced(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-cap-order")) { Require(argc==5,"critical cap requires db/resources/mode"); NativeCriticalCapOrder(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-damage-factor")) { Require(argc==5,"critical factor requires db/resources/mode"); NativeCriticalDamageFactor(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-critical-severity-draw")) { Require(argc==5,"severity draw requires db/resources/mode"); NativeCriticalSeverityDraw(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-backstab-awareness")) { Require(argc==5,"backstab awareness requires db/resources/mode"); NativeBackstabAwareness(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-melee-knife-critical")) { Require(argc==5,"knife critical requires db/resources/mode"); NativeMeleeKnifeCritical(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "equipped-melee-attack-cursor")) { Require(argc == 4, "equipped melee cursor requires db and res"); EquippedMeleeAttackCursor(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "melee-attack-cursor") && argc == 4) { MeleeAttackCursor(argv[2]); return true; }
    if (!std::strcmp(mode, "blocked-attack-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], false); return true; }
    if (!std::strcmp(mode, "reload-attack-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], true); return true; }
    if (!std::strcmp(mode, "exact-attack-preview-target") && argc == 4) { ExactAttackPreviewTarget(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "equal-group-attack-caption") && argc == 4) { GroupAttackPreview(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "mixed-group-attack-caption") && argc == 4) { GroupAttackPreview(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "missing-attack-target-caption") && argc == 4) { AttackCaptionWithoutHitChance(argv[2], false, true); return true; }
    if (!std::strcmp(mode, "pk-damage-resistance-gate") && argc==3) { PKDamageRouting(argv[2],12); return true; }
    if (!std::strcmp(mode, "pk-damage-penetrating-critical") && argc==3) { PKDamageRouting(argv[2],13); return true; }
    if (!std::strcmp(mode, "pk-world-stopped") && argc==4) { PKWorldDamageRouting(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "pk-world-bypass") && argc==4) { PKWorldDamageRouting(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "pk-world-rejected") && argc==4) { PKWorldDamageRouting(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "pk-world-penetrating") && argc==4) { PKWorldDamageRouting(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "pk-world-zero") && argc==4) { PKWorldDamageRouting(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "pk-damage-stopped") && argc==3) { PKDamageRouting(argv[2],0); return true; }
    if (!std::strcmp(mode, "pk-damage-penetrating") && argc==3) { PKDamageRouting(argv[2],1); return true; }
    if (!std::strcmp(mode, "pk-damage-bypass") && argc==3) { PKDamageRouting(argv[2],2); return true; }
    if (!std::strcmp(mode, "pk-damage-critical") && argc==3) { PKDamageRouting(argv[2],3); return true; }
    if (!std::strcmp(mode, "pk-damage-zero") && argc==3) { PKDamageRouting(argv[2],4); return true; }
    if (!std::strcmp(mode, "pk-damage-overkill") && argc==3) { PKDamageRouting(argv[2],5); return true; }
    if (!std::strcmp(mode, "pk-damage-godmode") && argc==3) { PKDamageRouting(argv[2],6); return true; }
    if (!std::strcmp(mode, "pk-damage-broken") && argc==3) { PKDamageRouting(argv[2],7); return true; }
    if (!std::strcmp(mode, "pk-damage-empty") && argc==3) { PKDamageRouting(argv[2],8); return true; }
    if (!std::strcmp(mode, "pk-damage-rejected") && argc==3) { PKDamageRouting(argv[2],9); return true; }
    if (!std::strcmp(mode, "pk-damage-human") && argc==3) { PKDamageRouting(argv[2],10); return true; }
    if (!std::strcmp(mode, "pk-damage-human-godmode") && argc==3) { PKDamageRouting(argv[2],11); return true; }
    if (!std::strcmp(mode, "no-manufactured-bleed-blocked") && argc==4) { NoManufacturedBleeding(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "no-manufactured-bleed-hit") && argc==4) { NoManufacturedBleeding(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "bleeding-readout") && argc==4) { BleedingReadout(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "native-direct-attack-xp")) { Require(argc==5,"direct attack XP requires db/resources/mode"); NativeDirectAttackXP(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "native-fatal-bleeding-xp")) { Require(argc==5,"fatal bleeding XP requires db/resources/mode"); NativeFatalBleedingXP(argv[2],argv[3],std::atoi(argv[4])); return true; }
    if (!std::strcmp(mode, "periodic-bleed-fractional") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "periodic-bleed-whole") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "periodic-bleed-low-nine") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "periodic-bleed-low-ten") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "periodic-bleed-truncation") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "periodic-bleed-stopped") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "periodic-bleed-old-event") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "periodic-bleed-replica") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "periodic-bleed-realtime") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "periodic-bleed-clue") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "periodic-bleed-unconscious") && argc==4) { PeriodicBleedingPipeline(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "ray-first-turn-zero") && argc==4) { RayShooterFirstTurn(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ray-first-turn-one") && argc==4) { RayShooterFirstTurn(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ray-first-turn-later") && argc==4) { RayShooterFirstTurn(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ray-first-turn-no-shooter") && argc==4) { RayShooterFirstTurn(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "critical-lifetime-refresh") && argc==4) { PeriodicCriticalLifetime(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "critical-lifetime-finish") && argc==4) { PeriodicCriticalLifetime(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "critical-lifetime-realtime") && argc==4) { PeriodicCriticalLifetime(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "critical-lifetime-permanent") && argc==4) { PeriodicCriticalLifetime(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "critical-lifetime-suspended") && argc==4) { PeriodicCriticalLifetime(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "periodic-clock-thresholds") && argc==4) { PeriodicUnitClocks(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "periodic-clock-independence") && argc==4) { PeriodicUnitClocks(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "fractional-bleeding-no-hit") && argc==4) { PeriodicBleedingAttack(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "integer-bleeding-hit") && argc==4) { PeriodicBleedingAttack(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "loose-ray-ricochet") && argc==4) { LooseRayRicochet(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "loose-ray-probability-zero") && argc==4) { LooseRayRicochet(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "loose-ray-normal-incidence") && argc==4) { LooseRayRicochet(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "loose-ray-probability-equality") && argc==4) { LooseRayRicochet(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "loose-ray-reflected-clear") && argc==4) { LooseRayRicochet(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "loose-ray-stop") && argc==4) { LooseRayTermination(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "loose-ray-terminal-energy") && argc==4) { LooseRayTermination(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-fragment") && argc==4) { LooseRayIncidental(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-certain") && argc==4) { LooseRayIncidental(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-zero-miss") && argc==4) { LooseRayIncidental(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-zero-equality") && argc==4) { LooseRayIncidental(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-threshold-equality") && argc==4) { LooseRayIncidental(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-threshold-miss") && argc==4) { LooseRayIncidental(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-intended") && argc==4) { LooseRayIncidental(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-ignored") && argc==4) { LooseRayIncidental(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-repeated") && argc==4) { LooseRayIncidental(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-reflected") && argc==4) { LooseRayIncidental(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-obstacle") && argc==4) { LooseRayIncidental(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-intended-obstacle") && argc==4) { LooseRayIncidental(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-world-damage") && argc==4) { LooseRayIncidental(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-day") && argc==4) { LooseRayIncidental(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "loose-ray-incidental-night") && argc==4) { LooseRayIncidental(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "loose-ray-bounds") && argc==4) { LooseRayBounds(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "restore-action-state") && argc == 4) { RestoreActionState(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "zone-reentry-action-state") && argc == 4) { RestoreActionState(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "restore-building-hulls") && argc == 4) { RestoreBuildingHulls(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "carried-corpse-stability") && argc == 4) { CarriedCorpseStability(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "replacement-person-inventory-clue") && argc == 4) { PersonClueCreation(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "inventory-ui-0") && argc==4) { InventoryNoPlaceUI(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "inventory-ui-1") && argc==4) { InventoryNoPlaceUI(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "inventory-ui-2") && argc==4) { InventoryNoPlaceUI(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "inventory-ui-3") && argc==4) { InventoryNoPlaceUI(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "inventory-ui-4") && argc==4) { InventoryNoPlaceUI(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "inventory-ui-5") && argc==4) { InventoryNoPlaceUI(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "corpse-snap-failure") && argc == 4) { FailedCorpseSnap(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "movement-critical-probe") && argc == 4) { MovementCriticalProbe(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "latest-route-command") && argc == 4) { LatestRouteCommand(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "oversized-dialogue-line") && argc == 4) { OversizedDialogueLine(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "markup-dialogue-progress") && argc == 4) { OversizedDialogueLine(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "markup-leading-whitespace") && argc == 4) { MarkupLeadingWhitespace(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "convenience-markup-symbols") && argc == 4) { MarkupConvenienceSymbols(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "dialogue-formatted-pagination") && (argc == 4 || argc == 5)) { OversizedDialogueLine(argv[2], argv[3], 3, argc==5?std::strtof(argv[4],nullptr):1); return true; }
    if (!std::strcmp(mode, "dialogue-normal-pagination") && argc == 4) { OversizedDialogueLine(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "forced-move-item-eligibility") && argc==4) { ForcedMoveItemEligibility(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "interrupted-movement-route-save") && argc==4) { InterruptedMovementRoute(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "interrupted-movement-route") && argc==4) { InterruptedMovementRoute(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "movement-full-cancel-action") && argc==4) { MovementFullCancelAction(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "movement-first-step-cancel") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "movement-next-step-cancel") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "movement-first-step-release") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 1, true); return true; }
    if (!std::strcmp(mode, "movement-next-step-release") && argc == 4) { MovementCallbackCancellation(argv[2], argv[3], 2, true); return true; }
    if (!std::strcmp(mode, "hide-cooldown") && argc == 4) { HideCooldown(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "explicit-hide-state") && argc == 4) { ExplicitHideState(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "group-hide-shortcut") && argc == 4) { GroupHideShortcut(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "script-hide-state") && argc == 4) { ScriptHideState(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "thrown-knife-orientation") && argc == 4) { ThrownKnifeOrientation(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-idle") && argc == 4) { IdleAI(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "ai-roaming-current-position") && argc == 4) { RoamingUsesCurrentPosition(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "mixed-hide-button") && argc == 4) { MixedActionButton(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "mixed-strafe-button") && argc == 4) { MixedActionButton(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "mounted-mode-button") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], 3); return true; }
    if (!std::strcmp(mode, "mounted-weapon-submenu") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "mounted-weapon-mode-ui") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "effective-weapon-mode") && argc == 4) { EffectiveWeaponMode(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "render-removal")) { RenderRemoval(); return true; }
    if (!std::strcmp(mode, "render-pass-compaction")) { RenderPassCompaction(); return true; }
    if (!std::strcmp(mode, "state-lifetime")) { ReentrantStateLifetime(); return true; }
    if (!std::strcmp(mode, "mounted-mode-command")) { MountedModeCommand(); return true; }
    if (!std::strcmp(mode, "invalid-ai-weapon")) { InvalidAIWeapon(); return true; }
    if (!std::strcmp(mode, "stale-cannon-selection")) { StaleWeaponReferences(0); return true; }
    if (!std::strcmp(mode, "stale-firearm-type")) { StaleWeaponReferences(1); return true; }
    if (!std::strcmp(mode, "stale-melee-type")) { StaleWeaponReferences(2); return true; }
    if (!std::strcmp(mode, "stale-ai-reaction")) { StaleAIReaction(); return true; }
    if (!std::strcmp(mode, "overlapping-unit-locks")) { OverlappingUnitLocks(); return true; }
    if (!std::strcmp(mode, "invalid-unit-locks")) { InvalidUnitLocks(); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-single") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 0); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-moving") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 1); return true; }
    if (!std::strcmp(mode, "dead-unit-locks-selection") && argc == 4) { DeadUnitLocks(argv[2], argv[3], 2); return true; }
    if (!std::strcmp(mode, "queue-lifetime")) { ReentrantQueueLifetime(false); return true; }
    if (!std::strcmp(mode, "queue-animation-lifetime")) { ReentrantQueueLifetime(true); return true; }
    if (!std::strcmp(mode, "pk-leave")) { ScriptPK(true); return true; }
    if (!std::strcmp(mode, "pk-wear")) { ScriptPK(false); return true; }
    if (!std::strcmp(mode, "pk-equipment")) { PKEquipmentTransfer(); return true; }
    if (!std::strcmp(mode, "pk-fixed-weapon")) { PKFixedWeaponPolicy(); return true; }
    if (!std::strcmp(mode, "door-collision")) { DoorCollision(); return true; }
    if (!std::strcmp(mode, "door-swing-direction") && argc == 4) { DoorSwingDirection(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "door-approach-heading") && argc == 4) { DoorApproachHeading(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "door-path-stop") && argc == 4) { DoorPathStop(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-animation-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-direction-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Direction); return true; }
    if (!std::strcmp(mode, "deferred-pose-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Pose); return true; }
    if (!std::strcmp(mode, "deferred-wish-pose-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::WishPose); return true; }
    if (!std::strcmp(mode, "deferred-shoot-mode-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::ShootMode); return true; }
    if (!std::strcmp(mode, "deferred-reload-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Reload); return true; }
    if (!std::strcmp(mode, "deferred-weapon-activation-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::ActivateWeapon); return true; }
    if (!std::strcmp(mode, "deferred-hide-command") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::Hide); return true; }
    if (!std::strcmp(mode, "deferred-create-activate-command") && argc == 4) { DeferredCreateAndActivateItem(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-take-object-command") && argc == 4) { DeferredTakeObject(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-shoot-command") && argc == 4) { DeferredScriptShot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-shoot-prepare-command") && argc == 4) { DeferredScriptShot(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "deferred-fly-waypoint-command") && argc == 4) { DeferredWaypointCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-attack-waypoint-command") && argc == 4) { DeferredWaypointCommand(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "deferred-take-corpse-command") && argc == 4) { DeferredCorpseCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-drop-corpse-command") && argc == 4) { DeferredCorpseCommand(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "deferred-leave-pk-command") && argc == 4) { DeferredPKCommand(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-wear-pk-command") && argc == 4) { DeferredPKCommand(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "empty-pk-route-lookup") && argc == 4) { DeferredPKCommand(argv[2],argv[3],false,true); return true; }
    if (!std::strcmp(mode, "deferred-script-command-order") && argc == 4) { DeferredUnitScriptCommand(argv[2],argv[3],DeferredScriptProducer::ShootMode,true); return true; }
    if (!std::strcmp(mode, "script-draw-weapon-slot") && argc == 4) { ScriptDrawWeaponSlot(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "deferred-draw-weapon-command") && argc == 4) { ScriptDrawWeaponSlot(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "deferred-draw-grenade-command") && argc == 4) { ScriptDrawWeaponSlot(argv[2],argv[3],true,true); return true; }
    if (!std::strcmp(mode, "script-held-grenade-activation") && argc == 4) { ScriptHeldGrenadeActivation(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "forced-action-recalc") && argc == 4) { ForcedActionRecalc(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "overlapping-doors")) { OverlappingDoors(); return true; }
    if (!std::strcmp(mode, "ai-selected-reload-availability") && argc==4) { AISelectedReloadActionCost(argv[2],argv[3],false); return true; }
    if (!std::strcmp(mode, "ai-selected-reload-spend") && argc==4) { AISelectedReloadActionCost(argv[2],argv[3],true); return true; }
    if (!std::strcmp(mode, "retail-reload-action-cost") && argc==3) { RetailReloadActionCost(argv[2]); return true; }
    if (!std::strcmp(mode, "global-game-difficulty") && argc==3) { GlobalGameDifficulty(argv[2]); return true; }
    if (!std::strcmp(mode, "weapon-familiarity")) { WeaponFamiliarity(); return true; }
    if (!std::strcmp(mode, "clue-slots")) { ClueSlotSelection(); return true; }
    if (!std::strcmp(mode, "conclusions") && argc == 4) { ConclusionRouting(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "pk-placement")) { PKPlacement(); return true; }
    if (!std::strcmp(mode, "critical-ids") && argc == 3) { CriticalIDs(argv[2]); return true; }
    if (!std::strcmp(mode, "corpse-target-information") && argc==4) { CorpseTargetInformation(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "mounted-gun-eligibility") && argc==4) { MountedGunEligibility(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "unit-diplomacy-identity") && argc==4) { UnitDiplomacyIdentity(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "cover-perk-multiplier") && argc==4) { CoverPerkMultiplier(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "moving-target-perk") && argc==4) { MovingTargetPerk(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "attacker-level-critical-rolls") && argc==3) { AttackerLevelCriticalRolls(argv[2]); return true; }
    if (!std::strcmp(mode, "head-body-damage") && argc==3) { HeadBodyDamage(argv[2]); return true; }
    if (!std::strcmp(mode, "fresh-skill-modifier-values") && argc==3) { FreshSkillModifierValues(argv[2]); return true; }
    if (!std::strcmp(mode, "repair-order-empty") && argc==4) { RepairOrderGate(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "repair-order-worn") && argc==4) { RepairOrderGate(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "repair-order-healthy") && argc==4) { RepairOrderGate(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "repair-order-far") && argc==4) { RepairOrderGate(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "repair-order-inner") && argc==4) { RepairOrderGate(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "repair-order-low-skill") && argc==4) { RepairOrderGate(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "repair-order-missing-perk") && argc==4) { RepairOrderGate(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "repair-order-missing-kit") && argc==4) { RepairOrderGate(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "repair-order-wrong-target") && argc==4) { RepairOrderGate(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "repair-order-no-target") && argc==4) { RepairOrderGate(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "repair-order-healthy-worn") && argc==4) { RepairOrderGate(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "repair-order-hostile") && argc==4) { RepairOrderGate(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "repair-order-exhausted") && argc==4) { RepairOrderGate(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "repair-order-medicine-on-pk") && argc==4) { RepairOrderGate(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "repair-order-final") && argc==4) { RepairOrderGate(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "repair-order-non-integral") && argc==4) { RepairOrderGate(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "repair-order-inactive") && argc==4) { RepairOrderGate(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "repair-preparation-dedicated") && argc==4) { RepairOrderGate(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "repair-preparation-ordinary") && argc==4) { RepairOrderGate(argv[2],argv[3],18); return true; }
    if (!std::strcmp(mode, "repair-preparation-medical-control") && argc==4) { RepairOrderGate(argv[2],argv[3],19); return true; }
    if (!std::strcmp(mode, "repair-action-empty") && argc==4) { RepairOrderGate(argv[2],argv[3],0,0); return true; }
    if (!std::strcmp(mode, "repair-action-worn") && argc==4) { RepairOrderGate(argv[2],argv[3],1,1); return true; }
    if (!std::strcmp(mode, "repair-action-small-empty") && argc==4) { RepairOrderGate(argv[2],argv[3],0,2); return true; }
    if (!std::strcmp(mode, "repair-action-small-worn") && argc==4) { RepairOrderGate(argv[2],argv[3],1,3); return true; }
    if (!std::strcmp(mode, "repair-action-budget-empty") && argc==4) { RepairOrderGate(argv[2],argv[3],0,4); return true; }
    if (!std::strcmp(mode, "repair-action-budget-worn") && argc==4) { RepairOrderGate(argv[2],argv[3],1,5); return true; }
    if (!std::strcmp(mode, "repair-action-ordinary-kit") && argc==4) { RepairOrderGate(argv[2],argv[3],0,6); return true; }
    if (!std::strcmp(mode, "repair-action-healthy-in-flight") && argc==4) { RepairOrderGate(argv[2],argv[3],0,7); return true; }
    if (!std::strcmp(mode, "repair-action-missing-in-flight") && argc==4) { RepairOrderGate(argv[2],argv[3],0,8); return true; }
    if (!std::strcmp(mode, "repair-finish-dedicated-empty") && argc==4) { RepairOrderGate(argv[2],argv[3],0,0,0); return true; }
    if (!std::strcmp(mode, "repair-finish-dedicated-worn") && argc==4) { RepairOrderGate(argv[2],argv[3],1,1,1); return true; }
    if (!std::strcmp(mode, "repair-finish-ordinary") && argc==4) { RepairOrderGate(argv[2],argv[3],0,6,2); return true; }
    if (!std::strcmp(mode, "repair-finish-cancel-dedicated") && argc==4) { RepairOrderGate(argv[2],argv[3],0,4,3); return true; }
    if (!std::strcmp(mode, "repair-finish-cancel-ordinary") && argc==4) { RepairOrderGate(argv[2],argv[3],0,6,4); return true; }
    if (!std::strcmp(mode, "repair-finish-last-dedicated") && argc==4) { RepairOrderGate(argv[2],argv[3],0,0,5); return true; }
    if (!std::strcmp(mode, "repair-finish-last-ordinary") && argc==4) { RepairOrderGate(argv[2],argv[3],0,6,6); return true; }
    if (!std::strcmp(mode, "wound-healer-perk-small") && argc==4) { WoundHealerPerk(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "wound-healer-perk-budget") && argc==4) { WoundHealerPerk(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "wound-healer-perk-without-small") && argc==4) { WoundHealerPerk(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "wound-healer-perk-without-budget") && argc==4) { WoundHealerPerk(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "wound-healer-perk-partly-dressed") && argc==4) { WoundHealerPerk(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-small") && argc==4) { WoundHealerPerk(argv[2],argv[3],0,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-budget") && argc==4) { WoundHealerPerk(argv[2],argv[3],1,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-without-small") && argc==4) { WoundHealerPerk(argv[2],argv[3],2,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-without-budget") && argc==4) { WoundHealerPerk(argv[2],argv[3],3,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-partly-dressed") && argc==4) { WoundHealerPerk(argv[2],argv[3],4,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-critical-first-control") && argc==4) { WoundHealerPerk(argv[2],argv[3],5,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-combined-strength") && argc==4) { WoundHealerPerk(argv[2],argv[3],6,true); return true; }
    if (!std::strcmp(mode, "wound-speed-perk-free-small-control") && argc==4) { WoundHealerPerk(argv[2],argv[3],7,true); return true; }
    if (!std::strcmp(mode, "treatment-ack-above-half") && argc==4) { TreatmentResultAck(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "treatment-ack-exact-half") && argc==4) { TreatmentResultAck(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "treatment-ack-below-half") && argc==4) { TreatmentResultAck(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "treatment-ack-spent-patient-ap") && argc==4) { TreatmentResultAck(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "treatment-ack-self") && argc==4) { TreatmentResultAck(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "treatment-ack-prior-dressing") && argc==4) { TreatmentResultAck(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "treatment-ack-critical-pending") && argc==4) { TreatmentResultAck(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "treatment-ack-critical-removed") && argc==4) { TreatmentResultAck(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "treatment-ack-critical-complete") && argc==4) { TreatmentResultAck(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "treatment-ack-repair-above-half") && argc==4) { RepairOrderGate(argv[2],argv[3],1,1,-1,0); return true; }
    if (!std::strcmp(mode, "treatment-ack-repair-exact-half") && argc==4) { RepairOrderGate(argv[2],argv[3],1,1,-1,1); return true; }
    if (!std::strcmp(mode, "treatment-ack-repair-below-half") && argc==4) { RepairOrderGate(argv[2],argv[3],1,1,-1,2); return true; }
    if (!std::strcmp(mode, "treatment-ack-repair-spent-pilot-ap") && argc==4) { RepairOrderGate(argv[2],argv[3],1,1,-1,3); return true; }
    if (!std::strcmp(mode, "treatment-ack-repair-empty-silent") && argc==4) { RepairOrderGate(argv[2],argv[3],0,0,-1,4); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-grenade") && argc==4) { NonUniformAttachment(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-gun") && argc==4) { NonUniformAttachment(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-heavy") && argc==4) { NonUniformAttachment(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-carried-heavy") && argc==4) { NonUniformAttachment(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-hidden") && argc==4) { NonUniformAttachment(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-inactive") && argc==4) { NonUniformAttachment(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-empty") && argc==4) { NonUniformAttachment(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-uniform-grenade") && argc==4) { NonUniformAttachment(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-uniform-gun") && argc==4) { NonUniformAttachment(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-uniform-carried-heavy") && argc==4) { NonUniformAttachment(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-uniform-hidden") && argc==4) { NonUniformAttachment(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-detach-grenade") && argc==4) { NonUniformAttachment(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-uniform-detach-grenade") && argc==4) { NonUniformAttachment(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-detach-gun") && argc==4) { NonUniformAttachment(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-detach-empty") && argc==4) { NonUniformAttachment(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "nonuniform-attachment-uniform-empty") && argc==4) { NonUniformAttachment(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-0") && argc==4) { PKRepairApproach(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-1") && argc==4) { PKRepairApproach(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-2") && argc==4) { PKRepairApproach(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-3") && argc==4) { PKRepairApproach(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-4") && argc==4) { PKRepairApproach(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-5") && argc==4) { PKRepairApproach(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-6") && argc==4) { PKRepairApproach(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-7") && argc==4) { PKRepairApproach(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-0") && argc==4) { PKRepairApproach(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-1") && argc==4) { PKRepairApproach(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-2") && argc==4) { PKRepairApproach(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-3") && argc==4) { PKRepairApproach(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-4") && argc==4) { PKRepairApproach(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-5") && argc==4) { PKRepairApproach(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-6") && argc==4) { PKRepairApproach(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-worn-7") && argc==4) { PKRepairApproach(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-full-empty") && argc==4) { PKRepairApproach(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-full-worn") && argc==4) { PKRepairApproach(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-wrong-kit") && argc==4) { PKRepairApproach(argv[2],argv[3],18); return true; }
    if (!std::strcmp(mode, "pk-repair-approach-empty-kit") && argc==4) { PKRepairApproach(argv[2],argv[3],19); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-standing-near") && argc==4) { ProneMedicalApproach(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-standing-wide") && argc==4) { ProneMedicalApproach(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-prone-near") && argc==4) { ProneMedicalApproach(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-prone-wide") && argc==4) { ProneMedicalApproach(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-prone-far") && argc==4) { ProneMedicalApproach(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-crouch-wide") && argc==4) { ProneMedicalApproach(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-qualified-prone-wide") && argc==4) { ProneMedicalApproach(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-prone-medic-standing") && argc==4) { ProneMedicalApproach(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "prone-medical-approach-both-prone") && argc==4) { ProneMedicalApproach(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-backpack-below") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-backpack-exact") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-backpack-above") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-held-below") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-held-exact") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-backpack-spend") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-held-spend") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-medical-cost-free-zero-budget") && argc==4) { AISelectedMedicalCost(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-backpack-current") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-active-current") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-active-planned") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-backpack-planned") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-healthy") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-missing-kit") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-empty-kit") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-planned-fly") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "ai-medical-probe-planned-inactive") && argc==4) { AISelectedMedicalProbe(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-standing-near") && argc==4) { ProneMedicalReach(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-standing-wide") && argc==4) { ProneMedicalReach(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-prone-near") && argc==4) { ProneMedicalReach(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-prone-wide") && argc==4) { ProneMedicalReach(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-prone-far") && argc==4) { ProneMedicalReach(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-crouched-wide") && argc==4) { ProneMedicalReach(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-perk58-prone-wide") && argc==4) { ProneMedicalReach(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-prone-medic-standing-patient") && argc==4) { ProneMedicalReach(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "prone-medical-reach-both-prone-wide") && argc==4) { ProneMedicalReach(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "medical-pose-stand") && argc==4) { MedicalOrderPose(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "medical-pose-crouch") && argc==4) { MedicalOrderPose(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "medical-pose-prone") && argc==4) { MedicalOrderPose(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "medical-pose-fly") && argc==4) { MedicalOrderPose(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "medical-pose-inactive") && argc==4) { MedicalOrderPose(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "medical-pose-ladder") && argc==4) { MedicalOrderPose(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "medical-pose-perk58-fly") && argc==4) { MedicalOrderPose(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "medical-pose-perk58-inactive") && argc==4) { MedicalOrderPose(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "medical-pose-perk58-ladder") && argc==4) { MedicalOrderPose(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "medical-pose-perk58-stand") && argc==4) { MedicalOrderPose(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "medical-requirements-low-skill") && argc==4) { MedicalRequirements(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "medical-requirements-skill-equality") && argc==4) { MedicalRequirements(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "medical-requirements-modifier-equality") && argc==4) { MedicalRequirements(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "medical-requirements-bandage-equality") && argc==4) { MedicalRequirements(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "medical-requirements-required-perk") && argc==4) { MedicalRequirements(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "medical-requirements-perk58-bypass") && argc==4) { MedicalRequirements(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "medical-requirements-perk58-repair") && argc==4) { MedicalRequirements(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "medical-requirements-empty-precedence") && argc==4) { MedicalRequirements(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "medical-requirements-missing-kit") && argc==4) { MedicalRequirements(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "medical-requirements-unrestricted-kit") && argc==4) { MedicalRequirements(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "medical-requirements-low-with-modifier") && argc==4) { MedicalRequirements(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-healthy") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-curable") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-noncurable") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-wound") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "medical-eligibility-critical-first-healthy") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "medical-eligibility-critical-first-curable") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "medical-eligibility-critical-first-noncurable") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "medical-eligibility-penalty-healthy") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "medical-eligibility-penalty-noncurable") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "medical-eligibility-penalty-curable") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "medical-eligibility-blood-stop-healthy") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "medical-eligibility-blood-stop-other-critical") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "medical-eligibility-blood-stop-bleeding") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "medical-eligibility-boost-preventive") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "medical-eligibility-temp-stop-preventive") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "medical-eligibility-wound-kit-modifier") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "medical-eligibility-empty-kit") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "medical-eligibility-missing-kit") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "medical-eligibility-hostile") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],18); return true; }
    if (!std::strcmp(mode, "medical-eligibility-wound-quota-equality") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],19); return true; }
    if (!std::strcmp(mode, "medical-eligibility-penalty-wound-only") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],20); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-hard-critical") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],21); return true; }
    if (!std::strcmp(mode, "medical-eligibility-penalty-suspended") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],22); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-mixed-criticals") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],23); return true; }
    if (!std::strcmp(mode, "medical-eligibility-normal-fully-dressed") && argc==4) { MedicalEffectEligibility(argv[2],argv[3],24); return true; }
    if (!std::strcmp(mode, "repair-planner-small") && argc==4) { RepairPlanner(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "repair-planner-full") && argc==4) { RepairPlanner(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "repair-planner-limited-ap") && argc==4) { RepairPlanner(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "repair-planner-healthy") && argc==4) { RepairPlanner(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "repair-planner-capacity") && argc==4) { RepairPlanner(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "repair-kind-small") && argc==4) { RepairKindImport(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "repair-kind-large") && argc==4) { RepairKindImport(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "repair-kind-controls") && argc==4) { RepairKindImport(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "critical-first-import") && argc==4) { CriticalFirstImport(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "critical-first-priority") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],25); return true; }
    if (!std::strcmp(mode, "critical-first-fallback") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],26); return true; }
    if (!std::strcmp(mode, "critical-first-normal-control") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],27); return true; }
    if (!std::strcmp(mode, "patient-dc-perk-extended") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],20); return true; }
    if (!std::strcmp(mode, "patient-dc-perk-without-perk") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],21); return true; }
    if (!std::strcmp(mode, "patient-dc-perk-too-hard") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],22); return true; }
    if (!std::strcmp(mode, "patient-dc-perk-legacy-limit") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],23); return true; }
    if (!std::strcmp(mode, "patient-dc-perk-exact-limit") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],24); return true; }
    if (!std::strcmp(mode, "critical-dc-perk-extended") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],15); return true; }
    if (!std::strcmp(mode, "critical-dc-perk-without-perk") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],16); return true; }
    if (!std::strcmp(mode, "critical-dc-perk-too-hard") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],17); return true; }
    if (!std::strcmp(mode, "critical-dc-perk-legacy-limit") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],18); return true; }
    if (!std::strcmp(mode, "critical-dc-perk-exact-limit") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],19); return true; }
    if (!std::strcmp(mode, "timed-critical-long") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "timed-critical-saved") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "timed-critical-short") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "timed-critical-too-hard") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "timed-critical-no-critical") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "timed-critical-fast-perk") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "timed-critical-missing-kit") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "timed-critical-cancel") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "timed-critical-wounds-first") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "timed-critical-normal-fallback") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "timed-critical-bleeding-perk") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "timed-critical-bleeding-gate") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "timed-critical-other-location") && argc==4) { TimedCriticalTreatment(argv[2],argv[3],14); return true; }
    if (!std::strcmp(mode, "critical-heal-award-one") && argc==4) { HealingMedalAwards(argv[2],argv[3],0,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-two-locations") && argc==4) { HealingMedalAwards(argv[2],argv[3],1,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-self") && argc==4) { HealingMedalAwards(argv[2],argv[3],2,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-no-bleeding") && argc==4) { HealingMedalAwards(argv[2],argv[3],3,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-missing-kit") && argc==4) { HealingMedalAwards(argv[2],argv[3],4,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-hostile") && argc==4) { HealingMedalAwards(argv[2],argv[3],5,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-disabled-npc") && argc==4) { HealingMedalAwards(argv[2],argv[3],6,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-both-actors") && argc==4) { HealingMedalAwards(argv[2],argv[3],7,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-zero-dc") && argc==4) { HealingMedalAwards(argv[2],argv[3],8,true); return true; }
    if (!std::strcmp(mode, "critical-heal-award-other-critical") && argc==4) { HealingMedalAwards(argv[2],argv[3],9,true); return true; }
    if (!std::strcmp(mode, "wound-patient-perk-small") && argc==4) { HealingMedalAwards(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "wound-patient-perk-large") && argc==4) { HealingMedalAwards(argv[2],argv[3],10); return true; }
    if (!std::strcmp(mode, "wound-patient-perk-without-small") && argc==4) { HealingMedalAwards(argv[2],argv[3],11); return true; }
    if (!std::strcmp(mode, "wound-patient-perk-without-large") && argc==4) { HealingMedalAwards(argv[2],argv[3],12); return true; }
    if (!std::strcmp(mode, "wound-patient-perk-partly-dressed") && argc==4) { HealingMedalAwards(argv[2],argv[3],13); return true; }
    if (!std::strcmp(mode, "authored-capacity-small") && argc==4) { HealingMedalAwards(argv[2],argv[3],8,false,false,false,1); return true; }
    if (!std::strcmp(mode, "authored-capacity-bandage") && argc==4) { HealingMedalAwards(argv[2],argv[3],8,false,false,false,6); return true; }
    if (!std::strcmp(mode, "wound-capacity-plan") && argc==4) { WoundCapacityPlan(argv[2],argv[3]); return true; }
    if (!std::strcmp(mode, "wound-complete-one") && argc==4) { HealingMedalAwards(argv[2],argv[3],0,false,false,true); return true; }
    if (!std::strcmp(mode, "wound-complete-several") && argc==4) { HealingMedalAwards(argv[2],argv[3],1,false,false,true); return true; }
    if (!std::strcmp(mode, "wound-complete-self") && argc==4) { HealingMedalAwards(argv[2],argv[3],2,false,false,true); return true; }
    if (!std::strcmp(mode, "wound-complete-already-dressed") && argc==4) { HealingMedalAwards(argv[2],argv[3],3,false,false,true); return true; }
    if (!std::strcmp(mode, "wound-complete-partly-dressed") && argc==4) { HealingMedalAwards(argv[2],argv[3],7,false,false,true); return true; }
    if (!std::strcmp(mode, "wound-complete-pending") && argc==4) { HealingMedalAwards(argv[2],argv[3],8,false,false,true); return true; }
    if (!std::strcmp(mode, "wound-budget-one") && argc==4) { HealingMedalAwards(argv[2],argv[3],0,false,true); return true; }
    if (!std::strcmp(mode, "wound-budget-several") && argc==4) { HealingMedalAwards(argv[2],argv[3],1,false,true); return true; }
    if (!std::strcmp(mode, "wound-budget-self") && argc==4) { HealingMedalAwards(argv[2],argv[3],2,false,true); return true; }
    if (!std::strcmp(mode, "wound-budget-already-dressed") && argc==4) { HealingMedalAwards(argv[2],argv[3],3,false,true); return true; }
    if (!std::strcmp(mode, "wound-budget-partly-dressed") && argc==4) { HealingMedalAwards(argv[2],argv[3],7,false,true); return true; }
    if (!std::strcmp(mode, "healing-wound-award-one") && argc==4) { HealingMedalAwards(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "healing-wound-award-several") && argc==4) { HealingMedalAwards(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "healing-wound-award-self") && argc==4) { HealingMedalAwards(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "healing-wound-award-zero") && argc==4) { HealingMedalAwards(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "healing-wound-award-missing-kit") && argc==4) { HealingMedalAwards(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "healing-wound-award-hostile") && argc==4) { HealingMedalAwards(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "healing-wound-award-disabled-npc") && argc==4) { HealingMedalAwards(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "healing-wound-award-clamped") && argc==4) { HealingMedalAwards(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "penalty-treatment-recipient") && argc==4) { PenaltyTreatment(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "penalty-treatment-patient-only") && argc==4) { PenaltyTreatment(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "penalty-treatment-self") && argc==4) { PenaltyTreatment(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "penalty-treatment-expiry") && argc==4) { PenaltyTreatment(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "penalty-treatment-missing-kit") && argc==4) { PenaltyTreatment(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "penalty-treatment-hostile") && argc==4) { PenaltyTreatment(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "penalty-treatment-weapon") && argc==4) { PenaltyTreatment(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "penalty-treatment-duration-perk") && argc==4) { PenaltyTreatment(argv[2],argv[3],7); return true; }
    if (!std::strcmp(mode, "penalty-treatment-saved") && argc==4) { PenaltyTreatment(argv[2],argv[3],8); return true; }
    if (!std::strcmp(mode, "penalty-treatment-no-critical") && argc==4) { PenaltyTreatment(argv[2],argv[3],9); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-both") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],0,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-patient-only") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],1,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-self") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],2,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-other-critical") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],3,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-missing-kit") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],4,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-hostile") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],5,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-weapon") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],6,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-repeat") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],7,true); return true; }
    if (!std::strcmp(mode, "temporary-bleeding-saved") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],8,true); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-both") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],0); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-patient-only") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],1); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-self") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],2); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-other-critical") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],3); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-missing-kit") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],4); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-hostile") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],5); return true; }
    if (!std::strcmp(mode, "bleeding-recipient-weapon") && argc==4) { BleedingTreatmentRecipient(argv[2],argv[3],6); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-no-bleeding-bypass") && argc==3) { CriticalCureGate(argv[2],7); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-no-bypass-high-dc") && argc==3) { CriticalCureGate(argv[2],8); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-wound-suspension") && argc==3) { CriticalCureGate(argv[2],0); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-patient") && argc==3) { CriticalCureGate(argv[2],1); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-bleeding") && argc==3) { CriticalCureGate(argv[2],2); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-eligible") && argc==3) { CriticalCureGate(argv[2],3); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-high-dc") && argc==3) { CriticalCureGate(argv[2],4); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-suspend-eligible") && argc==3) { CriticalCureGate(argv[2],5); return true; }
    if (!std::strcmp(mode, "critical-cure-gate-empty") && argc==3) { CriticalCureGate(argv[2],6); return true; }
    if (!std::strcmp(mode, "heal-wound-refresh-full") && argc==3) { HealWoundRefresh(argv[2],0); return true; }
    if (!std::strcmp(mode, "heal-wound-refresh-partial") && argc==3) { HealWoundRefresh(argv[2],1); return true; }
    if (!std::strcmp(mode, "heal-wound-refresh-after-criticals") && argc==3) { HealWoundRefresh(argv[2],2); return true; }
    if (!std::strcmp(mode, "heal-wound-refresh-zero") && argc==3) { HealWoundRefresh(argv[2],3); return true; }
    if (!std::strcmp(mode, "heal-wound-refresh-healthy") && argc==3) { HealWoundRefresh(argv[2],4); return true; }
    if (!std::strcmp(mode, "heal-wound-refresh-healthy-criticals") && argc==3) { HealWoundRefresh(argv[2],5); return true; }
    if (!std::strcmp(mode, "health-survival-threshold") && argc==3) { HealthSurvivalThreshold(argv[2]); return true; }
    if (!std::strcmp(mode, "empty-critical-application") && argc == 3) { EmptyCriticalApplication(argv[2]); return true; }
    if (!std::strcmp(mode, "idle-blocker") && argc == 4) { IdlePathBlocker(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "hidden-portrait")) { HiddenPortraitAck(); return true; }
    if (!std::strcmp(mode, "clue-initialization")) { ClueInitialization(); return true; }
    if (!std::strcmp(mode, "store-drop")) { StoreDropPosition(); return true; }
    if (!std::strcmp(mode, "inventory-actor")) { InventoryTransferActor(true); return true; }
    if (!std::strcmp(mode, "slot-actor")) { InventoryTransferActor(false); return true; }
    if (!std::strcmp(mode, "ownerless-inventory-drag")) { OwnerlessInventoryDrag(); return true; }
    if (!std::strcmp(mode, "geometry-hash")) { GeometryHashBuckets(); return true; }
    if (!std::strcmp(mode, "clue-notification") && argc == 4) { ImmediateClueNotification(argv[2], argv[3]); return true; }
    if (!std::strcmp(mode, "clue-pickup") && argc == 4) { ImmediateCluePickup(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "clue-capacity") && argc == 4) { ImmediateCluePickup(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "dead-pilot") && argc == 4) { PKShellWorld(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "pk-waypoint") && argc == 4) { PKShellWorld(argv[2], argv[3], false); return true; }
    if (!std::strcmp(mode, "pk-critical-feedback") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 1); return true; }
    if (!std::strcmp(mode, "pk-pose-feedback") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 2); return true; }
    if (!std::strcmp(mode, "human-sequence-hiding") && argc == 4) { HiddenUnitTint(argv[2], argv[3], true); return true; }
    if (!std::strcmp(mode, "pk-sequence-hiding") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 5); return true; }
    if (!std::strcmp(mode, "hidden-pk-material") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 4); return true; }
    if (!std::strcmp(mode, "pk-turn-animation") && argc == 4) { PKShellWorld(argv[2], argv[3], false, false, 3); return true; }
    if (!std::strcmp(mode, "pk-model-ownership") && argc == 4) { PKShellWorld(argv[2], argv[3], false, true); return true; }
    return false;
}
