# Перенос upstream исправлений

Репозиторий: `G:\SS\silent-storm-remaster`, `develop`.
Начальная ревизия: `1f3d5756`. Upstream snapshot:
`6facee6dbea4189b0d817f37d4f62f9318212152`; 281 коммит после общей точки.

Правило каждого независимого исправления: сначала поведенческий тест на
неисправленной production реализации, подтвердить ожидаемую причину RED,
затем фикс, GREEN и подходящие регрессии. Тест и фикс входят в один отдельный
коммит с полным `Upstream-Commit:` URL; после каждого коммита выполняется push.
Статус push проверяется по origin/develop. Ошибка сборки не считается RED.

Оригинальный баланс не меняется. Исключенные и требующие retail сверки части
описаны в [UPSTREAM-BALANCE-CHANGES.md](UPSTREAM-BALANCE-CHANGES.md).
Полный исходный список и текущие решения:
[upstream-import-status.tsv](upstream-import-status.tsv).
`pending` и `review-final-version` означают незавершенную работу, а не пропуск.

Сборка переиспользует `G:\SS\lab\build-x64-stage4`, конфигурация
`RelWithDebInfo`, исходники нового репозитория, `S2_BUILD_HD_TEXTURES=OFF`.
Готовые HD texture pack и PNG не генерируются. Копий игры и сборки на каждый
баг нет; сохраняются небольшие текстовые свидетельства RED/GREEN и необходимые
малые эталоны для проверки совместимости сохранений.

## Выполненные исправления

| Источник / исправление | RED | GREEN и проверка | Наш коммит |
|---|---|---|---|
| [8b4d7a8b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8b4d7a8bd8475d962cea881293a7c69a03fe0ef1) — Preserve retained render passes when splitting hidden-unit draws | `retained render passes overwrote lower passes` | 34/34; `render-pass-compaction` | `Upstream-Commit` в commit message |
| [dccaf48d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/dccaf48dce19c4e0064a7b7f48c817a7e85a2bd9) — Keep self-replacing unit states alive through startup callbacks | `self-replacing unit state was destroyed before its callback returned` | 35/35; `state-lifetime` | `Upstream-Commit` в commit message |
| [e5830013](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e583001380393e1e1034c33791f41c69b659f4ad) — Retain action executors across reentrant queue callbacks | `queue destroyed its executor while the executor callback was running` | 37/37; `queue-lifetime` | `Upstream-Commit` в commit message |
| [789470fe](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/789470fe4ed0e09151d0b1f4bfced1efe9a44803) — Skip missing and invalid RPG weapons in AI firearm scoring | `invalid AI weapon reached cover scoring` | 38/38; `invalid-ai-weapon` | `Upstream-Commit` в commit message |
| [48c24432](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/48c244323ed01b3ba955474d8160e7fde2073c8b) — Skip queued AI reactions after the actor becomes ineligible | `queued AI reaction updated an actor that can no longer operate` | 39/39; `stale-ai-reaction` | `Upstream-Commit` в commit message |
| [14f0706a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/14f0706a7dd4aa16db5739b94495c63aa764ce51) — Preserve enabled owners of overlapping unit footprint locks | `unlocking one unit cleared another unit's overlapping footprint` | 40/40; `overlapping-unit-locks` | `Upstream-Commit` в commit message |
| [48c24432](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/48c244323ed01b3ba955474d8160e7fde2073c8b) — Prevent stale path queries from relocking dead or invalid units | `path query restored an ineligible unit's movement footprint` | 44/44; `stale-unit-locks` | `Upstream-Commit` в commit message |
| [e97eb240](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e97eb240e2e422492c7c747096a8ebc285b0a959) — Copy live models when entering and leaving a Panzerklein | `PK mode change reused the shared RPG model` | 45/45; `pk-model-ownership` | `Upstream-Commit` в commit message |
| [5d36de28](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d36de28be48d2e4580369e26575fd1d250e1161) — Initialize transformed mesh bounds from the actual vertex extrema | `negative-coordinate mesh bounds incorrectly include the origin` | 46/46; `negative-mesh-bounds` | `Upstream-Commit` в commit message |
| [fedd8dc1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fedd8dc1cf676777917a82c2863c01eeb90ae67c) — Accept fire-mode commands while operating a mounted gun | `mounted-gun state rejected a fire-mode command` | 47/47; `mounted-mode-command` | `Upstream-Commit` в commit message |
| [e6ed847f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e6ed847ff0c042b27e71803de9111c3fb6490453) — Ignore invalid mounted and equipped weapon references | `invalid mounted weapon masked the equipped firearm` | 50/50; `stale-weapon-references` | `Upstream-Commit` в commit message |
| [97127c0d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/97127c0d592810910e08f9850e96803de919256e) — Change fire modes on the effective weapon including mounted guns | `fire-mode executor changed the personal weapon instead of the mounted gun` | 51/51; `effective-weapon-mode` | `Upstream-Commit` в commit message |
| [61b033f2](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/61b033f25a299389a648c0f84dc0f8140a6c7ef7) — Keep ragdoll restoration finite for collapsed and half-turn poses | `degenerate ragdoll restoration produced a non-finite rotation` | 53/53; `ragdoll-restoration` | `Upstream-Commit` в commit message |
| [61b033f2](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/61b033f25a299389a648c0f84dc0f8140a6c7ef7) — Bound nearest-position searches on empty navigation grids | `nearest-position search continued beyond its maximum radius on an empty grid` | 54/54; `nearest-empty` | `Upstream-Commit` в commit message |
| [61b033f2](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/61b033f25a299389a648c0f84dc0f8140a6c7ef7) — Reject invalid coordinates and networks before navigation lookup | `non-finite nearest-position coordinates reached navigation grid conversion` | 55/55; `nearest-invalid` | `Upstream-Commit` в commit message |
| [705cf766](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/705cf766b3adb9dcc034d77e1552a8920d21162a) — Preserve existing database references when overlay columns are absent | `partial database import cleared a reference whose column was absent` | 56/56; `partial-reference-import` | `Upstream-Commit` в commit message |
| [22d8c084](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/22d8c084e3344962ebb480635b1c014ba5d7b1f7) — Exclude completed missions from campaign recommendations | `completed mission with a leftover clue was recommended again` | 57/57; `completed-zone-recommendation` | `Upstream-Commit` в commit message |
| [0f46a627](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0f46a627025487ff620be94e045ce94efbc0b4fa) — Restore numeric parameters in shared perk tooltips | `perk tooltip omitted its numeric point parameter` | 58/58; `perk-tooltip-parameters` | `Upstream-Commit` в commit message |
| [c764609a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c764609a174a34932ba00dd0538d85f2af7b851e) — Select mechanical footsteps for worn Panzerkleins | `worn Panzerklein played the pilot armor's ordinary footsteps` | 59/59; `mechanical-step-sound` | `Upstream-Commit` в commit message |
| [97127c0d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/97127c0d592810910e08f9850e96803de919256e) — Use the effective weapon for fire-mode indicators and submenu selection | `mounted-gun mode indicator displayed the personal firearm mode` | 61/61; `mounted-weapon-mode-ui` | `Upstream-Commit` в commit message |
| [97127c0d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/97127c0d592810910e08f9850e96803de919256e) — Restore the mounted-gun fire-mode submenu button | `mounted-gun action bar omitted the fire-mode submenu button` | 62/62; `mounted-mode-button` | `Upstream-Commit` в commit message |
| [89617b3c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/89617b3c4af774fb012d352b34935d35382bbb61) — Reject Hide orders during the existing hide cooldown | `Hide order was accepted during the existing hide cooldown` | 63/63; `hide-cooldown` | `Upstream-Commit` в commit message |
| [2b42ca96](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2b42ca96b2e04b7e6f4a7b037555885bb949c788) — Preserve the held aim point when firing a sniper shot | `sniper Fire discarded the held called-shot location` | 64/64; `sniper-held-aim` | `Upstream-Commit` в commit message |
| [2b42ca96](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2b42ca96b2e04b7e6f4a7b037555885bb949c788) — Resolve targetless sniper Fire commands against the held target | `targetless sniper Fire did not resolve the held target` | 65/65; `sniper-targetless-probe` | `Upstream-Commit` в commit message |
| [1813d1cc](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1813d1ccceb5db6bc9b75e1313456371f25126da) — Create quest items directly when placing inventory clues | `inventory clue placement lost a quest item without an ordinary successor` | 66/66; `inventory-clue-creation` | `Upstream-Commit` в commit message |
| [1813d1cc](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1813d1ccceb5db6bc9b75e1313456371f25126da) — Count quest clues placed in unit inventories | `inventory clue placement did not count the clue on its carrier` | 67/67; `inventory-clue-count` | `Upstream-Commit` в commit message |
| [1813d1cc](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1813d1ccceb5db6bc9b75e1313456371f25126da) — Reject inventory clue placement when the backpack has no room | `access violation in CInventory::Place after FindPlace failed; confirmed by the debugger stack` | 68/68; `inventory-clue-capacity` | `Upstream-Commit` в commit message |
| [5f9977b7](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5f9977b7be4204857caa9f2a54f1391393e0da0c) — Honor empty optional string defaults in Lua argument checks | `Lua rejected an omitted optional string with an empty default` | 69/69; `lua-empty-default` | `Upstream-Commit` в commit message |
| [61b033f2](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/61b033f25a299389a648c0f84dc0f8140a6c7ef7) — Retain valid corpse positions when navigation snapping fails | `access violation in CPathNetwork::GetFloor after corpse Segment replaced a valid tile with the failed-search sentinel` | 70/70; `corpse-snap-failure` | `Upstream-Commit` в commit message |
| [9e417f87](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9e417f877825633a22c46317b0c870a1c1b9d3a9) — Preserve critical-injury rejection in fast movement previews | `fast movement preview discarded the existing critical-injury rejection` | 71/71; `movement-critical-probe` | `Upstream-Commit` в commit message |
| [c7ecaf8d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c7ecaf8d1f39de4086413873dad13bdfff3b8396) — Report Panzerklein command restrictions with the correct feedback | `PK command ban reported an ordinary critical-injury rejection` | 73/73; `pk-command-feedback` | `Upstream-Commit` в commit message |
| [5d97df47](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d97df475142a9f6a08fa56bf69d119d9115ecdc) — Preserve smart-aimer interpolation state at exact animation endpoints | `exact animation endpoint reseeded smart-aimer interpolation for the next transition` | 74/74; `smart-aimer-boundary` | `Upstream-Commit` в commit message |
| [362e197a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/362e197af937228f75180ec323bfed4dd3565c56) — Keep Hide and Strafe buttons visible for mixed unit selections | `Hide button disappeared for a selection with mixed hiding states` | 76/76; `mixed-action-buttons` | `Upstream-Commit` в commit message |
| [abd9981b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abd9981b01aa893fa1d1a136c2cfa7e3b34770fa) — Retain the latest accepted route command across movement interruptions | `movement interruption restored the old route after a new path was accepted` | 77/77; `latest-route-command` | `Upstream-Commit` в commit message |
| [e9f4cdd0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e9f4cdd052b4b80452879241d6fd309ca7b0ff49) — Preserve dialogue phrases when a text line exceeds the subtitle box | `oversized dialogue line aborted subtitle pagination` | 78/78; `oversized-dialogue-line` | `Upstream-Commit` в commit message |
| [e9f4cdd0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e9f4cdd052b4b80452879241d6fd309ca7b0ff49) — Guarantee progress when paginating markup-only dialogue pages | `dialogue pagination made no progress with markup-only content` | 80/80; `markup-dialogue-progress` | `Upstream-Commit` в commit message |
| [5d36de28](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d36de28be48d2e4580369e26575fd1d250e1161) — Compare legacy bound coordinates as floating-point values | `bound accumulator reversed negative floating-point coordinates` | 81/81; `signed-bound-accumulator` | `Upstream-Commit` в commit message |
| [e5830013](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e583001380393e1e1034c33791f41c69b659f4ad) — Stop reading movement queues emptied by step callbacks | `step callbacks emptied the movement queue before DoCommand fetched its continuation; debugger confirmed invalid command RTTI and corrupted queue teardown` | 83/83; `movement-callback-cancel` | `Upstream-Commit` в commit message |
| [e5830013](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e583001380393e1e1034c33791f41c69b659f4ad) — Retain movement executors through reentrant destruction callbacks | `a subsequent-step callback released the executor owner; DoCommand accessed the destroyed command state, confirmed by a CDB access-violation stack` | 85/85; `movement-callback-release` | `Upstream-Commit` в commit message |
| [115c1066](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/115c1066017479241d687e50bb49aeadea49ea91) — Preserve scenario-person clue protection when spawning mission units | `spawned scenario person lost its clue marker and corpse-cleanup protection` | 86/86; `person-clue` | `Upstream-Commit` в commit message |
| [4c2d34d5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c2d34d5ad82795c786e4344717abb7051ccc437) — Keep permanent clues assigned to their single mission map | `permanent clue lost its single map after ordinary clue capacity was consumed` | 87/87; `permanent-clue-map` | `Upstream-Commit` в commit message |
| [4c2d34d5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c2d34d5ad82795c786e4344717abb7051ccc437) — Place inventory clues on the replacement scenario person | `inventory clue was lost when its slot's authored person was replaced` | 88/88; `replacement-clue-inventory` | `Upstream-Commit` в commit message |
| [b7f57bd4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b7f57bd484d7d74f7d8dcf1d602b9738fc4bb4de) — Distinguish unarmed and knife attack cursors | `attack cursor did not distinguish fists, knife and firearm` | 89/89; `melee-cursor` | `Upstream-Commit` в commit message |
| [b7f57bd4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b7f57bd484d7d74f7d8dcf1d602b9738fc4bb4de) — Validate blocked attack captions and correct their regression record | `unavailable attack preview did not preserve its AP cost without a shooting percentage` | 91/91; `attack-caption-eligibility` | `Upstream-Commit` в commit message |
| [b7f57bd4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b7f57bd484d7d74f7d8dcf1d602b9738fc4bb4de) — Use the attack command target for hit-chance previews | `attack preview lost the exact 3D target when no movement tile was available` | 92/92; `exact-attack-target` | `Upstream-Commit` в commit message |
| [b7f57bd4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b7f57bd484d7d74f7d8dcf1d602b9738fc4bb4de) — Format one aggregate hit-chance line for the selected group | `equal selected-unit hit chances were displayed as a redundant percentage range` | 95/95; `group-attack-caption` | `Upstream-Commit` в commit message |
| [adea40c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/adea40c8c240da92fad84b5d714a27f94e641cb3) — Preserve building action state while restoring save runtime caches | `restoring save runtime caches started a new building action` | 97/97; `restore-action-state` | `Upstream-Commit` в commit message |
| [24a7e81d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24a7e81dd38990a628c5db8036946924d0c63f97) — Preserve existing AI support hulls when rebuilding save caches | `save cache restoration destroyed existing AI support hulls` | 98/98; `restore-building-hulls` | `Upstream-Commit` в commit message |
| [6c4f271d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6c4f271de266ee68955b63dab45404444b57784e) — Keep carried bodies attached during stale ground stability notifications | `old ground stability tracking dropped a corpse already attached to its carrier` | 99/99; `carried-corpse-stability` | `Upstream-Commit` в commit message |
| [438991a1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/438991a1fdc5ab68be3d163b67aa1c007d7d130e) — Pass the creating world to deferred ClickOfDeath damage | `deferred ClickOfDeath damage lost its creating world` | 100/100; `deferred-click-world` | `Upstream-Commit` в commit message |
| [5d97df47](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d97df475142a9f6a08fa56bf69d119d9115ecdc) — Preserve Panzerklein animation tracks at the start of a turn | `new turn replaced the Panzerklein animation with a normal standing track` | 101/101; `pk-turn-animation` | `Upstream-Commit` в commit message |
| [5d97df47](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d97df475142a9f6a08fa56bf69d119d9115ecdc) — Skip point-light shadow tracing when no scene is supplied | `access violation in CLightState::TraceDynamicLMPointLight: null pVis passed to TraceScene (confirmed by CDB)` | 102/102; `point-light-without-scene` | `Upstream-Commit` в commit message |
| [abc36d82](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abc36d82d852e44560eae4225df9b1a9fc333fbc) — Read retail point-light saves while retaining remaster compatibility | `structure field size mismatch: tag=2 wire=64 memory=56` | 104/104; `point-light-save-layout` | `Upstream-Commit` в commit message |
| [5d97df47](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d97df475142a9f6a08fa56bf69d119d9115ecdc) — Respect cached point-light shadow flags during dynamic lighting | `point light with shadows disabled still performed an occlusion test` | 105/105; `point-light-shadow-flag` | `Upstream-Commit` в commit message |
| [cb065556](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cb065556bfac5af1deaeacd4aa4e7504e1d38a6d) — Filter automatic action cameras while cinematic desktops own the view | `automatic action camera escaped cinematic desktop filtering` | 108/108; `cinematic-camera-filter` | `Upstream-Commit` в commit message |
| [abc36d82](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abc36d82d852e44560eae4225df9b1a9fc333fbc) — Preserve point-light shadow metadata through global illumination caches | `global illumination discarded the saved point-light shadow flags while building its cache` | 109/109; `global-point-light-shadow-flag` | `Upstream-Commit` в commit message |
| [abc36d82](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abc36d82d852e44560eae4225df9b1a9fc333fbc) — Restore point-light radiance normalization for lightmap caches | `point-light radiance failed to fold colour strength into its cached radius` | 110/110; `point-light-radiance` | `Upstream-Commit` в commit message |
| [abc36d82](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abc36d82d852e44560eae4225df9b1a9fc333fbc) — Discard zero and tiny point lights before caching illumination | `point-light cache retained zero/tiny radii that retail discards before lighting` | 111/111; `tiny-point-light-cache` | `Upstream-Commit` в commit message |
| [abc36d82](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abc36d82d852e44560eae4225df9b1a9fc333fbc) — Toggle console visibility directly without transient sliding state | `console close retained visible sliding state until a later render` | 112/112; `console-visibility-toggle` | `Upstream-Commit` в commit message |
| [8134756b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8134756b0f33ed8d384ace2ac880b51b2f0f2ff4) — Use notification values for AI reaction latches and retain contacts on reset | `consuming an AI reaction notification erased contacts or morale state` | 114/114; `ai-reaction-value` | `Upstream-Commit` в commit message |
| [77393cff](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/77393cff7835ccbad51a8e5979e7e079cedcfb45) — Retain contact dirty flags throughout nested AI event delivery | `AI event callback ran without holding all three contact collection locks` | 115/115; `ai-event-collection-locks` | `Upstream-Commit` в commit message |
| [5f9b0d8f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5f9b0d8f5e85a77d659575dc8dcea85d6407fd07) — Build navigation connectivity before the first incremental colour update | `first incremental navigation update left a new layer without its connectivity cache` | 116/116; `fresh-navigation-colouring` | `Upstream-Commit` в commit message |
| [5f9b0d8f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5f9b0d8f5e85a77d659575dc8dcea85d6407fd07) — Attach existing inter-layer transitions on the first navigation update | `first incremental transition update omitted existing inter-layer links` | 117/117; `fresh-navigation-transitions` | `Upstream-Commit` в commit message |
| [89ee0503](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/89ee0503cdb6a97e12d37d4ac2dfad94af32755d) — Exclude temporary unit occupancy while rebuilding scenery connectivity | `navigation rebuild cached a standing actor's temporary footprint as disconnected scenery` | 118/118; `navigation-colouring-occupancy` | `Upstream-Commit` в commit message |
| [89ee0503](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/89ee0503cdb6a97e12d37d4ac2dfad94af32755d) — Skip missing or invalid navigation caches in background recolouring jobs | `recolour job started changing occupancy for a missing or invalid navigation cache` | 120/120; `unavailable-navigation-colourer` | `Upstream-Commit` в commit message |
| [bab2713f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/bab2713f93ec512bb002e9cabd0139f29dd85745) — Pause and resume native AI routes when scripted AI control is toggled | `scripted AI deactivation left the current native route running` | 121/121; `script-ai-activation` | `Upstream-Commit` в commit message |
| [b6e82c68](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6e82c68c984aa1f4058d4f453e92f6ccbb3beb4) — Restore full dynamic ambient lighting when sky textures are unavailable | `dynamic objects lost the full ambient fallback when sky textures were unavailable` | 122/122; `dynamic-ambient-fallback` | `Upstream-Commit` в commit message |
| [884f91f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b) — Reject belt placement safely when an inventory has no uniform | `null uniform dereference in CInventory::GetPlaceBySubType, confirmed by CDB at RPGInventory.cpp:298` | 123/123; `inventory-without-uniform` | `Upstream-Commit` в commit message |
| [884f91f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b) — Drop every backpack item before clearing a dead units inventory | `ordinary death skipped a backpack item while removing earlier entries` | 124/124; `corpse-backpack-drop` | `Upstream-Commit` в commit message |
| [884f91f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b) — Let scripted dialogue preempt ordinary acknowledgement playback | `scripted dialogue was rejected behind an active ordinary acknowledgement` | 125/125; `scripted-dialogue-priority` | `Upstream-Commit` в commit message |
| [e36eeb2f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e36eeb2f0b68564688ddea1ca920b2b5535ddad8) — Gate mission interaction on the active desktop readiness | `mission remained interactive behind a noninteractive desktop overlay` | 126/126; `mission-overlay-readiness` | `Upstream-Commit` в commit message |
| [e36eeb2f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e36eeb2f0b68564688ddea1ca920b2b5535ddad8) — Block manual mission interaction during an AI-controlled turn | `AI-controlled active player remained eligible for manual mission interaction` | 127/127; `ai-player-mission-readiness` | `Upstream-Commit` в commit message |
| [e36eeb2f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e36eeb2f0b68564688ddea1ca920b2b5535ddad8) — Restore the one-shot world UI refresh latch | `fresh world did not preserve its pending UI refresh in save tag 47` | 128/128; `world-ui-refresh-latch` | `Upstream-Commit` в commit message |
| [b6e82c68](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6e82c68c984aa1f4058d4f453e92f6ccbb3beb4) — Use the acknowledgement owner for pain voices | `native hit processing used the persona voice instead of its acknowledgement owner` | 129/129; `pain-voice-owner` | `Upstream-Commit` в commit message |
| [b6e82c68](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6e82c68c984aa1f4058d4f453e92f6ccbb3beb4) — Suppress pain voices for already unconscious actors | `an already unconscious actor emitted a new pain voice on a hit` | 130/130; `unconscious-pain-voice` | `Upstream-Commit` в commit message |
| [e36eeb2f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e36eeb2f0b68564688ddea1ca920b2b5535ddad8) — Honor the icon visibility setting for sound and trap markers | `disabled overlay icons still queried the scene to rebuild markers` | 132/132; `overlay-icon-visibility` | `Upstream-Commit` в commit message |
| [e36eeb2f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e36eeb2f0b68564688ddea1ca920b2b5535ddad8) — Honor icon visibility for enemies while retaining quest markers | `disabled enemy icons were rebuilt for a visible hostile actor` | 133/133; `enemy-icon-visibility` | `Upstream-Commit` в commit message |
| [cf087f74](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cf087f74fcc63e48652755214e98dc42b8bcdd3f) — Keep finished music streams out of save-resume handling | `finished retained music stream entered restore logic and lost its authored silence interval` | 134/134; `finished-music-silence` | `Upstream-Commit` в commit message |
| [b6b20e29](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6b20e291ccdaeb82e5a67baf75df99f236b7c0b) — Hide the active heavy weapon mesh while it is holstered | `holstered active heavy weapon remained in the native render bindings` | 135/135; `holstered-heavy-weapon-mesh` | `Upstream-Commit` в commit message |
| [b6b20e29](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6b20e291ccdaeb82e5a67baf75df99f236b7c0b) — Persist scripted holster intent on the native unit | `scripted holstering did not persist the walk-without-weapon flag` | 136/136; `script-holster-persistence` | `Upstream-Commit` в commit message |
| [b6b20e29](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6b20e291ccdaeb82e5a67baf75df99f236b7c0b) — Preserve scripted holstering during automatic path preparation | `native movement queued automatic weapon activation despite persistent scripted holstering` | 137/137; `movement-holster-policy` | `Upstream-Commit` в commit message |
| [b6b20e29](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6b20e291ccdaeb82e5a67baf75df99f236b7c0b) — Avoid redundant weapon activation on short inactive paths | `short inactive path queued an unnecessary weapon draw and holster cycle` | 138/138; `short-inactive-movement` | `Upstream-Commit` в commit message |
| [b6b20e29](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6b20e291ccdaeb82e5a67baf75df99f236b7c0b) — Reconcile hidden weapon state when preparing movement | `hidden weapon was treated as drawn solely because its animator flag was active` | 139/139; `hidden-active-movement` | `Upstream-Commit` в commit message |
| [b6b20e29](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b6b20e291ccdaeb82e5a67baf75df99f236b7c0b) — Clear scripted holstering when an explicit weapon activation begins | `explicit equipment activation retained the scripted holster flag` | 141/141; `explicit-holster-reset` | `Upstream-Commit` в commit message |
| [e5830013](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e583001380393e1e1034c33791f41c69b659f4ad) — Reset the previous wait timer when a movement step starts | `starting a new physical movement step retained the previous wait timer` | 142/142; `movement-first-step-wait` | `Upstream-Commit` в commit message |
| [e5830013](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e583001380393e1e1034c33791f41c69b659f4ad) — Latch one-step movement completion when its final end command starts | `single-step end-move command did not latch the mover completion result` | 144/144; `movement-terminal-completion` | `Upstream-Commit` в commit message |
| [e5830013](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e583001380393e1e1034c33791f41c69b659f4ad) — Notify the automatic camera when preparing the next movement step | `native movement did not notify the automatic camera when preparing its next step` | 145/145; `movement-action-camera` | `Upstream-Commit` в commit message |
| [1e2f1d4f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1e2f1d4fe9ec2f7bacfeed195210ddb6c3b2c25e) — Restore the retail AI reload record format and clip ownership | `AI reload record did not write the next clip in retail save tag 4` | 148/148; `ai-reload-log` | `Upstream-Commit` в commit message |
| [156ccd17](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/156ccd173ff6c9671904e6d94e709397fc701d66) — Select the effective mounted weapon for default reloads | `default reload selected the personal firearm instead of the effective mounted weapon` | 149/149; `mounted-reload-weapon` | `Upstream-Commit` в commit message |
| [ece00052](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/ece000527fc42aa8f059bf82bb6e0e50f9f6f780) — Draw a holstered weapon before starting its reload action | `holstered reload started without scheduling weapon activation preparation` | 150/150; `holstered-reload-preparation` | `Upstream-Commit` в commit message |
| [61b033f2](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/61b033f25a299389a648c0f84dc0f8140a6c7ef7) — Restore the retail nested reload action save format | `reload action wrote a weapon reference in the retail slot-number field` | 151/151; `reload-action-wire` | `Upstream-Commit` в commit message |
| [07e6ad72](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/07e6ad72e4142862864e7383a102808072d02d52) — Skip inactive and undersized animated point lights | `inactive animated point light entered scene rendering before its parameters were ready` | 153/153; `animated-light-guard` | `Upstream-Commit` в commit message |
| [92816737](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9281673753420631d4a5d2bb8e2ab3d38d7c367c) — Connect fresh scenes to the retail animated ambient graph | `fresh scene omitted its retail animated top-ambient node` | 154/154; `fresh-ambient-graph` | `Upstream-Commit` в commit message |
| [476cd3b1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/476cd3b1115153828ab21298e179900dc1cbe612) — Write retail-compressed campaign and mission save files | `native snapshot command wrote an uncompressed save without the retail A3 marker` | 155/155; `compressed-save-writing` | `Upstream-Commit` в commit message |
| [d4de53d5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d4de53d5eef8444a372b7dbe582f8dbff2754b49) — Propagate changed window interfaces to the complete child tree | `changing a window's interface left its descendants attached to the old context` | 156/156; `window-interface-propagation` | `Upstream-Commit` в commit message |
| [d4de53d5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d4de53d5eef8444a372b7dbe582f8dbff2754b49) — Install parent and interface links when registering a child window | `AddChild retained a window without installing its parent link` | 157/157; `window-child-registration` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Count Lua callback arguments in the supplied execution state | `native Lua argument count came from another script's stack` | 158/158; `lua-argument-state` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Allow native UI scripts to be created without a tactical world | `native script factory rejected a UI script without a tactical world` | 159/159; `worldless-ui-script` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Preserve borrowed window references in native Lua userdata | `Lua window userdata discarded its borrowed native window reference` | 160/160; `lua-window-userdata` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Preserve the Lua VM stack across window setter tag callbacks | `Lua dotted window assignment corrupted the stack before the following native call` | 161/161; `lua-window-setter-stack` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Dispatch Lua UI calls through their owning execution state | `Lua window lookup/property dispatch ignored its owning state in favor of an unrelated global script` | 162/162; `lua-window-state-context` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Create and execute native window template Lua scripts | `window template Lua script was not created and executed in its own interface during Update` | 163/163; `window-template-script` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Dispatch native window messages to their owning Lua callbacks | `registered Lua onmessage handler did not receive the native window notification` | 164/164; `window-message-callbacks` | `Upstream-Commit` в commit message |
| [15d5d41f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/15d5d41fc37267ecc80dcc621df24be99e3b52a9) — Retain anonymous window callbacks until their registrations end | `anonymous window callback was reclaimed while still registered` | 165/165; `window-callback-lifetime` | `Upstream-Commit` в commit message |
| [d4de53d5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d4de53d5eef8444a372b7dbe582f8dbff2754b49) — Propagate active UI updates and suspend idle window branches | `idle UI branch never cleared its serialized update latch` | 166/166; `window-lazy-update` | `Upstream-Commit` в commit message |
| [d4de53d5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d4de53d5eef8444a372b7dbe582f8dbff2754b49) — Wake sleeping UI ancestors when inserting a descendant | `new active descendant did not wake all sleeping ancestor windows` | 167/167; `window-nested-wakeup` | `Upstream-Commit` в commit message |
| [fd6e670e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fd6e670eb20104ae2d7da195170af9adbdd5a6c2) — Resolve a source actor for ownerless inventory drags | `ownerless inventory drag did not use the first selected unit` | 168/168; `ownerless-inventory-drag` | `Upstream-Commit` в commit message |
| [a3051bb1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a3051bb1bd1a24c5901d7094b3d913426993a6ff) — Resolve loaded empty navigation slots through matching support surfaces | `loaded empty navigation slot did not resolve its real supporting surface floor` | 169/169; `unsupported-navigation-floor` | `Upstream-Commit` в commit message |
| [a3051bb1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a3051bb1bd1a24c5901d7094b3d913426993a6ff) — Reject unsupported height-field slots during navigation baking | `native navigation bake generated passable tiles without supporting surfaces` | 170/170; `unsupported-navigation-bake` | `Upstream-Commit` в commit message |
| [a3051bb1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a3051bb1bd1a24c5901d7094b3d913426993a6ff) — Refresh rendered living units after navigation floor changes | `living unit with a stale rendered floor did not refresh its native visibility binding` | 171/171; `living-unit-floor-refresh` | `Upstream-Commit` в commit message |
| [8853794e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8853794e2ad267be5df39bcc78ab246ac8414cb7) — Reconstruct omitted navigation colour caches on demand | `native navigation lookup did not reconstruct a cache omitted by a compact save` | 172/172; `lazy-navigation-colourer` | `Upstream-Commit` в commit message |
| [8853794e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8853794e2ad267be5df39bcc78ab246ac8414cb7) — Inherit compact-save mode when creating navigation layers | `navigation layer created after enabling compact saves did not inherit the small-save flag` | 173/173; `new-layer-compact-save` | `Upstream-Commit` в commit message |
| [2eee1b62](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18) — Keep AI commander ownership separate from tactical world views | `AI commander retained a foreign unit in its strongly owned roster` | 174/174; `owned-ai-commander-rosters` | `Upstream-Commit` в commit message |
| [8853794e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8853794e2ad267be5df39bcc78ab246ac8414cb7) — Keep replica cache assertions passive after lazy navigation restoration | `path colour cache leaked to replica or removed on host` | 1/1; `network-preparation-cache` | `Upstream-Commit` в commit message |
| [fa51df91](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fa51df91afc61e6a24f381efc5c60bd06e7b424a) — Restore translucent blue-gray rendering for hidden human units | `hidden human rendered with an opaque black overlay instead of the translucent blue-gray tint` | 175/175; `hidden-unit-tint` | `Upstream-Commit` в commit message |
| [f638aca9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f638aca9a410eef82afb20e926c97716dc448a90) — Guard native bullet construction against empty hit trails | `Access violation in CBulletServer constructor at wBullet.cpp:108 when indexing trailpointsSet[0]; verified with CDB stack` | 176/176; `empty-bullet-construction` | `Upstream-Commit` в commit message |
| [f638aca9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f638aca9a410eef82afb20e926c97716dc448a90) — Retire empty bullet trails without an unsigned endpoint underflow | `empty bullet never retired because its endpoint index underflowed` | 177/177; `empty-bullet-retirement` | `Upstream-Commit` в commit message |
| [a8311049](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a8311049890a7fa91a1a439a43fdfc2da9619a3e) — Release departing player footprints before destroying its units | `removed player left an occupied navigation tile after its unit was destroyed` | 178/178; `removed-player-footprints` | `Upstream-Commit` в commit message |
| [a8311049](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a8311049890a7fa91a1a439a43fdfc2da9619a3e) — Remove departing squad acknowledgement listeners during player teardown | `removed player retained acknowledgement listeners for its destroyed units` | 179/179; `removed-player-acks` | `Upstream-Commit` в commit message |
| [a8311049](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a8311049890a7fa91a1a439a43fdfc2da9619a3e) — Prune stale AI wrappers and cache entries during player removal | `surviving AI commander retained a wrapper for a destroyed player's unit` | 180/180; `removed-player-ai-rosters` | `Upstream-Commit` в commit message |
| [75341d49](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/75341d49319b927a76537c6021e6e655d635de95) — Preserve cannon identity in AI entry and exit command casts | `AI cannon entry lost its pose and docking commands during an incomplete-type cast` | 181/181; `mounted-ai-log-commands` | `Upstream-Commit` в commit message |
| [31cb3b6a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/31cb3b6a7081bc322e4586d078d61710475349a0) — Use the alien render material for hidden Panzerklein pilots | `hidden Panzerklein mesh did not use its bracketed alien material` | 182/182; `hidden-pk-material` | `Upstream-Commit` в commit message |
| [31cb3b6a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/31cb3b6a7081bc322e4586d078d61710475349a0) — Suspend hiding render effects during cinematic sequences | `cinematic sequence still rendered the human hiding overlay` | 184/184; `sequence-hiding-materials` | `Upstream-Commit` в commit message |
| [e15373d3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e15373d3e9a6c0e7b0fa0f76e94f94180f737743) — Read AI action targets from the acting unit threat state | `AI action selected another unit's shared target instead of its own contact` | 186/186; `owned-ai-target-selection` | `Upstream-Commit` в commit message |
| [e15373d3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e15373d3e9a6c0e7b0fa0f76e94f94180f737743) — Report live server positions from AI unit accessors | `AI position accessor exposed the speculative planning snapshot` | 188/188; `live-ai-position` | `Upstream-Commit` в commit message |
| [e15373d3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e15373d3e9a6c0e7b0fa0f76e94f94180f737743) — Preserve another unit context when an older combat chooser completes | `finishing an older combat chooser overwrote another unit's shared planning context` | 189/189; `combat-chooser-context` | `Upstream-Commit` в commit message |
| [e15373d3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e15373d3e9a6c0e7b0fa0f76e94f94180f737743) — Refresh begin-turn contacts independently of speculative AI health | `cached zero watcher HP suppressed the live begin-turn contact refresh` | 191/191; `begin-turn-live-contacts` | `Upstream-Commit` в commit message |
| [d247d37c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d247d37c06ebf6d73a10997ec44de64db57e689b) — Clear matching selected AI contacts immediately on removal | `removing the selected AI contact left a stale target for subsequent reactions` | 194/194; `removed-selected-ai-contact` | `Upstream-Commit` в commit message |
| [360344ca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387) — Retire finished AI logic once its installed world command is gone | `finished idle AI logic stayed alive merely because its RPG actor still existed` | 195/195; `finished-ai-logic-retirement` | `Upstream-Commit` в commit message |
| [e15373d3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e15373d3e9a6c0e7b0fa0f76e94f94180f737743) — Complete retreat logic when its live unit reaches the destination | `arrived retreat finished its chooser job but left the retreat logic unfinished` | 196/196; `arrived-retreat-completion` | `Upstream-Commit` в commit message |
| [8e620755](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8e620755aedcc01558c1b7f4bdd921126d9048cd) — Skip airborne points before normalizing dynamic path locks | `Access violation after normalizing a final airborne point with altitude bits 160 into a ground layer index; scenario CDB stack reaches GetSamePoints through ChangeDynamicLocks` | 197/197; `flying-dynamic-locks` | `Upstream-Commit` в commit message |
| [d247d37c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d247d37c06ebf6d73a10997ec44de64db57e689b) — Bind restored AI event trackers to their owning units | `restored owner-identity AI event tracker did not deliver to its bound unit threat state` | 200/200; `ai-event-owner-binding` | `Upstream-Commit` в commit message |
| [8e620755](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8e620755aedcc01558c1b7f4bdd921126d9048cd) — Use base-layer axes for airborne shooting direction | `airborne GetShootDirection indexed navigation layers with encoded altitude 160 (only two layers), causing an access violation; under CDB the invalid read instead produced a wrong direction` | 201/201; `flying-shoot-direction` | `Upstream-Commit` в commit message |
| [8e620755](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8e620755aedcc01558c1b7f4bdd921126d9048cd) — Preserve airborne positions in attack pose candidates | `native airborne shot preparation crashed in GetAttackOrigin -> CPathNetwork::GetDirection after GetDirectedPoints discarded the flight bit` | 202/202; `flying-attack-queue` | `Upstream-Commit` в commit message |
| [8e620755](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8e620755aedcc01558c1b7f4bdd921126d9048cd) — Allow scripted zone exits after cutscenes away from passages | `scripted LeaveToSubZone failed to leave after a cutscene while the party was away from the passage` | 203/203; `scripted-passage-transition` | `Upstream-Commit` в commit message |
| [8e620755](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8e620755aedcc01558c1b7f4bdd921126d9048cd) — Reject zone exits when no matching passage exists | `an entirely unconscious party left through a nonexistent passage` | 204/204; `empty-passage-exit` | `Upstream-Commit` в commit message |
| [d247d37c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d247d37c06ebf6d73a10997ec44de64db57e689b) — Commit RPG death before scripted enemy world death | `KillEmAll left a world-dead hostile's RPG persona alive` | 205/205; `scripted-enemy-death` | `Upstream-Commit` в commit message |
| [d247d37c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d247d37c06ebf6d73a10997ec44de64db57e689b) — Limit scripted enemy cleanup to fight-capable units | `KillEmAll killed an incapacitated hostile outside its fight-capable target set` | 206/206; `scripted-unconscious-enemy` | `Upstream-Commit` в commit message |
| [c9a7c93e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c9a7c93eb2924c072edec1d8c8a00a401a803a14) — Apply options screenshot treatment when adopting supplied pixels | `opening options with a supplied screenshot left it in the widget's default color mode` | 207/207; `supplied-options-screenshot` | `Upstream-Commit` в commit message |
| [c9a7c93e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c9a7c93eb2924c072edec1d8c8a00a401a803a14) — Respect nested cursor input locks for native pointer events | `a disabled native cursor consumed pointer input while its serialized nesting counter was positive` | 208/208; `disabled-cursor-input` | `Upstream-Commit` в commit message |
| [01c4688d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/01c4688d7c4750e7a5b51c9858ea42a01bfe84cd) — Apply defeat tint when adopting a supplied screenshot | `opening the defeat menu with a supplied screenshot left it in default color mode` | 209/209; `supplied-defeat-screenshot` | `Upstream-Commit` в commit message |
| [c9a7c93e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c9a7c93eb2924c072edec1d8c8a00a401a803a14) — Synchronize requested native cursor positions through SDL | `creating a native cursor at a requested position left SDL at the previous pointer location` | 210/210; `native-cursor-position` | `Upstream-Commit` в commit message |
| [b14bbb4f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b14bbb4f21ddb7bd3a1df837e63b49726ff1dfeb) — Restore saved party portrait selection without legacy image vectors | `native saved-format multi-unit panels crashed in CInfoPanelMultipleUnits::Draw while indexing the absent legacy selection-image vector, for both populated and empty rosters` | 212/212; `restored-portrait-selection` | `Upstream-Commit` в commit message |
| [f49af57f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f49af57f5fac26ad87cbab5206a3350ceb1f7509) — Honor save permission throughout the exit dialog | `the default exit dialog displayed Save despite its disabled save permission` | 214/214; `exit-menu-save-permission` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Exclude isolated and blocked copies from navigation aliases | `same-point enumeration included isolated or blocked navigation aliases instead of only the connected copy` | 216/216; `navigation-alias-destinations` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Expose connected navigation copies for isolated source cells | `an isolated occupied source without the same-point flag could not enumerate its connected escape copy` | 218/218; `isolated-source-aliases` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Retain connected nearby destinations over isolated lower copies | `nearby destination deduplication discarded a connected tile in favor of an isolated lower copy` | 219/219; `nearby-connected-destination` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Preserve direct transitions obscured by isolated lower copies | `an isolated lower copy suppressed a connected bidirectional navigation transition` | 220/220; `connected-direct-transitions` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Reject reverse shortcuts into isolated navigation destinations | `a reverse same-point shortcut allowed entry into an isolated destination` | 221/221; `isolated-transition-entry` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Require connected intermediate copies for navigation shortcuts | `a cross-layer shortcut borrowed movement through an isolated intermediate copy` | 222/222; `isolated-transition-shortcut` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Reject isolated aliases when matching recorded navigation links | `a recorded transition ending at an isolated copy matched a connected destination on another layer` | 223/223; `isolated-transition-link-match` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Exclude isolated destinations from zero-distance navigation moves | `native move enumeration offered a zero-distance step into an isolated alias` | 224/224; `connected-alias-moves` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Enumerate connected escape steps from isolated source tiles | `native move enumeration could not escape an isolated source lacking the same-point flag` | 225/225; `isolated-source-moves` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Check occupancy before emitting zero-distance alias moves | `native move enumeration offered a zero-distance step into a locked connected alias` | 226/226; `locked-alias-moves` | `Upstream-Commit` в commit message |
| [6d313faa](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6d313faaa22df6cfc05a5017621b684eb8a6d3d8) — Allow isolated starts to escape stale navigation zones without relaxing pose pruning | `upper-net zone pruning rejected a reachable route from an isolated source or stopped before its goal` | 227/227; `isolated-source-path` | `Upstream-Commit` в commit message |
| [26165318](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/26165318773cc357672f706aefd3d3dd93da3718) — Hide the Save tab when opening Load Game from the main menu | `loading from the main menu offered a Save tab without an active game` | 228/228; `main-menu-load-save-tab` | `Upstream-Commit` в commit message |
| [5c13b0d0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5c13b0d0219aed4684563a01b694d2d9da9aeccd) — Honor the standard confirmation command in the exit dialog | `the native exit dialog ignored its standard confirmation command` | 229/229; `exit-menu-confirmation` | `Upstream-Commit` в commit message |
| [5c13b0d0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5c13b0d0219aed4684563a01b694d2d9da9aeccd) — Report successful saves only after serialization completes | `a completed silent native save omitted its localized success feedback` | 230/230; `save-success-feedback` | `Upstream-Commit` в commit message |
| [5c13b0d0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5c13b0d0219aed4684563a01b694d2d9da9aeccd) — Report successful loads after the native slot and interface restore finish | `a completed silent native load omitted its localized success feedback` | 231/231; `load-success-feedback` | `Upstream-Commit` в commit message |
| [cc9fbeab](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc9fbeab94c206c59606fb91902c800c5690f0c9) — Name missing Lua globals in the diagnostic log without changing lookup results | `native Lua silently returned nil for a missing global without naming it in the diagnostic log` | 232/232; `missing-lua-global-warning` | `Upstream-Commit` в commit message |
| [cc9fbeab](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc9fbeab94c206c59606fb91902c800c5690f0c9) — Avoid counting the sweep radius twice in precalculated collisions | `a precalculated thin wall reported a collision outside its radius-padded bound because the sweep radius was counted twice` | 233/233; `precalc-sweep-bounds` | `Upstream-Commit` в commit message |
| [37e5bf12](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/37e5bf129ae99b76830786918d63c4d0fdbc078c) — Exclude mission HUD reachability from zone re-entry snapshots | `a native zone snapshot retained the HUD and mission graph through its world-script interface bridge` | 234/234; `zone-snapshot-excludes-hud` | `Upstream-Commit` в commit message |
| [d14e92f9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d14e92f95f112fa31d59f2e95866fd4e0bdf49ae) — Preserve actual facing when leaving prone through canonical path points | `leaving prone adopted the wave's canonical direction instead of retaining the actual facing` | 235/235; `prone-pose-change-facing` | `Upstream-Commit` в commit message |
| [c0c1914c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c0c1914c23df17eb98151f85184cab0f9bb81e2b) — Allow direct simple pose paths from prone sources | `simple native pathfinding rejected a direct pose change from prone` | 236/236; `simple-prone-pose-path` | `Upstream-Commit` в commit message |
| [c0c1914c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c0c1914c23df17eb98151f85184cab0f9bb81e2b) — Prefer unchanged facing when selecting direct pose paths | `simple pose-path selection chose direction zero over an equally valid unchanged facing` | 237/237; `simple-pose-facing-preference` | `Upstream-Commit` в commit message |
| [c0c1914c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c0c1914c23df17eb98151f85184cab0f9bb81e2b) — Separate layer entry from pose changes in direct navigation paths | `a simple cross-layer pose path omitted its layer-only entry point and combined the layer and pose change` | 238/238; `simple-cross-layer-pose-entry` | `Upstream-Commit` в commit message |
| [2826ebed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2826ebed4e5a7edd6e15161b21e347d025d4a2e5) — Consume complete elapsed intervals in the native sound mixer clock | `the native sound mixer lost most of a skipped interval by clamping its clock advance to one 150ms step` | 239/239; `sound-mixer-clock-catchup` | `Upstream-Commit` в commit message |
| [2826ebed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2826ebed4e5a7edd6e15161b21e347d025d4a2e5) — Wrap late looping sound seeks at the sample length | `a late looping native sound restarted at frame zero instead of wrapping its elapsed seek offset` | 240/240; `late-looping-sound-offset` | `Upstream-Commit` в commit message |
| [2006fec4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2006fec4aa455437c74537c25fed44547c81163c) — Preserve playing music when drawing a trackless sound scene | `a sound scene without its own music faded out another scene's playing track` | 242/242; `trackless-scene-music` | `Upstream-Commit` в commit message |
| [7cb6e2c5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7cb6e2c5d8c649d8fcd3d994deb7090a66c476f6) — Clear the chapter return destination when continuing the campaign | `continuing to the global map retained the old chapter return destination` | 243/243; `global-map-return-destination` | `Upstream-Commit` в commit message |
| [038b1bca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/038b1bca53b9b94432f7716c772674563917a143) — Allow explicit disarm targeting before hovering a trap | `forced disarm targeting queried generic Use eligibility instead of UseTool` | 244/244; `forced-tool-targeting-entry` | `Upstream-Commit` в commit message |
| [038b1bca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/038b1bca53b9b94432f7716c772674563917a143) — Refresh disarm targeting cursor and action-point captions | `the native disarm cursor retained valid targeting after the hovered action became unavailable` | 245/245; `tool-targeting-cursor-refresh` | `Upstream-Commit` в commit message |
| [1c81137e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1c81137e48730ece25e22156919f5390b1c4e667) — Offer replacement reloads when entering attack mode with an empty clip | `the mission's attack-mode entry rejected an empty clip despite an available replacement reload` | 246/246; `empty-clip-attack-entry` | `Upstream-Commit` в commit message |
| [360344ca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387) — Use the authored placement cursor for object traps | `object trap placement used the healing cursor instead of the authored trap-placement cursor` | 247/247; `object-trap-placement-cursor` | `Upstream-Commit` в commit message |
| [360344ca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387) — Use the authored placement cursor when setting mines | `mine placement used the healing cursor instead of the authored trap-placement cursor` | 248/248; `mine-placement-cursor` | `Upstream-Commit` в commit message |
| [72000d27](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/72000d27a460a0300bc6cae497e1d69b61d80f17) — Keep opposing hotseat players out of allied visibility sharing | `opposing hotseat players shared contacts, objects or traps merely because both used scenario ID zero` | 249/249; `hotseat-opponent-visibility` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Keep playing music when its database record is replaced | `replacing a music database record restarted its already-playing file instead of adopting the live stream` | 250/250; `reloaded-music-record` | `Upstream-Commit` в commit message |
| [2826ebed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2826ebed4e5a7edd6e15161b21e347d025d4a2e5) — Advance mission sound on the game clock including skipped intervals | `the native mission passed raw wall time to sound and omitted the fast-forwarded game interval` | 251/251; `mission-sound-game-clock` | `Upstream-Commit` в commit message |
| [2826ebed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2826ebed4e5a7edd6e15161b21e347d025d4a2e5) — Age world sound effects before scheduling and seeking their samples | `an expired world sound effect restarted after visibility was restored instead of seeking past its sample end` | 252/252; `delayed-sound-effect-age` | `Upstream-Commit` в commit message |
| [2826ebed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2826ebed4e5a7edd6e15161b21e347d025d4a2e5) — Keep infinitely repeated world effects looping independently of sample metadata | `an effect with infinite cycle count played once because its sample record was not itself looped` | 253/253; `infinite-sound-effect-loop` | `Upstream-Commit` в commit message |
| [2826ebed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2826ebed4e5a7edd6e15161b21e347d025d4a2e5) — Release failed and expired sound samples without registering a phantom channel | `the channel table retained an expired sound and its decoded sample under the invalid channel ID` | 254/254; `failed-sound-channel-lifetime` | `Upstream-Commit` в commit message |
| [9785abf3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9785abf308b0fab7b2b6bd3f0663a8e1e99b668e) — Create and retain the chapter party marker tooltip | `the chapter party marker never created its saved tooltip` | 255/255; `chapter-team-marker-tooltip` | `Upstream-Commit` в commit message |
| [5cdf627b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5cdf627bb6ce601188e39945b93742515216655a) — Refresh chapter marker hints during movement and zone transitions | `the chapter marker retained its previous hint when its real draw transitioned to another action` | 256/256; `chapter-marker-tooltip-transitions` | `Upstream-Commit` в commit message |
| [5cdf627b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5cdf627bb6ce601188e39945b93742515216655a) — Distinguish the chapter exit marker from mission entry | `the native chapter map selected a mission-entry marker for its campaign exit sector` | 257/257; `chapter-exit-marker-hint` | `Upstream-Commit` в commit message |
| [1ca0541b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1ca0541b5a1eb3ab4cba65e07e7f24de254ac98c) — Notify the originating mission when a restored clue modal closes | `closing a restored clue modal never notified the mission of its waiting UI event` | 258/258; `restored-clue-close-notification` | `Upstream-Commit` в commit message |
| [1ca0541b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1ca0541b5a1eb3ab4cba65e07e7f24de254ac98c) — Forward clue wait ownership through mission dispatch and modal construction | `fresh clue dispatch dropped the originating mission and its waiting script event ID` | 259/259; `fresh-clue-wait-release` | `Upstream-Commit` в commit message |
| [5528542f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5528542f16fdd90afe1ff2964c193f5af54d4f36) — Use the shared markup layout for screen-scaled editable text (заменено узкой native-адаптацией ниже) | `the native edit renderer wrapped screen-sized text at the unscaled logical control width` | 260/260; `scaled-edit-text-layout` | `Upstream-Commit` в commit message |
| [5528542f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5528542f16fdd90afe1ff2964c193f5af54d4f36) — Preserve packaged edit fonts while correcting screen-scaled layout bounds | `the edit renderer changed the remaster's original high-resolution font or its default point size` | 260/260; `preserve-native-edit-fonts` | `Upstream-Commit` в commit message |
| [5528542f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5528542f16fdd90afe1ff2964c193f5af54d4f36) — Select long-burst voice from the firing unit acknowledgement donor | `starting a real long burst played the fixed generic voice instead of the shooter's acknowledgement donor` | 261/261; `long-burst-voice-donor` | `Upstream-Commit` в commit message |
| [7a9e903d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7a9e903dd299c89f260c0e9311fbb15127bbd980) — Honor the authored visibility flag for active item meshes | `an active item marked invisible in hand still attached its native weapon mesh` | 262/262; `hidden-active-item-mesh` | `Upstream-Commit` в commit message |
| [7a9e903d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7a9e903dd299c89f260c0e9311fbb15127bbd980) — Honor the authored visibility flag for inactive carried heavy meshes | `an inactive heavy item marked invisible still attached its carried weapon mesh` | 262/262; `hidden-carried-heavy-mesh` | `Upstream-Commit` в commit message |
| [360344ca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387) — Let unavailable attack modes fall through shared action shortcuts | `an unavailable attack swallowed the shared shortcut instead of falling through to set-mine` | 264/264; `unavailable-attack-shortcut` | `Upstream-Commit` в commit message |
| [9af844d6](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9af844d6b62ed800bb59a64057b03e0eafc83878) — Probe equipped tools when caching use-tool availability | `the mission's use-tool cache advertised availability without consulting the equipped tool` | 265/265; `equipped-tool-action-availability` | `Upstream-Commit` в commit message |
| [7a9e903d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7a9e903dd299c89f260c0e9311fbb15127bbd980) — Select available shared-key actions in the original priority order | `shared action key selected set-mine ahead of available first aid` | 266/266; `shared-action-shortcut-priority` | `Upstream-Commit` в commit message |
| [9af844d6](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9af844d6b62ed800bb59a64057b03e0eafc83878) — Compute invalid mission action caches when they are queried | `querying an invalid action cache returned an uncomputed unavailable action` | 267/267; `invalid-action-cache-query` | `Upstream-Commit` в commit message |
| [6a66dadd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6a66daddde3e9090f9fbb95e30f79a4262097e12) — Initialize RPG unit full names from their authored persona records | `creating an RPG unit discarded its authored full display name` | 268/268; `persona-full-display-name` | `Upstream-Commit` в commit message |
| [6a66dadd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6a66daddde3e9090f9fbb95e30f79a4262097e12) — Show stored full names in the character panel without changing its fonts | `the character panel showed the nickname instead of the full display name` | 269/269; `character-panel-full-name` | `Upstream-Commit` в commit message |
| [da0fabc0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/da0fabc03f4e1fa4b7487b3dd35d458a792ee0c2) — Anchor heard-unit ear icons inside the sound marker head | `the heard-unit ear icon used visible-enemy clearance instead of its sound-marker head height` | 270/270; `heard-icon-anchor-height` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Resolve unavailable UI samples through the native lazy resource loader | `a missing UI sound never resolved to a stable silent sample` | 271/271; `missing-ui-sound-fallback` | `Upstream-Commit` в commit message |
| [f6455c9f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f6455c9f92eb6d4c66efebc74face61d1e1065bb) — Reject attack cursor targets belonging to the selected group | `the attack cursor created a shot at its own selected attacker` | 272/272; `selected-attack-target-rejection` | `Upstream-Commit` в commit message |
| [f6455c9f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f6455c9f92eb6d4c66efebc74face61d1e1065bb) — Hide out-of-reach melee percentages without changing shared hit calculations | `a melee target outside current-pose reach still displayed a hit percentage instead of only its AP preview` | 273/273; `melee-current-pose-caption` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Preserve authored item visibility when cancelling inventory animations | `cancelling inventory movement restored a held-weapon pose for an item marked invisible in hand` | 274/274; `cancelled-inventory-hidden-item-pose` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Respect active-item visibility when constructing a unit held pose | `a newly created actor treated an invisible active item as a held weapon` | 275/275; `initial-hidden-item-pose` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Respect active-item visibility when cancelling equipment activation | `cancelling equipment activation restored a held-weapon pose for an item marked invisible in hand` | 13/13; `cancelled-equipment-hidden-item-pose` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Clear the held item and weapon animation after the final medkit charge | `finishing the last medkit charge retained the held-item or healing weapon animation state` | 16/16; `spent-medkit-pose-cleanup` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Clear temporary holster flags when equipment activation completes | `completed equipment activation retained temporary item or heavy-weapon hiding flags` | 16/16; `equipment-completion-holster-flags` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Clear the held item and weapon animation after the final picklock charge | `consuming the last picklock charge retained the held-item or weapon animation state` | 17/17; `spent-picklock-pose-cleanup` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Defer mine successor activation until placement animation completion | `mine placement equipped the successor before its animation finished` | 279/279; `mine-placement-successor-handoff` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Defer door-trap successor activation until placement animation completion | `door trap placement equipped the successor before its animation finished` | 20/20; `trap-placement-successor-handoff` | `Upstream-Commit` в commit message |
| [7e46d2a6](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7e46d2a66d1ba9ca3bef8f41e14b684425febed3) — Preserve the throwing animation while retaining the successor knife | `the thrown-knife executor did not retain its successor in saved field four` | 21/21; `thrown-knife-successor-handoff` | `Upstream-Commit` в commit message |
| [7e46d2a6](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7e46d2a66d1ba9ca3bef8f41e14b684425febed3) — Restore ordinary positional knife-throw feedback outside worn Panzerkleins | `throwing a native knife omitted its ordinary positional feedback sound` | 283/283; `thrown-knife-feedback` | `Upstream-Commit` в commit message |
| [3cfccbee](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a) — Rebuild grenade throw calculations when continuing saved executors | `restoring a pending grenade throw dereferenced its unsaved to-hit calculator (null this, CDB-confirmed)` | 24/24; `restored-grenade-throw` | `Upstream-Commit` в commit message |
| [3cfccbee](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a) — Defer spare grenade equipment until the throwing animation finishes | `throwing a grenade equipped the successor before the animation completed` | 26/26; `grenade-successor-handoff` | `Upstream-Commit` в commit message |
| [3cfccbee](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a) — Honor saved grenade visibility requests when the action completes | `finishing a restored grenade action discarded its requested deferred visibility refresh` | 27/27; `grenade-completion-visibility` | `Upstream-Commit` в commit message |
| [3cfccbee](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a) — Stop grenade release when its recalculated trajectory is unavailable | `releasing a grenade after its native trajectory became unavailable consumed the item and launched stale parameters` | 28/28; `invalidated-grenade-trajectory` | `Upstream-Commit` в commit message |
| [3cfccbee](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a) — Refresh held-item flags before selecting a grenade throw animation | `a grenade throw after the native unarmed POSE fallback omitted its authored held-item throw animation` | 29/29; `grenade-throw-animation-refresh` | `Upstream-Commit` в commit message |
| [4c6b10fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4c6b10fb85afb750a58e1c28ffe29cdcbb05773c) — Detach default slot items without starting unrelated activation animations | `native default item detachment started an unrelated activation animation while processing equipment slots` | 31/31; `default-item-detachment` | `Upstream-Commit` в commit message |
| [0ffb568d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0ffb568d67bc6ca389d7ca63a9da4f3978c801a7) — Persist total experience when assigning a native unit level | `assigning a native unit level updated skill XP but discarded the corresponding total experience` | 292/292; `assigned-level-experience` | `Upstream-Commit` в commit message |
| [432b64c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/432b64c8bbe97a9f347a6c9468e7c69173d68394) — Reload the retreat reaction owner after switching an arrived unit to guard | `native AI pump crashed in GetColourer via HasPath after an arrived retreat replaced and cleared itself` | 26/26; `arrived-retreat-reaction-lifetime` | `Upstream-Commit` в commit message |
| [432b64c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/432b64c8bbe97a9f347a6c9468e7c69173d68394) — Localize rejected hit labels instead of displaying negative damage sentinels | `a rejected native HUD hit displayed the negative damage sentinel instead of its localized label` | 17/17; `rejected-hit-label-text` | `Upstream-Commit` в commit message |
| [c1ab9336](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c1ab93366661d3a4e25a0effe62625bae92c2d0c) — Report equipped melee weapons to the existing knife cursor without changing attack rules | `an equipped native melee weapon still advertised the unarmed fist state and cursor` | 11/11; `equipped-melee-attack-cursor` | `Upstream-Commit` в commit message |
| [d919d910](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d919d91066b15951e2a14cc4c78e70745aa0ea88) — Clamp shooting torso animation turns without changing the selected aim | `the native shooting animation twisted the torso past its allowed horizontal angle` | 25/25; `shooting-torso-turn-limit` | `Upstream-Commit` в commit message |
| [394d64fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/394d64fb59f1f82e80cc928bf195e0280cfb7999) — Log successful hit rolls at the equality boundary | `a native hit-roll equality returned the hit ratio but was logged as a miss` | 10/10; `hit-roll-equality-log` | `Upstream-Commit` в commit message |
| [d919d910](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d919d91066b15951e2a14cc4c78e70745aa0ea88) — Preserve homogeneous coordinates when unprojecting native rays | `a translated native projective ray disagreed with the renderer's homogeneous point conversion` | 9/9; `projective-ray-unprojection` | `Upstream-Commit` в commit message |
| [7de2769d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7de2769dedfde7e6cf1ddabfcb4dfee8dc05404c) — Freeze shared facial idle when native units are incapacitated | `a downed native world unit left its shared portrait head in living facial idle` | 14/14; `downed-head-idle` | `Upstream-Commit` в commit message |
| [ccdb6ca7](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/ccdb6ca7c7ee525b4e8aeb1c5c9c0c18effa9ff0) — Keep SDL Escape available to cancel focused edit dialogs | `the focused native edit swallowed Escape before the save/load modal's cancel binding` | 18/18; `edit-keyboard-routing` | `Upstream-Commit` в commit message |
| [4cdc1ead](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4cdc1eada6c182c0a9093364e95c750a6166d4c8) — Resolve active door geometry through the native hull registry | `native object lookup returned an inactive door-pose hull` | 17/17; `active-door-hull-lookup` | `Upstream-Commit` в commit message |
| [70d12946](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778) — Fit global-sector description panels to their native captions | `the native global-sector description kept its fixed height instead of fitting its actual caption and border` | 10/10; `global-sector-description` | `Upstream-Commit` в commit message |
| [e9dc218f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e9dc218fcbf45f8ae952b0381112beba1193ca02) — Initialize the native campaign chapter world and saved mission graph | `the native chapter initialized without the world required by its saved mission state` | 9/9; `chapter-world-graph` | `Upstream-Commit` в commit message |
| [e9dc218f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e9dc218fcbf45f8ae952b0381112beba1193ca02) — Queue chapter entry scripts in their native world context | `the native chapter never evaluated its assigned ScriptID entry point` | 23/23; `chapter-script-initialization` | `Upstream-Commit` в commit message |
| [e9dc218f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e9dc218fcbf45f8ae952b0381112beba1193ca02) — Resume chapter sound when a native menu returns focus | `returning from a native menu left the chapter audio channel permanently paused` | 24/24; `chapter-focus-audio` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Stop scripted sound through the mission-owned channel list | `native Lua StopSound cleared its weak handle but left the actual sound playing` | 43/43; `script-sound-stop` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Keep pending UI commands alive across native Lua collection | `an unassigned Lua PlaySound result was collected before its native UI command could start playback` | 313/313; `script-sound-unassigned` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Start fresh scripted positional sounds at the beginning of their sample | `a fresh scripted sound inherited absolute interface time as a sample start offset` | 66/66; `script-sound-start-offset` | `Upstream-Commit` в commit message |
| [70d12946](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778) — Maintain native chapter counters and saved frame timing | `the native chapter Step did not advance its actual game and interface clocks` | 67/67; `chapter-clock-step` | `Upstream-Commit` в commit message |
| [70d12946](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778) — Advance the native chapter world during active frames | `the real active chapter Step never advanced its world or executed the assigned Lua script` | 68/68; `chapter-active-script` | `Upstream-Commit` в commit message |
| [70d12946](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778) — Dispatch chapter script commands through the remaster interfaces | `the chapter hint command never released its real Lua UI wait` | 73/73; `chapter-world-commands` | `Upstream-Commit` в commit message |
| [70d12946](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778) — Pump deferred native audio in active chapter frames | `the active chapter created a script sound wrapper but never started native PCM playback` | 77/77; `chapter-script-audio` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Dispatch native script sounds from rendered backdrop menus | `the active native backdrop ignored its initialized map script's sound command` | 79/79; `backdrop-script-audio` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Start the main-menu map script without the obsolete second launcher | `the native main menu launched an extra DB script after its map entry was already initialized` | 81/81; `main-menu-map-script` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Drive main-menu Lua only from native world segments | `the main menu executed a sleeping Lua coroutine between native world segments` | 82/82; `main-menu-script-cadence` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Consume loading input after writing the mission restart snapshot | `restart snapshot completion did not consume a key release accumulated during loading` | 317/317; `mission-loading-input` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Connect native audio acknowledgement checkboxes to saved settings | `the native audio page did not initialize its acknowledgement checkboxes from config`; `the native response checkbox did not apply its change to config` | 85/85; `audio-option-checkboxes` | `Upstream-Commit` в commit message |
| [e399ef89](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e399ef8901588f689462ea9282cc260326097549) — Restore registered audio defaults without restarting native playback | `the native audio Default button did not restore registered settings` | 86/86; `audio-option-defaults` | `Upstream-Commit` в commit message |
| [dbb74c6e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/dbb74c6e661c0344061a79a86b0ebf3e01617526) — Preserve native selection outlines over lighting stencil marks | `selection outline changed over lighting stencil 0x80 at 47,47: expected ff407f20, got ff000000` | Windows 104/104; Linux 100/100 + дополнительные GPU/registration; `selection-stencil` | `Upstream-Commit` в commit message |
| [02d85e5b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72) — Reflect the incoming save permission in the actual in-game Save button | `the in-game Save button ignored its explicit save permission` | 91/91; `in-game-save-button` | `Upstream-Commit` в commit message |
| [02d85e5b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72) — Open Load for an in-game Save command when saving is forbidden | `a forbidden in-game Save shortcut opened an empty native save/load page` | 92/92; `in-game-save-shortcut` | `Upstream-Commit` в commit message |
| [02d85e5b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72) — Open Load for a tactical mission Save-menu command when saving is forbidden | `a forbidden mission Save-menu command opened an empty native save/load page` | 93/93; `mission-save-menu` | `Upstream-Commit` в commit message |

## Продолжение

Повторная сверка текущей реализации, без нового изменения кода:

- `14ee5eae97d42f60beed1b5100a2b96786d68fd3`: стрелки изменений навыков уже
  используют текстуры 946/947 в `UpdateCharacterSkillUI`, наша реализация
  `cfc1e480b0536e63569bfeb17fa1b98a56d39397`. Переносить повторно нечего.
- `1f617b76`: старый upstream фильтр `bKeepPrevious` относится к другому
  варианту `CLightmapTracker`. Здесь обычный режим после
  `5be2875ee21a271ccbe5d1dbe1b3052f9c4fe7ea` использует одинаковые фиксированные
  HQ sky maps при восстановлении и в покое, а RGB точечных источников обновляет
  отдельно. Повторного переключения на грубую оценку неба в этом пути нет.
  Старое вращающееся уточнение включается только диагностической переменной.
  Основание решения: текущий `CatchUp`/`UseProgressiveSky` и прежние измерения
  в [LIGHTING-MOTION-20261003.md](LIGHTING-MOTION-20261003.md).
  Новый GPU-прогон именно вспышек выстрела в рамках этой сверки не выполнялся.

- `f876bfeac5f1d198e2126f4e40e54321eef31269`: проверен PE-заголовок настоящего
  Windows `Game.exe` из `checkpoint-271-all-green-build.log`. Бинарник AMD64
  (`0x8664`), PE32+ (`0x20b`), flags `0x22` уже содержит
  `IMAGE_FILE_LARGE_ADDRESS_AWARE` (`0x20`). Исторический флаг для 32-битной
  сборки не добавляет исправления памяти нынешней x64-игре. SHA256 бинарника
  записан в `native-game-large-address-aware.json`. x86-бинарник не проверялся.

Эти три пункта отмечены как `already-present`, а не как новые TDD-переносы.

Локальная предпосылка музыкальных проверок: `headless-music-stream` воспроизвел,
что музыка игнорировала `S2_AUDIO_HEADLESS` и продвигала PCM-позицию через физическое
аудиоустройство. Нативный stream теперь передает существующий headless-флаг
в `NativeMusicPlayer::Init`; обычный запуск без флага сохраняет прежний режим.
Тест открывает настоящий временный WAV, проверяет playback и точный seek 250ms,
ждет 100ms и подтверждает отсутствие аппаратного продвижения. Файл удаляется.
Это исправление нашего backend, а не перенос upstream: исходная интеграция
`d0cd6acd96d98be395de6f27e2edda1c0ef134b8` указана как `Original-Commit`.
Windows профиль после исправления: 241/241. Это не подтверждение успеха GitHub Actions.

Сначала priority findings отчета: render compaction, invalid AI weapon,
state/queue lifetime, dead AI/overlapping locks, PK model ownership,
mounted gun modes, sniper fire, partial DB import, clues/route, tooltips,
ragdoll rotation, negative bounds, mechanical footsteps, dialogue edge cases.
После них пройти все остальные pending/split/retail/adapt строки ведомости.
Нельзя считать задачу завершенной только по двадцати приоритетным находкам.

## Регистрация Linux и Windows

Новые регрессии подключены к обеим веткам CMake. Проверка
`UpstreamRegistrationTests.py` сначала выявила 16 отсутствующих Linux
регистраций, затем подтвердила совпадение 47 точек входа. Windows CTest:
52/52, включая эту проверку и тесты с оригинальными ресурсами.
Linux Game и UpstreamBugTests собраны. Регрессии Linux: 53/53,
включая ragdoll. Исправлено неявное преобразование CObj в тестовой оснастке,
отвергнутое Clang (MSVC допускал его); production код не меняется.
Более широкий Windows набор без внешних ресурсов/дисплея: 170/170.
Полный ресурсный прогон с extended-сценариями остановлен: массовый
NetworkEncounterMapsTests превысил установленный общий лимит 120 секунд;
результат этого прогона не заявляется как GREEN.

## Проверка после переноса интерфейса станкового оружия и Hide

Windows и Linux: 63/63 upstream регрессии с оригинальными ресурсами.
Windows расширенный набор без внешних ресурсов/дисплея: 175/175.
Полная Windows сборка и Linux Game/UpstreamBugTests проходят; сборки
переиспользуются, HD pack не генерируется. Проверка реальных UI-контролов
станкового оружия обходится без открытия окна игры. Исторические значения
в таблице выше относятся к состоянию на момент соответствующего коммита.

## Проверка после диалогов и legacy bounds

Текущий Windows ALL_BUILD и Linux Game собираются. На обеих платформах
проходят 81/81 проверка: upstream-группа и NativeBoundAccumulatorTests.
Отдельный реальный x86 тест сначала упал на отрицательных координатах,
затем прошел после фикса; x64 вариант прошел и до него. Дополнительных
копий сборок не создавалось: использованы прежние x64/Linux/x86 каталоги.

На контрольном коммите `4b5d63b0` более широкий Windows набор без внешних
ресурсов/дисплея прошел 177/177, реальные игровые сценарии стрельбы и
перезарядки — 4/4. Это историческая контрольная точка до последующих
исправлений маршрута, диалогов и x86 bounds; она не выдается за отдельный
полный прогон всех ресурсных/extended тестов на каждом новом коммите.

## Проверка сценарных улик и подписей атаки

На коммите `87757413` Windows ALL_BUILD и Linux Game собираются; на обеих
платформах проходят 95/95 регрессионных проверок. Более широкий Windows
набор без внешних ресурсов и дисплея — 179/179, отдельные игровые сценарии
стрельбы, сохранения выстрела и перезарядки — 4/4. Использованы те же каталоги
сборки, без копий игры и генерации HD pack.

Коммит `26048494` был ошибочно отправлен после неуспешного общего прогона и
с повторенным сообщением предыдущего исправления курсоров. Два новых теста
подписи до production-фикса падали по ожидаемой причине; общий прогон выявил
недостающий режим времени в старой тестовой миссии. `8fa7f09f` исправил эту
оснастку и зафиксировал достоверный GREEN 91/91. Исходный upstream-хеш в обоих
коммитах относится к правильному источнику `b7f57bd4`; общая история сохранена.

## Проверка совместимости кэша точечных источников света

`point-light-save-layout` сначала воспроизвел несовпадение размера двух
retail-записей: 64 байта в сохранении против 56 в памяти. Отдельный тест
старого формата remaster выявил обратное несовпадение при прямом переходе
на 32 байта. Совместимый читатель принимает оба размера; новая запись
сохраняет retail-формат, цвет, положение, радиус и флаг теней. Для прежних
28-байтовых записей сохраняется старое поведение проверки теней.

После этой адаптации Windows ALL_BUILD и Linux Game собираются; на каждой
платформе проходят 104/104 регрессии. Windows набор без внешних ресурсов
и дисплея — 183/183, отдельные сценарии стрельбы/сохранения/перезарядки —
4/4. Это локальные проверки; результат GitHub Actions отслеживается отдельно.

### Контроль после света и консоли

После отдельных переносов `cb065556` и оставшихся частей `abc36d82`
полная Windows `ALL_BUILD` и Linux `Game` собираются; профильная группа
112/112 на каждой платформе. Более широкий набор без окна на Windows —
191/191; реальные выстрелы, сохранение и загрузка — 4/4.
Логи: `lighting-console-checkpoint-ci-test.log`,
`lighting-console-shots-full-test.log`, `console-linux-test.log`
в общей папке свидетельств. Это промежуточная проверка, перенос остальных
коммитов и финальная проверка GitHub Actions ещё продолжаются.

### Совместимость удержания владельца AI

После `4252cc86` Linux-сборка выявила неоднозначное для Clang преобразование
`CPtr<IAIUnit>` в `CObj<IAIUnit>` в добавленном коде удержания владельца
состояния. Явный `GetPtr()` устраняет ошибку компиляции без изменения логики.
Повторные Windows и Linux проверки — 115/115, Linux `Game` собран.
Это исправление переносимости адаптации `77393cff`, а не новый upstream баг.
Свидетельства: `ai-locks-linux-build.log` (исходная ошибка),
`ai-locks-portable-linux-build.log`, `ai-locks-portable-linux-test.log`,
`ai-locks-portable-green-test.log`.

### Контроль после навигационных исправлений

`5f9b0d8f` и `89ee0503` перенесены отдельными исправлениями первого построения
связности, первичного соединения слоёв, временных occupancy locks и
недоступного кэша фоновой задачи. Windows `ALL_BUILD` и Linux `Game`
собираются; профильная группа — 120/120 на обеих платформах.
Более широкий Windows набор — 197/197, реальные выстрелы с save/load — 4/4.
Логи: `navigation-checkpoint-linux-test.log`,
`navigation-checkpoint-ci-test.log`, `navigation-checkpoint-shots-full-test.log`.
Перенос остальных источников и итоговая проверка GitHub Actions продолжаются.

### Сверка выпадения рюкзака с оригиналом

Оригинальный Steam `Game.exe` SHA-256
`4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705`
содержит `DropItems` по адресу `0x750a10`. После цикла запуска моделей
отдельный цикл `0x750e23..0x750e54` берёт первый предмет из `GetItems()`
и вызывает `IInventory::Take` до опустошения списка, включая записи без
модели. Поэтому перенос сохраняет исходную политику, устраняя пропуск
записей при удалении внутри индексного обхода. Дизассемблирование сохранено
в `corpse-backpack-retail-disassembly.txt` общей папки свидетельств;
оригинальный EXE только читался, процесс оригинальной игры не запускался.

### Ограничение вложенности запускателя регрессий MSVC

При добавлении проверки приоритета реплик из `884f91f4` MSVC выдал C1061:
цепочка `else if` в запускателе достигла ограничения вложенности. Эта ошибка
сборки не считается runtime RED исправления диалогов. Диспетчер заменён на
плоские проверки с ранним возвратом; обработка исключений и аргументов
сохранена. Все существующие 124 сценария Windows проходят; неизвестный режим
и отсутствующие аргументы по-прежнему завершаются с ошибкой. Исходный C1061
зафиксирован в выводе компилятора в этой задаче; повторный RED диалогов
перезаписывает журнал его сборки. Отдельные свидетельства успешной поправки:
`regression-dispatch-green-build.log`, `regression-dispatch-green-test.log`.
Это отдельная служебная поправка тестов, игровая логика не меняется.

### Контроль после инвентаря, выпадения предметов и приоритета диалога

Windows `ALL_BUILD` и Linux `Game` собираются; профильная группа — 125/125
на каждой платформе. Более широкий набор Windows без окна и внешних
ресурсов — 199/199; настоящие выстрелы, сохранение и загрузка — 4/4.
Логи: `dialogue-priority-linux-test.log`,
`dialogue-backpack-checkpoint-ci-test.log`, `dialogue-backpack-shots-full-test.log`.
Остальные upstream источники и итоговая проверка GitHub Actions остаются
в работе; этот контроль не означает завершения всей цели.

### Размер объектного файла тестовой программы MSVC

При добавлении теста для `cf087f74` MSVC остановился с C1128 (предел числа
секций COFF); это ошибка сборки, а не поведенческий RED игрового бага.
Для `UpstreamBugTests` включен `/bigobj` только под MSVC. Незавершенный тест
временно убран в один небольшой патч вне репозитория, прежние 133/133
проверки прошли; после коммита патч возвращается и удаляется. Логи:
`upstream-bigobj-compiler-red-build.log`, `upstream-bigobj-green-build.log`,
`upstream-bigobj-green-test.log`. Это поддержка тестового стенда; сам
`cf087f74` пока не отмечен перенесенным.

### Контроль после готовности интерфейса, голосов, музыки и scripted holster

Профильная группа Windows/Linux — 141/141 на каждой платформе.
Windows `ALL_BUILD` и Linux `Game` собраны с отключенной генерацией HD.
Расширенный Windows набор без окна/внешних ресурсов — 203/203;
настоящая стрельба, сохранение и загрузка — 4/4. Логи:
`checkpoint-141-all-build.log`, `checkpoint-141-ci-test.log`,
`checkpoint-141-shots-test.log`, `explicit-reset-linux-test.log`.
Один текущий каталог сборки на платформу переиспользован; временный маленький
патч, использованный для отдельного коммита `/bigobj`, возвращен и удален.
Остальные upstream источники и итоговый запуск GitHub Actions остаются в работе.

### Контроль после завершения частей `e5830013`

Windows/Linux профильная группа — 145/145; Windows `ALL_BUILD` и Linux `Game`
собраны. Расширенный Windows набор — 203/203, настоящая стрельба и
сохранение/загрузка — 4/4. Логи `checkpoint-145-all-build.log`,
`checkpoint-145-ci-test.log`, `checkpoint-145-shots-test.log`,
`movement-camera-linux-test.log`. Параметры автоматической камеры проверены
на настоящем UI-пакете. Состояние последней команды проверено именно в
одношаговой ветке upstream; более длинная ветка сохранена.
Общий покоммитный перенос и итоговый GitHub Actions еще не завершены.

### Контроль после формата журнала перезарядки ИИ

Windows/Linux — 148/148; Windows `ALL_BUILD`, Linux `Game` собраны.
Расширенный Windows набор — 206/206; настоящая стрельба, сохранение и
загрузка — 4/4. Логи `checkpoint-148-all-build.log`, `checkpoint-148-ci-test.log`,
`checkpoint-148-shots-test.log`, `ai-reload-log-linux-test.log`.
Три новых проверки журнала ИИ работают без внешних игровых ресурсов и
включены в набор GitHub. Завершение всего переноса и итоговый CI впереди.

### Контроль после подготовки оружия к перезарядке

Windows/Linux — 150/150; Windows `ALL_BUILD` и Linux `Game` собраны.
Нативный тест выполняет всю последовательность: доставание оружия, его
появление на time label, запуск reload после конца подготовительной анимации,
загрузка патронов и завершение счетчика действия. Место, поза и направление
солдата сохраняются. Расширенный Windows набор — 206/206; настоящая стрельба,
сохранение и загрузка — 4/4. Логи `checkpoint-150-all-build.log`,
`checkpoint-150-ci-test.log`, `checkpoint-150-shots-test.log`,
`reload-preparation-linux-test.log`. Итоговый GitHub Actions и оставшиеся
источники отчета еще в работе.

### Контроль после формата действия перезарядки

Windows/Linux — 151/151; Windows `ALL_BUILD` и Linux `Game` собраны.
Расширенные проверки Windows — 206/206; настоящая стрельба и
сохранение/загрузка — 4/4. Логи `reload-action-wire-{red,green}-test.log`,
`reload-wire-linux-test.log`, `checkpoint-151-all-build.log`,
`checkpoint-151-ci-test.log`, `checkpoint-151-shots-test.log`.
В первой версии теста не был перемотан поток перед чтением; после исправления
стенда игровой фикс временно снят, правильный RED воспроизведен заново,
затем фикс возвращен. Внешние ссылки магазина зарегистрированы в стенде;
проверка завершения использует FINISHED, поскольку счетчик действия живет
до освобождения executor. Временный патч исходников удален; копий сборок нет.
Все части `61b033f2` завершены. Оставшиеся источники и итоговый GitHub Actions
продолжают проверяться.

### Контроль после новых сцен и анимированных источников света

Профильная группа Windows/Linux — 154/154, Windows `ALL_BUILD`, Linux `Game`
собраны. Расширенные проверки Windows — 209/209; настоящая стрельба и
сохранение/загрузка — 4/4. Логи `fresh-ambient-graph-{red,green}-test.log`,
`fresh-ambient-linux-test.log`, `checkpoint-154-all-build.log`,
`checkpoint-154-ci-test.log`, `checkpoint-154-shots-test.log`.
Нативная сцена создается без окна/GPU; тест проверяет реальные связи узлов,
обновление цвета, конец анимации и возврат к настроенному фоновому свету.

### Контроль после сжатой записи сохранений

Профильная группа Windows/Linux — 155/155; Windows `ALL_BUILD`, Linux `Game`
собраны. Расширенные проверки Windows — 210/210; настоящая стрельба и
сохранение/загрузка — 4/4. В двух существующих post-shot проверках мира
использован новый сжатый writer, включая сохранение в слот и восстановление.
Логи `compressed-save-writing-{red,green}-test.log`,
`compressed-save-linux-test.log`, `checkpoint-155-all-build.log`,
`checkpoint-155-ci-test.log`, `checkpoint-155-shots-test.log`.
Тест настоящей `CICSaveFile` работает в одном отдельном каталоге внутри
текущей сборки. Блоки распаковываются по отдельности и сравниваются побайтно
с обычным writer; 1024 настоящие команды восстанавливаются с исходными слотами.
Несжатая запись и сетевой формат сохранены. Дополнительных копий сборок нет.

### Контроль после связей оконного дерева

Windows/Linux — 157/157; Windows `ALL_BUILD` и Linux `Game` собраны.
Расширенные проверки Windows — 212/212; стрельба и сжатое сохранение/загрузка
мира — 4/4. Логи `window-child-registration-{red,green}-test.log`,
`window-child-linux-test.log`, `checkpoint-157-all-build.log`,
`checkpoint-157-ci-test.log`, `checkpoint-157-shots-test.log`.
Дополнительный ownership-контроль сначала ошибочно отпускал `CObj`, хотя UI
владеет детьми через `CMObj`. Стенд исправлен на настоящий `CMObj`; игровые
две строки временно сняты, RED на отсутствующем parent воспроизведен повторно,
после возвращения фикса все проверки прошли. Семантика самих smart pointers
не менялась. Dirty latch и bool-return обновления из `d4de53d5` еще в работе.

### Разделение фабрики UI-скриптов и ссылок на окна

Фабрика проверена правильным RED после повторного снятия четырехстрочного
фикса, затем GREEN — 159/159 на Windows/Linux. Проверяются контекст интерфейса,
настоящий `GetCursorPos`, выполнение Lua и прежний тактический путь фабрики.
Первый дополнительный тест записи `window.x` обнаружил самостоятельную дыру:
`luaS_newudata` принимает только tags 6/7, а tag 8 оставляет без ссылки;
`lua_touserdata` тоже знает только прежние виды. Debugger подтверждает tag 8,
три корректных аргумента и null перед RTTI cast в `CheckProperty`.
Это отдельная адаптация, необходимая для callback-части `15d5d41f`;
оконный setter не считается исправленным фабричным коммитом.
Логи `worldless-ui-script-{red,green}-test.log`, `worldless-ui-linux-test.log`,
`worldless-ui-property-conditions.txt`. Связи callback/шаблонов еще в работе.

### Адаптация ссылок окна для callback-части `15d5d41f`

Это необходимая локальная адаптация Lua для переноса параметра `tagLuaWindow`
из upstream, а не дословный hunk его allocator: исходный diff не меняет
`lstring.cpp`/чтение ссылок. Новый tag 8 хранит borrowed `CPtr`, поскольку
деревом окон владеет UI через `CMObj`. Сохраненные owning-записи tag 8
продолжают читаться по прежнему полю `pObj`. Обычные tags 6/7 не изменены.
Тест подтверждает, что закрытие скрипта не уничтожает окно UI, а удаление
окна не откладывается из-за Lua. Явные `GetWindow/windowSetProperty` работают.

Windows/Linux — 160/160; Windows `ALL_BUILD`, Linux `Game` собраны.
Расширенный Windows набор — 215/215; стрельба/сжатое сохранение/загрузка — 4/4.
Логи `lua-window-userdata-{red,green}-test.log`, `window-userdata-linux-test.log`,
`checkpoint-160-all-build.log`, `checkpoint-160-ci-test.log`,
`checkpoint-160-shots-test.log`. Неправильная короткая форма `w.x=...`
диагностирована отдельно: setter уже вызывается, но threaded VM теряет
stack slot после tag callback, затем сообщает `attempt to call a userdata value`.
Это еще не считается исправленным; отдельный следующий RED предусмотрен.

### Контроль после Lua-контекста интерфейса

Windows/Linux — 162/162; Windows `ALL_BUILD` и Linux `Game` собраны.
Расширенные проверки Windows — 217/217; стрельба/сжатое сохранение/загрузка
мира — 4/4, включая новые state-context ссылки скрипта на самого себя.
Логи `lua-window-state-context-{red,green}-test.log`,
`state-context-linux-test.log`, `checkpoint-162-all-build.log`,
`checkpoint-162-ci-test.log`, `checkpoint-162-shots-test.log`.
Проверены собственное окно при чужом глобальном скрипте, отсутствие глобального
скрипта, сохраненный запасной путь и безопасный отказ полностью unbound Lua.
Шаблонные скрипты и onmessage из `15d5d41f` еще в работе.

### Контроль после оконных скриптов и ленивого обновления UI

`15d5d41f` завершён: шаблонные скрипты, уведомления и шесть событий мыши
выполняются в своём Lua state. Дескриптор кодируется как closure-index + 1:
наша VM выделяет слот 0 настоящей функции, поэтому upstream-проверка на 0
теряла первый обработчик. Анонимные обработчики удерживаются ключами функций
в существующем сериализуемом registry; счётчики позволяют делить одну функцию
между окнами и освобождать её при снятии регистрации или уничтожении окна.
Это локальная адаптация времени жизни, отсутствующая в буквальном diff upstream.

`d4de53d5` завершён: добавлены результат активности `Update`, стиль 0x80,
сброс и пробуждение флага tag 4; сохранены современные настройки экрана,
размещение combo popup и проверки недействительных детей окна.
Windows/Linux — 166/166; Windows `ALL_BUILD`, Linux `Game` собраны.
Расширенный Windows набор — 220/220; стрельба/сжатое сохранение/слот — 4/4.
Логи `window-message-callbacks-{red,green}-test.log`,
`window-callback-lifetime-{red,green}-test.log`,
`window-lazy-update-{red,green}-test.log`, `window-lazy-linux-test.log`,
`checkpoint-166-all-build.log`, `checkpoint-166-ci-test.log`,
`checkpoint-166-shots-test.log`. Это локальные проверки; финальный GitHub Actions
всё ещё должен быть проверен на конечном HEAD всего переноса.

### Контроль после исправления пустых навигационных поверхностей

`a3051bb1` завершён тремя отдельными RED/GREEN-коммитами: определение этажа
загруженной пустой ячейки, построение сетки без фиктивной опоры и обновление
уже нарисованной модели живого бойца при изменении этажа без перемещения.
На реальной карте 810 до исправления было 338 проходимых ячеек, из них
32 без поверхности; после исправления — 306, все с опорой.
Тест обновления модели проходит через настоящий visibility binding и
render visitor, проверяет подтверждение отрисовкой и сохранение позиции/AP.

Windows/Linux — 171/171; Windows `ALL_BUILD` и Linux `Game` собраны.
Расширенные Windows проверки — 223/223; стрельба/сжатое сохранение/слот — 4/4.
Логи `unsupported-navigation-{floor,bake}-{red,green}-test.log`,
`unsupported-navigation-bake-counts.log`, `living-unit-floor-refresh-{red,green}-test.log`,
`living-floor-linux-test.log`, `checkpoint-171-all-build.log`,
`checkpoint-171-ci-test.log`, `checkpoint-171-shots-test.log`.

### Уже имеющееся покрытие больших UV и DPI

[8d646658](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8d646658a98829230854e4029522fa7a025de712) — DPI защищён текущей SDL 3.4.16:
её Windows backend по умолчанию включает Per-Monitor V2, а игра использует
HIGH_PIXEL_DENSITY и перевод координат drawable/UI. Старый system-aware
manifest переносить не требуется.

[ed8cb002](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/ed8cb00231ddd8637b52db341a810ecdec583021) — большие UV уже защищены адаптивной упаковкой
в `Main/TextureSampling.h` / `Main/GfxUtils.cpp`: масштаб выбирается до записи
SHORT2 и учитывается при выводе, смена масштаба разделяет пакеты.
`TextureSamplingTests` покрывает границу 4096, размеры 8192/16384, NPOT8204,
отрицательные координаты и диапазон за 32768.

`SDLPlatformInputTests` и `TextureSamplingTests` прошли в расширенном
`checkpoint-171-ci-test.log` (223/223). В этих двух пунктах код уже был исправлен
в репозитории; новые RED/GREEN-переносы не заявляются.

### Контроль разделения владения бойцами ИИ и сведений о мире

Часть `2eee1b62` переносит регистрацию бойца только его командиру. Существующие
адаптеры союзников/противников получают живых бойцов из мира и сохраняют прежние
фильтры своего игрока/враждебной стороны. Расширение на нейтральных союзников,
новые alarm-запросы, обновление combat snapshot и доставка событий ещё в работе.

Windows/Linux — 174/174; полная Windows сборка и Linux `Game` собраны.
Расширенный Windows набор — 225/225; стрельба/сжатое сохранение/слот — 4/4.
Из шести дополнительных сетевых сценариев проходят Match, PartialMovement и
StopResume. ClientUI падает на `movement AP did not replicate`, Preparation —
на `path colour cache leaked to replica or removed on host`, ObjectStages —
на сравнении исходной модели либо с ошибкой доступа после нескольких стадий.
Все три воспроизвелись при временном удалении только текущих изменений владения
из `wMain.cpp`/`aistate.cpp`, то есть на функциональном состоянии HEAD `c6f4a03b`.
После сравнения изменения восстановлены с проверкой SHA-256 diff, временный
патч удалён. Это не подтверждает, что сбои существовали до всего переноса.
Они требуют отдельного исправления; зелёным весь сетевой набор не объявляется.

Логи: `owned-ai-commander-rosters-{red,green}-test.log`,
`owned-rosters-linux-test.log`, `checkpoint-174-{all,restored}-build.log`,
`checkpoint-174-ci-test.log`, `checkpoint-174-shots-test.log`,
`checkpoint-174-network-test.log`, `owned-rosters-network-baseline-test.log`.

Подготовка сетевой игры исправлена отдельной адаптацией теста после `8853794e`:
проверка теперь читает поле кэша реплики напрямую, а не вызывает создающий
его `GetColourer`. Полный `NetworkPreparationTests` прошёл для отрядов из 1 и
6 бойцов (49,63 с); логи `network-preparation-cache-{red,green}-test.log`.
Ошибки ClientUI/ObjectStages пока остаются открытыми.

### Локальная синхронизация проверки AP сетевого интерфейса

`NetworkClientUITests` из [782eb719](https://github.com/mrartanis/silent-storm-remaster/commit/782eb719e1325f3a6ad3f2feeeab01d898a4d415)
проверял AP реплики сразу после завершения серверной команды. Подтверждённый
RED: до движения 53 AP, после него сервер уже имел 49, а реплика ещё 53.
Проверка теперь ждёт применения клиентом версии снимка на момент завершения
и затем отдельно проверяет AP. Дополнительно подтверждается расход AP самим
движением. Ожидание по версии снимка не заменяет проверку содержимого снимка.

Это локальное исправление теста, а не скопированная игровая логика upstream.
`NetworkClientUITests` прошёл после исправления и ещё три раза подряд;
логи `network-client-ap-wait-{red,green}-test.log`,
`network-client-ap-wait-repeat.log`. ObjectStages остаётся в разборе.

### Стабильное сопоставление объектов в сетевой проверке повреждений

Локальный тест из [ef3ad054](https://github.com/mrartanis/silent-storm-remaster/commit/ef3ad054da1529a36a647089238b44282f3aabe2)
сохранял итератор клиентского списка между применениями сетевых снимков,
которые пересоздают этот список. RED: `initial object presentation differs`
после полного прохода стадий первого объекта; в предыдущих запусках также
проявлялась ошибка доступа. Теперь пары объектов собираются до изменения мира
и хранятся как слабые ссылки; проверяются исходные DB ID и валидность ссылок.
Игровой код и владение объектами не изменены, новая upstream-логика не заявляется.

GREEN: ObjectStages 1/1 и три повторения подряд, затем весь выбранный набор
из шести сетевых сценариев 6/6 (50,21 с). `NetworkEncounterMapsTests` в этот
набор не входит и остаётся отдельной незавершённой проверкой.
Логи: `network-object-pairs-{red,green}-test.log`,
`network-object-pairs-repeat.log`, `network-object-pairs-full-network-test.log`.

### Промежуточная проверка после очистки игроков и отображения скрытности

На HEAD `b0d649d2` собран Windows `ALL_BUILD` и Linux `Game`.
Общий upstream-набор с NativeBind/NativeBound — Windows и Linux 184/184.
Расширенный набор без игровых ресурсов/окон — 226/226; стрельба, сжатое
сохранение и слот — 4/4; шесть дополнительных сетевых сценариев — 6/6.
Это промежуточный результат, а не итоговый GitHub Actions на финальном HEAD.
`NetworkEncounterMapsTests` и семь отдельных проверок графического окна
не входят в объявленные здесь успешные наборы.

Логи: `checkpoint-184-all-build.log`, `checkpoint-184-ci-test.log`,
`checkpoint-184-shots-test.log`, `checkpoint-184-network-test.log`,
`checkpoint-184-linux-{configure,build,test}.log`. Предыдущий Linux этап
очистки игроков — `player-teardown-linux-{configure,build,test}.log` (179/179).

Повторный отбор ещё продолжается: из 281 источника 54 перенесены полностью,
8 частично, 35 уже решены локально; 177 остаются в разборе. Ещё 7 строк —
исторический импорт, документация, подтверждённое исключение баланса и
проверка последующих версий. Эти числа описывают исходные коммиты,
а не число отдельных исправлений.

Уточнение тестов `begin-turn-live-contacts`: исходная миссия предоставляет
одного человеческого бойца, поэтому дружеский контроль создан штатным
`AddUnitInGame` на свободной клетке. Первый вариант проверки также требовал
немедленной очистки выбранного контакта — это отдельная часть `d247d37c`.
Диагностический запуск показал правильные списки после HP-исправления и
оставшиеся выбранные указатели. Проверки сужены до списков контактов; обе
исходные HP-проверки в движке временно восстановлены, RED подтверждён заново,
затем применён текущий фикс и получен GREEN 191/191. Дополнительный лог —
`begin-turn-live-contacts-diagnostic-test.log`.

### Проверка AI-пакета и падение сценария 5247

На `dde095cf` Windows `ALL_BUILD` и Linux `Game` собраны, Linux upstream-набор
196/196, расширенный Windows набор 231/231, выбранные сетевые сценарии 6/6.
Стрельба/сжатое сохранение/слот с самостоятельным выстрелом ИИ после загрузки
прошли 4/4. Дополнительно впервые в этом этапе запущен
`NativeWorldMission5247PartySaveTurn`: общий список из пяти тестов дал 4/5,
сценарий 5247 упал с ошибкой доступа при скриптовом `UnitSetDirection`.
Это не свидетельство, что падение существовало до всего переноса.

CDB показал `CNodesLayer::GetSamePoints` через `ChangeDynamicLocks`.
Отдельный RED воспроизвёл нормализацию летящей точки (`final=1`,
`integral=1`, высота в поле слоя 160 при одном слое навигации).
После переноса соответствующего guard из `8e620755` upstream-набор прошёл
197/197, затем заново скомпонованный `NativeWorldInitProbe` выполнил
`NativeWorldMission5247PartySaveTurn` — 1/1. Остальные части `8e620755`
переносятся отдельно.

Логи: `checkpoint-196-{all-build,ci-test,shots-test,network-test}.log`,
`checkpoint-196-linux-{configure,build,test}.log`,
`checkpoint-196-turn-red-cdb.log`, `flying-dynamic-locks-{red,green}-test.log`,
`flying-dynamic-locks-scenario-green-{build,test}.log`.

### Linux registration correction after the airborne attack test

The new `UpstreamFlyingAttackQueue` registration in `e3668019` incorrectly used
Windows-only `S2_GAME_DIR` in the standalone Linux configuration. The first
Linux run at checkpoint 203 built `Game`, but this test and the still-uncommitted
scripted passage test were `Not Run` because the working directory was empty.
This is a local test-registration mistake, not an engine RED.
The Linux registration now uses the existing `S2_GAME_DB_PATH`,
`_s2_upstream_res_dir`, and `_s2_upstream_game_dir` variables.
After reconfiguration the full Linux group passed 203/203, including both new
tests; Windows passed 203/203 and the native shot/save/load/5247 scenarios 5/5.
Evidence: `checkpoint-203-linux-registration-red-test.log`,
`checkpoint-203-linux-{configure,build,test}.log`,
`checkpoint-203-scenario-green-{build,test}.log`.
This commit contains only the already-committed airborne test registration
repair; the scripted passage runtime correction is committed separately.

### Контрольная точка 206 — `aa3f388f`

Windows `ALL_BUILD` собран; Linux `Game` собран и upstream-набор прошёл
206/206. Расширенный Windows набор без игровых ресурсов и семи display/runtime
проверок прошёл 233/233. Выбранные сетевые сценарии прошли 6/6, а стрельба,
сохранение, слот с самостоятельным выстрелом ИИ после загрузки и сценарий
`NativeWorldMission5247PartySaveTurn` — 5/5.

Это не результат всего CTest: `NetworkEncounterMapsTests` с ранее отмеченным
таймаутом не входил в выбранные шесть сетевых сценариев; семь display/runtime
проверок также не запускались. GitHub Actions на итоговой ревизии всей задачи
ещё предстоит проверить; текущие промежуточные запуски отменяются новыми push.

В реестре 281 исходный коммит: 56 imported, 10 partially-imported,
35 already-present, 173 pending, 2 review-final-version, 3 documentation-only,
1 historical-own-import и 1 excluded-balance. Задача переноса не завершена.

Логи: `checkpoint-206-all-build.log`,
`checkpoint-206-linux-{configure,build,test}.log`,
`checkpoint-206-ci-test.log`, `checkpoint-206-network-test.log`,
`checkpoint-206-shots-test.log`.

### Контрольная точка 214 — `9bea3d14`

Windows `ALL_BUILD` и Linux `Game` собраны. Upstream-набор прошёл 214/214
на обеих платформах; после расширения проверок разрешённого сохранения в меню
полный набор запускался заново. Расширенный Windows набор прошёл 237/237,
включая тест позиции курсора с виртуальным SDL backend. Шесть выбранных сетевых
сценариев прошли 6/6, стрельба/сохранения/загрузка/ход сценария 5247 — 5/5.

Ограничения этой контрольной точки прежние: отдельный `NetworkEncounterMapsTests`
и семь display/runtime тестов не входят в эти результаты. Окончательная проверка
GitHub Actions и оставшийся покоммитный разбор ещё не завершены.

Логи: `checkpoint-214-all-build.log`,
`checkpoint-214-linux-{configure,build,test}.log`,
`checkpoint-214-ci-test.log`, `checkpoint-214-network-test.log`,
`checkpoint-214-shots-test.log`.

### Контрольная точка 227 — завершение `6d313faa`

Навигационные части исходного коммита перенесены отдельными исправлениями:
поиск копий площадок, поиск ближайших целей, сохранение прямых переходов,
проверка переходов и записанных ссылок, выдача шагов и проверка занятости,
полный поиск пути из изолированного старта. Перед каждым исправлением получен
свой runtime RED. Проверки не меняют стоимость движения или игровые формулы.

Последний тест использует настоящий публичный `NAI::FindPath` и его кэш зон.
Локальный поиск с лимитом 39 достигает цели, но исходный поиск с лимитом 100
не находит маршрут. Дополнительная временная замена на полное `DoNotCheck`
воспроизводит другую ошибку: маршрут заканчивается в `(4,2)` вместо `(5,2)`.
Окончательный фикс сохраняет ограничения позы и снимает только отсечение по
зонам для изолированного старта. Оба временных изменения восстановлены;
итоговый тест достигает точной цели и отвергает старт без связанного выхода.

Windows `ALL_BUILD` и Linux `Game` собраны. Upstream-набор прошёл 227/227
на обеих платформах, расширенный Windows набор — 250/250, выбранные сетевые
сценарии — 6/6, стрельба/сохранения/загрузка/ход сценария 5247 — 5/5.
Сценарий 5247 выполнен за 2.77 секунды. Проверки прошли в существующих каталогах
сборки, без копий старых сборок и генерации HD-текстур.

Эти результаты не включают `NetworkEncounterMapsTests` и семь display/runtime
проверок. Итоговый GitHub Actions ещё предстоит проверить после оставшегося
переноса. В реестре 281 коммит: 61 imported, 10 partially-imported,
35 already-present, 168 pending, 2 review-final-version, 3 documentation-only,
1 historical-own-import и 1 excluded-balance. Общая задача ещё не завершена.

Логи: `isolated-source-path-{red,green}-test.log`,
`isolated-source-path-pose-control-red-test.log`,
`isolated-source-path-all-green-build.log`,
`isolated-source-path-{world,network}-green-test.log`,
`checkpoint-227-linux-{configure,build,test}.log`, `checkpoint-227-ci-test.log`.

### Контрольная точка 233 — меню, сообщения и столкновения

После навигационного пакета полностью перенесены `26165318` (вкладка Save
в загрузке из главного меню), `5c13b0d0` (подтверждение выхода и сообщения
после успешного сохранения/загрузки) и `cc9fbeab` (диагностика отсутствующей
Lua-переменной и устранение повторного учёта радиуса в столкновениях).
Независимые исправления имеют отдельные коммиты и runtime RED перед фиксом.

Сообщения проверены на реальных слотах в отдельных тестовых профилях:
сохранённый заголовок и сжатый payload прочитаны обратно, при загрузке
восстановлен новый native-интерфейс. Заблокированная запись и отсутствующий
слот не сообщают об успехе. Промежуточный RED отдельно подтвердил, что простое
добавление сообщения после старого пустого catch ложно сообщает об успехе.

Тест столкновений генерирует штатную voxel-сетку для коробки толщиной 0.07.
Исходный код принимает округлённый voxel за столкновение в x=0.381 за точной
границей 0.38. Фикс расширяет только область поиска, сохраняя исходный радиус
0.31 и прежние проверки действительных столкновений в покое и движении.

Windows `ALL_BUILD` и Linux `Game` собраны, upstream-набор — 233/233 на обеих
платформах. Расширенный Windows набор — 252/252, выбранные сетевые сценарии —
6/6, игровые сценарии стрельбы/сохранения/загрузки/хода 5247 — 5/5; последний
занял 2.69 секунды. Использованы прежние каталоги сборки, без HD-генерации.

В реестре 281 коммит: 64 imported, 10 partially-imported, 35 already-present,
165 pending, 2 review-final-version, 3 documentation-only,
1 historical-own-import и 1 excluded-balance. Общая задача продолжается;
итоговый GitHub Actions ещё не проверен. `NetworkEncounterMapsTests` и семь
display/runtime тестов не входят в приведённые результаты.

Логи: `main-menu-load-save-tab-{red,green}-test.log`,
`exit-menu-confirmation-{red,green}-test.log`,
`save-success-feedback-{red,green,failure-control-red}-test.log`,
`load-success-feedback-{red,green,world-green}-test.log`,
`missing-lua-global-warning-{red,green,world-green}-test.log`,
`precalc-sweep-bounds-{red,green,world-green,network-green}-test.log`,
`precalc-sweep-bounds-all-green-build.log`,
`checkpoint-233-linux-{configure,build,test}.log`, `checkpoint-233-ci-test.log`.

### Контрольная точка 238 — снимки зон и прямые пути

Полностью перенесён `37e5bf12`: native-снимок зоны исключает связь скрипта с
HUD, восстанавливая её после окончания записи. Отдельный полный save/restore
настоящей `CMission` сохраняет эту связь. Первый тестовый подкласс не был
зарегистрирован в сериализаторе; ошибка окружения и CDB сохранены отдельно.
Окончательный RED получен на штатном классе до изменения рабочего кода.

`d14e92f9` сохраняет направление при выходе из положения лёжа, оставляя явно
заданные повороты. `c0c1914c` перенесён тремя независимыми исправлениями:
прямой подъём из положения лёжа, предпочтение прежнего направления при выборе
пути и точка перехода на новый слой до смены позы. Ранжирование кандидатов не
меняет цены AP. Часть исходных проверок уже эквивалентна native-коду:
`IsPassable` соответствует `AIP_YES`, проверка проходимости уже расширяет
занимаемую область для лежащего бойца, а sentinel равен 16. Контроль с
заблокированной соседней клеткой тела прошёл и до последнего RED; фиктивного
падающего теста для уже работающего поведения не создавалось.

Windows `ALL_BUILD` и Linux `Game` собраны. Upstream-набор — 238/238 на обеих
платформах, расширенный Windows набор — 256/256, выбранные сетевые сценарии —
6/6, игровые сценарии — 5/5; ход сценария 5247 занял 3.00 секунды.
`NetworkEncounterMapsTests` и семь display/runtime проверок не включены.
Итоговый GitHub Actions всей задачи ещё предстоит проверить.

В реестре: 67 imported, 10 partially-imported, 35 already-present,
162 pending, 2 review-final-version, 3 documentation-only,
1 historical-own-import и 1 excluded-balance. Перенос продолжается.

Логи: `zone-snapshot-excludes-hud-{red,green}-test.log`,
`zone-snapshot-excludes-hud-fixture-{test,red-cdb}.log`,
`prone-pose-change-facing-{red,green}-test.log`,
`simple-prone-pose-path-{red,green}-test.log`,
`simple-pose-facing-preference-{red,green}-test.log`,
`simple-cross-layer-pose-entry-{red,green,world-green,network-green}-test.log`,
`simple-cross-layer-pose-entry-all-green-build.log`,
`checkpoint-238-linux-{configure,build,test}.log`, `checkpoint-238-ci-test.log`.

### Локальное исправление регистрации для CI без игровых ресурсов

Существующая проверка сравнивала исходные списки Windows и Linux, но 31 тест
без игровых аргументов был ошибочно помещён под одинаковое условие наличия
игры. Новый runtime-контроль сгенерированного списка CTest дал RED в
конфигурации с пустым `S2_GAME_DIR`: отсутствуют именно эти 31 регистрации.
После выноса регистраций и свойства SDL dummy из условия набор прошёл 125/125;
все 122 теста движка без ресурсных аргументов присутствуют. Обычный путь
`G:/SS/lab/baseline` восстановлен, контроль регистрации снова прошёл 1/1.
Linux `Game` собран, полный upstream-набор прошёл 238/238.

Это локальная ошибка регистрации. В commit message указаны `Original-Test-Commit`
для 25 исходных коммитов регистраций и первоначальной проверки паритета;
ложного `Upstream-Commit` нет. Код движка и реестр 281 upstream-коммита не меняются.
Доказательства и полное происхождение тестов описаны в
[UPSTREAM-TEST-REGISTRATION.md](UPSTREAM-TEST-REGISTRATION.md).
Это не подтверждение реального GitHub Actions на окончательной ревизии задачи.

### Проверка после звука, режимов инструмента и разделения обзора противников

Последовательные Windows/Linux профили достигли 249/249, Linux `Game` собран.
Windows `ALL_BUILD` также собран. После исправления hotseat-обзора широкий
локальный набор без игровых ресурсов прошёл 261/261; шесть выбранных сетевых
сценариев — 6/6. Пять проверок нативного мира, выстрела, сжатого сохранения,
слота загрузки и command bridge прошли 5/5; сценарий 5247 завершился за 2.66s.

Широкий набор сохраняет прежние исключения: семь проверок экранного рендера,
тесты с baseline-ресурсами и метку extended. Это локальная проверка, не полный
CTest и не подтверждение GitHub Actions на финальной ревизии.

Свидетельства: `hotseat-opponent-visibility-{red,green}-test.log`,
`hotseat-opponent-visibility-all-green-build.log`,
`checkpoint-249-linux-{configure,build,test}.log`, `checkpoint-249-ci-test.log`,
`checkpoint-249-{world-green,network-green}-test.log` в общей папке evidence.

### Проверка после завершения звуковых исправлений `2826ebed` и подсказки карты главы

На контрольной точке 255 Windows `ALL_BUILD` и Linux `Game` собраны,
профили upstream прошли 255/255 на обеих платформах. Широкий локальный набор
без игровых ресурсов прошёл 264/264; выбранные сетевые сценарии — 6/6;
нативные проверки мира, выстрела, сохранения и загрузки — 5/5, включая 5247 за 2.63s.
Прежние исключения широкого набора перечислены выше. Это не полный CTest
и не подтверждение реального GitHub Actions окончательной ревизии.

Свидетельства: `checkpoint-255-all-green-build.log`,
`checkpoint-255-linux-{configure,build,test}.log`, `checkpoint-255-ci-test.log`,
`checkpoint-255-{world-green,network-green}-test.log`.

### Проверка после подсказок карты, ожидания окна улики и разметки редактируемого текста

Контрольная точка 260: Windows `ALL_BUILD` и Linux `Game` собраны;
upstream-профили прошли 260/260 на каждой платформе. Широкий локальный набор
без игровых ресурсов прошёл 266/266, выбранные сетевые сценарии — 6/6,
проверки нативного мира/выстрела/сохранения/загрузки — 5/5. Сценарий 5247
завершился за 4.87s при параллельном запуске других наборов.
Прежние исключения широкого набора сохраняются; это не полный CTest и не
подтверждение GitHub Actions окончательной ревизии.

Свидетельства: `checkpoint-260-all-green-build.log`,
`checkpoint-260-linux-{configure,build,test}.log`, `checkpoint-260-ci-test.log`,
`checkpoint-260-{world-green,network-green}-test.log`.

### Уточнение переноса редактируемого текста: сохранить собственные шрифты

После замечания пользователя перенос `3cf392b945550460e7982f3bce7fdba61ad287e9`
пересмотрен. Смена `CEdit` на базовую markup-разметку была слишком широкой:
старый formatter использует 22pt для неформатированного поля, новый путь — 16pt.
Сами файлы шрифтов не менялись, но этот побочный эффект исходная проверка
переданной ширины не обнаруживала.

Усиленная проверка с настоящими packaged vector fonts воспроизвела изменение
шрифта/размера на отправленном `3cf392b9` (`committed-head-red`). Отдельно
исходный formatter без исправления сохранял устаревшие метрики после смены
UI scale, поскольку его размеры оставались логическими и не обновляли узел.

Итоговая адаптация сохраняет весь прежний formatter, его размеры по умолчанию,
форматы, high-resolution font provider и off-wire caches. От кода до `3cf392b9`
отличается только передача `GetSize() * S2UI::Scale()` в пикселях. Шрифты,
их растеризация и другие потребители текста не изменены.

`UpstreamScaledEditTextLayout` теперь действительно форматирует и сравнивает
геометрию каждого символа, включая кириллицу, с неизменённым native formatter.
Проверяются System 22pt, Courier 16pt, Times 18pt и Arial 20pt при 100%,
150% и 200%, identity используемого font atlas, исходный текст и позиция курсора.
Все 12 сочетаний проходят. Это проверка геометрии и источника шрифта без GPU,
а не утверждение о пиксельном сравнении скриншотов игры.

Свидетельства: `preserve-native-edit-fonts-committed-head-red-test.log`,
`preserve-native-edit-fonts-red-test.log`,
`preserve-native-edit-fonts-real-glyphs-green.log`,
`preserve-native-edit-fonts-font-assets-green-test.log` (3/3: vector rasterizer,
portable/native font wire). Ранняя ошибка компиляции сравнения `CTRect`
исправлена до действительных runtime RED. Случайный запуск устаревшего exe
после той ошибки не принят за RED; основной RED-log затем перезаписан
действительным результатом окончательного fixture.

### Контрольная проверка после исправления кэша действий

Windows `ALL_BUILD` и Linux `Game` собраны. Регрессионные профили прошли
267/267 на каждой платформе; широкий локальный набор — 268/268;
сетевые сценарии — 6/6; нативный мир, выстрелы и сохранения — 5/5.
Сценарий 5247 завершился за 2.70s. Это локальная проверка выбранных наборов;
окончательный GitHub Actions после завершения всех переносов ещё требуется.

Свидетельства: `lazy-action-cache-all-green-build.log`,
`invalid-action-cache-query-green-test.log`,
`checkpoint-267-linux-{configure,build,test}.log`,
`checkpoint-267-ci-test.log`, `checkpoint-267-{network,world}-green-test.log`.

### Контрольная проверка после исправления позы исчерпанной отмычки

Windows `ALL_BUILD` собран на контрольной точке из 277 регрессий; полный
профиль этой точки прошёл 277/277. После добавления проверки отмычки и её
исправления пересобраны Windows `Game` и тестовый executable, связанный набор
прошёл 17/17. Linux `Game` собран с исправлением, полный профиль — 278/278.
Текущий широкий локальный набор Windows — 268/268, сетевые сценарии — 6/6,
нативный мир, выстрелы и сохранения — 5/5. Сценарий 5247 занял 2.63s.
Проверка `UpstreamScaledEditTextLayout` повторно прошла в широком наборе;
прежние packaged fonts, размеры и formatter сохранены.

Первоначально новая проверка отмычки была добавлена только в Windows-список.
`UpstreamRegistrationTests` обнаружил пропуск Linux-регистрации. Оба списка
исправлены и проверены повторно; первоначальные журналы с этим отказом и
прерванным повторным прогоном сохранены отдельно. Это ошибка регистрации
нового теста, а не игровой RED для исправления позы. Игровой RED отдельно
подтверждает оставшуюся позу после последнего заряда отмычки.

Свидетельства: `checkpoint-277-all-build.log`,
`checkpoint-277-full-test.log`, `spent-picklock-pose-cleanup-{red,green}-test.log`,
`spent-picklock-pose-cleanup-game-build.log`,
`spent-picklock-linux-{configure,build,test}.log`, `checkpoint-278-ci-test.log`,
`checkpoint-278-{network,world}-green-test.log`.
Это локальные проверки выбранных наборов; окончательный GitHub Actions после
завершения всех переносов ещё требуется.

### Передача следующей мины после окончания установки

Из `4c6b10fb` перенесено завершение `CExecSetMine`; вспомогательное выделение
следующего предмета заимствовано из `7e46d2a6`. Новый тест вызывает настоящие
обработчики метки и завершения анимации. Проверены видимая запасная мина,
отсутствие запасной и скрытый предмет из игровых данных; восстановление
обработчика из его полей сохраняет отложенную ссылку в добавленном теге 3.
Это проверка native field fragment, а не полного игрового save-файла.

Финальный RED воспроизведён с количеством предмета 1 на исходном production
коде после временного снятия всех его изменений и пересборки. Ранняя ошибка
подготовки количества в fixture исправлена до принятого RED. Текущий source
diff удерживался только в памяти инструмента; отдельные старые сборки и
временные копии исходников не сохранялись. Прежнее поведение boolean-варианта
`TearOffItem` сохранено для ещё не перенесённых обработчиков гранаты и ножа.
Дверная ловушка остаётся отдельным следующим исправлением.

Windows `ALL_BUILD` и Linux `Game` собраны; полные профили — 279/279 на каждой
платформе. Широкий локальный набор — 268/268, сетевые сценарии — 6/6, проверки
нативного мира и сохранений — 5/5; сценарий 5247 занял 3.20s. Проверки сохраняют
позицию и сложность разминирования установленной мины, AP, количество предметов
и исходную анимацию до завершения. Шрифты и HD-данные не изменены.

Свидетельства: `mine-placement-successor-handoff-{red,green}-{build,test}.log`,
`mine-placement-successor-handoff-game-build.log`,
`mine-placement-linux-{configure,build,test}.log`, `mine-placement-linux-full-test.log`,
`checkpoint-279-all-build.log`, `checkpoint-279-ci-test.log`,
`checkpoint-279-{network,world}-green-test.log`.
Окончательный GitHub Actions на финальном HEAD остаётся обязательным после
завершения всех переносов.

### Завершение установки дверной ловушки

`UpstreamTrapPlacementSuccessorHandoff` и
`UpstreamEngineerTrapPlacementSuccessorHandoff` по отдельности воспроизвели
раннее экипирование следующей гранаты до исправления `CExecSetTrap`.
Обе ветки теперь сохраняют отложенный предмет в добавленном теге 3 и передают
его при завершении анимации. Проверяются обычная/инженерная граната, запасной
предмет, его отсутствие, флаг скрытого предмета, перенос режима `SETTRAP`,
восстановленный обработчик и старые поля без тега 3 с уже экипированным
преемником. Такой старый формат дополнительно проверен для обработчика мины.
Это field-fragment проверки native обработчиков, а не полный игровой save.

Windows `Game` и Linux `Game` собраны. Связанный набор — 20/20 на каждой
платформе, широкий текущий локальный набор — 268/268. Последние полные профили
и `ALL_BUILD` относятся к предыдущей контрольной точке 279; полный текущий
профиль из 281 проверки в этом шаге не заявляется. Положение двери, замок,
ловушка, сложность разминирования, AP, количество и режим гранаты сохранены.
Прежний boolean-wrapper пока нужен отдельным не перенесённым обработчикам
гранаты и ножа; после их миграции его нужно удалить по итоговому upstream.

Свидетельства: `trap-placement-successor-handoff-{red,green}-{build,test}.log`,
`trap-placement-successor-handoff-game-build.log`,
`trap-placement-linux-{configure,build,test}.log`, `checkpoint-281-ci-test.log`.

### Передача запасного метательного ножа без перезапуска анимации

Следующий независимый фрагмент `7e46d2a6` использует уже перенесённый overload
снятия предмета. Настоящий `CExecThrowKnife::TimeLabelReached` воспроизвёл
отсутствующую ссылку на следующий нож в существующем поле сохранения 4.
Теперь нож экипируется на метке без новой `ActivateItem`-анимации, а обработчик
удерживает ссылку на него. Это соответствует knife-ветке upstream; в отличие
от установки мины/ловушки, экипирование здесь не ждёт отдельного finish-callback.

Тест требует настоящую анимацию броска, посещает новый `CKnifeServer`, проверяет
его израсходованный нож и бросившего юнита, один снаряд, окончание команды,
сохранённого преемника, восстановленное состояние, неизменные времена анимации,
скрытую руку во время броска, AP, VP и количество предметов. Есть варианты со
следующим ножом, без него и с флагом скрытого предмета. Проверка восстановления
по-прежнему относится к native field fragment, а не полному игровому save.

Windows/Linux `Game` собраны, связанный набор — 21/21 на каждой платформе;
Windows configured registration — 1/1. Последние полные профили относятся к
точке 279, широкий локальный набор 268/268 — к точке 281 перед этим исправлением.
Звуковой фрагмент `7e46d2a6` остаётся отдельным следующим исправлением.
Формулы броска из `5780c0be`, шрифты и HD-данные не переносились.

Свидетельства: `thrown-knife-successor-handoff-{red,green}-{build,test}.log`,
`thrown-knife-successor-handoff-game-build.log`,
`thrown-knife-successor-handoff-registration-test.log`,
`thrown-knife-linux-{configure,build,test}.log`.

### Звук метательного ножа и контрольная точка 283

Последний функциональный фрагмент `7e46d2a6` восстановил позиционный звук
`0x3285` после метки броска, с исходным подавлением при надетом Panzerklein.
Native test воспроизвёл отсутствующий звук, затем проверил новые `C3DSound` и
вычисленные функции их позиции: одна запись в точке обычного бросающего,
ни одной для бойца, действительно надевшего созданный native PK. Оба варианта
проходят проверку пригодности команды и создают один нож; обычный вариант
требует настоящую анимацию. Проверяется создание world sound records,
а не декодирование файла или слышимое воспроизведение через устройство.
`MakeSound` не вызывает `MakeAISound` и не меняет правила слышимости AI.

Этим завершён `7e46d2a6`: shared reservation, удержание следующего ножа и звук
проверены независимо. У `4c6b10fb` остаётся удаление временного boolean-wrapper
после миграции отдельных обработчиков гранаты. Параметры физики и попаданий
из соседних upstream-коммитов в эту группу не включались.

Windows `ALL_BUILD` и Linux `Game` собраны. Полные профили — 283/283 на каждой
платформе, широкий локальный набор — 268/268, сеть — 6/6, мир и сохранения — 5/5.
Сценарий 5247 занял 2.56s. Полные профили выполнялись последовательно по одному
процессу; сборки переиспользованы. Исходные шрифты и HD-данные сохранены.

Свидетельства: `thrown-knife-feedback-{red,green}-{build,test}.log`,
`thrown-knife-feedback-focused-test.log`,
`thrown-knife-feedback-linux-{configure,build,test}.log`,
`thrown-knife-feedback-linux-full-test.log`, `checkpoint-283-all-build.log`,
`checkpoint-283-ci-test.log`, `checkpoint-283-{network,world}-green-test.log`.
Окончательный GitHub Actions на финальном HEAD остаётся обязательным после
завершения всех переносов; локальная контрольная точка его не заменяет.

### Продолжение сохранённого броска гранаты

Отдельная часть `3cfccbee` исправляет потерю runtime-only калькулятора при
восстановлении `CExecThrowGrenade`. Тест сначала действительно выполняет `Run`,
инициализируя траекторию и action counter, затем восстанавливает native
field fragment и вызывает `TimeLabelReached` без повторного `Run`.
Прямая ветка проходила, восстановленные обычная/инженерная гранаты падали.
В обоих CDB-стеках `rcx=0` в `CGrenadeToHitCalcer::GetMaxGrenadeVelocity`,
вызванном из `CExecThrowGrenade::ThrowGrenade`.

Ссылка на гранату теперь записывается в уже существующий тег 4; временный
калькулятор создаётся при выполнении запроса. Отдельный RED промежуточного
исправления воспроизвёл пропуск броска у прежних локальных сохранений с пустым
полем 4. Для них сохранён старый lookup активной гранаты из `00af3fcb`;
существующая сохранённая ссылка не подменяется. После продолжения ссылка
вновь присутствует в сохранённых полях. Полный игровой save-файл в этом тесте
не создаётся; проверяется native обработчик и его действующие ссылки.

Проверены прямая ветка, новый и прежний форматы для обоих типов гранаты,
один настоящий `CGrenadeServer`, выбранный тип, бросающий и инженерный навык,
прежняя запрашиваемая цена, AP после старта, VP, количество и in-flight counter.
Fixture удовлетворяет исходным skill/perk gates и ищет доступную native цель;
ранние отказы подготовки не приняты за игровой RED. В real-time fixture AP
сохраняются по штатным правилам; повторного запуска `Run` после загрузки нет.

Windows/Linux `Game` собраны, связанный набор — 24/24 на каждой платформе,
Windows configured registration — 1/1. Последние полные профили и `ALL_BUILD`
по-прежнему относятся к точке 283, а не к текущим 285 проверкам.
Функции `RPGToHit`, числовые формулы и RNG не менялись. Остальные физические,
perk/PK/Hide-части описаны отдельно в `UPSTREAM-BALANCE-CHANGES.md`;
передача следующей гранаты, обработка недоступной траектории и animation flags
остаются независимыми следующими исправлениями.

Свидетельства: `restored-grenade-throw-{red,green}-{build,test}.log`,
`restored-{engineer-,}grenade-throw-red-cdb.log`,
`restored-grenade-throw-legacy-red-{build,test}.log`,
`restored-grenade-throw-game-build.log`,
`restored-grenade-throw-registration-test.log`,
`restored-grenade-linux-{configure,build,test}.log`.

### Запасная граната после окончания анимации

Native human-броски обычной и инженерной гранаты отдельно воспроизвели раннее
экипирование запасной гранаты на метке. Теперь ссылка удерживается в существующем
поле 5, а экипирование выполняется в `AnimationFinished`. Правило AI сохранено:
запасная граната остаётся в рюкзаке и автоматически не выбирается.

Оба типа проверены с запасным предметом, без него, с флагом скрытого предмета
перед завершением, с AI-запасом, восстановленными полями после метки и прежним
локальным форматом, где следующий предмет уже был активен. Проверяются один
снаряд, времена и флаги анимации, AP после старта, VP, количества, команды и
счётчик летящей гранаты. Native fixture извлечён из уже существующих тестов
загрузки; их контрольные случаи остались зелёными до нового production-фикса.
Проверяются field fragments, а не полный игровой save-файл.

Windows/Linux `Game` собраны; связанный набор — 26/26 на каждой платформе,
configured Windows registration — 1/1. Последняя полная контрольная точка —
283, полный текущий набор из 287 проверок здесь не заявляется.
Обработка неудачной траектории, animation flags и сохранённый флаг обновления
видимости остаются отдельными частями; числовые/Hide-изменения по-прежнему
описаны в `UPSTREAM-BALANCE-CHANGES.md` и не включены в этот фикс.

Свидетельства: `grenade-successor-handoff-{red,green}-{build,test}.log`,
`grenade-successor-handoff-game-build.log`,
`grenade-successor-handoff-registration-test.log`,
`grenade-handoff-linux-{configure,build,test}.log`.

### Сохранённый запрос видимости после броска гранаты

Отдельный RED `grenade-completion-visibility` подтвердил потерю уже сохранённого
`bUpdateVision`: при завершении восстановленного native executor отложенный
пересчёт мира не ставился в очередь. `AnimationFinished` теперь обслуживает
этот флаг, как в `3cfccbee`. Контроль без запроса не создаёт обновление;
с запросом выставляется native world queue flag. Скрытность и AP сохраняются.
Проверяется очередь в режиме отложенного пересчёта, а не новый результат
восприятия или полный игровой save-файл. Условия снятия Hide не переносились.

Windows/Linux Game собраны; связанный набор — 27/27 на каждой платформе,
Windows configured registration — 1/1. Последняя полная контрольная точка
остаётся 283; весь текущий набор из 288 проверок здесь не заявляется.
Свидетельства: `grenade-completion-visibility-{red,green}-{build,test}.log`,
`grenade-completion-visibility-game-build.log`,
`grenade-completion-visibility-registration-test.log`,
`grenade-visibility-linux-{configure,build,test}.log`.

### Траектория, ставшая недоступной до выпуска гранаты

Отдельный native RED воспроизвёл расход гранаты и запуск старых параметров
после перемещения бросающего на реальную свободную клетку карты во время
замаха. До перемещения Run успешно запускает бросок; после перемещения
штатный CanDoIt возвращает TARGET_OUT_OF_RANGE. Обработчик метки раньше
игнорировал тот же отказ FindGrenadeParams и продолжал выпуск. Теперь он
возвращается до разброса, запала, изъятия предмета и создания снаряда.

Обычная и инженерная гранаты проверены: нового CGrenadeServer нет, предмет
остаётся экипирован, следующий предмет не резервируется, завершение штатное,
AP после старта, VP и количество не меняются. Исходные прямые/сохранённые
успешные броски проверяются соседними регрессиями. Формулы физики, диапазоны
и RNG не изменены. Первоначальная попытка выбрать позицию другого бойца
не дала недоступной цели и не была принята за RED; принятый тест использует
свободное проходное место native path network.

Windows/Linux Game собраны; связанный набор — 28/28 на каждой платформе,
Windows configured registration — 1/1. Последняя полная контрольная точка
остаётся 283, весь текущий набор из 289 проверок здесь не заявляется.
Свидетельства: `invalidated-grenade-trajectory-{red,green}-{build,test}.log`,
`invalidated-grenade-trajectory-game-build.log`,
`invalidated-grenade-trajectory-registration-test.log`,
`invalidated-grenade-linux-{configure,build,test}.log`.

### Бросок гранаты после запасной стойки без предмета

Native RED подтвердил пропуск существующего authored USE/F1 клипа гранаты,
когда Stand выбрал допустимую запасную стойку без предмета. Тест временно
снимает только item eligibility у POSE-записей действующего скелета в памяти;
unarmed POSE и USE-клипы остаются штатными, оригинальные маски восстанавливаются.
Это проверка поддерживаемого fallback-сценария, а не утверждение, что каждый
стандартный скелет поставляется без стойки гранаты.

Однострочная часть `3cfccbee` пересчитывает item flags после Stand и до выбора
USE-клипа. Тест обходит настоящий native граф анимации и проверяет присутствие
клипа броска, прежние AP/предмет и штатный post-throw active flag. GPU-кадр
не проверяется. Первоначальный oracle, сравнивавший адреса прежних/новых
клипов, был нестабилен из-за повторного использования адреса освобождённого
объекта. Он заменён проверкой отсутствия/наличия подходящего authored клипа
в действующем графе до первого броска и после него; повторный RED подтверждён
на исправленном тесте без production-строки, затем GREEN и 10 повторов подряд.
Ошибки ранней подготовки fixture не использованы как свидетельство бага.

Windows/Linux Game собраны; связанный набор — 29/29 на каждой платформе,
Windows registration — 1/1. Последняя полная контрольная точка остаётся 283;
весь текущий набор из 290 проверок здесь не заявляется. Числовые, perk/PK и
Hide-правила из смешанного `3cfccbee` остаются описаны отдельно и не перенесены.
Свидетельства: `grenade-throw-animation-refresh-{red,green}-{build,test}.log`,
`grenade-throw-animation-refresh-game-build.log`,
`grenade-throw-animation-refresh-registration-test.log`,
`grenade-animation-repeat-test.log`,
`grenade-animation-linux-{configure,build,test}.log`.

### Снятие предметов без лишнего доставания из другого слота

Последняя часть `4c6b10fb` удаляет временный boolean-wrapper TearOffItem,
который добавлял ActivateItem даже после обработки пустого слота. Native RED
через публичный MakeUnconscious(VNULL3,true,false) подтвердил лишнюю анимацию
при выпадении предмета из активного второго слота. Скриптовый вариант явно
подавляет death clip; обработка пустого первого слота всё равно сдвигала время
и включала active flag. Теперь вызовы по умолчанию идут прямо в уже перенесённый
reservation overload, без анимационных побочных эффектов.

Проверены scripted unconscious hand-drop, прямое снятие из пустого слота и
настоящий CreateLostWeapon/Run. Выпавший предмет появляется в native debris
ровно один раз; запасной остаётся в рюкзаке. AP, VP и количества не меняются;
явная очистка позы при критическом выбивании сохраняется. Проверка позы
не утверждает результат GPU-рендеринга или обычного death clip. Ранняя попытка
вызвать закрытый DropItems не компилировалась и не принята за RED; принятый
тест идёт через действующий публичный игровой путь.

Этим `4c6b10fb` завершён: первоначальная/отменённая видимость, завершение
экипирования, исчерпанные меднаборы/отмычки, мины/ловушки и удаление wrapper
проверены независимо. Ножи и гранаты уже переведены на явное резервирование.
Windows/Linux Game собраны; связанный набор — 31/31 на каждой платформе,
Windows registration — 1/1. Последняя полная контрольная точка — 283,
весь текущий набор из 291 проверок здесь не заявляется.
Свидетельства: `default-item-detachment-{red,green}-{build,test}.log`,
`default-item-detachment-game-build.log`,
`default-item-detachment-registration-test.log`,
`default-detachment-linux-{configure,build,test}.log`.

### Общий опыт после явного задания уровня

Повторная проверка нового репозитория опровергла старую пометку аудита о том,
что XP-часть `0ffb568d` уже присутствует. Native `CUnit::SetXPLevel` задавал
XP-компоненты навыков, но затенял сохранённый `CSkilledObject::fXP` локальной
переменной. RED `assigned-level-experience` подтвердил устаревший общий опыт.
Минимальный фикс убирает локальное объявление и сохраняет результат прежнего
GetXPForSkill в действующий накопитель; формулы и пороги не меняются.

Проверены назначения уровней 3, 5 и снова 3, native saved base-field 2,
повторное обновление опыта, первое обычное начисление и сохранение личности/
инвентаря. Это native field fragment, а не полный игровой save-файл.
CUnit::AddXP учитывает интеллект и perks: ранний oracle «ровно +1» был неверен
(у выбранного персонажа штатно +1.25). Проверка сохраняет обычное начисление
от заданного общего опыта; её RED повторно подтверждён без production-фикса.
Оставшаяся FreezeToMax-часть исходного коммита не включена и описана отдельно.

Контрольная точка 292: Windows ALL_BUILD и Linux Game собраны; полные профили
Upstream/NativeBind/NativeBound — 292/292 на каждой платформе (207.45s/205.88s),
выполнены последовательно по одному test process. Широкий локальный набор —
268/268, включая configured registration; сеть — 6/6, мир/сохранения — 5/5.
Таким образом полные проверки охватывают также всю новую группу гранат и
завершение переноса `4c6b10fb`, а не только тест накопителя XP.
Это перечисленные локальные профили, не весь CTest и не подтверждение
GitHub Actions на текущем HEAD. Последний проверенный успешный hosted run
остаётся `40246010705` на `00af3fcb` (270/270 выполненных проверок); окончательный
workflow нужен после завершения всех переносов.

Шрифты, исходные текстовые размеры и HD pack не менялись. Сборочные деревья
переиспользованы; отдельных копий под баги нет, генерация HD/PNG выключена.
Свидетельства: `assigned-level-experience-{red,green}-{build,test}.log`,
`assigned-level-linux-{configure,build,test}.log`, `checkpoint-292-ci-test.log`,
`checkpoint-292-{network,world}-green-test.log`. Работа по исходному списку
281 коммита продолжается; pending-строки не считаются выполненными.

### Реакция отступления после передачи бойца охране

Независимая часть `432b64c8` воспроизведена через настоящий CAIRetreatReaction,
нативного владельца AI и SAIUpdateTracker. У точки отступления SetReaction
устанавливал Guard и освобождал последний CObj прежней реакции. CPtr счётчика
удерживал её allocation, но не содержимое. Хвост Update продолжал использовать
прежний cached unit и очищенную точку отступления. Native RED — access violation;
CDB подтвердил SAIUpdateTracker::Update → CAIRetreatReaction::Update →
CreateAIMoveToPositionLogic/HasPath → CPathNetwork::GetColourer.

Как в upstream, ссылка на владельца заново читается через GetUnit перед хвостом.
Правила испуга, выбора точки и движения не менялись. Проверены прибытие,
недопустимая точка и настоящая достижимая точка до прибытия. Первые два случая
оставляют Guard без новой movement logic; третий сохраняет live retreat и
штатную movement logic. AP, VP, место бойца и игровые команды не меняются;
нативный tracker потребляет ровно одну очередь. Тест не держит дополнительный
CObj реакции, который скрыл бы потерю последнего владельца.

Windows/Linux Game собраны; связанный набор — 26/26 на каждой платформе,
Windows configured registration — 1/1. Последняя полная контрольная точка —
292, весь текущий набор из 293 проверок здесь не заявляется. Inventory PK-часть
уже присутствует; group Hide, hit-label presentation и PK damage остаются
отдельными частями смешанного источника. Изменения PK-расчётов описаны в
UPSTREAM-BALANCE-CHANGES.md и сюда не включены.

Свидетельства: `arrived-retreat-reaction-lifetime-{red,green}-{build,test}.log`,
`arrived-retreat-reaction-lifetime-red-cdb.log`,
`arrived-retreat-reaction-lifetime-game-build.log`,
`arrived-retreat-reaction-lifetime-registration-test.log`,
`retreat-reaction-linux-{configure,build,test}.log`.

### Локализованный текст вместо отрицательного hit sentinel

Следующая независимая часть `432b64c8` исправляет текст отклонённого попадания.
Native world hit event проходит через CMissionUI::UpdateHits и создаёт настоящий
CHitTracker. Его Draw раньше помещал служебное -1 в CText вместо существующей
локализованной строки 21060; это подтверждено runtime RED. Теперь отрицательное
значение заменяется этой строкой, а 0 и положительный урон форматируются как
раньше. Цветовые/физические/PK damage-части исходного коммита сюда не включены.

Проверены human/PK, -1/0/42, начало/половина TTL и истечение TTL. Тест читает
действительный CText после native Draw и прерывает view на входе в layout;
он не утверждает результат GPU-рисования или отрисованные пиксели glyphs.
Текущие цветовые теги, fade, шрифты и размеры сохраняются.
Windows/Linux Game собраны; связанный UI/font набор — 17/17 на каждой платформе,
Windows configured registration — 1/1. Последняя полная контрольная точка —
292; весь текущий набор из 294 проверок здесь не заявляется.

Свидетельства: `rejected-hit-label-text-{red,green}-{build,test}.log`,
`rejected-hit-label-text-game-build.log`,
`rejected-hit-label-text-registration-test.log`,
`rejected-hit-label-linux-{configure,build,test}.log`.

### Правильный курсор у экипированного melee оружия

Runtime RED части `c1ab9336` подтвердил: настоящее authored nonthrowing melee
оружие в native CUnitServer даёт ST_NORMAL_MELEE и курсор кулака 8, хотя UI
уже умеет курсор ножа 9. Исправлен только GetState: внутри прежней AT_MELEE
ветки активный IMeleeWeaponItem даёт существующий ST_NORMAL_KNIFE, пустая рука
сохраняет ST_NORMAL_MELEE. Новых wire values и action types не вводится.

Тест проходит через native инвентарь/сервер, настоящий CUnitTracker и
CStateAttack::UpdateCursor, без подмены GetState. Проверены пустая рука,
экипирование и снятие. Сохраняются AT_MELEE dispatch, native CCriticalsBan
для object/tile-команд, melee AP-cost query, AP/VP, количество и отсутствие
новой команды. Ограничения от ранений и обработка повреждений не меняются.
Это адаптация самостоятельной UI-части; полное upstream-разделение типов
затрагивает существующую legacy таблицу критик и требует отдельной сверки.

Windows/Linux Game собраны; связанный cursor/preview набор — 11/11 на каждой
платформе, Windows configured registration — 1/1. Последняя полная контрольная
точка — 292; весь текущий набор из 295 проверок здесь не заявляется.
Свидетельства: `equipped-melee-attack-cursor-{red,green}-{build,test}.log`,
`equipped-melee-attack-cursor-game-build.log`,
`equipped-melee-attack-cursor-registration-test.log`,
`equipped-melee-cursor-linux-{configure,build,test}.log`.

### Полная контрольная точка 295

На `024a8839` проверена вся накопившаяся группа: crash прибывшей реакции
отступления (`57decfb7`), локализация hit sentinel (`b40b9b00`) и native cursor
экипированного melee оружия (`024a8839`). Windows ALL_BUILD и Linux Game собраны.
Полные Upstream/NativeBind/NativeBound профили — 295/295 на каждой платформе
(211.21s/209.89s), выполнены последовательно по одному процессу. Широкий
локальный набор — 268/268 с configured registration; сеть — 6/6 (55.89s),
мир/сохранения — 5/5 (5.81s), включая 5247 за 2.63s.

Свидетельства: `checkpoint-295-all-build.log`,
`checkpoint-295-upstream-test.log`,
`checkpoint-295-linux-{configure,build,test}.log`,
`checkpoint-295-ci-test.log`,
`checkpoint-295-{network,world}-green-test.log`.
Это перечисленные локальные профили, не весь CTest и не успешный hosted
workflow на окончательном HEAD. Финальный GitHub Actions остаётся обязательным
после окончания всей очереди. Подтверждённая ранее hosted-точка — 00af3fcb,
run 40246010705 (Build и Test success, 270/270 выполненных проверок).
Исходные шрифты/размеры и HD pack сохранены; сборки переиспользованы,
генерация HD/PNG выключена, отдельных копий под баги нет.

### Предел горизонтального поворота корпуса при выстреле

Независимая анимационная часть `d919d910` воспроизведена на настоящем native
бойце с огнестрельным оружием. CUnitAnimator::Attack создаёт реальный
CATurnBody в графе скелета; его native GetFrame успешно вычисляет кадр.
Небольшой угол 0.5 проходит, при запросе 2.4 в узел без ограничения попадало
2.4 вместо допустимых 1.2 — runtime RED подтверждён после оценки кадра.

Только горизонтальный угол этого узла ограничен диапазоном [-1.2,1.2],
как в upstream. Проверены 0.5, +2.4, -2.4 и 0; native aimDir, входной луч,
положение/направление бойца, AP/VP, боеприпасы и игровые команды сохраняются.
Оценивается действительный граф/кадр CPU-скелета, не GPU-картинка. Реальный
выстрел с повреждением здесь не выполняется; боевые формулы не менялись.

Windows/Linux Game собраны; связанный animation/attack набор — 25/25 на каждой
платформе, Windows configured registration — 1/1. Последняя полная контрольная
точка — 295, весь текущий набор из 296 проверок здесь не заявляется.
Другая часть смешанного коммита, homogeneous GetDir в CFastRenderer, остаётся
для отдельного воспроизведения через его штатный InitProjective. Изменение
закрытой матрицы тестом само по себе не подтвердило бы ошибку игрового вызова.

Свидетельства: `shooting-torso-turn-limit-{red,green}-{build,test}.log`,
`shooting-torso-turn-limit-game-build.log`,
`shooting-torso-turn-limit-registration-test.log`,
`shooting-torso-linux-{configure,build,test}.log`.

### Диагностика попадания на равенстве порога

Полностью перенесён `394d64fb`: только два сравнения в RPG-логе CheckToHit/
CheckTileToHit меняются с < на <=. Функции по-прежнему возвращают прежнее
отношение chance/roll; RNG, фильтрация бросков и расчёт процента не меняются.
Нативные RED подтверждены независимо: для бойца chance=roll=92, для точки
chance=roll=87, возвращается 1, а прежний лог пишет false.

Тесты используют настоящих бойцов/оружие, native CalcCovers/CalcCoversForTile,
обычные расчёты и штатные случайные броски fixture. Через реальный ILogNotify
перехватывается строка RPG-лога. Проверены roll ниже/равно/выше chance; на
равенстве native PeekRay принимает hit-ветку без bIsMiss. Входные проценты
и RNG не подменяются; AP/VP/боеприпасы и команды сохраняются. Повреждение
живой цели в этой проверке не наносится.

Windows/Linux Game собраны; связанный набор — 10/10 на каждой платформе,
Windows configured registration — 1/1. Последняя полная контрольная точка —
295, весь текущий набор из 298 проверок здесь не заявляется.
Свидетельства: `hit-roll-equality-log-{red,green}-{build,test}.log`,
`hit-roll-equality-log-game-build.log`,
`hit-roll-equality-log-registration-test.log`,
`hit-roll-log-linux-{configure,build,test}.log`.


### Контрольная точка 301 и особенности форка

Проверена кодовая ревизия `33653150`: Windows ALL_BUILD успешен; полный
native upstream-набор Windows — 301/301 (233.77 с), Linux — 301/301 (228.72 с).
Локальный CI-набор — 269/269 (26.04 с), сетевой профиль — 6/6 (55.58 с),
world/save-профиль — 5/5 (5.88 с), включая 5247 (2.64 с). Профили запускались
последовательно, один тестовый процесс на платформу, в прежних каталогах
сборки. Это не утверждение об успешности всего CTest: известный отдельный
NetworkEncounterMapsTests здесь не запускался.

После контрольной точки 295 добавлены torso clamp, два hit-roll log теста,
projective ray test и два downed-head теста. `d919d910` теперь завершён:
`3b601b8b` переносит оставшийся CVec4/Unhomogen hunk. Тест сравнивает реальные
GetDir/GetPoints для 90 лучей на заданном публичном trace interval. Первоначально
более широкая проверка roundtrip выявила независимую погрешность инверсии
матриц даже после upstream-исправления; математические алгоритмы и точность
вычислений в этом переносе дополнительно не менялись. Тест не моделирует
полную collision scene и не проверяет GPU.

`7de2769d` перенесён коммитом `33653150`: игровой юнит сообщает о смерти или
потере сознания через CSetRender в уже существующий CHeadsController::KillHead.
Переписанный движок лицевой анимации, вычисление вершин, resource decoder и
графический backend не заменяются. Проверены реальный world render, общий
контроллер и портрет, переход IDLE_NORMAL -> IDLE_DEATH и сохранение состояния
после обновления/удаления портрета. CPU recording view удерживает входные
объекты без GPU-ресурсов: внешний вид лица и death-mask вершины не проверялись.

Уточнение пользователя для всех следующих переносов: сначала сверять смысл
ошибки с текущей реализацией форка. SDL, bgfx, native audio/media, собственная
лицевая анимация и изменённый HD pipeline учитываются отдельно; устаревший
backend-код upstream не возвращается механически. Эквивалентное существующее
решение подтверждается по текущему коду и подходящим runtime-проверкам.
Собственные HD-шрифты, их исходные размеры и замороженный data/res-hd сохраняются.

Реальный GitHub Actions run `40257943770` для более раннего HEAD
`645acb520c3f7ac582f47183ab6e7082e5249bd3` завершился успешно: Build и Test,
270/270 выполненных тестов (275 зарегистрированы, 5 native-audio отключены
в hosted-конфигурации). Это не проверка окончательного HEAD; после всех
переносов требуется новый успешный запуск для итоговой ревизии.

Свидетельства: `checkpoint-301-all-build.log`,
`checkpoint-301-upstream-test.log`, `checkpoint-301-linux-{configure,build,test}.log`,
`checkpoint-301-ci-test.log`, `checkpoint-301-network-green-test.log`,
`checkpoint-301-world-green-test.log`, `projective-ray-unprojection-*`,
`projective-ray-linux-*`, `downed-head-idle-*`, `downed-head-linux-*`,
`github-40257943770-{result.json,test.log}`.


### SDL-клавиатура и Escape в активном поле ввода

`ccdb6ca7` адаптирован к существующему SDL backend. Исторические DirectInput
таблицы и idNames не перенесены: распознавание клавиатуры использует текущий
SDLKeyTable, а интерфейс исключает Escape из поглощаемых физических событий.
Остальные клавиши передаются также при отпускании и отсутствии virtual-key
перевода; Unicode остаётся отдельным SDL text input. CEdit, его шрифты,
размеры и текстовый рендерер в этом исправлении не меняются.

Первый native RED: настоящий SDL Escape при активном CEdit поглощался раньше
реального cancel handler меню сохранения/загрузки. GREEN проверяет постановку
CICExitModal и единственное закрытие модального интерфейса. Setup восстанавливает
фрагмент полей настоящего handler с native cursor/interface; это не полное
сохранение игры и не проверка authored screenshot UI. Второй RED проверяет
отпускания и клавиши без virtual-key перевода. GREEN включает реальные A/KANA
SDL edges, Unicode `ZЖ`, а также отрицательные mouse/unknown controls.

Выбранный набор Windows/Linux — 18/18 на каждой платформе, обе Game собраны,
registration — 1/1; отдельно пересобранный Windows SDL профиль — 3/3.
Новый полный набор содержит 303 теста; последняя полная контрольная точка — 301.
Оба новых теста не требуют proprietary assets и зарегистрированы с SDL dummy
для hosted CI.

Расширенный Linux SDLPlatformInputTests проверяет настоящий native window и
fullscreen, поэтому ему требуется Xvfb + Openbox + noreset. Первые попытки без
дисплея, с dummy driver и с Xvfb без WM провалились по этим условиям окружения.
С правильным виртуальным дисплеем весь набор 18/18 прошёл без изменения его
проверок. Новые asset-free UI-тесты проходят в dummy driver.
Свидетельства: `edit-keyboard-routing-{red,green}-{build,test}.log`,
`edit-keyboard-routing-game-build.log`, `edit-keyboard-routing-registration-test.log`,
`edit-keyboard-routing-sdl-{build,test}.log`, `edit-keyboard-linux-*`.


### Контрольная точка 306 — компоновка и состояние карты главы

Кодовая ревизия `ab1d8566` прошла Windows ALL_BUILD, полный native-набор
Windows 306/306 (223.58 с), Linux 306/306 (224.28 с), локальный CI-набор
271/271 (26.03 с), network-профиль 6/6 (53.93 с) и world/save-профиль 5/5
(5.64 с), включая 5247 (2.52 с). Профили переиспользуют прежние сборки и
исполняют один тестовый процесс на платформу. Отдельный известный
NetworkEncounterMapsTests не включён; весь CTest этим не объявляется успешным.

После контрольной точки 301 добавлены SDL Escape/keyboard edges (eeaa1251),
активный hull двери (e2c81468), описание сектора глобальной карты (f79b0b89)
и мир главы (ab1d8566). Каждый независимый фикс имел свой native RED/GREEN,
отдельный commit с полным upstream URL и немедленный push.

Глобальная подпись в authored шаблоне 329 уже существовала как CText:
предположение об отсутствии текста не подтвердилось и не использовалось как
RED. Реальный RED — фиксированная высота панели 299 при измеренном тексте 216.
Подключён существующий CFrameText для подгонки высоты и рамки, с проверкой
реального GetRealSize на масштабах100/150/200. Сохранены исходная строка,
font markup и DB rectangle; GPU-картинка и обрезка текста не заявляются.

CICBeginChapter теперь создаёт штатный мир, player tracker/commander,
сцену/render, раздельные камеры игрока и cinematics, общий world-clock sound
scene и стек desktop. Native проверка сериализует в память полный граф
CGlobalGame + настоящего CChapterMap, загружает независимый граф, проверяет
все связи и продолжает реальные тики мира. Штатный AddPlayer размещает
живой отряд: тест проверяет его состав и идентичность RPG-объектов, а не
ошибочное предположение о пустом мире. Файловый save-slot и GPU не проверяются.
Импорт ScriptID/campZones.clear/save tags в DBFormat уже был реализован в
собственном typed database loader форка и повторно не переносился.

Остаются независимые части e9dc218f: запуск chapter script без tactical
warm-up, штатный OnGetFocus и минимальный frame interval. До переноса нового
цикла Step/очереди команд из70d12946 также нужна native проверка совместимости
со старыми собственными chapter saves, где прежний 2D Initialize сохранял
null world. Пропуск simulation по null guard не будет считаться переносом
необходимой логики. Сохранённый полноценный мир должен продолжаться без
повторной инициализации и сброса игрового состояния.

Актуальный GitHub Actions для кодового HEAD ещё выполнялся при проверке;
финальный успешный run на окончательной ревизии по-прежнему обязателен.
Свидетельства: `checkpoint-306-all-build.log`,
`checkpoint-306-upstream-test.log`, `checkpoint-306-linux-{configure,build,test}.log`,
`checkpoint-306-ci-test.log`, `checkpoint-306-network-green-test.log`,
`checkpoint-306-world-green-test.log`, `global-sector-description-*`,
`global-description-linux-*`, `chapter-world-graph-*`, `chapter-world-linux-*`.

### Сценарий главы: порядок исполнения в собственном Lua

Из e9dc218f отдельно перенесён вызов назначенного сценария главы в её
собственном мире. Импорт ScriptID в собственном загрузчике уже существовал.
Тест открывает настоящую карту главы через CICBeginChapter, временно связывая
её запись с контролируемым CScript в памяти; файлы базы не изменяются.
На коде без исправления первый настоящий сегмент мира не исполнял этот скрипт.

Первоначальное ожидание немедленного исполнения оказалось неверным для
текущего Lua форка: DoString создаёт отложенный поток. Проверка исправлена,
производственный код возвращён к HEAD, затем повторно получен подтверждённый
runtime RED. После исправления тело скрипта исполняется первым сегментом,
OnEnterZone — следующим, ровно один раз. Принудительный запуск потоков и
тактический прогрев не добавлены. Проверены независимость другого Lua-состояния,
его стека, состава отряда и назначения главы.

Связанный набор прошёл 23/23 на Windows и Linux, обе игры собраны;
регистрация Windows — 2/2. Последний полный checkpoint остаётся 306,
новое полное число проверок — 307. Проверяется CPU/runtime, без утверждения
о GPU-картинке или файловом save-slot. Свидетельства:
`chapter-script-initialization-{red,green}-{build,test}.log`,
`chapter-script-initialization-game-build.log`,
`chapter-script-initialization-registration-test.log`, `chapter-script-linux-*`.

Исходный коммит остаётся частично перенесённым: OnGetFocus и минимальный
интервал кадра требуют отдельных проверок; перед новым Step из70d12946
нужно проверить совместимость со старыми сохранениями форка с null world.

### Возврат звука главы после меню

Следующая независимая часть e9dc218f убирает пустой CChapterMap::OnGetFocus,
который перекрывал существующий CMissionBase::OnGetFocus. Теперь используются
его ResetTiming и Pause(false); собственный звуковой движок не изменён.
Native RED: после реального CICExitMenu/CICExitModal канал главы оставался
на паузе. Тест запускает настоящий PCM-канал через sound scene главы и
дважды проверяет остановку курсора под меню и продолжение после закрытия.
Независимый контрольный канал продолжает воспроизведение во время паузы главы.
Контролируемый PCM и запись кэша находятся только в памяти; физическое
аудиоустройство, GPU-картинка и фактический вывод на динамики не заявляются.

Набор главы/глобальной карты/меню/звука — 24/24 на Windows (21.17 с) и
Linux (20.60 с), обе игры собраны, регистрация Windows — 2/2.
Свидетельства: `chapter-focus-audio-{red,green}-{build,test}.log`,
`chapter-focus-audio-game-build.log`,
`chapter-focus-audio-registration-test.log`, `chapter-focus-linux-*`.
Последний полный checkpoint — 306, текущее полное число — 308.
Остаются минимальный интервал кадра и проверка/миграция старых собственных
chapter saves с null world; миграция должна также обеспечить безопасный
возврат фокуса после загрузки такого сохранения.

### Совместимость с прежней 2D-картой главы нашего форка

Это собственная адаптация переноса e9dc218f, отсутствующая в upstream.
Настоящим CICBeginChapter с историческим iChapterMap.cpp из f79b0b89
создан один эталон полного графа CGlobalGame + CChapterMap размером 132791 байт.
Остальные исходники при генерации — fb97e31c. Это контейнер нативного
CStructureSaver с корневыми тегами 2/3, без заголовка файлового save-slot;
происхождение и SHA-256 описаны в `fixtures/chapter-before-world-init.md`.
Существующая сборка переиспользована, исторический исходник восстановлен
к HEAD до подтверждения RED. Отдельных копий игры/сборки нет.

Подтверждённый RED: штатный OnSnapshotRestored оставлял старую главу без
мира, сцены и рендера. Теперь недостающий мир создаётся существующей логикой
инициализации главы. Сохранённые интерфейс, курсор, маршрут (цель 113,227),
сложность, назначение главы и RPG-объекты бойцов сохраняются. Интерфейс
подключается к общей звуковой сцене нового мира с выключением старого
независимого UI sound pump. Полноценный сохранённый мир использует только
RestoreRuntimeCaches, без CreateRestored/StartGame, прогрева или сброса времени.

Тест читает исторический граф, мигрирует его, продолжает реальные тики,
повторяет hook, сохраняет полный новый граф, загружает независимую копию и
снова проверяет продолжение тиков и безопасность потери/возврата фокуса.
Этот же Windows-эталон успешно читается на Linux. Проверяется именно граф
и общий hook загрузки; фактический путь через файловый save-slot не заявляется.

Набор с регистрацией — 28/28 на Windows (23.45 с) и Linux (22.87 с), обе
игры собраны. Последний полный checkpoint — 306, текущее полное число — 309.
Свидетельства: `legacy-chapter-world-migration-fixture-{build,test}.log`,
`legacy-chapter-world-migration-{red,green}-{build,test}.log`,
`legacy-chapter-world-migration-game-build.log`, `legacy-chapter-linux-*`.
Для завершения e9dc218f остаётся отдельная проверка минимального интервала
кадра; цикл Step и команды главы из70d12946 ещё не перенесены.

### Контрольная точка 309 — звук и старые сохранения главы

Кодовая ревизия 9a598684 прошла Windows ALL_BUILD, полные native-наборы
Windows 309/309 (231.20 с) и Linux 309/309 (230.57 с), обе игры собраны.
После ALL_BUILD повторены проверки обновлённых вспомогательных программ:
локальный CI-набор 271/271 (26.52 с), сеть 6/6 (57.62 с), world/save 5/5
(5.93 с), включая 5247 (2.66 с). Профили выполнялись последовательно,
по одному тестовому процессу на платформу. Известный отдельный
NetworkEncounterMapsTests не включён; успешным весь CTest не объявляется.

Новые независимые изменения: fcef9581 — постановка сценария главы в очередь
собственного Lua, fb97e31c — возврат звука после меню, 9a598684 — собственная
миграция старой 2D-карты главы. Каждое исправление имеет подтверждённый
runtime RED, GREEN, отдельный коммит с источником и немедленный push.
Миграция использует один эталон на 130 КиБ; архивных сборок/копий игры нет.

Проверка покоммитного плана продолжается: из 281 исходного коммита 85 imported,
21 partially-imported, 131 pending, 38 already-present, один требует проверки
окончательной версии, остальные — исторический импорт/документация/баланс.
Эта контрольная точка не означает завершение всего переноса.

Для e9dc218f остаётся минимальный интервал кадра, который нужно проверять
через поведение нового Step, а не только чтением поля. У70d12946 остаётся
сам Step и очередь команд. Дополнительно обнаружено, что вызываемая им в
upstream ExecWorldSoundCommand в текущем форке ещё отсутствует: обработка
PlaySound находится внутри CMission. Общая ветка и связанное время жизни
звука из e399ef89 требуют отдельного native RED, с учётом собственных
Lua/SDL/аудиореализаций. Простое изменение частоты GC без воспроизведения
ошибки не считается проверенным переносом.

GitHub Actions для этой кодовой ревизии:
[40298002103](https://github.com/mrartanis/silent-storm-remaster/actions/runs/40298002103).
При проверке Configure завершился успешно, Build ещё выполнялся, Test
оставался pending. Успех текущего запуска или окончательного HEAD не заявляется;
фактическая финальная проверка GitHub Actions остаётся обязательной.

Свидетельства: `checkpoint-309-all-build.log`,
`checkpoint-309-upstream-test.log`, `checkpoint-309-linux-*`,
`checkpoint-309-ci-test.log`, `checkpoint-309-network-green-test.log`,
`checkpoint-309-world-green-test.log` и журналы отдельных фиксов выше.

### StopSound и владение звуком сценариев

Из e399ef89 отдельно перенесена общая обработка звуковой команды и владение
каналами в CMissionBase::soundsList. На текущем коде форка оба native RED
подтвердили: StopSound очищал слабую ссылку команды, но настоящий 2D- или
3D-канал продолжал играть. Теперь StopSound ставит команду обратно в очередь
без записи звука, а обработчик удаляет соответствующего владельца канала.
Сохранены existing tag38 списка каналов и слабый tag5 ссылки команды.
Собственный звуковой backend и порядок исполнения Lua не заменяются.

Два теста исполняют настоящие PlaySound/Play3DSound/StopSound в Lua мира,
вызывают существующий CMission::ExecWorldCommands и sound scene, декодируют
PCM и проверяют реальные каналы/курсоры через native headless mixer.
Проверены повторная остановка без перезапуска, независимый контрольный канал,
продолжение воспроизведения после GC уже обработанной команды и освобождение
звука при освобождении миссии. Сохранность команды ДО её обработки при
неприсвоенном результате PlaySound этим не заявляется — это отдельная проверка.
Физический вывод на динамики и GPU-картинка не проверяются.

Для изоляции исправления сохранён прежний GetTime в аргументе старта 3D-звука.
Изменение этого аргумента на 0 из того же upstream требует собственного RED.
Частота GC не менялась. Общий обработчик теперь доступен CMissionBase,
но его подключение к циклам backdrop/главы ещё не считается перенесённым.

Связанный набор с регистрацией — 43/43 на Windows (33.03 с) и Linux
(31.74 с), обе игры собраны. Последний полный checkpoint — 309,
текущее полное число проверок — 311. Свидетельства:
`script-sound-stop-{red,green}-{build,test}.log`,
`script-sound-stop-game-build.log`, `script-sound-stop-linux-*`.

### Ожидающие команды и неприсвоенный результат PlaySound

Вторая независимая часть e399ef89 адаптирована к владению объектами в форке.
Upstream менял частоту GC; здесь сохранена прежняя частота, а ожидающие
команды принадлежат очереди мира до обработки. Native RED для 2D и 3D:
результат PlaySound/Play3DSound не присваивается переменной, полный GC
выполняется до обработки очереди, и звук не запускается. После исправления
запускается настоящий PCM-канал и обрабатывается следующая команда Pause.

Первый вариант фикстуры с одним живым userdata маскировал ошибку: текущий
collectudata пропускает соседний слот после помеченного объекта. Два живых
корня позволяют проверить другой слот. Отдельный недостижимый userdata
действительно освобождается, а живая глобальная переменная сохраняется —
прохождение теста не объясняется отключённым GC. Сам collectudata не менялся.

Также исправлена дополнительная GC-проверка теста StopSound из75378994:
luaC_collect без маркировки был только фазой удаления. Теперь вызывается
полный сборщик через luaC_checkGC с контролем освобождения недостижимого
объекта и сохранности живых корней. Исходный RED StopSound происходил раньше
и не зависел от этой проверки.

Очередь CWorld::uiCmdsList использует CObj; существующая передача через
Extract и потребители CPtr сохранены для асинхронных исполнителей камер.
Звуковая команда получает владельца на время обработки; после обработки
её сборка не останавливает канал из soundsList. Tag8 и форма записи указателя
в сохранении не меняются. Проверки PCM/курсоров, повторной остановки и
завершения миссии выполняются обоими звуковыми режимами.

Расширенный набор камер/сценариев/подсказок/звука — 64/64 на Windows
(47.84 с) и Linux (47.09 с). Полные наборы — 313/313 на Windows (235.15 с)
и Linux (231.43 с), Windows ALL_BUILD и обе игры собраны. Свидетельства:
`script-sound-unassigned-{red,green}-{build,test}.log`,
`script-sound-unassigned-all-build.log`,
`script-sound-unassigned-game-build.log`, `script-sound-unassigned-linux-*`,
`script-sound-unassigned-linux-full-*`. Успех GitHub Actions этим не заявляется.

### Контрольная точка 313 — очередь мира и звук сценариев

После исправления времени жизни ожидающих UI-команд успешно выполнены
Windows ALL_BUILD, обе игры и полные native-наборы: Windows 313/313
(235.15 с), Linux 313/313 (231.43 с). На актуальных пересобранных программах
прошли локальный CI-набор 271/271 (25.91 с), сеть 6/6 (52.11 с) и world/save
5/5 (5.64 с), включая 5247 (2.55 с). Профили шли последовательно по одному
процессу на платформу; NetworkEncounterMapsTests по-прежнему не включён.
Успех всего CTest не заявляется.

Свидетельства полного native-набора находятся в
`script-sound-unassigned-green-test.log` и
`script-sound-unassigned-linux-full-test.log`; остальных проверок —
`script-sound-unassigned-all-build.log`, `checkpoint-313-ci-test.log`,
`checkpoint-313-network-green-test.log`, `checkpoint-313-world-green-test.log`.
Сборки переиспользованы, архивных копий на каждый баг нет.

Общий обработчик звука теперь присутствует в CMissionBase. У70d12946 ещё
остаются сам Step, общая ветка BeginZone и подключение очереди главы.
Уe399ef89 отдельно остаются начальный offset 3D-звука, ввод после загрузки
и инициализация/обновление backdrop-меню. Поведение неприсвоенного результата
PlaySound исправлено через владение очередью; частота GC не менялась.

При последней проверке GitHub Actions для75378994
([40301213742](https://github.com/mrartanis/silent-storm-remaster/actions/runs/40301213742))
ещё выполнял Build; Configure прошёл, Test оставался pending. Успех
окончательного HEAD на GitHub остаётся отдельным обязательным условием
завершения всего переноса. Непросмотренные и частично перенесённые строки
покоммитного плана по-прежнему означают незавершённую работу.

### Начальное смещение нового 3D-звука

Из e399ef89 отдельно перенесён нулевой offset нового Add3DSound.
Раньше обработчик передавал GetTime как начальное смещение внутри записи.
При времени интерфейса 500 мс native RED подтвердил, что новый Play3DSound
начинался внутри PCM, а контрольный PlaySound — с нуля.

Тест управляет настоящими часами интерфейса через SetTimeForHarness,
восстанавливая прежнее значение при выходе. Новая вспомогательная функция
используется тестами; обычный игровой цикл продолжает задавать время как
прежде. Проверяется позиция реального PCM-канала до ручного рендеринга,
затем воспроизведение, StopSound, GC и независимый звуковой канал.
Нормальная работа собственного аудиодвижка и частота GC сохраняются.

Связанный набор — 66/66 на Windows (50.95 с) и Linux (48.69 с), обе игры
собраны. Последний полный checkpoint — 313; текущее полное число — 315.
Свидетельства: `script-sound-start-offset-{red,green}-{build,test}.log`,
`script-sound-start-offset-game-build.log`, `script-sound-start-linux-*`.
Оставшиеся части e399ef89 — ввод после загрузки и инициализация/обновление
backdrop-меню; их перенос ещё не завершён.

### Базовые часы главы и интервал обновления

Перенесены независимая часть Step из70d12946 и оставшийся default 5 мс
из e9dc218f. Native RED: настоящий CChapterMap::Step при контролируемом
времени не продвигал GetGameTime/GetUITime. Тест проверяет базовые счётчики
на неактивной карте: первая точка задаёт baseline, обновление через 3 мс
пропускается, после нужного интервала счётчики продвигаются. На паузе
останавливается игровой счётчик, интерфейсный продолжает идти; возврат
не проигрывает паузу повторно. Мир при неактивном интерфейсе не симулируется.

Проверены новая глава, настоящий исторический null-world граф, прежний
полноценный граф форка с нулевым интервалом и сохранённый нестандартный
интервал 7 мс. Нулевой старый placeholder заменяется на 5 мс при загрузке;
ненулевой интервал сохраняется. Повторная запись и независимая загрузка
полного графа сохраняют значения часов и заново устанавливают baseline
внешнего времени без скачка. Состав отряда, сложность и маршрут не меняются.

Учитывается собственный расчёт ChapterTravel: в Step/RenderFrame интерфейса
сохранён прежний GetTime. Подмена его на ограниченный GetUITime из upstream
могла бы замедлить перемещение при низком FPS; часть отложена в
`UPSTREAM-BALANCE-CHANGES.md`. ChapterTravelTests проходят на Windows и
Linux (на Linux тот же тест собран Clang в существующем каталоге сборки).

Связанный набор — 67/67 на Windows (52.04 с) и Linux (50.21 с), обе игры
собраны. Последний полный checkpoint — 313, текущее полное число — 316.
Свидетельства: `chapter-clock-step-{red,green}-{build,test}.log`,
`chapter-clock-step-game-build.log`, `chapter-clock-step-travel-test.log`,
`chapter-clock-linux-*`, `chapter-clock-linux-travel-{build,test}.log`.
Это CPU/runtime-проверка часов, без GPU-кадра и исполнения мира в активном
кадре. Для70d12946 ещё нужны world pump, BeginZone и обработка очереди главы.
Все части e9dc218f теперь учтены: предыдущие world/save graph, script hook,
OnGetFocus, собственный DB loader и данный default/frame interval.

### Активные кадры карты главы исполняют мир и Lua

Из70d12946 отдельно перенесён вызов существующего
CRenderGame::UpdateViewWorld внутри активного CChapterMap::Step. Native RED
получен со скрытым SDL-окном и настоящим bgfx Direct3D 11: интерфейс рисовал
кадр, но мир и назначенный Lua-сценарий не продвигались.

После исправления настоящий NMainLoop::StepApp исполняет инициализацию
сценария, затем возобновляет Sleep(2) по сегментам мира. OnEnterZone и
инициализация исполняются по одному разу. Неактивные кадры и активная пауза
останавливают симуляцию; после возврата она продолжается. Исправление
использует существующие классы мира/рендера/контроллеров форка. Сохранён
собственный GetTime для отрисовки и перемещения карты главы.

Новый NativeChapterActiveScript помечен extended/native-display и отдельно
проверяет активный путь. Его данные — настоящий шаблон главы с временно
назначенным CScript в памяти; стартовые уведомления погашены в тестовых
данных, чтобы не перекрывать главу модальными окнами. Файлы базы не меняются.
Скрытое окно и контекст рендера освобождаются штатно. Не используются
подменённый мир или фиктивный графический backend.

Связанный набор — 68/68 на Windows (54.12 с) и Linux (53.08 с), обе игры
собраны. В Windows активный тест прошёл на Direct3D 11, GPU10de:2482;
в Linux — на Vulkan, GPU0005:0000, под Xvfb/Openbox. Это доказательство
настоящих активных кадров и CPU/Lua-состояния; правильность пикселей,
мимики и обработка UI-команд этим не заявляются. Новый display-тест
не включён в прежний CPU-prefix: его число остаётся 316, плюс отдельный
NativeChapterActiveScript. Последний полный checkpoint пока 313.

Свидетельства: `chapter-active-script-{red,green}-{build,test}.log`,
`chapter-active-script-game-build.log`, `chapter-active-script-linux-*`,
`chapter-active-script-{windows,linux}-native-test.log`.
Для завершения70d12946 остаются очередь команд, общая ветка BeginZone,
обновления player tracker и desktop; их перенос ещё продолжается.


## Проверка обработки сценарных команд карты главы — `70d12946`

RED получен после успешной сборки на настоящей активной главе с SDL/bgfx:
ShowHint при выключенных подсказках навсегда оставлял Lua UI wait; включённая
подсказка и tutorial override не открывали интерфейс; BeginZone('ffight')
не создавал команду перехода. Четыре native-display случая проверяют именно
CChapterMap::Step через StepApp, с настоящим миром, Lua и существующими UI.

Добавлены обработка очереди и остановка кадра перед переходом, общий BeginZone
handler, существующие потребители подсказки, звука и меню отряда, загрузка
шаблона через прежний autosave helper и сохранённый флаг reenter. Отдельный
native RED на Linux проверил необходимую адаптацию порядка обновления:
закрытие меню отряда на первом кадре повторно добавляло прежнего бойца.
Список player trackers обновляется до dispatch; настоящий UI открывается,
закрывается, мир продолжает работу с теми же объектами бойцов и их количеством.

У главы сохраняются обычный вход без повторной обработки выхода из зоны,
собственный фон загрузки и прежний raw GetTime для UI/travel. Тактический
BeginZone сохраняет bEmulateChapter=true. Это проверено на реально созданных
командах перехода обоих путей; новый сектор в этом тесте не загружается.
Для подсказки проверены origin/event ID, реальное закрытие, возобновление Lua,
прекращение продвижения покрытого мира и tutorial override. Параметры шрифтов,
баланс, собственные SDL/bgfx/audio/facial реализации не заменяются.

Связанный набор: 73/73 Windows (62.22 с) и Linux (60.92 с), включая шесть
native-display случаев. Windows использует Direct3D 11, Linux — Vulkan/lavapipe
в Xvfb/Openbox. Это проверка работы native кадров, мира и UI-команд; отдельной
оценки пикселей шрифта/лица или скриншота загрузки здесь нет. Малый test-only
TakeInterfaceCommandForHarness позволяет проверить созданный переход и снять
его с очереди без исполнения; обычная игра его не вызывает.

Свидетельства: chapter-world-commands-red-{build,test}.log,
chapter-world-commands-roster-red-linux-{build,test}.log,
chapter-world-commands-green-{build,test}.log,
chapter-world-commands-linux-{configure,build,test}.log и отдельные
chapter-world-commands-{windows,linux}-native-test.log с успешным выводом
рендерера. Обе существующие сборки переиспользуются, копий игры не создаётся.
Источник остаётся частичным: собственная звуковая сцена главы требует отдельной
проверки запуска PCM через активный кадр; наличие handler само по себе
не доказывает воспроизведение. Подмена UI clock и повторы leave-zone
bookkeeping разобраны в UPSTREAM-BALANCE-CHANGES.md.


Итоговая проверка текущего кода после последней адаптации: CPU 316/316
Windows (241.81 с) и Linux (240.49 с); Windows ALL_BUILD и обе Game собраны.
После ALL_BUILD повторно проверены локальный CI-набор 271/271 (26.45 с),
сеть 6/6 (57.98 с), world/save/command bridge 5/5 (5.85 с).
Свидетельства: checkpoint-316-chapter-final-{all-build,full-test,ci-test,
network-test,world-test,linux-test}.log. Результат GitHub Actions для итогового
HEAD остаётся отдельным условием завершения всех переносов; эти локальные
проверки его не заменяют. Общая задача ещё не завершена.


## Звук сценария в активной главе — завершение адаптации `70d12946`

Четыре успешно собранных native-display теста воспроизвели одинаковый RED:
реальный Lua PlaySound/Play3DSound создавал сценовый объект, но активный кадр
главы не запускал ни одного PCM-канала. Проверены также звук и ShowHint в одном
кадре: обновление звука должно предшествовать выходу к новому интерфейсу.

В CChapterMap::Step после обработки команд добавлено получение listener transform
от существующей выбранной камеры и Draw существующей звуковой сцены. Это
адаптация сценарного звука к отложенному запуску в нашем native backend;
она не переносит чужой движок. Общая синхронизация звуков скрытых объектов
тактического мира не вызывается. Raw GetTime для UI/travel, баланс, HD-пак,
шрифты и правила перехода в сектор сохраняются.

Native тесты используют настоящий декодированный PCM в существующем кеше,
активные SDL/bgfx кадры и headless native audio. Проверены offset=0, продвижение
PCM cursor, пауза под настоящим меню и под подсказкой, поставленной в очередь
вместе со звуком; независимый канал продолжает работу. Возврат возобновляет звук,
Lua StopSound останавливает его, повторный PlaySound работает, закрытие главы
освобождает её канал без остановки независимого. Исходные данные WAV молчат;
это проверка запуска и жизни native канала, а не оценки слышимости или загрузки
холодного файла из пакета. Фикстура закрывает только своё скрытое окно.

GREEN: 77/77 Windows (68.92 с), 77/77 Linux (65.18 с); обе Game собраны.
В этот набор входят десять настоящих native-display случаев, четыре новых
помечены также native-audio и S2_AUDIO_HEADLESS=1. Windows — Direct3D 11,
Linux — Vulkan/lavapipe в Xvfb/Openbox. CPU-набор по-прежнему содержит 316
тестов; последний полный checkpoint316 на обеих платформах предшествует
этому изменению активного кадра. После успешного связанного набора он здесь
повторно не запускался. Обе прежние сборки переиспользованы.

Свидетельства: chapter-script-audio-red-{build,test}.log,
chapter-script-audio-green-{build,test}.log, chapter-script-audio-game-build.log,
chapter-script-audio-linux-{configure,build,test}.log и успешный вывод рендереров
в chapter-script-audio-{windows,linux}-native-test.log.

Все полезные части source 70d12946 учтены: framed description — f79b0b89,
часы — 3590693a, активный мир — 6e4b076e, обработчик команд и связанные
адаптации — cca92dbf, отложенный native звук — этот перенос. Статус источника
переведён в imported. Несовместимые буквальные изменения UI clock и повторной
обработки выхода описаны в UPSTREAM-BALANCE-CHANGES.md; они не применяются.
Это завершение одного источника; полный аудит 281 коммита ещё продолжается.

## Сценарный звук общих фоновых интерфейсов — часть `e399ef89`

Native CRenderBaseInterface::Initialize(2425) уже получает сценарий карты
через RunPostInit: настоящий вариант 4075 ссылается на сценарий 10. В тесте
его ссылка временно заменяется контролируемым Lua entry в памяти и затем
восстанавливается. Инициализатор исполняет entry один раз; нового запуска
сценария это исправление не добавляет, игровые файлы не переписываются.

Два успешно собранных native-display RED подтвердили, что активный общий
интерфейс не обрабатывал 2D/3D PlaySound из этого entry. В Step добавлены
исходный цикл GetUICommand -> существующий ExecWorldSoundCommand и очистка
ссылок. Имеющийся RenderFrame продолжает world/sound/scene/UI updates.
Тестовый наследник вызывает общий Step и полный native RenderFrame так же,
как фоновые интерфейсы, используя настоящий шаблон меню и его камеру 26.

GREEN: запуск реального декодированного cached PCM с offset=0, продвижение
cursor, Lua StopSound после world segments, повторный PlaySound, однократные
entry/OnEnterZone и освобождение звука при закрытии фона. Независимый native
канал продолжает работу. Входной WAV молчит; слышимость, пиксели и холодная
загрузка файла не оцениваются. Diagnostic-only NativeDisplayContext также
переиспользуется десятью тестами главы, которые продолжают проходить.

Результат: 79/79 Windows (72.98 с), 79/79 Linux (68.26 с), обе Game собраны.
Двенадцать случаев помечены extended/native-display; два новых также
native-audio с S2_AUDIO_HEADLESS=1. Windows — Direct3D 11, Linux —
Vulkan/lavapipe в Xvfb/Openbox. CPU count остаётся 316; последний полный
checkpoint316 предшествует этим локальным изменениям активного интерфейса.
Две прежние сборки переиспользуются. Своё скрытое окно фикстура закрывает.

Свидетельства: backdrop-script-audio-red-{build,test}.log,
backdrop-script-audio-green-{build,test}.log, backdrop-script-audio-game-build.log,
backdrop-script-audio-linux-{configure,build,test}.log и успешные блоки
backdrop-script-audio-{windows,linux}-native-test.log с renderer/variant IDs.
Источник остаётся частичным: принудительный script85/ручной ExecuteOwnScript
в конкретном главном меню и post-load input purge проверяются отдельно.
Наши рендерер, звук, SDL, мимика, шрифты, HD-пак и формулы сохраняются.

## Единственный сценарий карты главного меню — часть `e399ef89`

Первый native RED на настоящем CICMainMenu: после однократного variant entry
исполнялся ещё принудительно поставленный DB script85. Второй RED использует
настоящие тела сценариев 10 и 85: регистрация с передачей в исходный native
ObjectPlayAnimation показывает неправильный порядок команд для объекта Man.
Самостоятельный сценарий 10 уже содержит его последовательность анимаций,
фоновый PlaySound15691 и штатный sleeping camera thread. В 85 находится
повторная анимация и прежний camera fly. Содержимое проверено в текущем game.db.

Удалены только константа 85 и дополнительный launcher в Initialize. Существующий
RunPostInit продолжает запускать script10. Ручной ExecuteOwnScript в Step
сохранён для следующего независимого TDD-исправления. Тест использует реальный
интерфейс меню, шаблон 2425, библиотеки Scripts из baseline working directory,
изолированные каталоги пользовательских данных и восстановление временного
in-memory кода. Тело authored10 сохраняется, а регистрирующая Lua-функция
передаёт ObjectPlayAnimation исходной native-функции. Тело authored85 в проверке
анимации остаётся без изменений. Считываются native action state, порядок
2520/2520/2521 и продвижение настоящего cached headless PCM от PlaySound15691.
Это проверка вызовов, состояния и воспроизведения; пиксели лица не оцениваются.

GREEN 81/81 Windows (76.23 с), Linux (70.01 с), обе Game собраны. Новые два
native-display случая доводят их число до 14; authored case также native-audio.
CPU count остаётся 316; последний полный checkpoint316 здесь не повторялся.
Свидетельства: main-menu-map-script-red-{build,test}.log,
main-menu-map-script-green-{build,test}.log, main-menu-map-script-game-build.log,
main-menu-map-script-linux-{configure,build,test}.log и успешные renderer blocks
в main-menu-map-script-{windows,linux}-native-test.log. Исходник остаётся
частичным: ручной tick Lua и post-load input purge ещё требуют проверки.

## Корутины меню следуют времени мира — часть `e399ef89`

Отдельный compiled native RED воспроизвёл лишнее исполнение Lua в Step:
после установки baseline и постановки корутины четыре активных кадра с тем же
временем не продвигают world aim-time, но исполняют корутину. Дополнительный
ExecuteOwnScript вызван отдельно от уже существующего world update в RenderFrame.

Удалены только этот вызов и неиспользуемые заголовки. VM, GC cadence и прочие
интерфейсы не меняются. Новый тест на настоящем CICMainMenu требует отсутствия
исполнения между world segments, затем продолжает время и проверяет повторное
возобновление Sleep. Инициализация и OnEnterZone остаются однократными.
Прежний authored-тест снова подтверждает native Man action/clip sequence и
фоновый cached PCM из сценария 10. Шрифты, мимика и численные правила сохраняются.

GREEN 82/82 Windows (78.02 с), Linux (72.15 с), обе Game собраны. Из них 15
native-display случаев; CPU count по-прежнему 316, последний полный checkpoint316
предшествует этим изменениям конкретного меню. Используются прежние две сборки.
Свидетельства: main-menu-script-cadence-red-{build,test}.log,
main-menu-script-cadence-green-{build,test}.log, main-menu-script-cadence-game-build.log,
main-menu-script-cadence-linux-{configure,build,test}.log и успешные native blocks
в main-menu-script-cadence-{windows,linux}-native-test.log.

Полный патч e399ef89 перепроверен: кроме post-load input purge остаются части
AudioOptions. Наши ползунки уже применяются сразу и сохраняют config, однако
обработка checkbox ответов/субтитров, default и уведомлений требует отдельного
native RED. В upstream также переставлены serialized tags страницы; буквальный
перенос поверх существующего формата форка недопустим без проверки совместимости.
Старый DirectInput buffer-size hunk соотносится с нашим SDL polling отдельно.
Источник и общая задача остаются незавершёнными.

## Ввод после снимка начала миссии — часть `e399ef89`

Compiled native RED открывает настоящую тактическую миссию 810 через
CICBeginMission, удерживает две SDL-клавиши через реальные bindings и извлекает
поставленный миссией отложенный restart snapshot. До его исполнения отпускает
одну клавишу и добавляет однократное событие. Снимок успешно записывается,
но старая реализация оставляет отпущенную привязку активной. Ошибка подготовки
bindings до вызова UpdateBinds не считается RED.

Постановка снимка остаётся отложенной: он должен сохранять уже установленную
миссию. Его отдельный исполнитель после записи вызывает InitializeComplete;
тот завершает экран загрузки и очищает ввод через штатный binding layer.
Слабая ссылка проверяется на валидность. LAN сохраняет свой путь подготовки
без локального restart snapshot и завершает загрузку непосредственно.

Учтён собственный SDL backend: PurgeEvents сначала забирает события платформы,
учитывает её действительный фокус и принудительно опрашивает SDL input.
Обычный frame polling по-прежнему сохраняет ограничение dt==0; его однопараметровый
API также сохранён. Устаревшее увеличение DirectInput buffer до 1024 не переносится:
наш SDL polling уже обрабатывает очередь без такого лимита.

Тест требует освобождения отпущенной клавиши, сохранения второй удерживаемой,
пустой очереди и работоспособности новых keyboard/posted событий. Состав и
объекты отряда, world aim-time не меняются. Дополнительно восемь очисток должны
реально попасть в ту же миллисекунду, что и обычный опрос. Отдельный compiled
RED с оставленным обычным SDL guard подтверждает, что прямого копирования
upstream PurgeEvents недостаточно: теряется same-tick release. После адаптации
оба сценария проходят на D3D11 Windows и Vulkan Linux. Это не оценка пикселей
прогресса загрузки: его существующее ограничение частоты отрисовки сохраняется.

Свидетельства в прежнем каталоге upstream-remaster-20261024:
mission-loading-input-red-{build,test}.log,
mission-loading-input-fast-poll-red-{build,test}.log,
mission-loading-input-green-{build,test}.log,
mission-loading-input-all-build.log и native renderer blocks
mission-loading-input-{windows,linux}-native-test.log.
AudioOptions части источника ещё требуют отдельного воспроизведения;
источник остаётся partially-imported, вся задача не завершена.

Свежий полный CPU-набор 316 случаев плюс новая native загрузка: 317/317 Windows
(241.96 с) и Linux (249.35 с). Windows ALL_BUILD успешен; обе Game собраны.
Широкий локальный набор, подобранный по CI-ограничениям, прошёл 271/271 (25.99 с).
Ещё 27/27 Windows (89.82 с) покрывают 16 настоящих native интерфейсов, шесть
сетевых сценариев и пять world/save/slot проверок. На Linux все 16 native
интерфейсов прошли повторно с окончательным кодом. Отдельные SDL platform/input
и исходный input.cfg с 256 ссылками на клавиши также проходят без ошибок.

В дополнительном Linux display-запуске SDLPlatformInvalidDriver сначала упал:
заданная снаружи SDL_VIDEODRIVER=x11 перекрыла его намеренно неверный SDL hint.
Повторная проверка с unset только этой переменной прошла 1/1; production код
не менялся. Это ограничение команды проверки, не свидетельство runtime бага.
Полный и display-наборы сохранены в mission-loading-input-full-{green-test,
linux-test}.log, mission-loading-input-native-network-world-test.log,
mission-loading-input-ci-test.log, mission-loading-input-display-linux-test.log
и mission-loading-input-linux-{invalid-driver,platform,platform-bindings}-test.log.
GitHub Actions окончательной ревизии всей задачи этим не заявляется.

## Переключатели реплик в настройках звука — часть `e399ef89`

Два compiled native RED проверяют настоящий CICOptions(OS_AUDIO), authored
container 162, активный bgfx кадр и CCheckButton. Первый обнаруживает, что при
config responses=1/subtitles=0 галочки не инициализированы. Второй выставляет
только стартовое визуальное состояние, затем нажимает настоящий checkbox
через down/up и обнаруживает, что конфиг не изменился. Подготовка renderer,
контролов, поля прежнего wire layout и нормализованная громкость проходят
до обеих ожидаемых ошибок.

Добавлены чтение настроек при TEMPLATELOADCOMPLETE и применение/сохранение
только изменённого checkbox при EVENT_NOTIFY. Дочерние элементы разрешаются
через существующее дерево; новых сериализованных или transient ссылок нет.
Старые tags 2=Apply, 3=Default, 4=Sound, 5=Music, 6=Output сохраняются. Тест
читает реальные поля через CStructureSaver и проверяет их прежние объекты;
upstream перестановка bool/checkbox/slider на те же номера не переносится.
Полный старый бинарный save fixture в этом тесте не используется.

GREEN требует независимого переключения обеих настроек, сохранения в
изолированный config, его загрузки после изменения значений и правильных
галочек после повторного открытия страницы. Громкости 0.23/0.61 и положения
23/61 при max=100 сохраняются. Независимый настоящий PCM channel продолжает
проигрывание. Это проверка живого канала и cursor advance с silent PCM,
а не слышимости или пикселей текста.

Наши CComplexScroll уже преобразуют int slider в float 0..1; CSlider::SetValue
не посылает уведомления. Повторный integer normalization и добавление guard
ради этих setters не требуются; тест открытия также проверяет отсутствие
побочного изменения громкости. Используется существующий SaveConfig, который
перенаправляет config.cfg в S2_USER_DATA_DIR на обеих платформах.

Связанные 85/85 прошли Windows (81.31 с) и Linux (74.68 с), обе Game собраны.
Из них 18 native-display случаев. CPU count остаётся 316; последний полный
316 плюс loading case 317/317 был выполнен непосредственно перед этим UI-фиксом
в e2bbf516. Новая полная проверка 316 этим не заявляется. Использованы прежние
два каталога сборки, HD pack не пересобирался, тестовые процессы завершены.
Свидетельства: audio-option-checkboxes-red-{build,test}.log,
audio-option-checkboxes-green-{build,test}.log,
audio-option-checkboxes-related-green-test.log,
audio-option-checkboxes-game-build.log,
audio-option-checkboxes-linux-{configure,build,test}.log и native renderer blocks
audio-option-checkboxes-{windows,linux}-native-test.log.
Кнопка Default остаётся отдельной неперенесённой частью источника.

## Сброс настроек звука — завершение адаптации `e399ef89`

Compiled native RED нажимает настоящий Default на открытой и отрисованной
странице звука: вместо зарегистрированных значений возвращаются жёстко
заданные 1.0/0.5, а настройки реплик не сбрасываются. Тест сначала независимо
читает четыре зарегистрированных default, затем изменяет все четыре настройки;
редактирования registration/default значений в production нет.

Кнопка теперь вызывает ResetVar для двух громкостей, реплик и субтитров,
обновляет существующие controls и сохраняет config. Общий UpdateFromConfig
используется и при открытии страницы. GREEN проверяет фактические 0.75/0.75,
обе галочки, загрузку сохранённого config, последующее нажатие настоящего плюса
громкости и повторный сброс. Существующий native PCM channel продвигается
с прежней позиции. Дополнительный compiled RED с буквальным sound_update из
upstream останавливает этот канал: наша команда вызывает DoneSound и новый Init.
Этот вызов не переносится — собственные volume callbacks уже применяют сброс
к живому микшеру, перезапуск устройства для него не требуется.

Связанные 86/86 прошли Windows (84.17 с) и Linux (76.77 с), обе Game собраны.
Из них 19 native-display случаев; CPU count остаётся 316. Последний полный
CPU checkpoint плюс loading case был 317/317 в e2bbf516; полный набор здесь
повторно не заявляется. В обоих каталогах сборки используются те же файлы,
новых копий игры и HD-генерации нет; тестовые процессы завершены.
Свидетельства: audio-option-defaults-red-{build,test}.log,
audio-option-defaults-reset-device-red-{build,test}.log,
audio-option-defaults-green-{build,test}.log,
audio-option-defaults-related-green-test.log,
audio-option-defaults-game-build.log,
audio-option-defaults-linux-{configure,build,test}.log и
audio-option-defaults-{windows,linux}-native-test.log.

Полный исходный e399ef89 повторно сопоставлен с текущим кодом. Все полезные
части теперь учтены; источник переведён в imported:

- FModSound и Sound: stable silent 2D fallback, имя живого music stream и
  повторное использование музыки после замены DB record уже перенесены.
- SoundFormat: 2D sample идёт через существующий lazy request/decoder с
  default sample при пустом ресурсе. Прежний синхронный backend не возвращается.
- iMissionBase/iMission/wUICommands/scriptSequence: scene-owned playback,
  слабый channel handle, повторный dispatch StopSound и начальный offset
  позиционного sample проверены отдельно; общий backdrop dispatcher перенесён.
- MainMenu: остаётся authored map script 10; лишний DB85 launcher и tick
  корутин между world segments удалены с независимыми native RED.
- Input/iMission: завершение после реального restart snapshot, SDL polling
  и purge уже проверены, включая same-tick releases и отдельный LAN путь.
  Старый DirectInput buffer size не относится к используемому backend.
- AudioOptions: live normalized sliders уже были в форке; checkbox и Default
  перенесены двумя независимыми исправлениями. Сериализованные tags 2..6
  сохранены. Legacy Apply/output objects оставлены ради прежнего graph layout;
  тест подтверждает, что они невидимы и имеют нулевой размер. Старый output
  mode не регистрируется. Наши setters не уведомляют parent, поэтому чужой
  notify guard и перестановка bool/ptr tags не нужны.
- Script/lgc: задержка сборки мусора до 1000 проверок не копируется как
  workaround. Текущий CWorld::uiCmdsList хранит CObj и удерживает команду до
  dispatch; native тест unassigned PlaySound с принудительным GC уже подтвердил
  lifetime без изменения частоты GC. Владение playing channel отдельно у сцены.

Это завершение одного upstream источника. Из 281 остаются 131 pending,
88 imported, 18 partially-imported, 38 already-present, по одному
historical-own-import, review-final-version и excluded-balance, три
documentation-only. Общий перенос и реальный GitHub Actions итогового HEAD
ещё не завершены. Формулы и оригинальный баланс в этих двух UI-фиксах не менялись.

## Контур выделения поверх освещения — `dbb74c6e`

Compiled native RED подтверждён на нашем D3D11 bgfx backend. Тест создаёт
настоящие CObjectInfo/CNonePart/CCombinedPart и два одинаковых CSelection,
освежает native vertex combiner и вызывает существующий CGScene::DrawSelection.
Только доступ к private проходу открыт через test-only member accessor:
production API и реализация прохода ради теста не менялись. Это позволяет
заранее заполнить stencil и не запускать поверх него несвязанные lighting passes.
Изображение читается с GPU; расчёт контура и оба geometry прохода настоящие.

Контроль с нулевым stencil даёт 201 пиксель цвета 64/127/32, пустую внутренность
силуэта и однократный цвет при наложении двух выделений и обоих winding mesh.
Затем та же сцена с lighting marker 0x80 теряет пиксель 47,47: цвет ff407f20
становится ff000000. Это ожидаемый runtime RED, не ошибка подготовки GPU.
Первое ошибочное предположение fixture о fixed-function TnL было исправлено
до этого RED: наш bgfx использует свой programmable vsGlow путь, он и проверен.

В production меняются только два stencil вызова и общий marker: силуэт пишет
0x67, расширенный контур проверяет неравенство этому marker и отмечает уже
нарисованные пиксели. Прежний equals-zero/increment ошибочно отбрасывал stencil
освещения. После исправления вся картинка побитно совпадает с контролем при
начальных значениях 0, 0x80, 0x40, 0xc0 и 1. Внутренний силуэт исключается,
дублирующиеся outline triangles не увеличивают цвет. Geometry, ширина контура,
шейдеры и native bgfx backend не заменяются; численные игровые правила не меняются.

Вторая часть upstream уже была в форке: TObject() инициализирует nil tag
и нулевое значение в aa4fe70e53b9a7965c75c8596fb13223295bd26f от 27 сентября.
Lua production код не менялся. Новый resource-free native тест проверяет
placement construction поверх 0xa5, новые/расширенные слоты CLuaThread и
реальный CStructureSaver round-trip. Два живых числовых значения сохраняются,
1059 свободных слотов становятся nil; намеренно stale string pointer выше top
очищается существующим thread saver до сериализации. Это проверка имеющейся
реализации, а не заявленный новый RED/фикс Lua. NativeLuaStateWireTests проверяет
другие codec структуры, поэтому вместо него добавлен этот предметный тест.

Связанные Windows 104/104 (89.80 с), Linux 100/100 (79.80 с) прошли. На обеих
платформах 20 native-display случаев включают настоящий pixel тест выделения,
и обе Game собраны. Разница наборов — Windows дополнительно запускает три
старых GPU режима и регистрацию; на Linux они отдельно проверены тем же
BgfxRendererTests: specular projection, partial geometry, index ring, затем
UpstreamRegistrationTests 1/1. В частности, native Linux Vulkan даёт те же
201 пиксель и полное совпадение для всех пяти stencil значений.

Регистрация подтверждает 311 одинаковых upstream entry points и наличие всех
138 resource-free проверок; новый Lua случай находится вне условий наличия
игровой базы. SelectionStencilRuntimeTests также не требует игровых ресурсов,
но требует настоящий GPU/software Vulkan display, как существующие bgfx тесты.
Native Lua verification добавляет один CPU случай: теперь полный CPU count 317,
его полный прогон здесь не заявляется. Последний полный checkpoint предшествует
двум audio UI исправлениям; текущие выбранные проверки включают их actual UI.

Свидетельства в прежнем evidence каталоге: selection-stencil-red-{build,test}.log,
selection-stencil-green-{build,test}.log, selection-stencil-lua-{build,test}.log,
selection-stencil-game-build.log, selection-stencil-related-green-test.log,
selection-stencil-linux-{configure,build,test}.log,
selection-stencil-linux-{specular-projection,partial,index-ring,registration}-test.log
и selection-stencil-{windows,linux}-native-test.log. Сборки переиспользованы,
HD pack не пересобирался; собственные test windows/processes завершены.

Источник теперь imported. Реестр 281: 130 pending, 89 imported,
18 partially-imported, 38 already-present, по одному historical-own-import,
review-final-version и excluded-balance, три documentation-only.
Остальной аудит и реальный GitHub Actions окончательного HEAD ещё не завершены.

## Разрешение кнопки Save в игровом меню — часть `02d85e5b`

Compiled native RED открывает и рисует настоящий CICInGameMenu с явно
переданным save permission=false. Интерфейс сохраняет входной false, но кнопка
Save остаётся enabled: он не передаёт permission в CInGameMenuUI и UI не
применяет его к кнопке. Fixture использует authored container 158 и реальный
bgfx screenshot/background, отдельный профиль в S2_USER_DATA_DIR; миссия и
пересчёт сложности в этой проверке не создаются.

Разрешение передано в существующий конструктор страницы и в STYLE_ENABLED
кнопки. Формат полей сохранён. GREEN проверяет оба значения false/true,
реальный down/up: отключённый Save не выдаёт события; разрешённый открывает
настоящий SAVE view; Load продолжает работать при false и передаёт тот же
флаг в настоящий LOAD view. Restart остаётся отключён независимо от разрешения
Save. CStructureSaver читает существующие tags 2/3/4/8 UI и подтверждает флаги,
того же игрока и ту же кнопку. Полный старый бинарный save здесь не проверяется.

Связанные 91/91 Windows (87.10 с) и Linux (77.70 с), обе Game собраны.
Из них 21 actual native-display случай. Новый тест вне CPU prefix, поэтому
полный CPU count остаётся 317, его полный прогон не заявляется. Регистрация
311 upstream entry points / 138 resource-free случаев проходит на обеих
платформах. Используются прежние две сборки; шрифты/HD не пересобирались,
тестовые процессы завершены. Свидетельства: in-game-save-button-red-{build,test}.log,
in-game-save-button-green-{build,test}.log,
in-game-save-button-related-green-test.log,
in-game-save-button-game-build.log,
in-game-save-button-linux-{configure,build,test}.log и
in-game-save-button-{windows,linux}-native-test.log.

Исправлено применение уже принятого решения, а не правило разрешения Save.
Main/iMission.cpp, его LAN menu/shortcut routing и difficulty coefficients
не меняются. Сетевые миссии используют собственный networkMenuControls,
что подтверждено чтением текущего CMission::ProcessEvent; эта UI fixture
не заявляется проверкой полного сетевого матча.

02d85e5b остаётся partially-imported: прямой Save shortcut и такой же вызов
из миссии ещё могут создать SAVE mode при false, где текущий SetMode(SAVE)
не создаёт view. Это отдельный следующий runtime RED. Вычисление live
save permissions, QuickSave consumption, mapping сложности и selected style
рассматриваются отдельно; потенциальное влияние правил и выбора сложности
описано в UPSTREAM-BALANCE-CHANGES.md. Реестр: 129 pending, 89 imported,
19 partially-imported, 38 already-present, остальные шесть записей прежних
категорий. Весь аудит и проверка окончательного GitHub Actions не завершены.

## Команда Save при запрете сохранения — часть `02d85e5b`

Отдельный compiled runtime RED после исправления кнопки: существующий
NativeInGameSaveButton проходит, новый NativeInGameSaveShortcut посылает
реальный bound command savegame прямо в отрисованное игровое меню с false
permission. Команда создаёт настоящий CICSaveLoadMenu, но SAVE при false
не создаёт pModeView. Тест обнаруживает пустую страницу через существующие
поля живого native UI. Это не тест физического F-key: используется штатный
PostEvent и настоящий обработчик команды.

Изменена одна строка CInGameMenuInterface::ProcessEvent: false открывает LOAD,
true сохраняет SAVE. Флаг передаётся без изменения. GREEN проверяет не только
mode enum, но существование настоящего view и его конкретный class id:
CLoadView/CSaveView. Оба значения разрешения, настоящий button path и Load,
прежние permission/player/button fields сохраняются. Новых правил вычисления
разрешения и изменений формата данных нет.

Связанные 92/92 Windows (89.04 с) и Linux (79.26 с), обе Game собраны,
22 native-display случая. Регистрация и предыдущий stencil pixel test проходят.
CPU count 317 остаётся прежним; полный CPU прогон и реальный окончательный
GitHub Actions здесь не заявляются. Свидетельства:
in-game-save-shortcut-red-{build,test}.log,
in-game-save-shortcut-green-{build,test}.log,
in-game-save-shortcut-related-green-test.log,
in-game-save-shortcut-game-build.log,
in-game-save-shortcut-linux-{configure,build,test}.log и
in-game-save-shortcut-{windows,linux}-native-test.log.
Сборки переиспользованы, HD generation не запускалась, собственные процессы
завершены. Источник остаётся частичным: такой же Save-menu вызов в CMission,
QuickSave consumption, live permission policy, mapping сложности и selected
style ещё рассматриваются отдельно. Правила/сложность отмечены в BalanceMD.

## Меню сохранения тактической миссии — часть `02d85e5b`

Compiled runtime RED создаёт настоящую миссию 810 через CICBeginMission,
записывает её реальный queued restart snapshot, затем задаёт уже существующий
saved permission через CMissionBase tag 30. Команда savemenu проходит через
настоящий CMission::ProcessEvent и исполняет фактически созданный CICSaveLoadMenu.
Флаг false доходит до диалога, но его pModeView отсутствует: прежний SAVE/false
не создаёт содержимое. Fixture использует отдельный профиль и удаляет свой
active slot, не меняя пользовательских сохранений.

Изменена одна строка выбора режима: при false открывается LOAD, при true — SAVE.
Новая NativeMissionSaveMenu проверяет реальные CLoadView/CSaveView, оба значения
permission и штатное закрытие модального диалога. Оно возвращает тот же объект
миссии, тот же мир и отряд с теми же объектами бойцов; aim time и запись сложности
не меняются. Это применение готового разрешения, без пересчёта difficulty policy.

Связанные 93/93 Windows (90.49 с) и Linux (80.98 с), обе Game собраны;
23 native-display случая. CPU count остаётся 317, полного прогона здесь нет.
Свидетельства: mission-save-menu-red-{build,test}.log,
mission-save-menu-green-{build,test}.log,
mission-save-menu-related-green-test.log,
mission-save-menu-game-build.log,
mission-save-menu-linux-{configure,build,test}.log и
mission-save-menu-{windows,linux}-native-test.log.

Один промежуточный запуск проверки был ошибочно начат до завершения линковки:
старый exe был занят, новая линковка получила LNK1168. Он не считается GREEN
или новым RED; сохранён отдельно как mission-save-menu-build-in-use-{build,test}.log.
Затем сборка и тест выполнены последовательно, прошли и составляют результаты выше.
Использованы прежние каталоги сборки, HD generation не запускалась, процессы
проверок завершены. Source 02d85e5b остаётся частичным: QuickSave consumption,
live permission policy, mapping сложности и selected style ещё требуют работы.

## Адаптация к собственному Linux-кодированию имён сохранений

При переносе QuickSave из 02d85e5b отдельная Linux-проверка обнаружила дефект
собственного порта: GetQuickSaveSlot возвращает штатный S2U8: identifier, а
CICSave отвергает двоеточие в транспортном представлении как символ имени файла.
Windows QuickSave в тех же проверках работал. Новый UpstreamEncodedSaveCommand
отделяет проблему от миссии и её permission: настоящий silent CICSave с
подготовленной текстурой и корректным Unicode-именем получает compiled runtime
RED "Invalid save file name S2U8:..." и не создаёт слот.

В Linux-ветке CICSave проверяется декодированное имя: штатный codec должен
успешно его прочитать, IsSafeSaveComponent — принять, прежний список запрещённых
символов — отсутствовать. Windows-проверка сохраняет прежнее выражение.
Проверка читает настоящий save header и сериализованный пустой interface stack;
кириллица и японские символы сохраняются, недопустимые имена и повреждённый
Linux identifier не создают слотов и не сообщают об успехе.

В первом связанном прогоне дополнительная проверка списка была ошибочно
применена и к старому untagged ASCII случаю: Linux возвращает список уже в
каноническом encoded виде. Уточнена только область этой новой проверки:
строгое encoded-равенство относится к encoded fixture; прежний ASCII случай
по-прежнему обязательно читает настоящий файл, header и сериализованные данные.
Этот промежуточный сбой сохранён как encoded-save-name-canonical-fixture-linux-test.log.

Это отдельный compatibility fix нашего порта, а не буквальный upstream hunk.
Происхождение адаптера: собственный коммит
590320324eda981f7e41b58f22a36b938f4612a6. CICLoad содержит аналогичную проверку;
его encoded-путь требует отдельного воспроизведения и пока здесь не исправлен.

Compiled runtime RED и GREEN: encoded-save-name-red-linux-{configure,build,test}.log,
encoded-save-name-final-green-linux-{configure,build,test}.log и
encoded-save-name-final-green-{build,test}.log. Связанные пять проверок прошли
на Windows и Linux; Linux Game собран. Нативный screenshot заранее задан
тестом: это проверка файла и команд сохранения, а не GPU readback.
Использованы прежние каталоги сборки, HD generation не запускалась.

## Быстрая запись тактической миссии — часть 02d85e5b

Compiled runtime RED новой NativeMissionQuickSave запускает настоящую миссию 810,
исполняет её restart snapshot и задаёт существующее сохранённое разрешение через
CMissionBase tag 30. Команда save проходит через настоящий StepApp с обработкой
событий и общим fallback. При false миссия не поглощала команду: fallback создал
настоящий слот, число сохранений выросло с нуля до одного.

В CMission добавлен собственный bind команды save. При готовом true permission
он создаёт обычный CICSave с GetQuickSaveSlot(false); при false показывает
существующее сообщение и поглощает событие до глобального обработчика.
Правила вычисления permission, сложность и сериализованные поля не меняются.
Собственный LAN menu routing остаётся прежним; старый hotseat код не возвращается.

GREEN проходит последовательность false/true/false через реальный dispatcher:
запрещённая команда не создаёт слот и не меняет байты уже существующего сохранения,
разрешённая пишет штатный заголовок, screenshot и сериализованную миссию.
Проверяются прежние объекты миссии, мира и отряда, aim time и запись сложности.
Файлы создаются только в отдельном тестовом профиле и удаляются после проверки.
Это тест команды, а не физического F-key; отдельного полного LAN UI запуска
данная fixture не выполняет.

На Windows первоначальная полная проверка переноса прошла 353/353 (349.73 с),
включая CPU-набор, 24 native-display случая, шесть сетевых и пять world/save
проверок. После отдельного e2db8844 и добавления encoded-name regression
повторно собраны ALL_BUILD и Game, связанные 95/95 прошли за 96.01 с,
локальные CI-like 272/272 — за 27.37 с. На Linux финальный полный текущий
CPU-набор (318) вместе с 24 native-display и renderer case прошёл 343/343
за 271.42 с; Game собран. Это локальные проверки, не финальный GitHub Actions.

Первый Linux-прогон 94 случаев имел единственный сбой в разрешённой ветке
QuickSave из-за собственного encoded-name guard. Он сохранён без подмены
результата, причина исправлена отдельным TDD-коммитом e2db8844, затем проведён
финальный полный прогон. Свидетельства: mission-quick-save-red-{build,test}.log,
mission-quick-save-green-{build,test}.log, mission-quick-save-full-test.log,
mission-quick-save-linux-{configure,build,test}.log,
mission-quick-save-final-all-build.log, mission-quick-save-final-related-test.log,
mission-quick-save-final-ci-test.log, mission-quick-save-final-linux-{configure,build,test}.log
и отдельные native success logs на обеих платформах.
Один временный pending diff на 17 KB для независимого compatibility-коммита
восстановлен и удалён; копий игры или сборки не создавалось, HD pack не менялся.

## Загрузка имён сохранений в собственном Linux-порте

После отдельного исправления записи e2db8844 новый UpstreamEncodedLoadCommand
создаёт настоящий native CICExitMenu, сохраняет его под корректным Unicode-именем,
затем исполняет настоящий silent CICLoad. Compiled runtime RED: запись проходит,
но загрузка отвергает S2U8: identifier и оставляет старый interface вместо
нового объекта, восстановленного из файла.

Существующая проверка имени из CICSave выделена в общий локальный helper и
используется также CICLoad. Windows сохраняет прежнее условие, Linux проверяет
штатно декодированное имя и прежние запрещённые символы. Кодек, расположение
файлов, заголовок и сериализация сохранений не меняются.
GREEN требует нового настоящего interface из записанного файла и обычное
сообщение об успешной загрузке; недопустимые decoded имена не заменяют interface,
не меняют список слотов и не сообщают об успехе. Прежний missing-file контроль
и отдельные ASCII Save/Load случаи сохраняются. Fixture удаляет только свои
слот и active snapshot из отдельного профиля.

Это собственная совместимость порта из 590320324eda981f7e41b58f22a36b938f4612a6,
выявленная при переносе 02d85e5b; другой анимационный, графический или файловый
backend не возвращается. Полного текущего CPU-прогона после этого узкого
исправления здесь не заявляется; предыдущий полный Linux checkpoint — 343
при 318 CPU cases. Новый encoded-load case повышает CPU count до 319.

Связанные 13/13 прошли на Windows (14.99 с) и Linux (12.07 с), включая обе
encoded команды, прежние ASCII Save/Load, invalid-name/missing-file controls,
compressed writing, регистрацию, меню и native mission QuickSave. Обе Game
собраны. Свидетельства: encoded-load-name-red-linux-{configure,build,test}.log,
encoded-load-name-green-linux-{configure,build,test}.log,
encoded-load-name-green-{build,test}.log, encoded-load-name-game-build.log
и сохранённые native success logs. Использованы прежние каталоги сборки,
HD generation не запускалась, собственные тестовые процессы завершены.

## Ширина выпадающего списка по его пунктам — завершение c0f64c74

Независимый compiled runtime RED после фикса clipping: настоящий CComboBoxItem
имеет ширину 250, popup остаётся 160, HitTest у правого края пункта false.
Новый resource-free UpstreamComboPopupItemWidth использует тот же настоящий
оконный путь и проверяет доступность края, а не только численное поле ширины.
Геометрия пунктов задаётся native SetSize; это не проверка отрисовки конкретного
DB-template или текста.

В существующий CComboBox::Update добавлен максимум ширин реальных пунктов.
В отличие от буквального upstream override списка, позиция и ширина вычисляются
вместе, сохраняя наше выравнивание по правой границе. Начальная либо восстановленная
ширина popup остаётся минимальной шириной artwork; runtime cache вынесен за ZDATA,
старые поля и номера сохранений не меняются. Поэтому временное расширение под
широкий пункт не превращается в постоянный новый минимум при обычном обновлении.

GREEN проверяет доступность всего пункта, размеры 160 -> 250 -> 300 -> 250 -> 160,
прежнюю высоту списка и правую границу при изменении поля/пункта; пустой список
сохраняет artwork minimum и имеет нулевую высоту. Предыдущая native clipping
fixture также остаётся обязательной. Шрифты, изображения, HD pack, правила игры
и сериализация списка не переписываются.

Финальный код: Windows ALL_BUILD и обе Game собраны, связанные 27/27 на Windows
(19.44 с) и Linux (57.11 с), локальные CI-like 274/274 (26.57 с).
DisplayRuntimeTests проходит те же пять размеров до 3840x2160, сохранены 12
FONT-COMPAT controls на каждой платформе. Current CPU count 321; это не новый
полный прогон всех 321 и не результат окончательного GitHub Actions.
Свидетельства: combo-popup-item-width-red-{build,test}.log,
combo-popup-item-width-green-{build,test}.log,
combo-popup-item-width-final-all-build.log,
combo-popup-item-width-final-related-test.log,
combo-popup-item-width-final-ci-test.log,
combo-popup-item-width-final-linux-{configure,build,test}.log и native success logs.
Оба полезных hunk c0f64c74 перенесены с адаптацией к нашему layout; источник
полностью учтён. Сборки переиспользованы, HD generation выключена.

## Локализованный заголовок улик сектора — часть 29554756

Новый UpstreamChapterClueHeading открывает настоящую главу через CICBeginChapter,
находит её настоящий CZoneSector, получает существующую размещённую улику штатными
ScenarioTracker API и передаёт положение курсора в существующее поле CInterface.
Реальный CChapterMapUI::Draw вызывает GetDescription и заполняет настоящий CText
панели описания. Проверка останавливается на границе рисования; glyph/pixel output
и физическое движение мыши здесь не проверяются.

Первая подготовка fixture выбрала произвольную карту 12, не принадлежащую сценарию
1: все её native zone pointers были null. Это не RED бага. Свидетельства сохранены
как chapter-clue-heading-fixture-setup-test.log и fixture-probe-{build,test}.log.
Fixture теперь выбирает подходящую авторскую карту по настоящему CChapterInfoLoader,
сопоставлению scenario zone и размещённой улике. Production до настоящего RED
не менялась.

Compiled runtime RED на карте 22, scenario 1, zone 2: текст самой полученной улики
уже присутствует в реальной панели, но её локализованного заголовка нет; вместо
него отображается hardcoded English Clues found. Исправлена одна строка:
используется существующий DB string 0x4f35. Механика размещения, получения улик
и их сценарные последствия не меняются.

GREEN проверяет авторскую строку и её заведомо отличающийся временный вариант
с кириллицей; временная запись восстанавливается после теста. Текст улики
обязательно остаётся в описании. Получение улики и отключение второго кадра
уведомлений относятся только к подготовке отдельного тестового сценария.
Обычные marker/tooltip/exit-marker случаи продолжают использовать тот же
настоящий chapter fixture.

Финальный код прошёл связанные 25/25 на Windows (34.26 с) и Linux (27.95 с),
обе Game собраны; сохранены 12 FONT-COMPAT controls на каждой платформе.
Авторская heading record имеет 31 символ, без font tags и count placeholder:
дополнительное форматирование или подмена наших шрифтов не требуются.
CPU count 322 подтверждён текущим CTest inventory; полный новый 322-набор
и окончательный GitHub Actions здесь не заявляются.

Один промежуточный Linux-прогон имел 24/25: существующий NativeChapterScriptSound9
получил unrelated interface command. Исходный полный LastTest сохранён как
chapter-clue-heading-final-linux-initial-native-test.log (24 passed / 1 failed).
Одиночный повтор использовал прежний prefix и заменил CTest summary, но raw
LastTest был сохранён до повторения. Затем 20 последовательных запусков
этого случая прошли, после добавления failure-only диагностики типов interface
и command прошёл финальный набор 25/25. Условие assert осталось прежним;
diagnostic extraction выполняется только после фиксации false результата.
Причина единичного сбоя не установлена и не объявляется исправленной.

Свидетельства: chapter-clue-heading-final-red-{build,test}.log,
chapter-clue-heading-green-{build,test}.log, chapter-clue-heading-game-build.log,
chapter-clue-heading-verified-green-{build,test}.log,
chapter-clue-heading-verified-linux-{configure,build,test}.log,
chapter-sound9-probe-repeat-test.log и native success logs.
Source 29554756 остаётся частичным: camp eligibility и medal screenshot
проверяются отдельно с учётом нашего renderer; cursor перенесён следующим
независимым исправлением. HD generation выключена.

## Видимый курсор в окне медали — часть 29554756

UpstreamMedalCursorVisible открывает настоящий CICShowMedal с авторскими
side/medal и заданным screenshot, читает его реальные cursor/interface через
существующие saved fields 5/6. Native CCursor и CImageDraw работают в нашем
software fallback; видимый контрольный курсор с той же стрелкой выдаёт две
image submissions, а курсор окна медали в compiled runtime RED — ноль.

В Initialize изменён только bShowCursor с false на true. Наша SDL/native cursor
реализация сохраняется: этот флаг управляет обеими её ветвями. GREEN выдаёт
те же две картинки стрелки и проверяет обычное закрытие реального modal через
cancel/CICExitModal без оставшихся interfaces/commands. Физическая видимость
OS cursor и glyph/pixel output этим fixture не заявляются. Медаль не выдаётся
игровому отряду, боевые и кампанийные параметры не меняются.

Compiled RED и GREEN сохранены в medal-cursor-visible-{red,green}-{build,test}.log.
Связанные 26/26 прошли на Windows (19.98 с) и Linux (16.33 с), обе Game собраны.
BgfxRendererTests и DisplayRuntimeTests входят в этот набор; сохранены native
LastTest logs с 12 FONT-COMPAT controls на каждой платформе. Свидетельства:
medal-cursor-visible-related-green-test.log, medal-cursor-visible-linux-*.log,
medal-cursor-visible-{renderer,game}-build.log и platform-native-test.log.
Source оставался частичным до следующей проверки screenshot/gamma и eligibility.

## Замороженный фон окна медали — завершение UI-частей 29554756

NativeChapterMedalScreenshot использует настоящий скрытый SDL window 320x240,
bgfx и authored chapter UI. Первый Draw действительно рисует карту; две
подготовленные notification bits ещё не должны ставить modal в очередь.
Следующий native CChapterMapUI::Draw проходит реальный second-frame drain
и останавливается на границе последующей отрисовки. Compiled runtime RED:
первый настоящий CICShowMedal открывает CScreenShot с null texture, хотя
GPU framebuffer уже содержит 59706 отличающихся от первого пикселей.

Минимальный фикс захватывает CScreenshotTexture перед каждой постановкой
CICShowMedal и передаёт её существующему аргументу команды. Upstream
Generate(true) и расширение ScreenShot API не копируются: наш GfxBgfx берёт
raw scene framebuffer и применяет gamma при Present; повторная correction
в замороженной текстуре изменила бы его яркость ещё раз. Generate() уже
использует нужный нашему renderer false. Пересборки шрифтов/HD нет.

GREEN сравнивает все RGBA pixels снимка с фактически нарисованной картой при
gamma 1 и 2. Положительный контроль MakeScreenShot(true) при gamma 2 обязан
отличаться от raw более чем на 100 pixels. Между проверками двух queued awards
первый настоящий medal modal рисуется на GPU и обязан изменить framebuffer;
второй snapshot всё равно содержит исходную карту. Проверены размеры,
обычное восстановление chapter после modal и однократный drain notification
bits. Состояние двух наград подготовлено только в fixture через существующие
поля; scoring/probability/thresholds не менялись и здесь не проверяются.

Первый GREEN выявил ошибку второго чтения fixture stream (не было Seek(0));
она исправлена в тесте, отдельно сохранена как fixture-stream-test.log и
не считается новым production багом. Начальный compiled RED был именно
null backdrop и сохранён до последующих CTest в red-native-test.log.
Linux runner один раз не стартовал после сохранения CRLF; восстановлен LF,
это также не считается runtime RED и не вызвало изменения production.

Eligibility-часть 29554756 зависит от CanEnter/CanReenterZone в 5cdf627b:
меняет доступность посещённых/скрытых миссий и возможный camp fallback.
Обе эти policy части исключены из автоматического переноса и описаны в
UPSTREAM-BALANCE-CHANGES.md. Все полезные non-balance части обоих sources
учтены; imported в TSV относится к ним, не к переносу исключённых правил.

Свидетельства: chapter-medal-screenshot-red-{build,test}.log,
red-native-test.log, green-verified-{build,test}.log, all-build.log,
final-{build,test}.log, final-linux-{configure,build,test}.log и сохранённые
native logs. Финальные связанные 35/35 прошли на Windows (33.25 с) и Linux
(27.63 с), включая 12 FONT-COMPAT controls на каждой платформе. Windows
ALL_BUILD и обе Game собраны. Полный all281 аудит и
окончательная проверка фактического GitHub Actions остаются незавершёнными.

## Форматирование подписи медали — часть 0611f275

NativeMedalPanelCaption открывает настоящую tactical mission 810 с roster 54,
синхронизирует её native player tracker и выбирает реального world actor.
Одна существующая награда отмечается gained только в fixture; реальный
CMedalsPanel::Draw строит настоящий CMedalsPanelItem через Generate. Чтение
происходит из CText трёх настоящих CButton states (0/1/255), после остановки
на границе рисования. Compiled runtime RED: название есть, но 46 символов
авторского formatting prefix стоят после него (prefix-first=0/name-first=1).

Исправлены только три выражения составления текста: DB 0x1d85/0x1d86 теперь
предшествуют названию. Содержимое этих строк, наши шрифты/растеризация, цвета,
размеры и ассеты не заменяются. GREEN проверяет точный prefix+name и ненулевой
native text layout в normal/hover/selected. Это подтверждение текста и layout,
не glyph/pixel output, визуальный выбор hover мышью здесь не имитируется.

Начальная ошибка compilation (несуществующий overload GetChildByID), ранние
fixture failures из-за несинхронизированного tracker/пустого списка не являются
RED бага. Они сохранены отдельно; production менялась только после compiled
verified-red с настоящим неправильным текстом. Cached unit panel field 3
сбрасывается в fixture, чтобы Draw действительно перестроил уже нарисованную
во время загрузки selection. Startup CICSaveFile берётся из очереди, но не
выполняется: fixture не делает копию сохранения. Профиль изолирован.

Связанные 28/28 прошли на Windows (22.88 с) и Linux (18.91 с), обе Game и
Windows ALL_BUILD собраны. Bgfx/DisplayRuntime и 12 FONT-COMPAT controls на
каждой платформе сохранены в native logs. Свидетельства medal-panel-caption:
verified-red-{build,test}.log, verified-red-native-test.log, green-{build,test}.log,
related-green-test.log, all-build.log, linux-{configure,build,test}.log и
{windows,linux}-native-test.log. Источник остаётся частичным: preview camera
и перенумерация сохраняемых panel bits проверяются следующим шагом.

## Камера предпросмотра медали и совместимость панелей — завершение 0611f275

NativeMedalPreviewCamera повторяет реальный tactical panel fixture, выбирает
авторскую медаль с моделью и читает матрицу настоящего CModelDraw через его
saved field 6. Base CModel field 2 читается явным вызовом существующего
сериализатора, без копирования private CMedalsPanelView или его vtable.
Compiled runtime RED: medal 1 / item 457 имеет разные authored CAMERA_NORMAL
и CAMERA_RELOADBUTTON (max matrix delta 6.576942); фактическая матрица точно
совпадает с NORMAL, ошибка относительно award camera — 6.576942.

В CMedalsPanelView::Set заменён только индекс камеры на CAMERA_RELOADBUTTON.
GREEN: ошибка относительно authored award camera 0, относительно NORMAL
6.576942. Сама камера, модель и её данные не редактируются. Это проверка
параметров настоящего preview renderer; модельные pixels не проверяются.
Первый compile с несуществующим SHMatrix::m исправлен в fixture до runtime
RED; ошибка компиляции не считается подтверждением бага.

Оставшийся enum hunk не переносится. Наши собственные snapshot поля содержат
INVENTORY=0x4, MEDALS=0x10, BIOGRAPHY=0x20. Upstream замена констант без
миграции прочитала бы собственный inventory как medals, а medals как biography.
CMission field 22 и dialogue field 4 хранят эти значения напрямую. Текущий
CMission::SetPanelState уже ограничивает первую миссию по именам MEDALS и
BIOGRAPHY, поэтому исходная проблема numeric mask 0x14 решена у нас.

Добавлены native compatibility controls: чтение существующих raw 4/16/32
в настоящий CMission field 22 и проверка семантики GetPanelState; при first
mission field 34=true установка MEDALS|BIOGRAPHY остаётся запрещённой,
inventory=4 сохраняется. Это проверка native fields/flags, не загрузка целого
исторического slot и не pixel-проверка открытых окон. Разделение retail и
нашего encoding требует отдельного versioned importer, слепая перенумерация
не является полезным исправлением для текущего форка. Все source hunks учтены.

Финальные связанные 29/29 прошли на Windows (24.56 с) и Linux (20.10 с),
обе Game и Windows ALL_BUILD собраны. По 12 FONT-COMPAT controls сохранены.
Свидетельства medal-preview-camera: compiled-red-build.log, red-test.log,
red-native-test.log, green-{build,test}.log, all-build.log,
final-{build,test}.log, final-linux-{configure,build,test}.log и
final-{windows,linux}-native-test.log. Оригинальный баланс, scoring и HD
остаются прежними. All281/final actual GitHub Actions ещё не завершены.

## Направление движения двери по её hull — часть 68595fd2

UpstreamDoorSwingDirection создаёт настоящий authored world 810 без mission
scripts и проверяет его дверь с двумя различными collision hulls. Ожидаемые
центры получаются существующими AIMap FlipDoorWindow/GetHull/GetUnitHLPos:
сначала открывается доступ к закрытому hull, затем к открытому. Направление
не выводится из grid locks или скопированного private CConvexHull layout.
Fixture возвращает collision state через RAII и не меняет door.IsOpen.

Compiled runtime RED: длина старого вектора 0.673095, отклонение от направления
между центрами настоящих hull 0.747133 в обе стороны. Production выбирала
первую произвольную запись unordered grid locks, а для >100 locks возвращала
ноль. Минимальный перенос добавляет CAIMap::GetWindowPos, получает оба hull
через существующий tracker с filter=false (включая inactive state), использует
их штатные центры и нормализует разность в CWindowDoor. Новый virtual добавлен
в конец нашего IAIMap, чтобы не сдвигать прежние slots. Saved fields не меняются.

GREEN: длина 1, ошибки forward/reverse 0 и при закрытой, и при открытой
collision state. Проверена одна authored дверь fixture 810; отдельное покрытие
больших ворот и всех карт здесь не заявляется. Настоящее открывание действием
бойца, его AP и shove animation этим тестом не исполняются. Цены действий,
урон и существующий порог shove 0.707 не редактируются.

Из-за общего IAIMap header выполнены Windows ALL_BUILD и обе Game, полный
актуальный набор 329: upstream/native CPU base 324 (включая bind и bound), три
native medal случая и Bgfx/DisplayRuntime. Обе платформы прошли 329/329: Windows за 277.70 с, Linux за 280.05 с.
В native logs по 12 FONT-COMPAT controls на каждой платформе.
Windows CI-like 274/274 также прошёл за 26.15 с. Windows full summary содержит
329 passed, но его raw LastTest сохранил 328 blocks: самый первый Bound был
затёрт header/нулевым промежутком при nested CTest --show-only из существующего
UpstreamRegistrationTests.py. Исходный raw оставлен как есть; пропущенный
Bound отдельно прошёл 1/1 (0.02 с), его raw сохранён как
door-swing-direction-bound-log-recheck-native-test.log. Это ограничение
локального журнала, а не failure теста; nested logging требует отдельного
исправления при заключительной проверке CI.
Source остаётся частичным: отдельное смешивание heading в CanSimplyOpen
должно получить собственное native RED/GREEN до изменения production.
Свидетельства door-swing-direction-{red,green}-{build,test}.log,
red-native-test.log, green-native-test.log, all-build.log, full-test.log,
linux-{configure,build,test}.log и platform-native-test.log.

## Доворот у авторской точки двери — завершение 68595fd2

UpstreamDoorApproachHeading использует настоящий roster actor и GetApproaches
двери world 810: позиции и направления берутся из authored Unit bones.
CExecQueue::AddPath получает реальное действие PA_OPEN; проверяется путь
первого настоящего movement executor, содержащий реальные CCmdRotate.
Обычный поворот того же actor через двухточечный native path служит обязательным
положительным контролем перед проверкой двери. Очередь подготавливается,
но Run/Segment и игровое открывание не исполняются; временная позиция actor
восстанавливается, AP и логическое состояние двери остаются прежними.

Первый probe ошибочно требовал non-null GetCurrentPath для отсутствующего
поворота. Этот API намеренно возвращает null, когда нет двух точек: это ошибка
fixture, не RED бага. Диагностический probe-path затем показал, что все
проверяемые направления оставались без доворота. Production не менялась.
Финальный compiled RED сначала подтверждает aligned 4->4 и настоящий обычный
поворот 5->4, затем actual door queue не содержит требуемого поворота 5->4.
Свидетельства probe и настоящего red сохранены раздельно.

Перенесён только nearby heading hunk CanSimplyOpen: при квадрате расстояния
менее 0.5 учитывается направление authored approach, включая circular wrap.
GREEN использует конкретные случаи (authored/from/expected) 4/4/4, 4/5/4,
4/6/5, 0/4/4, 0/6/7, без копирования production выбора кандидата в expected.
Они проходят для стоящей и присевшей authored позиции — десять случаев.
Для aligned/opposite лишний поворот отсутствует; остальные реальные queued
paths заканчиваются ожидаемым направлением. Это проверка подготовки очереди,
не полной анимации открытия/расходования AP. Формулы и цены не редактируются.

Связанные movement/queue/retreat/pose/door случаи, native medal и GPU/display:
39/39 на Windows (36.78 с) и Linux (33.31 с), обе Game и Windows ALL_BUILD
собраны; сохранены по 12 FONT-COMPAT controls. Предыдущий полный checkpoint
c97d9971 имел 329/329 на обеих платформах и CI-like 274/274; после отдельного
heading hunk полный новый CPU base 325 не заявляется.
Свидетельства door-approach-heading: red-{build,test}.log, red-native-test.log,
green-{build,test}.log, green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log и {windows,linux}-native-test.log.
Все hunks 68595fd2 учтены. All281 и final actual GitHub Actions ещё не завершены.

## Сценарная анимация через очередь командира — часть 51a184b8

UpstreamDeferredAnimationCommand использует настоящий human roster actor world
810, native CScript/UnitPlayAnimation и существующий authored animation 4437.
Lua выполняется через штатный ExecuteThreads, без последующего world step,
поэтому граница producer/consumer наблюдается непосредственно. Compiled runtime
RED: Lua вернулась успешно, commander-pending=0, unit-command=1 — действие
установлено немедленно из сценарного вызова.

Добавлен общий NScript::DoCommand из source и переведён только command-mode
UnitPlayAnimation. Он ставит CCmdSetCommand с animation, затем с Continue
на существующий owning commander. В нашей адаптации входной command удерживается
CObj до передачи, включая invalid-owner возврат; native queue и её формат
не меняются. Freeze-last-frame флаг передаётся прежним образом.

GREEN проверяет commander-pending=1/unit-command=0 для freeze false/true,
извлекает две настоящие queued CCmdSetCommand и проверяет target actor,
animation 4437, freeze и точный порядок Continue без лишних записей. Actor
не получает действие, AP и положение не меняются до исполнения очереди.
Custom-idle ветка возвращается из Lua без новых queued commands; само её
присваивание и движок анимации не переписываются. Тест не исполняет queued
действия и не заявляет clip/face pixels, устранение конкретного визуального
рывка или сетевое воспроизведение этой анимации.

Связанные script/animation/forced/recalc/cannon/movement/door случаи плюс native
medal и GPU/display прошли 42/42 на Windows (44.25 с) и Linux (41.33 с).
Обе Game и Windows ALL_BUILD собраны; по 12 FONT-COMPAT controls сохранены.
Windows NetworkSerializer/Match/PartialMovement/StopResume прошли 4/4 за 24.97 с.
Наши LAN validation и wire types не меняются; эти четыре проверки не являются
отдельным тестом script animation по сети.

Свидетельства deferred-animation-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-test.log и network-native-test.log.
51a184b8 остаётся частичным: dismount clip, recalc/stop flags и forced-action
условия требуют самостоятельной проверки с учётом нашего lifecycle. Общий
helper теперь доступен для оставшихся producers source 14ffbcc8; они ещё
не перенесены. Полный последний checkpoint CPU base 324 предшествует двум
новым случаям, полный новый CPU base 326 здесь не заявляется. All281/final
actual GitHub Actions остаются незавершёнными.

## Сценарный поворот через очередь командира — часть 14ffbcc8

UpstreamDeferredDirectionCommand переиспользует настоящий world/actor/Lua
fixture проверки animation dispatch. Compiled runtime RED: UnitSetDirection
с requested=1 возвращается из Lua, но commander-pending=0/unit-command=1.
Production меняется только после этого RED: две прямые Do заменены одним
вызовом существующего DoCommand с CCmdLook и Continue.

GREEN проверяет поворот в обе стороны относительно текущего направления
(в fixture это 1 и 7), две настоящие queued CCmdSetCommand, тип именно LOOK,
direction, исходные tile/pose/network, target actor и порядок Continue без
лишних команд. До consumer step положение/AP и unit command не меняются.
Старый animation fixture сохранён в общей функции и повторно проходит оба
freeze варианта и custom-idle контроль. Это producer/queue проверка, не
выполнение поворота, его визуальная интерполяция или расход AP.

Связанные 43/43 прошли на Windows (47.42 с) и Linux (40.96 с), обе Game и
Windows ALL_BUILD собраны, по 12 FONT-COMPAT controls сохранены. Дополнительные
Windows NetworkSerializer/StopResume прошли 2/2 за 3.90 с. Native LAN protocol
и validation не менялись. Текущий CPU base 327 не означает новый полный прогон:
последний полный checkpoint c97d9971 остаётся 329/329 с CPU base 324.

Свидетельства deferred-direction-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-test.log и network-native-test.log.
14ffbcc8 остаётся частичным: остальные script producers, UnitHide bool и
выбор/активация предметов требуют собственных native RED/GREEN. Общие правила
боя и наш переписанный facial pipeline не меняются.

## Сохранение принудительного действия при recalc — часть 51a184b8

UpstreamForcedActionRecalc использует две последовательные in-memory world 810
fixtures. Нормальный контроль запускает настоящий CExecPlayAnimation через
обычные CCmdSetCommand/Continue. Основной случай запускает тот же native тип
через публичный RunCriticalExecutor: native код сам выставляет forced flag.
Тест не подставляет этот флаг и не копирует private executor layout. Пара
настоящих полей CUnitServer 7/12 читается существующим сериализатором.

Compiled runtime RED: обычный cancelable executor снимается с forced flag 0,
затем принудительный тоже снимается с actor, оставляя forced flag 1.
Локальный CObj witness удерживает executor для наблюдения и cleanup; значение
executing=1 в trace относится к этому witness, а retained=0 — к настоящему
pExec бойца. Это не утверждение, что удалённый без witness executor продолжил
бы исполняться.

Добавлен только !bIsRunningForcedAction в TBS_RECALC_COMMAND gate. Наши
IExecMove/visibility условия PlaceUnit и cannon guard сохраняются; stop flags
из оставшейся части source не копируются. GREEN сохраняет native identity и
forced flag при recalc, затем переводит только fixture clock на существующий
animator.TimeEnd и вызывает настоящий actor.Segment. Его AnimationFinished/
CheckCmdExecState освобождают pExec и forced flag штатным путём. Обычный
контроль по-прежнему снимается; удержанный witness отменяется для cleanup.
Положение и AP не меняются.

Это проверка forced-entry/retirement lifecycle с настоящим animation executor,
не критического броска/урона, visual/face pixels или сетевого playback клипа.
Две world fixtures создаются в памяти последовательно, старых копий игры,
сборок и файлов сохранения не делается.

Связанные script/animation/recalc/route/retreat/queue/movement/door случаи,
native medal и GPU/display прошли 49/49 на Windows (58.10 с) и Linux (49.28 с).
Обе Game и Windows ALL_BUILD собраны; по 12 FONT-COMPAT controls сохранены.
Windows NetworkSerializer/StopResume прошли 2/2 за 3.90 с. Наш собственный
PauseNetworkAction уже имел forced guard, его правила и wire types не менялись.
Текущий CPU base 328 не заявляется полным новым прогоном; последним полным
checkpoint остаётся c97d9971 с CPU base 324 и 329/329 обеих платформ.

Свидетельства forced-action-recalc: red-{build,test}.log, red-native-test.log,
green-{build,test}.log, green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-test.log и network-native-test.log. Source 51a184b8 остаётся частичным:
dismount clip и дополнительные animator/stop/recalc flags проверяются отдельно.
All281 и final actual GitHub Actions остаются незавершёнными.

## Выпадающий список вне clip вложенной строки — часть c0f64c74

Новый resource-free UpstreamComboPopupClipping создаёт настоящие CInterface,
вложенную панель, строку высотой 20, CComboBox и его настоящий CComboBoxList
с двумя CComboBoxItem. Compiled runtime RED: координаты popup уже правильные
(95,130), но parent=row обрезает popup; HitTest внутри его первой строки false.
Это проверка действующей оконной геометрии, а не сравнение текста реализации.

Popup прикрепляется к CInterface. Расчёт его позиции учитывает пространство
родителя, сохраняет наше выравнивание по правой границе и authored artwork width.
При отсутствии интерфейса остаётся прежний sibling layout. Update также
переносит popup из старого вложенного parent с временным CMObj ownership,
чтобы RemoveChild не уничтожил его при reparent. Скрытие/отключение предков
теперь учитывается явно, поскольку popup больше не находится в их child tree.
Сериализованные поля и наш адаптивный layout не заменяются.

GREEN проверяет HitTest за границей исходной строки, nonzero origin интерфейса,
перемещение вложенной панели, сужение поля при сохранении artwork width,
старую вложенную parent-связь, disabled/hidden предков, настоящую выборку ID
через CListView с уведомлением combo, закрытие по outside click и уничтожение
popup вместе с combo. Старый parent в fixture смоделирован действующими API;
это не прогон полного исторического сохранения.

Связанные 26/26 прошли на Windows (19.89 с) и Linux (15.20 с), обе Game собраны.
DisplayRuntimeTests прошёл на 1024x768, 1280x720, 1920x1080, 3440x1440 и
3840x2160, включая прежнюю адаптивную right-edge/artwork проверку combo без
интерфейса. Native clipping fixture сама не рисует glyph/pixel output.
Свидетельства: combo-popup-clipping-red-{build,test}.log,
combo-popup-clipping-green-{build,test}.log,
combo-popup-clipping-related-green-test.log,
combo-popup-clipping-{renderer,game}-build.log,
combo-popup-clipping-linux-{configure,build,test}.log и native success logs.
CPU count 320; полный текущий CPU-набор не повторялся для этого UI-фикса.
Source c0f64c74 остаётся частичным: отдельный item-width hunk ещё проверяется.
