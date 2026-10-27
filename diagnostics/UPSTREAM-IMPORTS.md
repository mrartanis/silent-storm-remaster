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
`excluded-own-behavior` означает полностью проверенный diff, который отменяет
намеренное поведение нашего форка; причины и границы проверки указаны ниже.

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

## Сценарная смена позы через очередь — часть 14ffbcc8

UpstreamDeferredPoseCommand использует общий native world/actor/Lua fixture;
producer выбирается явным enum, прежние animation и direction случаи
сохранены. Compiled runtime RED: UnitSetPose(CRAWL=0) возвращается из Lua
с commander-pending=0/unit-command=1. После RED только этот producer переведён
на DoCommand с прежним CCmdPath(pos.pos, PF_USE_POSEDIR) и Continue.

GREEN проверяет CRAWL=0 и WALK=2 через реальный Lua callback. Из native очереди
извлекаются ровно два CCmdSetCommand с тем же actor, настоящим PATH и Continue.
Проверяются pose, heading, tile/network и PF_USE_POSEDIR; ожидаемое представление
позы получено существующим SUnitPosition::SetPose. Положение/AP/unit action
до исполнения queue не меняются. Это граница producer/consumer, не исполнение
полного pose transition, его pixels или AP cost.

Связанные pose/script/animation/forced/route/queue/movement/door случаи,
native medal и GPU/display прошли 61/61 на Windows (63.37 с) и Linux (59.24 с).
Обе Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT controls сохранены.
Windows NetworkSerializer/StopResume прошли 2/2 за 3.86 с. Формулы, movement
rules, наш native facial pipeline и сетевые wire types не редактируются.
Текущий CPU base 329 не является повтором последнего полного selected 329:
checkpoint c97d9971 включал CPU base 324 плюс пять native/GPU случаев.

Свидетельства deferred-pose-command: red-{build,test}.log, red-native-test.log,
green-{build,test}.log, green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-test.log и network-native-test.log. 14ffbcc8 остаётся частичным;
остальные producers, UnitHide bool и выбор/активация предметов не перенесены
этим исправлением. All281/final actual GitHub Actions ещё не завершены.

## Предпочтительная поза через очередь — часть 14ffbcc8

UpstreamDeferredWishPoseCommand переиспользует настоящий world/actor/Lua
fixture с отдельным producer enum. Compiled runtime RED: вызов с CRAWL=0
оставляет commander-pending=0/unit-command=0 и сразу меняет живой GetWishPose
с WALK=2 на 0. Это подтверждение преждевременной мутации, а не требование
наличия длительного executor у мгновенной команды.

После RED только UnitSetWishPose переведён на существующий DoCommand с тем
же CCmdWishPose и Continue. GREEN проверяет requested 0/2, точный WISHPOSE
payload и две native wrappers с прежним actor/порядком без лишних записей.
Живое предпочтение остаётся исходным WALK=2 до consumer; logical position,
AP и unit action также не меняются. Само движение/его цена и применение
команды в полном world step этим producer test не проверяются.

Связанные 62/62 прошли на Windows (64.86 с) и Linux (58.58 с), обе Game и
Windows ALL_BUILD собраны; по 12 FONT-COMPAT controls сохранены. Windows
NetworkSerializer/StopResume прошли 2/2 за 3.95 с. Все ранее добавленные
producer/forced-recalc проверки входят в связанный набор.
Текущий CPU base 330 не заявляется новым полным прогоном; последний полный
checkpoint остаётся c97d9971 с CPU base 324 плюс пять native/GPU случаев.

Свидетельства deferred-wish-pose-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-test.log и network-native-test.log.
Другие producers и inventory/UnitHide hunks 14ffbcc8 остаются отдельной работой.
Формулы и исходные правила выбора позы не редактируются.

## Сохранена наша обработка nil script arguments — fa5cab13

Полностью проверен diff
[fa5cab134d2c74cf26263efdab0e4efb3589bf0b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fa5cab134d2c74cf26263efdab0e4efb3589bf0b):
единственный runtime hunk удаляет нашу ветку nil hardening для Lua-параметра
`u`. После этого nil попадает в userdata type check и прекращает текущий
thread. В нашем коде nil предупреждает и передаёт null в обработчик, который
проверяет аргумент. Такое намеренное поведение форка сохраняем.

Native UpstreamDeferredShootCommand на Windows и Linux подтверждает для
UnitShoot обе комбинации nil actor и nil target: Lua доходит до sentinel=13,
нет заказа или действия. Проверка входит в 76/76 связанных тестов a176da78.
Вывод о последствиях удаления ветки получен из diff/CheckArgs; сам upstream
patch поверх нашего кода не запускался. Не заявляем, что эти два controls
проверяют каждый `u` consumer или весь tutorial. Полезных независимых hunks
в этом source нет, статус excluded-own-behavior; production не редактируется.

## Очередь скриптового выхода из панцеркляйна — часть 14ffbcc8

UpstreamDeferredLeavePKCommand использует настоящего human pilot world 810
и authored CRPGPers suit. Свободное место найдено через native path network/
IsBigLockerPlacePassable; корпус создан world.AddUnitInGame. Nil pilot и
пилот без костюма возвращают nil без заказа. Worn fixture готовится штатными
WearAsPK/SetPosition/FlipPanzerklein(false,false), без записи private flags.
IsEmptyPK у shell остаётся true как признак класса; факт ношения проверяется
через pilot.IsWearingPK/GetWearingPK, не через инверсию shell.IsEmptyPK.

Compiled runtime RED: leave=1, queued=0/unit-command=1, wearing=1,
shell-empty=1. Возврат исходного корпуса в Lua уже корректен и отдельно
проверяется настоящим luaGetPtr. После RED только ExitPK/Continue dispatch
заменён на DoCommand. Capture pPK и возвращаемое значение не изменены.

GREEN проверяет CCmdExitPK, две wrappers с прежним pilot/Continue и return
того же shell. До consumer сохраняются wearing relation, обе позиции и AP пилота,
два native inventory (руки, порядок предметов рюкзака, active slots), нет
unit action. Реальный queued exit/animation/footprint transition и AP cost
не исполняются этим producer test; прежние native PK проверки проходят.

Связанные 75/75 прошли на Windows (73.62 с) и Linux (68.56 с), обе Game и
Windows ALL_BUILD собраны с task-local parallel 2. Включены PK equipment/
fixed weapon/placement/waypoint/model/feedback, mounted/pose/holster/corpse,
native medal/GPU/display и по 12 FONT-COMPAT controls. Текущий CPU base 349
не заявляется новым полным прогоном. Windows NetworkSerializer/StopResume
прошли 2/2 за 5.66 с.

Свидетельства deferred-leave-pk-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь скриптовой команды положить тело — часть 14ffbcc8

Отдельный UpstreamDeferredDropCorpseCommand запускает drop branch native
corpse fixture. После living/nil controls реальный RPG Kill/DeathState готовит
труп; публичные CExecCorpse::Run и TimeLabelReached выполняют штатный handoff
к CUnitStateCorpseCarrier/BeTaken. Проверяются GetCorpse, IsCarryingCorpse и
обратный GetCorpseCarrier. Это подготовка через native callback, не прогон
полной pickup animation/естественного clock label; приватные flags не подменены.

Compiled runtime RED: drop=1, queued=0/unit-command=1, carrying=1. После RED
только UnitDropCorpse/Continue переведён на DoCommand. GREEN проверяет две
wrappers/actor/Continue, CCmdDropCorpse с пока null pCorpse: фактическое тело
разрешает существующий carrier state при исполнении. До consumer сохраняются
положения обоих, AP, carry state/body attachment и death state, нет unit action.
Новый queued drop, его animation, placement и AP cost здесь не исполняются.

Связанные 57/57 прошли на Windows (57.32 с) и Linux (59.57 с), обе Game и
Windows ALL_BUILD собраны, native medal/GPU/display и по 12 FONT-COMPAT
controls сохранены. Windows ALL_BUILD сначала получил LNK1102 (out of memory)
только при link NativeExplosionVoxelRendererTests. Исходный failed log
сохранён; этот target успешно пересобран с --parallel 1, task-local runner
снижен с parallel 6 до 2, затем ALL_BUILD успешно повторён. Игровые исходники
для обхода memory error не менялись, копий сборки нет. Два network tests и
восстановленный NativeExplosionVoxelRendererTests прошли 3/3 за 6.49 с.
Текущий CPU base 348 не заявляется новым полным прогоном.

Свидетельства deferred-drop-corpse-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-initial-build.log, all-recovery-build.log, all-build.log,
related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь скриптового переноса тела — часть 14ffbcc8

UpstreamDeferredTakeCorpseCommand выбирает двух настоящих ordinary actors
world 810. Живая цель — positive control: UnitTakeCorpse не выдаёт заказа,
Lua продолжает callback. Затем native RPG Kill и CUnitStateDeath готовят
настоящий труп; собственный actor остаётся жив и CanFight. Nil actor/target
controls сохраняются. Первый compiled runtime RED: queued=0/unit-command=0.
До production дополнительно усилено предусловие живого CanFight переносчика;
verified compiled RED: queued=0/unit-command=1, carrying=0. Причина в обоих
случаях — отсутствие запроса в owning commander queue.

После verified RED только dispatch UnitTakeCorpse/Continue заменён на
DoCommand. Guard CanFight(actor)/!CanFight(body) остаётся прежним. GREEN
проверяет CCmdTakeCorpse с настоящим body, actor, две wrappers/Continue.
До consumer не меняются AP переносчика и позиции обоих, corpse/carrier binding,
IsCarryingCorpse, death state и unit action. Полный подход, pickup animation,
стоимость AP и завершение переноса здесь не исполняются. Drop mode fixture
подготовлен отдельно, но этим случаем ещё не запускался.

Связанные corpse/producer/queue/pose/holster/active-item и native medal/GPU/
display проверки прошли 56/56 на Windows (54.43 с) и Linux (59.05 с), обе
Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT controls сохранены.
Windows NetworkSerializer/StopResume прошли 2/2 за 5.89 с. Текущий CPU base
347 не заявляется новым полным прогоном.

Свидетельства deferred-take-corpse-command: red-{build,test}.log,
red-native-test.log, verified-red-{build,test}.log,
verified-red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь атаки по waypoint — часть 14ffbcc8

Отдельно зарегистрирован UpstreamDeferredAttackWaypointCommand: тот же
настоящий world/actor и loaded ground/3D waypoint fixture вызывает реальную
UnitAttackWaypoint. Nil actor/missing point controls проходят. Compiled
runtime RED: fly=0/3D=0, queued=0/unit-command=1. После RED только dispatch
ShootTile(cp)/Continue переведён на DoCommand; прежний GetCP и flags сохранены.

GREEN для обеих точек проверяет CCmdShootTile, точный target GetCP,
reload replacement=false, actor и ровно две wrappers/Continue. До consumer
нет unit action, перемещения/AP/ammo/смены оружия или изменения registry
point. Fly companion сохраняет свои позиционные проверки. Фактическая
стрельба/попадание/урон, AP cost и LAN playback здесь не исполняются.

Связанные producer/queue/waypoint/flying/shoot/reload/mounted и native medal/
GPU/display проверки прошли 52/52 на Windows (51.47 с) и Linux (50.20 с),
обе Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT controls сохранены.
Windows NetworkSerializer/StopResume прошли 2/2 за 6.08 с. Текущий CPU base
346 не заявляется новым полным прогоном.

Свидетельства deferred-attack-waypoint-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь полёта к waypoint — часть 14ffbcc8

UpstreamDeferredFlyWaypointCommand использует настоящий ordinary actor world
810 и две CMapWaypoint через world.AddWaypoint: обычную наземную и authored
b3DWaypoint. Никакой записи в runtime waypoint.pos или подмены path network
нет. Nil actor/missing point controls завершают callback без заказа.
Compiled runtime RED: fly=1/3D=0, queued=0/unit-command=1. Начальная ошибка
компиляции fixture (неуказанный namespace F_3D_STEP) исправлена только в
тесте на NAI::F_3D_STEP и не считается RED.

После runtime RED только dispatch Fly/Continue в scriptRoute.cpp заменён
на DoCommand. Сохранены прежние ветка IsFinal, MakeFlyPos и bRun=false.
GREEN проверяет тип CCmdFly, final encoding, bRun=false, XY destination и
Z в пределах native fly-height quantization. Для уже загруженной 3D-точки
проверяются точные p/pNet bits, включая altitude/heading; повторного её
преобразования нет. Две wrappers адресованы прежнему actor с Continue.
До consumer нет unit action, actor position/AP/weapon/ammo и registry point
не меняются. Полная flight animation, movement cost или LAN flight playback
не исполняются этим producer test. UnitAttackWaypoint ещё отдельная работа.

Связанные 96/96 прошли на Windows (124.95 с) и Linux (122.38 с), обе Game и
Windows ALL_BUILD собраны. Набор включает прежние FlyingRestoration,
FlyingShootDirection/DynamicLocks/AttackQueue, PKWaypoint, native medal/GPU/
display и по 12 FONT-COMPAT controls. Текущий CPU base 345 не заявляется
новым полным прогоном. Windows NetworkSerializer/StopResume прошли 2/2 за 4.78 с.

Свидетельства deferred-fly-waypoint-command: fixture-scope-build.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log и
network-{test,native-test}.log. Source 14ffbcc8 и all281/final actual Actions
ещё не завершены.

## Очередь скриптовой скрытности — часть 14ffbcc8

UpstreamDeferredHideCommand использует настоящего visible human actor world
810; EnableHide только подготавливает cooldown fixture. Compiled runtime
RED: commander-pending=0/unit-command=0, IsHiding 0->1 ещё внутри Lua callback.
После RED только dispatch существующего CCmdHide() и Continue заменён на
DoCommand. GREEN проверяет точный native type, actor, две wrappers/Continue,
нет раннего hiding transition, AP/position/unit action.

Сохраняются нынешняя сигнатура UnitHide("u"), legacy command wire/base tag2
и toggle/eligibility/AP/training. Optional bool из 14ffbcc8 не переносится
этим исправлением: он зависит от stateful CCmdHide/CExecHide из 432b64c8. Их retail
base1/bool2 нельзя вслепую записать поверх нашего legacy base2; нужен отдельный
native state/wire/save compatibility тест. Training AC_HIDE из 2e15 — ещё
одна самостоятельная часть, не зависимость конструктора bool.

Связанные 92/92 прошли на Windows (109.13 с) и Linux (108.95 с), обе Game и
Windows ALL_BUILD собраны. Набор включает MixedHideButton, HumanSequenceHiding,
PKSequenceHiding, HideCooldown, прежние producer/pose/holster/inventory,
native medal/GPU/display и по 12 FONT-COMPAT controls. Текущий CPU base 344
не заявляется новым полным прогоном; реальный queued hide не исполняется.
Windows NetworkSerializer/StopResume прошли 2/2 за 3.82 с.

Свидетельства deferred-hide-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь скриптового подбора предмета — часть 14ffbcc8

UpstreamDeferredTakeObjectCommand создаёт настоящий CWeaponItem из authored
DB и его CDFrozenItem через world.AddFrozenItem в world 810 рядом с actor.
Контроль GetFrozenItem связывает реальный world body с содержимым; рюкзак
очищен native API и способен вместить предмет. Lua получает именно world
body. Nil actor/target и передача голого inventory item отдельно завершают
callback без заказа — сохраняется контракт нашего IItem adapter.

Compiled runtime RED нормального вызова: queued=0/unit-command=0,
ground-item-retained=0. До завершения Lua callback предмет уже отсутствует
в world.GetFrozenItem. После RED изменён только dispatch Move(src,dst) и
Continue на DoCommand. Наша GetInvItem/pWorldItem адаптация сохранена.

GREEN проверяет две queued wrappers, actor, CCmdMoveInventoryItem,
GROUND source с null owner/slot=-1/тем же inventory item и world body,
BACKPACK target с actor/position=(-1,-1), Continue и отсутствие лишних команд.
До consumer body и содержимое остаются на земле, рюкзак пуст, прежние active
weapon/slot/ammo, AP и position не меняются; нет unit action. Полный подход
к предмету, стоимость AP и итоговый перенос в рюкзак здесь не исполняются.

Связанные 87/87 прошли на Windows (103.19 с) и Linux (102.63 с), обе Game и
Windows ALL_BUILD собраны; native medal/GPU/display и по 12 FONT-COMPAT
controls включены. Windows NetworkSerializer/StopResume прошли 2/2 за 3.82 с.
Текущий CPU base 343 не заявляется новым полным прогоном.

Свидетельства deferred-take-object-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь создания и активации предмета — часть 14ffbcc8

UpstreamDeferredCreateActivateCommand использует реальные actor/inventory
world 810, authored CRPGItem и native CWeaponItem. Три последовательных мира
в памяти: SLOT_1 пуст; SLOT_1 занят и рюкзак свободен; SLOT_1 занят и рюкзак
заполнен реальными предметами через FindPlace/Place. Активен SLOT_2.
Nil actor control завершает Lua и не выдаёт заказа. Compiled runtime RED
для первого случая: occupied=0/full=0, queued=0/unit-command=1.

После RED только dispatch CreateAndActivateItem заменён на DoCommand с
прежним CCmdCreateAndActivateInventoryItem и Continue. GREEN для трёх случаев
проверяет исходный DB record, fixed SLOT_1, needMove, actor и две wrappers.
Для занятой руки сохраняются реальный old item/source SLOT_1, BACKPACK при
свободном месте или GROUND при полном рюкзаке, target position (-1,-1).
До consumer не меняются руки, active SLOT_2, предметы/порядок рюкзака, AP,
position и unit action. Создание нового экземпляра, stash animation, полный
ground drop/transfer и AP cost этим producer test не проверяются.

Связанные 86/86 прошли на Windows (101.12 с) и Linux (102.26 с), обе Game и
Windows ALL_BUILD собраны; native medal/GPU/display и по 12 FONT-COMPAT
controls включены. Windows NetworkSerializer/StopResume прошли 2/2 за 3.88 с.
Raw logs обоих связанных наборов содержат все 86 блоков без NUL gap после
отдельного CI-фикса f3448f16. Текущий CPU base 342 не заявляется новым полным прогоном.

Свидетельства deferred-create-activate-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## CI: проверка inventory больше не портит общий CTest log

Локальный дефект введён
[276aee6e5ac39dec305c1f070994ce312887f425](https://github.com/mrartanis/silent-storm-remaster/commit/276aee6e5ac39dec305c1f070994ce312887f425):
вложенный ctest --show-only=json-v1 в рабочем build root Windows перезаписывает
активные отчёты родительского CTest. Summary остаётся правильным, но начало
LastTest.log теряется. Это не падение NativeBoundAccumulatorTests и не
потеря игрового теста; ранее сохранённые исходные raw logs не редактируются.

До фикса запрос выделен в query_inventory без изменения поведения и добавлен
UpstreamRegistrationInventoryReadOnly. Он использует настоящий CTest и
маленькое generated test tree во временном каталоге, с nested test и
конфигурационной веткой audio. RED на Windows: inventory корректен, но
read-only запрос изменил заранее существующие отчёты. Это не mock CTest
и не сравнение исходного текста реализации.

После RED запрос запускается из отдельного временного root с subdirs на
настоящий build tree. Generated test files, executable paths и свойства
читаются на месте; копий сборки и игры нет. Временный root удаляется штатно.
GREEN проверяет неизменность всех файлов fixture, nested names, команды,
config on/off, native-audio label и DISABLED=true. Настоящая registration
проверка продолжает видеть все нужные resource-free тесты и audio availability.

Три целевых теста (Bound, Registration, ReadOnly) прошли на Windows (0.33 с)
и Linux (0.19 с). Raw logs содержат по три полных блока, включая Bound, без
NUL gap. Windows CI-like набор прошёл 275/275 serial за 24.45 с и 275/275
при --parallel 4 за 11.13 с, как concurrency в GitHub workflow; оба raw logs
содержат все 275 блоков, без NUL gap. Игра и обе сборки переиспользованы;
изменены Python/CMake, игровые исходники и workflow exclusions не меняются.

Текущий CPU base 341 подтверждён configured inventory; полный asset-backed
набор 341 не запускался заново для этого CI-фикса. Final actual GitHub Actions
после завершения всех переносов остаётся обязательной незавершённой проверкой.

Свидетельства ci-inventory-log: red-{build,test}.log, red-native-test.log,
green-test.log, green-native-test.log, linux-{configure,build,test}.log,
linux-native-test.log, ci-tests.txt, ci-{test,native-test}.log и
ci-parallel-{test,native-test}.log.

## Очередь скриптовой активации оружия — часть 14ffbcc8

UpstreamDeferredWeaponActivationCommand использует настоящий ordinary armed
actor world 810 и реальную UnitActivateWeapon(nil/1). Compiled runtime RED:
requested=0, commander-pending=0/unit-command=1, walk-without-weapon=1.
Отдельно сохраняем различие между persistent intent и выполнением действия.

После RED только непосредственные PATH/Continue заменены на DoCommand с тем
же payload. SetWalkWithoutWeapon остаётся на прежнем месте и записывается
сразу. GREEN для inactive/active проверяет точную destination/network,
PF_DEFAULT, false leaveZone/findNotExact, needActiveItem, actor и две wrappers
с Continue. Живые animator.IsActiveItem, weapon identity/ammo, AP/position и
unit action до consumer не меняются; persistent holster preference уже
соответствует просьбе Lua. Существующий ScriptHolsterPersistence без
изменений также проверяет сериализованный tag26 и default argument.

Связанные 84/84 прошли на Windows (99.46 с) и Linux (98.42 с), обе Game и
Windows ALL_BUILD собраны. Набор включает прежние holster/movement/active-item,
native medal panel/camera, DisplayRuntimeTests и по 12 FONT-COMPAT controls.
Windows NetworkSerializer/Preparation/StopResume прошли 3/3 за 33.07 с.
Известный nested CTest logging defect снова теряет первый Bound block в raw
Windows LastTest (83 блока при корректном summary 84/84). Исходный raw
сохранён; NativeBoundAccumulatorTests отдельно прошёл 1/1 с отдельным raw.
Исправление самой read-only inventory проверки остаётся отдельной CI работой.
Текущий CPU base 340 не заявляется новым полным прогоном. Это producer test,
не полный draw/holster clip, стоимость AP, hold-aim или facial animation.

Свидетельства deferred-weapon-activation-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь скриптовой подготовки выстрела — часть 14ffbcc8

Отдельно зарегистрирован UpstreamDeferredShootPrepareCommand с тем же
настоящим actor/enemy fixture и реальной UnitShootPrepare. Compiled runtime
RED: prepare=1/body=0, commander-pending=0/unit-command=0. Nil actor/target
controls проходят до этого сбоя. После RED изменён только dispatch прежнего
CCmdShootObject с уже установленным bOnlyPrepareToShoot=true на DoCommand.

GREEN для BODY/HEAD проверяет prepare=true, остальные исходные flags/target/
actor/extra AP=0, две wrappers и Continue. Обычный UnitShoot параллельно
сохраняет prepare=false. Нет раннего unit action, изменений HP цели,
weapon/ammo, AP/position. Это producer proof, не запуск aim animation,
hold-aim lifecycle, face animation, расчёт попадания или LAN playback.

Связанные 77/77 прошли на Windows (91.09 с) и Linux (90.88 с), обе Game и
Windows ALL_BUILD собраны. DisplayRuntimeTests и по 12 FONT-COMPAT controls
включены. Windows NetworkSerializer/StopResume прошли 2/2 за 3.81 с.
Текущий CPU base 339 не заявляется новым полным прогоном.

Свидетельства deferred-shoot-prepare-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 и all281/final actual Actions ещё не завершены.

## Очередь скриптового выстрела — часть 14ffbcc8

UpstreamDeferredShootCommand использует настоящий вооружённый ordinary actor
и другого живого юнита с реальной ENEMY diplomacy в world 810. Пустые actor
и target отдельно проверяют нашу nil-hardening policy: Lua продолжает thread
и не выдаёт заказ. Compiled runtime RED для корректных объектов: prepare=0,
body=0, commander-pending=0/unit-command=0. Это потеря queued request;
тест не утверждает, что непосредственная команда в этом fixture выстрелила.

После RED только UnitShoot переведён на DoCommand с прежним CCmdShootObject.
GREEN для BODY=0/HEAD=1 проверяет ровно две queued wrappers, actor, target,
hit location, extra AP=0, prepare=false, reload replacement=false и Continue.
До consumer не меняются AP, position, weapon identity/ammo и HP/CanFight
цели, нет unit action. Nil controls сохраняются. Полный выстрел, попадание,
урон, выбор цели AI и LAN playback этим producer test не проверяются.
UnitShootPrepare остаётся отдельным ещё не исправленным producer.

Связанные 76/76 прошли на Windows (93.66 с) и Linux (91.19 с), обе Game и
Windows ALL_BUILD собраны. DisplayRuntimeTests и по 12 FONT-COMPAT controls
включены. Windows NetworkSerializer/Match/StopResume прошли 3/3 за 19.19 с.
Текущий CPU base 338 не заявляется новым полным прогоном.

Свидетельства deferred-shoot-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Source 14ffbcc8 остаётся частичным; all281/final actual Actions не завершены.

## Очередь скриптовой перезарядки — часть 14ffbcc8

UpstreamDeferredReloadCommand выбирает настоящий ordinary actor с оружием
и совместимыми запасными патронами. Native DiscardAmmo используется только
для подготовки fixture; CanReload подтверждает доступность перезарядки.
Compiled runtime RED: commander-pending=0/unit-command=1, ammo=0 до/после.
Первый build fixture обнаружил обращение DiscardAmmo через IWeaponItem;
исправлена только типизация fixture через CWeaponItem, это не считается RED.

После runtime RED только UnitReload переведён на существующий DoCommand с
прежним CCmdReload(-1) и Continue. GREEN проверяет точный тип, active-weapon
slot=-1, actor и ровно две wrappers в нужном порядке. До consumer сохраняются
weapon identity, inner clip, его ammo, порядок/identity предметов рюкзака,
количество патронов запасных clips, AP, position и отсутствие unit action.
Режимы стрельбы и mixed script order повторно проходят. Стоимость reload,
параметры perks, минимумы AP и реальное выполнение reload здесь не меняются.

Связанные 73/73 прошли на Windows (90.70 с) и Linux (87.83 с), обе Game и
Windows ALL_BUILD собраны; DisplayRuntimeTests и по 12 FONT-COMPAT controls
включены. Windows NetworkSerializer/StopResume прошли 2/2 за 3.81 с.
Текущий CPU base 337 не заявляется новым полным прогоном.

Свидетельства deferred-reload-command: fixture-interface-build.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log и
network-{test,native-test}.log. Source 14ffbcc8 остаётся частичным;
all281/final actual GitHub Actions ещё не завершены.

## Активация гранаты в неактивной руке — часть 14ffbcc8

UpstreamScriptHeldGrenadeActivation вызывает настоящую UnitSwitchToGrenade
в world 810. В руках — настоящая граната и оружие, в рюкзаке — запасная
обычная граната. Compiled runtime RED: ordinary held SLOT_1=0, active=1,
queued=0/unit-command=0: Lua завершилась, но граната осталась неактивной.
После добавления controls verified RED повторяет тот же сбой; четыре случая
инженерной гранаты и уже активная обычная граната проходят до failing case.

Новый helper использует исходный DB CRPGGrenade predicate и существующий
DoCommand(CCmdSetActiveItem, Continue). Он запускается перед прежней runtime
проверкой. Последняя сохраняется: upstream также менял fallback при
CRPGEngGrenade в руке; эта отдельная классификация отложена и описана в
UPSTREAM-BALANCE-CHANGES.md. Не объявляем её ребалансом без retail сверки.

GREEN проверяет восемь сочетаний ordinary/engineer, SLOT_1/SLOT_2 и
active/inactive. Только неактивная ordinary требует ровно двух queued
wrappers: правильный actor, CCmdSetActiveItem с held SLOT и Continue. Уже
активная граната не выдаёт лишнего заказа; spare в рюкзаке не извлекается.
До consumer руки, активный слот, рюкзак, unit command и AP не меняются.
Полная анимация переключения, оба grenade одновременно в руках и AP cost
этим тестом не проверяются.

Связанные 64/64 прошли на Windows (81.08 с) и Linux (84.16 с), обе Game и
Windows ALL_BUILD собраны. В набор входят DisplayRuntimeTests и 12
FONT-COMPAT controls на каждой платформе. Windows NetworkSerializer/Match/
StopResume прошли 3/3 за 21.18 с. Текущий CPU base 336 не заявляется
новым полным прогоном. Численные формулы и наш facial engine не меняются.

Свидетельства script-held-grenade-activation: red-{build,test}.log,
red-native-test.log, verified-red-{build,test}.log,
verified-red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.

## Очередь извлечения оружия и гранаты — часть 14ffbcc8

Два новых native Lua теста UpstreamDeferredDrawWeaponCommand и
UpstreamDeferredDrawGrenadeCommand используют тот же реальный inventory
fixture, в grenade case — настоящий CGrenadeItem с authored DB record,
quantity=1; обе руки удерживают оружие. Пустой рюкзак по-прежнему не выдаёт
команды. Compiled runtime RED 2/2: queued=0/unit-command=1 для обоих producers.

После RED изменён только общий dispatch в DrawFirstInventoryItem: существующий
DoCommand получает прежний exchange payload и Continue. GREEN 3/3 включает
старую проверку SLOT. Оба producers и обе активные руки проверяют ровно две
настоящие queued wrappers, actor, exchange source/target items/slots/activation
и порядок Continue, отсутствие ранних изменений рук, рюкзака и AP.
Это проверка границы очереди, не полное исполнение обмена или стоимость AP.
Held-grenade activation/priority остаётся отдельной задачей.

Связанные 61/61 прошли на Windows (73.04 с) и Linux (70.40 с), обе Game и
Windows ALL_BUILD собраны. Windows NetworkSerializer/Preparation/StopResume
и DisplayRuntimeTests прошли 4/4 за 32.02 с. Font control прошёл на Windows;
Linux font/display — 2/2 за 0.94 с под Xvfb, по 12 FONT-COMPAT controls.
Первый дополнительный Linux display-запуск ошибочно был без Xvfb: display
не запустился, font прошёл; исходные no-display logs сохранены, production
для повтора не менялся. Начальный regex связанного набора выбрал только 12
тестов: после уточнения выполнен указанный набор 61, исходные logs сохранены.
Текущий CPU base 335 не заявляется новым полным прогоном.

Свидетельства deferred-draw-inventory-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log,
font-{test,native-test}.log и linux-font-display-{test,native-test}.log.
All281 и final actual GitHub Actions остаются незавершёнными.

## Выбор руки при извлечении оружия — часть 14ffbcc8

UpstreamScriptDrawWeaponSlot запускает настоящую Lua UnitDrawWeapon на actor
world 810 с двумя реальными оружиями в руках и третьим в рюкзаке. Каждый
из двух active SLOT проверяется в отдельном мире в памяти, без копий сборки.
Пустой рюкзак служит контролем отсутствия команды. Compiled runtime RED,
затем повторный verified RED после уточнения fixture: active=0, expected=1,
actual=0/target=0. Причина — сравнение указателя GetActive() с нулём вместо
индекса GetActiveSlot().

Изменена только формула выбора противоположного SLOT в общем helper.
GREEN для active 0/1 проверяет реальные CCmdExchangeInventoryItems, оба
индекса target, source/target item и владельца, намерение активации. Тест
принимает непосредственную или queued доставку, сохраняя одинаковые проверки
payload: отдельная ошибка доставки ещё не исправлена. Полное перемещение
предмета, цена AP, пустая активная рука и grenade fallback здесь не проверены.

Связанные 75/75 прошли на Windows (71.80 с) и Linux (71.60 с), обе Game и
Windows ALL_BUILD собраны; по 12 FONT-COMPAT controls сохранены. Windows
NetworkSerializer/Preparation/StopResume прошли 3/3 за 31.82 с. Текущий CPU
base 333 не заявляется новым полным прогоном. Наши шрифты, facial engine,
HD и численные параметры оружия не меняются. Source 14ffbcc8 остаётся частичным.

Свидетельства script-draw-weapon-slot: red-{build,test}.log,
red-native-test.log, verified-red-{build,test}.log,
verified-red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-test.log и network-native-test.log.

## Режим стрельбы и порядок смешанного сценария — часть 14ffbcc8

UpstreamDeferredShootModeCommand использует настоящий вооружённый actor world
810 с поддержкой SNAP/AIMED; он может принадлежать AI. Исходный SNAP выбран
существующим weapon API только в fixture. Compiled runtime RED: requested
AIMED=1, commander-pending=0/unit-command=0, живой режим уже 1.

До изменения production добавлен второй самостоятельный native случай
UpstreamDeferredScriptCommandOrder: реальная Lua вызывает LOOK, SHOOTMODE,
WISHPOSE и PLAYANIMATION последовательно. Verified RED 2/2: отдельный вызов
меняет режим сразу, смешанный создаёт шесть queued wrappers вместо восьми,
а live mode уже 1. Остальные три producer ранее перенесены и сохраняют пары
с Continue; так подтверждается конкретное нарушение порядка из-за mode call.

После этих RED только UnitSetShootMode переведён на существующий DoCommand
с прежним CCmdShootMode. GREEN проверяет AIMED/SNAP, тип и value payload,
actor identity и точный Continue. Живой weapon остаётся тем же и в SNAP до
consumer. Смешанный случай извлекает восемь настоящих wrappers, проверяет
порядок LOOK/Continue, SHOOTMODE/Continue, WISHPOSE/Continue,
PLAYANIMATION/Continue и параметры. До потребления live mode=0/wish=2,
положение/AP/unit action не меняются. Другие ещё не перенесённые producers
этим смешанным тестом не охватываются. Это проверка очереди, не выполнение
четырёх действий, визуальная анимация, точность/цена выстрела или LAN playback.

Связанные script/pose/movement/forced/route/queue/fire-mode и mounted mode UI,
native medal/GPU/display прошли 67/67 на Windows (68.18 с) и Linux (65.59 с).
Обе Game и Windows ALL_BUILD собраны; по 12 FONT-COMPAT controls сохранены.
Windows NetworkSerializer/Match/StopResume прошли 3/3 за 22.74 с. Формулы,
поддерживаемые weapon modes, наш выбор effective/mounted weapon и wire types
не переписываются. Текущий CPU base 332 не заявляется новым полным прогоном.

Свидетельства deferred-shoot-mode-command: initial-red-{test,native-test}.log,
verified-red-{build,test}.log, verified-red-native-test.log,
green-{build,test}.log, green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-test.log и network-native-test.log. Source 14ffbcc8 остаётся частичным:
остальные producers, UnitHide bool и inventory/held-item hunks ещё проверяются.
All281/final actual GitHub Actions остаются незавершёнными.

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

## Очередь скриптовой посадки в панцеркляйн — часть 14ffbcc8

Перед production изменением обновлён прежний resource-free UpstreamPKWear:
существующие CPlayer/CAICommander и ScriptActor.owner установлены до Lua вызова,
проверяются настоящие два queued wrapper вместо непосредственного actor.Do.
Новый UpstreamDeferredWearPKCommand использует тот же native world/pilot/shell
fixture, что LeavePK, но без предварительного ношения. Nil actor/target и живой
солдат вместо пустого корпуса не создают команд. Compiled runtime RED 2/2:
старый тест обнаруживает обход owning queue, native — queued=0/unit-command=0
при валидном пустом корпусе. Затем изменена только отправка TakeCorpse(shell)
на DoCommand(...,true). Наш правильный IsEmptyPK guard сохранён.

GREEN 6/6 проверяет обе Lua операции, legacy resource-free PK controls,
точные TakeCorpse(shell)/Continue, actor и отсутствие лишних wrappers.
Native Wear не меняет отношения pilot/shell, обе позиции, AP, реальные hands/
backpack identities/order/active slots до consumer. Полный подход, анимация
посадки, переход footprint и стоимость AP этим fixture не доказываются.

Связанные 76/76 прошли на Windows (72.14 с) и Linux (72.66 с).
Обе Game и Windows ALL_BUILD собраны; PK/mounted/corpse/pose/holster,
native medal, DisplayRuntime и 12 FONT-COMPAT на каждой платформе прошли.
Оба raw LastTest содержат все 76 блоков без NUL. NetworkSerializer и
NetworkStopResume: 2/2 (3.83 с). CPU base 350; полный набор 350 не запускался.

Свидетельства deferred-wear-pk-command: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Все перечисленные queue producers source14 теперь перенесены отдельно.
UnitHide optional bool зависит от stateful API 432b64c8; both-held-grenade
priority coverage и DB-only engineering classification остаются на проверке.
Source14 и весь аудит/final actual Actions ещё не завершены.

## Доступность горячей клавиши скрытности — часть 432b64c8

UpstreamHideShortcutEligibility вызывает настоящий CMission::ProcessEvent
через PostEvent/CBind, настоящий action cache и CUnitTracker. Outbound
Command перехватывается на границе: тест не исполняет игровое действие и
не подменяет расчёт его стоимости. В cache заданы четыре сочетания разрешений
Hide/Strafe. Два согласованных контроля проходят до divergent RED:
Hide разрешён, Strafe запрещён — команда отсутствует, запрошен UA_STRAFE.
После runtime RED только bindHide GetActionInfo заменён на UA_HIDE.

GREEN: все четыре сочетания, точный CCmdHide/выбранный адресат, отсутствие
команды при запрещённом Hide и доступном Strafe, пустой выбор и отсутствие
посторонних interface commands. Этот тест подтверждает правильный routing
готовой action availability; реальная возможность скрыться в конкретном бою,
групповая операция и stateful Hide остаются отдельными проверками.

Связанные Hide/Hiding/Strafe/Shortcut/Registration/font/GPU/input проверки:
20/20 Windows (10.00 с), 20/20 Linux (9.54 с), обе Game и Windows ALL_BUILD
собраны. По 12 FONT-COMPAT; оба raw LastTest полные, без NUL.
Свидетельства hide-shortcut-eligibility: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log. CPU base 351, полный 351 не запускался.
Source432 и all281/final actual Actions не завершены; API/serialization,
групповой Hide, PK damage и цвет/fade не входят в этот однострочный фикс.

## Явное состояние команды скрытности с legacy compatibility — часть 432b64c8

Новый native UpstreamExplicitHideState сначала подтвердил runtime RED:
после двух старых toggle controls явный запрос false для открытого human actor
получает стоимость AC_HIDE вместо нуля. Запрос подготовлен через serializer,
чтобы тест собирался до появления bool-конструктора. Начальные ошибки fixture
(private operator&, отсутствующий CanDoIt у base executor, попытка создать
незарегистрированный one-shot CExecHide по ID) исправлены до содержательного
RED, их отдельные логи сохранены.

В CCmdHide и CExecHide добавлены bState/tag3 и bExplicitState/tag4.
Старый base/tag2 сохранён; upstream base1/bool2 буквально не переносится.
CCmdHide() и отсутствующие новые поля оставляют live toggle, bool ctor задаёт
явное состояние. Factory передаёт оба признака исполнителю. Явное повторение
уже установленного состояния не переключает его и не вызывает DoAction.
Явное открытие имеет нулевую стоимость и обновляет visibility при переходе.
Прежние cooldown/PK guards и формула стоимости скрытия не изменены.

GREEN проверяет десять native выполнений: два legacy toggle controls и все
initial/requested false/true с восстановлением executor fields и без него.
Отдельно проверены новый bool constructor и зарегистрированный graph round trip
самой CCmdHide. Legacy payload формируется старым base-only/tag2 serializer;
проверяется совместимость полей, не полный исторический save. CExecHide не имеет
saveload class ID и остаётся one-shot: fixture восстанавливает его поля через
native CreateExecutor с borrowed actor references, не заявляет восстановление
произвольного save с этим executor. Ни создание, ни восстановление не меняют
живое состояние до Run; после Run проверены FINISHED, position и action counter.

Проверка усилена настоящим GivePlayerTurn: не realtime/sequence, точное
списание неизменённой native AC_HIDE стоимости и ноль для unhide/no-op.
На Windows финальные target 5/5 и связанные 54/54 (47.19 с); Linux связанные
54/54 (45.69 с), затем повторная сборка актуального усиленного fixture и
ExplicitHideState 1/1 (8.97 с). Обе Game и Windows ALL_BUILD собраны,
по 12 FONT-COMPAT, оба related raw LastTest полные без NUL.
Существующие NetworkSerializer/Match/StopResume 3/3 (39.19 с); отдельный
боевой LAN Hide сценарий этим прогоном не заявляется.

Свидетельства explicit-hide-state: fixture-interface-build.log,
fixture-registration-test.log, red-{build,test}.log, red-native-test.log,
green-{build,test}.log, green-{initial-,}native-test.log, all-build.log,
related-test.log, linux-{configure,initial-build,build,related-test,test}.log,
{windows,linux,linux-final}-native-test.log, network-{test,native-test}.log.
CPU base 352, полный 352 не запускался. Group Hide UI и Lua optional bool
ещё отдельные исправления; source432 и all281/final actual Actions не завершены.

## Общее состояние скрытности выбранного отряда — часть 432b64c8

UpstreamGroupHideShortcut выбирает реального human actor и создаёт второго
того же владельца через AddUnitInGame/NRPG::CreateUnit на свободном native
path-network месте. Это два настоящих бойца с authored RPG, а не fake states.
Тест вызывает настоящий ProcessEvent через PostEvent/CBind и lazy GetActionInfo.
Availability probe и outbound Command перехвачены на границе; исходящие команды
затем исполняются настоящими CreateExecutor/Run в native player turn.
Compiled runtime RED: два открытых выбранных бойца, requests=0 вместо двух.

Добавлен локальный GetGroupHideState в iMission.cpp: непустой полностью
скрытый выбор означает открыть, остальные — скрыть. Он используется и при
lazy создании UA_HIDE preview, и при отправке одного explicit CCmdHide на каждого.
Upstream eager UpdateActionsInfo буквально не копируется: у нас lazy cache
и собственный network preview. Новые поля CMission и сериализация не нужны.

GREEN покрывает все четыре пары initial states в обоих порядках выбора:
восемь групповых случаев, один общий target, точные адресаты/порядок,
совпадение preview и requests, отсутствие ранней смены состояния/AP,
реальное исполнение обоих участников и точную прежнюю стоимость AC_HIDE.
Недоступная операция и пустой выбор не отправляют команд.
Старый resource-free routing fixture имел bare CUnitServer без RPG; после
нового чтения IsHiding он падал. Ему добавлен explicit visible-state override;
native group fixture продолжает читать настоящее RPG. Первые failed logs
сохранены, затем обе платформы пересобраны и повторно проверены.

Target 4/4, связанные 24/24 Windows (15.48 с) и Linux (13.92 с).
Обе Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT, raw24 полные без NUL.
NetworkSerializer/ClientUI/StopResume: 3/3 (5.54 с). Это регрессии существующего
LAN; отдельный multiplayer group-hide gameplay сценарий не заявляется.
Свидетельства group-hide-shortcut: red-{build,test}.log, red-native-test.log,
fixture-eligibility-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
linux-fixture-{test,native-test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log. CPU base 353, полный 353 не запускался.
Source432 остаётся частичным из-за независимых PK damage/colour/fade hunks;
all281/final actual Actions ещё не завершены.

## Явное/default состояние Lua UnitHide — часть 14ffbcc8

Отдельный UpstreamScriptHideState использует настоящего human actor, Lua
CScript, owning commander и native TBS turn. Nil actor возвращается без
команды. Первый вызов UnitHide(actor) для открытого актёра проходит; compiled
runtime RED при повторении для уже скрытого: requested=1, actual=0,
explicit=0. Это ошибочное переключение, а не воспроизведение по тексту кода.

После RED меняются только signature на ub[true] и конструктор CCmdHide,
получающий luaParams[1].b. Наши nil hardening и DoCommand/Continue сохраняются.
GREEN: шесть случаев — default/nil/1 в Lua 4, начальное hidden false/true.
Проверены callback return, ровно два wrapper/точный actor/CCmdHide+Continue/
порядок/нет extras, состояние/AP/position до consumer, затем настоящее
CreateExecutor/Run, итоговое состояние и точная прежняя стоимость AC_HIDE.
Сохранённые request objects отдельно подтверждают explicit flag и bool target,
независимые от последующего live state. Полный Lua scheduler/world playback
и отдельный LAN script-hide сценарий не заявляются.

Target 4/4, связанные 54/54 Windows (48.92 с) и Linux (47.73 с),
обе Game и Windows ALL_BUILD собраны. По 12 FONT-COMPAT; raw54 полные без NUL.
NetworkSerializer/StopResume 2/2 (4.01 с). CPU base 354, полный 354 не запускался.
Свидетельства script-hide-state: red-{build,test}.log, red-native-test.log,
green-{build,test}.log, green-native-test.log, all-build.log,
related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
UnitHide bool dependency теперь перенесена. Source14 остаётся частичным:
both-held priority controls и DB-only engineer classification/retail review.
All281/final actual Actions ещё не завершены.

## Общий checkpoint после script dispatch и Hide — HEAD 69d75801

До следующего изменения выполнен полный актуальный набор 359:
CPU base 354, три native medal и Bgfx/DisplayRuntime. Windows 359/359
(299.84 с), Linux 359/359 (299.98 с), включая 14 native-audio случаев и
по 12 FONT-COMPAT. Raw LastTest на обеих платформах содержит все 359 blocks
без NUL; исправление nested CTest больше не теряет первый Bound.

Windows CI-like набор без baseline/extended/GPU: 276/276 (24.54 с),
raw276 полный, без NUL, с FONT-COMPAT controls. Свидетельства
script-dispatch-checkpoint: full-test.log, windows-native-test.log,
linux-{configure,build,test}.log, linux-native-test.log,
ci-test.log и ci-native-test.log. Это локальный checkpoint точного HEAD.
GitHub Actions run 40421094390 при наблюдении ещё собирал проект, Test pending;
он не объявляется успешным и не заменяет финальную проверку после всех переносов.

## Безопасный запрос маршрута пустого панцеркляйна — часть 8da24a09

UpstreamEmptyPKRouteLookup расширяет существующий native PK fixture отдельной
query веткой: настоящий world/pilot, authored suit через AddUnitInGame, валидный
пустой корпус без владельца. До опасного вызова проходят nil/non-AI controls,
а настоящий AI actor получает CAIRouteLogic/Wait(100) через SetRouteLogic.
Lua возвращает точную route identity; RouteIsFinished даёт nil до Finish
и 1 после него. Первая подготовка использовала SetLogic (combat slot) и
не прошла fixture precondition; она исправлена до содержательного RED,
исходный fixture-slot-test.log сохранён.

Compiled runtime RED: после сообщения об успешных controls
UnitGetRoute(ownerless empty PK) завершается SEGFAULT. Только затем
прямая цепочка GetPlayer/GetCommander заменена существующим NAI::GetAIUnit,
который проверяет nil owner и EmptyPK. GetRouteLogic сохранён.

Наш RouteIsFinished по CAIRouteLogic::IsRouteComplete не изменяется.
Собственный bc7e00aa74e21cf895ba56c2574238fcc7916f05 уже восстановил ожидание;
upstream generic IAILogic::IsFinished не подставляется вместо нашего latch.
Это адаптация к текущему движку, а не повторный перенос старого route stub.

GREEN 5/5, связанные 48/48 Windows (41.00 с) и Linux (40.30 с),
обе Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT, raw48 полные без NUL.
NativeScriptRouteWaitTests и NetworkSerializer/StopResume: 3/3 (4.03 с).
Проверены nil возврат/завершение callback/нет pilot orders и настоящая route
identity/completion; полный сценарий движения/sequence/LAN не заявляется.

Свидетельства empty-pk-route-lookup: fixture-slot-test.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log. CPU base 355; последний полный checkpoint
остаётся base354+5=359 на HEAD69d75801. Source8da теперь partially-imported:
hint slot placement, world spawning и exhaustion cleanup ещё проверяются.
All281/final actual Actions не завершены.

## Удаление книг после исчерпания подсказок — часть 8da24a09

Новый native UpstreamExhaustedHintPickups создаёт два настоящих CHintItem
через CreateHintItem и world.AddFrozenItem. Третий frozen предмет создан как
обычный CDummyItem с тем же DB438 и моделью: это контроль marker type,
а не иной record ID. Первая AddNextUIHint(true) продвигает реальную authored
последовательность и сохраняет все три предмета. Затем без подмены cursor/
flags исчерпывается настоящая таблица подсказок. Compiled runtime RED:
hints=34, remaining-books=2, ordinary-retained=1. Первоначальная ошибка
fixture overload AddFrozenItem исправлена до runtime RED, лог сохранён.

RemoveAllHintItems удаляет только showFrozenItems с IHintItem. Метод вызывается
после bNoMoreHints=true перед прежним UpdateVisible. Новых persistent fields
и virtual slots нет; порядок/выбор подсказок, threshold exhaustion и AP не
редактируются. Устаревший комментарий об отсутствии hint subsystem исправлен.

GREEN подтверждает удаление обеих книг, сохранение обычного предмета с той же
моделью/DB, отсутствие продвижения cursor/hintsSet и потери ordinary предмета
при повторном exhausted вызове, отсутствие gameplay action. Это проверка
очистки существующих pickups, не доказательство ещё отсутствующей map-slot
цепочки появления книг. По коду pickup executor извлекает GROUND source до
AddNextUIHint(false), после вызова сразу возвращается; полный pickup animation/
AP/interaction сценарий этим прямым fixture не исполняется.

Target 1/1, связанные 38/38 Windows (28.42 с) и Linux (81.34 с),
обе Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT, raw38 полные без NUL.
NetworkSerializer/StopResume: 2/2 (4.09 с). CPU base 356; последний полный
checkpoint остаётся base354+5=359 на HEAD69d75801.

Свидетельства exhausted-hint-pickups: fixture-interface-build.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log. Source8da остаётся частичным:
hintSlots/MapBuild transport и вызов PlaceHintsToMap ещё проверяются.
All281/final actual Actions не завершены.

## Появление интерактивных книг из map markers — завершение 8da24a09

UpstreamAuthoredHintPlacement проходит настоящий CWorld::CreateRandom/
MapBuild на варианте 810. Fixture добавляет один marker DB438 только в
загруженную БД: CFinalElement получает инициализированные поля authored
placement через serializer, новый CPlacableObject с настоящим book record
и контролируемый transform в root template. Файлы/данные поставки не
переписываются. Это контролируемый marker fixture, не утверждение о наличии
такого marker в неизменённой поставке карты 810. Первый вариант с подменой
clue marker не попадал в выбранную карту: fixture precondition fail сохранён,
до содержательного RED подготовка исправлена.

Compiled native RED: map-book-records=1, interactive-hints=0 — маркер
становился обычным инертным предметом. Arena control перед ним проходил.
После RED добавлены SMapInfo.hintSlots, выделение DB438 в AddSimpleElements,
перенос при SetOnLayer/Transfer, terrain FixHeights и вызов существующего
PlaceHintsToMap после Sync геометрии. Векторы SMapInfo transient, save/wire
поля мира не заменяются. Комментарии о неиспользуемом helper обновлены.

Сохранена наша специфика LAN: PlaceHintsToMap вызывается только вне
bNetworkArena и пока кампания не exhausted. Native arena с новым marker и
неисчерпанной game sequence проверяется отдельно: campaign hint pickups
в нём отсутствуют. GREEN также подтверждает настоящий IHintItem в кампании,
отсутствие продвижения hint sequence от размещения, реальное исчерпание
последовательности/удаление сгенерированных книг и создание следующего мира
с тем же exhausted game без новых книг/смены cursor/hintsSet.
Отдельные nested floors, все shipped карты и pixel output не заявляются.

Из-за изменения создания мира выполнен полный текущий набор 362:
CPU base 357 + три native medal + Bgfx/DisplayRuntime.
Windows 362/362 (294.35 с), Linux 362/362 (325.96 с), обе Game и Windows
ALL_BUILD собраны. По 14 native-audio и 12 FONT-COMPAT; raw362 полные без NUL.
Windows NetworkSerializer/Match/ClientUI/StopResume 4/4 (24.36 с).
Windows CI-like 276/276 (24.08 с), raw276 полный без NUL. Это локальные
проверки; финальный actual GitHub Actions всего переноса ещё не завершён.

Свидетельства authored-hint-placement: fixture-marker-test.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, full-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log, ci-{test,native-test}.log.

Все применимые части source8da теперь учтены: безопасный UnitGetRoute —
9290a0a3, exhaustion cleanup — c3f52234, marker pipeline — этот перенос.
Route slots/ожидание ранее восстановлены собственным bc7e00aa; наш
CAIRouteLogic::IsRouteComplete сохранён вместо generic IsFinished и
подтверждён native route controls. Остальные aiUnit hunks — устаревшие
комментарии, соответствующая реализация уже существует. Source переведён
в imported с этой адаптацией. Число незавершённых источников 142,
не число будущих fix commits; весь аудит 281 ещё продолжается.

## Ориентация модели метательного ножа — часть fcc994bc

UpstreamThrownKnifeOrientation создаёт настоящий projectile через
CreateKnifeServer в native world, с modelled authored throwing-knife record.
CTRndModel разрешается штатным CreateModel; настоящий IVisObj::Visit передаёт
этот конкретный model и DG pose в AddItemMesh visitor. Тест оценивает root pose
на текущем времени и направление local -Z. Начальные fixture compile ошибки
(model pool против CModel, Visit на отдельном IDynamicObject interface)
исправлены до runtime RED, их логи сохранены.

Compiled runtime RED: vertical-down control имеет error=0, а при velocity
(10,0,0) модель направлена (-1,0,0), squared direction error=4.
После RED только qRot constructor меняет знаки axis/acos в соответствии с
forward -Z. acos выполняется внутри прежнего nonzero-axis guard.
GetFrame position/velocity, lifetime, attack/trace/AP/damage и save fields
не изменены.

GREEN: восемь native projectiles — vertical-down, четыре горизонтальных
направления, три наклонных. Проверены направление (<1e-6), конкретный model,
один root bone, начальная позиция и освобождение action counter после удаления
projectile. Это pose/render-boundary proof, не pixel screenshot и не прогон
коллизий/урона/времени полёта. Zero velocity и vertical-up не покрываются;
новое исправление этих входов не заявляется.

Target 3/3 с прежними KnifeFeedback/SuccessorHandoff, связанные 18/18 Windows
(13.62 с) и Linux (13.37 с), обе Game и Windows ALL_BUILD собраны.
По 12 FONT-COMPAT; raw18 полные без NUL. NetworkSerializer/StopResume 2/2
(4.00 с). CPU base 358; последний полный checkpoint a196329d —
base357+5=362 на обеих платформах, CI-like276.

Свидетельства thrown-knife-orientation: fixture-model-build.log,
fixture-visitor-build.log, red-{build,test}.log, red-native-test.log,
green-{build,test}.log, green-native-test.log, all-build.log,
related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log.
Sourcefcc теперь partially-imported; независимые melee hull, coloring,
attack pose, equipment и door-path hunks ещё проверяются. All281/final actual
Actions не завершены.

## Разделение invalid/valid областей при пересчёте связности — часть fcc994bc

Новый resource-free UpstreamInvalidRegionColouring использует настоящие
CNodesLayer, CNodesLayerProxy и CMapColourer::ConstructColouring.
Сначала проходят valid-edge controls: cardinal/diagonal, stand/crouch/lay/HC
с прежними weights 1/2/3/5, stand-only restrictions и locked destination.
Затем контролируемая сетка моделирует invalid source с оставшейся movement
mask и valid neighbour. Compiled native RED: invalid=1, valid=1,
internal-invalid=0, edge-cost=2 — реально построенная область сливается с valid.
Это воспроизведение на grid/cache API, не replay такого состояния shipped карты.

Перед production изменением старый FreshNavigationColouring fixture получил
два настоящих walkable islands вместо требования разных ID у всех blocked
клеток. Он по-прежнему сравнивает full/first incremental cache и отсутствие
перестройки без dirty region. Обновлённый старый тест прошёл на исходном коде
в том же RED наборе; основной новый тест упал после valid-cost controls.

После RED добавлена только ветка invalid source в GetOneMoveCost:
не соединять его с valid target, но соединять внутренние invalid клетки.
GREEN проверяет native region separation/internal coalescing, оба режима,
прежние cardinal/diagonal invalid-region weights и valid-edge controls.
Valid-node cost formulas, RPG AP, поля/serialization и layout не заменяются.

В Windows MSVC 19.44 трижды падал при компиляции нового test loop, код
-529706956/InvokeCompilerPassW/CloseTypeServerPDB. Удаление /MP и пересоздание
только task-private vc143.pdb/cmake_pch.pch не помогли; исходные логи сохранены.
После выделения test-only ColouringMoveCost с noinline штатная MaxSpeed/Ob1
сборка прошла. Это связь с test-loop inlining по наблюдению, не установленная
внутренняя причина MSVC. Native proxy по-прежнему исполняется; исходное
ConstructColouring assertion не заменено helper. Оптимизация игры не меняется.

Windows final target 3/3, ALL_BUILD и обе Game собраны. Полный набор
364/364 Windows (300.07 с), 364/364 Linux (294.85 с), CPU base359+5,
по 14 native-audio и 12 FONT-COMPAT, raw364 полные без NUL.
Linux полный прогон предшествовал только test-call outlining; финальный
fixture пересобран и отдельно прошёл 3/3 (0.05 с). Production при этом
не менялся, поэтому полный Linux повторно не запускался.
NetworkSerializer/Match/StopResume 3/3 (21.74 с), CI-like277/277
(23.80 с), raw277 полный без NUL. Actual GitHub Actions всего переноса
ещё не объявляется успешным.

Свидетельства invalid-region-colouring: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
green-{compiler-crash,serial-compiler-crash,rebuilt-cache-compiler-crash}-build.log,
all-build.log, full-test.log, windows-native-test.log,
linux-{configure,initial-build,build,initial-test,test}.log,
linux-{initial-,final-,}native-test.log,
network-{test,native-test}.log и ci-{test,native-test}.log.
Sourcefcc остаётся частичным: остальные независимые hunks проверяются.
All281 и финальный actual Actions не завершены.

## Видимость предмета при явной смене руки — часть fcc994bc

Новый UpstreamExplicitItemActivationVisibility использует настоящего человека,
inventory, CCmdSetActiveItem, CreateExecutor и Run/TimeLabelReached/
AnimationFinished. Четыре случая: authored bPlaceInHand true/false для обычного
и heavy оружия. Флаг меняется только в загруженной fixture DB; файлы ассетов
не правятся. Переключение идёт на другой slot с отличным authored record.
В настоящем player turn проверены AP, ammo, позиция, прежний предмет,
завершение FSM, отмена persistent holster preference и освобождение action.

Compiled runtime RED после двух visible positive controls:
ordinary authored-visible=0, animator-active=1. Только после RED нормальная
ветка CExecSetActiveItem::AnimationFinished передаёт существующий
!IsActiveItemToShow в ActivateItem для обеих веток subType.
Уже правильный Cancel и особенности нашей holster policy сохранены.
Это исправление состояния body/item activation, не лицевой анимации.
Тест вызывает публичные FSM callbacks; естественный animation clock,
pixel render и ladder/corpse fallback этим тестом не подтверждаются.

GREEN target4/4; связанные 30/30 Windows (33.79 с), Linux (31.05 с),
обе Game и Windows ALL_BUILD собраны, по 12 FONT-COMPAT,
raw30 полные без NUL. NetworkSerializer/StopResume 2/2 (4.21 с).
Первый дополнительный network selection имел ошибочный префикс имени и
не выбрал тестов; затем исправлен selector, оба реальных теста прошли.
CPU base360, последний полный checkpoint d33ec7da base359+5=364/CI-like277.
Свидетельства explicit-item-activation-visibility: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Sourcefcc остаётся частичным; all281/final actual Actions не завершены.

## Видимость предмета при команде пути ITEM_ACTIVE — часть fcc994bc

Новый UpstreamMovementItemActivationVisibility расширяет прежний fixture:
CCmdSetActiveItem.Run/Cancel устанавливает настоящую активную руку,
затем настоящий одноточечный ITEM_INACTIVE path убирает видимый предмет.
Прямой записи private/animator flags для подготовки нет. После этого
настоящий CreateSimpleMoveExecutor с ITEM_ACTIVE исполняет собственную
CCmdActivateItem ветку нашего mover; проверяется полное завершение её FSM.

Compiled runtime RED: прежний ExplicitItemActivationVisibility проходит,
два visible movement controls проходят, ordinary hidden остаётся
animator-active=1. После RED только CExecMove::DoCommand передаёт существующий
!IsActiveItemToShow в три ActivateItem call sites (heavy/detector/ordinary).
Планирование ConvertPath, persistent/short-path holster policy, AP и wire
не заменяются. GREEN: normal/heavy x visible/hidden, AP/ammo/позиция/обе руки,
отмена старого holster preference и action release. Отдельный detector record,
путешествие/auto-activation/лестницы/pixels/natural animation clock не покрыты.

Target7/7; связанные 45/45 Windows (50.08 с) и Linux (49.66 с),
обе Game и Windows ALL_BUILD, по 12 FONT-COMPAT, raw45 полные без NUL.
NetworkSerializer/PartialMovement/StopResume 3/3 (6.30 с).
CPU base361; последний полный checkpoint d33ec7da base359+5=364/CI-like277.
Свидетельства movement-item-activation-visibility: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Sourcefcc остаётся частичным; all281/final actual Actions не завершены.

## Остановка пути перед отдельным открыванием двери — часть fcc994bc

UpstreamDoorPathStop на настоящей карте находит authored door/approach,
реальный свободный соседний tile и подход через FindPath. Единственная
fixture door оказалась locked; она отпирается штатным LockDoor(false), без
изменения DB/assets. В path добавляется PA_OPEN в точке взаимодействия,
как при разделении более длинного пути. Проверка выполняет настоящий
front mover, созданный CExecQueue, через Run/TimeLabelReached/AnimationFinished
в player turn,
останавливаясь до запуска отдельного OpenClose executor.

Первая fixture compile ошибка с Windows macro near и неуспешные поиски
unlocked подхода исправлены до runtime RED; их логи сохранены.
Настоящий RED: ordinary moving-endpoint control сохраняет moving=1,
explicit-stopped door control даёт moving=0, а moving door endpoint
оставляет actor moving=1 перед отдельным открыванием. Во всех случаях
реальный расход 4 AP совпадает с native GetActionAP, дверь ещё закрыта.
После RED добавлена одна SetMoving(false) для конца разделённого сегмента.
GREEN даёт moving=0 для третьего случая, сохраняя оба контроля и те же 4 AP.

Проверяется конкретный подход с уже верным heading; дополнительный поворот,
INACTIVE/ladder, полное открывание двери и natural animation clock не заявлены.
Не меняются AP tables, ключи/взлом, flipper geometry и wire fields.
Target5/5; связанные38/38 Windows (30.77 с), Linux (29.33 с),
обе Game и Windows ALL_BUILD, по12 FONT-COMPAT/raw38 полные без NUL.
NetworkSerializer/ObjectStages/StopResume3/3 (5.73 с).
CPU base362; полный checkpoint d33ec7da base359+5=364/CI-like277.
Свидетельства door-path-stop: fixture-name-build.log,
fixture-{approach,pose,lock}-{test,native-test}.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log. Sourcefcc частичный; all281/final Actions pending.

## Уведомление о росте навыка от обычного действия — часть c55ed688

UpstreamActionSkillAcknowledgment создаёт два настоящих CUnit из authored
ordinary classed persona. Одинаковый AddXP открывает cap для обучения.
Восемь RegisterAction cases: melee, grenade/knife throw, shoot/prepared shoot,
burst, side/diagonal movement. Каждый вызов сравнивается с прежним native
CUnit::UseSkill(skill,0), включая value, XP part, fractional progress, max
всех skills и общий XP. Для ordinary persona PK skill offset равен нулю.

Compiled native RED: AC_NONE и четыре fractional uses не создают ack, на
первом реальном melee improvement action20/skill0/use4 native training
совпадает с reference, но ack отсутствует. Только после RED пять call sites
RegisterAction используют уже существующий CUnitMission::UseSkill wrapper.
Он сохраняет наш GetSkillAddValue (в том числе PK offset), прежний native
CUnit::UseSkill и добавляет N_ACK_SKILL только при его true return.
Формулы обучения, cap/rounding, XP/perks, AP и damage не переписываются.

GREEN: по два настоящих improvement для каждого из восьми действий,
один N_ACK_SKILL с null attacker на improvement; на fractional/no-training
действиях нет уведомлений и дубликатов. Сохранено точное совпадение всех
training fields с неизменной native формулой на каждом use.
Это RPG acknowledgment queue boundary, не проигрывание голоса/портрета.

Target3/3; связанные22/22 Windows (14.75 с) и Linux (14.53 с),
обе Game и Windows ALL_BUILD, по12 FONT-COMPAT/raw22 полные без NUL.
NetworkSerializer/ClientUI/StopResume3/3 (8.45 с). CPU base363,
полный checkpoint d33ec7da base359+5=364/CI-like277.
Свидетельства action-skill-acknowledgment: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log.
Sourcec55 теперь partial; отдельные snipe/VP/interrupt producers ещё проверяются.
All281/final actual Actions pending.

## Уведомление о росте Interrupt — часть c55ed688

UpstreamInterruptSkillAcknowledgment расширяет native training fixture:
четыре сочетания mutual/was-shot и отдельный случай с AP, потраченными
штатным SpendAP. Настоящий CheckInterrupt вызывает native обучение.
Перед вызовом копируется ISAAC state; контроль потребляет один Get(100),
после вызова четыре следующих слова совпадают с ним. Проверяется диапазон
вероятности [5,95] и прежний нулевой результат probability при exhausted AP.
Это не независимый oracle точной формулы вероятности перехвата.

Compiled native RED: прежний ActionSkillAcknowledgment проходит,
AC_NONE/four fractional uses и RNG checks проходят; ST_INTERRUPT
реально растёт на use4, ack=0. После RED изменён только этот UseSkill call
на существующий mission wrapper. GetSkillAddValue/PK offset, native training,
probability code, RNG calls и возвращаемый результат не переписываются.

GREEN: по два improvement во всех пяти случаях; все skills/value/XP part/
fractional progress/max и общий XP совпадают с прежним native UseSkill
reference на каждом use. Один N_ACK_SKILL/null attacker на рост,
нет fractional/no-training/duplicate ack. Старое обучение при exhausted AP
также сохранено; новая политика обучения не вводится. Это RPG queue boundary,
не реальный world interrupt dispatch и не voice/portrait playback.

Target3/3; связанные23/23 Windows (15.78 с), Linux (15.13 с),
обе Game и Windows ALL_BUILD/12 FONT-COMPAT/raw23 полные без NUL.
NetworkSerializer/ClientUI/StopResume3/3 (5.40 с). CPU base364,
последний полный checkpoint d33ec7da base359+5=364/CI-like277.
Свидетельства interrupt-skill-acknowledgment: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log.
Sourcec55 partial: snipe/VP producers остаются. All281/final Actions pending.

## Уведомление о росте Snipe при настоящем выстреле — часть c55ed688

UpstreamSnipeSkillAcknowledgment создаёт authored ordinary persona и настоящий
sniper rifle, public SaveAP(4,target), вызывает native CreateAttack.
Ordinary ammo-spending shot и sniper preview не тренируют Snipe, не создают
ack; preview не расходует патрон. Настоящий sniper shot расходует ровно один
патрон, его все сериализуемые attack fields совпадают с preview до роста skill.
Saved target/aiming AP сохраняются. Familiarity отключена существующим
bAdaptWeapon=false для изоляции этого производителя уведомлений.

Compiled runtime RED после этих контролей и четырёх fractional uses:
anonymous=0/use4/Snipe advanced/ack0, native training fields совпадают с reference.
После RED меняется только sniper UseSkill call на существующий mission wrapper.
GREEN: identified/anonymous атаки, по два настоящих skill improvement,
все skills/value/XP part/fractional progress/max и общий XP совпадают с прежним
CUnit::UseSkill reference на каждом use. Один N_ACK_SKILL/null attacker на рост,
нет preview/ordinary/fractional/duplicate ack; ammo/attack fields/saved AP
контролируются на каждой атаке. Перезаряд fixture — установка свежего rifle
того же DB record через inventory при исчерпании ammo; reload gameplay не заявлен.
Это создание native attack portion, не полёт projectile/попадание/урон миру.

Target3/3; связанные42/42 Windows (36.85 с), Linux (36.45 с),
обе Game и Windows ALL_BUILD/12 FONT-COMPAT/raw42 полные без NUL.
NetworkSerializer/ObjectStages/StopResume3/3 (5.62 с). CPU base365,
полный checkpoint d33ec7da base359+5=364/CI-like277.
Свидетельства snipe-skill-acknowledgment: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log.
Sourcec55 partial: ProcessAttack VP producer остаётся; all281/final Actions pending.

## Уведомление о росте живучести при получении урона — завершение c55ed688

UpstreamVitalitySkillAcknowledgment создаёт authored ordinary persona/body
armor и выбирает damage type, разрешённый этой бронёй. Настоящий native
IUnitMission::ProcessAttack получает контролируемую portion: penetration100000,
body damage1..1, critical probability0, unconscious probability-1,
target=this mission. World=null использует поддерживаемый CalcStructDmg
контракт без campaign difficulty factors. Это изоляция producer ack,
не физическая коллизия пули/полный world damage dispatch.

Zero-penetration control: нет урона/обучения/ack. Затем четыре дробных
training uses проходят, на use4 skill реально растёт, но ack0 — native
compiled RED после прежних Action/Interrupt/Snipe green controls.
До production изменения сверяются все training fields/общий XP с прежним
native CUnit::UseSkill(VP,0); reference также получает один пункт урона.
Нулевая вероятность critical обходит crit RNG, target=this обходит dodge;
ISAAC по-прежнему потребляет один unconscious Get(100), четыре следующих
слова совпадают с контрольной копией.

После RED меняется только этот VP UseSkill call на существующий mission
wrapper, сохраняя наш PK offset/обычную native формулу. GREEN:
identified/anonymous attacks, по два growth, ровно1 damage, без death/KO/
wound modifier; все training fields/XP совпадают с reference.
Один N_ACK_SKILL/null attacker на рост, нет blocked/fractional/duplicate ack.
Между ударами оба fixture unit восстанавливают полученный пункт через
публичное изменение VP; это не проверка medical gameplay/лечения.

Оба Game и Windows ALL_BUILD собраны. Полный checkpoint371/371 Windows
(312.00 с),371/371 Linux (310.77 с), CPU base366+5,14 native-audio/12
FONT-COMPAT на каждой платформе, raw371 полные без NUL.
Target4/4 (3.93 с); NetworkSerializer/ClientUI/ObjectStages/StopResume4/4
(7.05 с). CI-like277/277 (24.15 с), raw277 полный без NUL.
Actual hosted Actions на прежнем6222ff0b проверяется отдельно от локального
CI-like; весь перенос/all281 ещё не завершён.

Свидетельства vitality-skill-acknowledgment: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, full-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log,
ci-{test,native-test}.log и ci-tests.txt.

Sourcec55ed688 завершён в адаптации к нашей двухаргументной training формуле:
RegisterAction13526c34, CheckInterrupt249b8a21, sniper CreateAttack6222ff0b,
damage ProcessAttack — этот фикс. Прямой pRPGUnit->UseSkill остался только
внутри уже существующего wrapper с GetSkillAddValue.
Статус281:119 pending/96 imported/21 partial/38 already/3 docs,
по1 historical/review/excluded-balance/excluded-own.141 исходный коммит
ещё требует разбора; это не число будущих fix commits.

## Разбор четырёх balance-only источников после checkpoint371

Полностью прочитаны diff eb45f18a/2e15a102/24a00fd1/aa724eca и
сопоставлены действующие Upgrade/RegisterAction/GetWeaponBurstAP/
ProcessCriticalsAndRegenerations. Независимых engine/UI-only частей в них
не осталось: меняются пороги обучения, начисление Stealth, AP дополнительных
пуль/минимум/sentinel и награда за смерть от bleeding соответственно.
Эффекты и ограничения retail attribution подробно дописаны в
UPSTREAM-BALANCE-CHANGES.md. Они исключены из текущего no-balance pass
для пользовательского рассмотрения; production не изменён, новые тесты
для документационного решения не требуются.
Новые статусы281:115 pending/96 imported/21 partial/38 already/3 docs,
по1 historical/review/excluded-own и5 excluded-balance.137 исходников ещё
разбираются, не137 заранее определённых fix commits. Последний полный
checkpoint остаётся4da43aaf:371/371 обе платформы/CI-like277.

## Пустой critical при уже действующем ранении — адаптация853fbb71

UpstreamEmptyCriticalApplication создаёт authored ordinary CUnit/mission.
В текущем release/NDEBUG коде CreateCritical(C_NONE) возвращает null.
ApplyCritical(C_NONE) без ранений корректно ничего не меняет.
Следующий valid control устанавливает настоящий CAPCritical: AP max55->28,
duration3. Затем повторное public ApplyCritical(C_NONE) приводит к native
SEGFAULT при Merge(null). Первый RED не сохранил buffered stdout; добавлен
только fflush для контрольного сообщения и RED повторён до production
изменения. В окончательном raw виден успешно установленный valid modifier
непосредственно перед падением. Это API-level native repro, не replay
конкретного пользовательского save; debug assertion policy factory не меняется.

После RED добавлен только ранний IsValid guard в ApplyCritical(CCritical*).
GREEN: два пустых применения при активном ранении сохраняют тот же объект,
duration/value, все skills/value/XP part/progress/max, общий XP и inventory.
Список типов, Merge rules, AP penalties и медицинские правила не заменены.

Соседняя часть upstream сбрасывает nBleedingStopAmount при новом bleeding.
В нашем форке этот private member сейчас только объявлен, инициализирован
и сериализуется (tag35); runtime consumers/producers отсутствуют.
Поэтому запись в неиспользуемое поле не переносилась; схема/старое значение
сейва сохранены. Возможный эффект после будущего medical restoration
описан в UPSTREAM-BALANCE-CHANGES.md. Все применимые части853 разобраны:
source imported в этой адаптации, без заявления о переносе bleeding policy.

Target3/3; связанные16/16 Windows (7.43 с) и15/15 Linux (6.52 с),
обе Game и Windows ALL_BUILD, по12 FONT-COMPAT/raw полные без NUL.
Первый Linux набор выбрал несобранный PortableCriticalTests: остальные14
прошли, этот тест Not Run. Target собран в прежнем build; повтор15/15.
NativeCriticalTests есть в Windows inventory, Linux использует portable
counterpart; отсюда разница количества. NetworkSerializer/StopResume2/2
(3.99 с). CPU base367; полный checkpoint4da43aaf base366+5=371/CI-like277.

Свидетельства empty-critical-application: unflushed-{build,test,native-test}.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, linux-unbuilt-{test,native-test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Статус281:114 pending/97 imported/21 partial/38 already/3 docs/5 excluded-balance,
по1 historical/review/excluded-own;136 исходных коммитов ещё разбираются.
All281/final actual Actions не завершены.

## Одинаковая target policy Snipe до входа в held aim — 1dc30007

UpstreamSnipeTileRestriction использует authored native rifle/Snap+Snipe,
настоящего бойца, native command factory и очередь подготовки.
Positive controls: Snap tile принимается, Snipe unit target принимается,
существующий CUnitStateSniping::CreateExecutor уже отвергает tile с
UCR_UNAVAILABLE. До входа в held state тот же mode всё ещё классифицируется
AT_SHOOT и ошибочно принимает tile как обычный выстрел.
Compiled RED: replacement-reload=false/result0/executor1 после трёх контролей.

Held-state control создаётся настоящим constructor, который устанавливает
saved aiming AP даже без установки состояния. Его side effect замечен до
production изменения: fixture восстанавливает исходный SaveAP через public
API и повторяет окончательный RED. Первый held-control raw сохранён.
Actor остаётся !IsSniping/AT_SHOOT; новое правило не имитируется private flags.

После RED добавлен только guard SM_Snipe в tile AT_SHOOT branch, до создания
очереди/reload replacement. GREEN: оба значения replacement-reload flag дают
UCR_UNAVAILABLE/result1 без executor, сохраняя AP/ammo/позицию/saved aim/
nonSniping и action counter. Нормальные Snap tile и Snipe unit controls
сохраняются, старые HeldAim/TargetlessProbe также проходят.
Формулы прицеливания/урона/стоимости, другие режимы, наши result ordinals и
reload policy не заменены. Проверяется command eligibility/queue boundary,
не реальный выстрел/пустой clip reload/все сценарные cutscenes.

Target4/4; связанные40/40 Windows (31.40 с), Linux (30.60 с),
обе Game и Windows ALL_BUILD/12 FONT-COMPAT/raw40 полные без NUL.
NetworkSerializer/ClientUI/StopResume3/3 (5.44 с). CPU base368;
полный checkpoint4da43aaf остаётся base366+5=371/CI-like277.
Свидетельства snipe-tile-restriction: held-control-{build,test,native-test}.log,
red-{build,test}.log, red-native-test.log, green-{build,test}.log,
green-native-test.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log. Единственный hunk источника перенесён.
Статус281:113 pending/98 imported/21 partial/38 already/3 docs/5 excluded-balance,
по1 historical/review/excluded-own.135 исходных коммитов ещё разбираются;
all281/final actual Actions не завершены.

## Отсутствующая запись глобальной карты — часть b91ea228

UpstreamGlobalMapMissingRecord расширяет прежний native ReturnDestination
fixture. Настоящий CICShowGlobal сначала открывает authored campaign map3.
Отдельный CGlobalGame с тем же scenario/party, bGlobalMapSet=true, но
отсутствующим DB id987654 передаётся реальным CICShowGlobal/CICContinueGlobal.
Fixture заранее подтверждает отсутствие record. Compiled native RED:
прежний ReturnDestination проходит, valid map/interface есть; первый
invalid overview приводит к SEGFAULT после flushed positive-control message.

После RED Initialize возвращает bool и проверяет map/info до создания UI.
Адаптация к нашим callers: временный CGlobalMap держится CObj; SetInterface/
PushInterface выполняются только при успешном Initialize. Недостаточно просто
возвратить false внутри initializer и затем опубликовать недостроенный объект.
Новые data fields, base classes, serialized tags, clocks и шаблоны не добавлены.

GREEN: обе ошибочные команды сохраняют именно прежний active interface,
не оставляют deferred interface command, не меняют ID/return destination
отдельной valid campaign и не исправляют bad ID выдуманной картой.
Затем прежний positive ContinueGlobal действительно открывает valid map и
сбрасывает chapter return flag как раньше. Это public command/real UI
construction boundary, не pixel screenshot и не проверка всех mod failures.
Флаг chapter у отдельной invalid campaign в этом тесте изначально false;
rollback этого флага на failed continue здесь не заявляется.

Target2/2; связанные21/21 Windows (16.61 с), Linux (15.84 с),
обе Game и Windows ALL_BUILD/12 FONT-COMPAT/raw21 полные без NUL.
NetworkSerializer/ClientUI/StopResume3/3 (5.36 с). CPU base369,
полный checkpoint4da43aaf base366+5=371/CI-like277.
Свидетельства global-map-missing-record: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log и network-{test,native-test}.log.
Sourceb91 partial: desktop ownership/drawing и optional globe ещё разбираются.
Статус281:112 pending/98 imported/22 partial/38 already/3 docs/5 excluded-balance,
по1 historical/review/excluded-own.135 исходных коммитов остаются в разборе;
all281/final actual Actions не завершены.

## Сохранение chapter return при отказе перехода — own adaptation b91ea228

Отдельный UpstreamFailedGlobalMapChapterReturn расширяет прежний missing-record
fixture: invalid campaign уже имеет chapter flag=true, id24680/position7,8.
Первый invalid overview сохраняет его; следующий failed ContinueGlobal
оставляет текущий interface, но очищает chapter flag — compiled native RED
(retained-chapter=0). Оба прежних теста проходят в том же RED наборе.

Перенесён только момент существующего bChapterMapSet=false: после успешного
Initialize, до SetInterface. Это дополнительная адаптация нашей continuation
логики к bool initialization failure из b91, не заявление, что такая строка
прямо присутствует в upstream diff. Нормальный успешный переход по-прежнему
очищает return flag. Наш глобальный UI использует chapter flag вместе с
show-mode для текущего chapter marker; continuation show-mode=false,
поэтому временно сохранённый flag при construction не включает этот marker.

GREEN: missing-record Overview/Continue сохраняют flag/id/position и прежний
interface; отдельно отсутствие bGlobalMapSet также сохраняет chapter/interface.
После отказов valid Continue всё ещё открывает реальную глобальную карту и
очищает valid chapter return. Это command/state boundary, не полный save
roundtrip или pixel comparison. Fields/schema/templates/маршруты/баланс не менялись.

Target3/3; связанные22/22 Windows (17.82 с), Linux (16.89 с),
обе Game и Windows ALL_BUILD/12 FONT-COMPAT/raw22 полные без NUL.
NetworkSerializer/ClientUI/StopResume3/3 (5.38 с). CPU base370;
полный checkpoint4da43aaf base366+5=371/CI-like277.
Свидетельства failed-global-map-chapter-return: red-{build,test}.log,
red-native-test.log, green-{build,test}.log, green-native-test.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log, network-{test,native-test}.log.
Sourceb91 остаётся partial (desktop/drawing/optional globe);135 исходных
коммитов ещё разбираются, all281/final actual Actions не завершены.

## Ветка выделения в legacy16-bit mode — db4db390 неприменим к нашему backend

Полностью прочитан исходный diff: меняется только ветвь 16-bit targets без
stencil. При обычном режиме source сохраняет прежний stencil prepass и
outline pass. В действующем Main/GfxBgfx.cpp Is16BitMode всегда false;
SetMode создаёт D24S8 depth/stencil. Windows CMake исключает Gfx.cpp,
подключает GfxBgfx.cpp; Linux делает то же самое.

Поэтому ни один изменённый путь этого коммита в нашей сборке не выбирается.
Он отмечен excluded-own-behavior. Legacy16-bit mode ради применения ветки
не добавлялся, код рендера не менялся. Полный371/Bgfx checkpoint относится
к нашему активному backend, не заявляется тестом отсутствующего режима.
Для документирования неприменимости новые искусственные тесты не нужны.

Статус281:111 pending/98 imported/22 partial/38 already/3 docs/5 excluded-balance,
2 excluded-own, по1 historical/review.134 исходных коммитов остаются в разборе.
Последний полный checkpoint4da43aaf:371/371 обе платформы/CI-like277.

## Освобождение CPU-кэша геометрии в low-RAM — 82d44558

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/82d44558cb94a3ae3356efc42efc4b0c657a39e9

LowRamCombinerRuntimeTests запускает наш настоящий SDL/bgfx backend, создаёт
CObjectInfo/CNonePart/CAutomaticCombiner и native CVBCombiner. В обычном режиме
реальная загруженная геометрия даёт контрольное изображение квадрата.
Compiled native RED до production изменений: контроль пройден, после переключения
gfx_low_ram=1 сохраняются capacity4 у xformedPositions и128 у gfxData (код203).
Первый ошибочный запуск с null material в fixture сохранён отдельно под
fixture-*: это не свидетельство бага движка; после исправления стенда RED повторён.

Recalc теперь вызывает DoRecalc, а затем освобождает оба CPU-массива через
swap с пустыми vector. Это выполняется после выхода всех buffer-lock объектов,
включая ранние return. Флаг bDroppedXForm заставляет следующий пересчёт заново
получить позиции даже без изменения mesh/provider. Флаги инициализированы.
FreeMemory идемпотентен и сохраняет GPU geometry; serialized tags не менялись.
Сохранены наши SGfxCacheTransformer/partial upload и S2Perf scopes.

Native GREEN проверяет LT_NONE/LT_POSITION × CT_STATIC/CT_DYNAMIC: три
последовательных low-RAM пересчёта освобождают capacity обоих массивов до0;
каждый полученный кадр128×128 точно совпадает с обычным режимом. Возврат в
обычный режим восстанавливает кэш без явного XFormPosition/изменения provider.
Дополнительно проверены повторный явный FreeMemory при сохранении GPU geometry,
его восстановление, пустая геометрия/ранний return и повторное появление mesh.
Границы проверки: это real bgfx upload/readback и два типа combiner; отдельного
pixel fixture для LT_TNL/LT_TNL_SELECTION/lightmap ветвей здесь нет. Их общая
обёртка освобождения проверена по коду; обычные регрессии рендера выполняются
отдельно. Баланс, HD pack и шрифты не изменялись.

Проверки после переноса: полный набор376/376 на Windows (323.38 с) и
Linux (338.51 с), включая новый native-display тест,14 native-audio и
12 FONT-COMPAT. Оба сырых LastTest содержат все376 записей, без NUL.
Переиспользованы прежние build directories; собраны обе Game и Windows
ALL_BUILD. Дополнительно11/11 renderer/geometry/network проверок на Windows
(9.17 с) и2/2 native GPU на Linux (0.43 с). CPU base остаётся370:
новая проверка геометрии — дополнительный GPU case, полный набор base370+6.
Свидетельства low-ram-combiner: fixture-{build,test,native-test}.log,
red-{build,test,native-test}.log, green-{build,test,native-test}.log,
all-build.log, full-test.log, {windows,linux}-native-test.log,
related-{test,native-test}.log, linux-renderer-{test,native-test}.log.
Linux full console log при отдельном GPU запуске заменён его2-test выводом;
полный Linux raw376 сохранён заранее в linux-native-test.log.

CI-like277/277 (23.90 с), raw277 полный без NUL. Статус281:
110 pending/99 imported/22 partial/38 already/3 docs/5 excluded-balance,
2 excluded-own, по1 historical/review.133 исходных коммита ещё в разборе;
итоговая проверка актуального HEAD в GitHub Actions остаётся обязательной.

## Туман поверх decal overlays — 73eae08e

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/73eae08e02a5c20611a629127c5b1be7fe7ca81c

UpstreamFogSurfaceEligibility использует настоящие фабрики девяти материалов,
CSceneFragments и AddFinalOps. RED получен на исходной реализации: шесть
проверок opaque/alpha-tested/self-illuminated solid для static/dynamic fog
проходят, затем настоящий decal overlay ошибочно получает RO_FOG_STATIC.
Полностью перенесён единственный hunk: самостоятельный fog pass добавляется
для solid материалов NORMAL/ALIEN, собственный AddOperations вызывается как
прежде. Никаких изменений RPG/AI visibility, погоды, формул и баланса нет.

GREEN:9 native материалов × RP_FASTEST/RP_GF3_CL × static/dynamic fog.
Проверены обычный и exact decal, прозрачность, occluder, alien fallback,
explosion decal; pass50, глубина/смешивание и указатель fog parameters
для solids сохраняются. Собственная RO_SOLID_COLOR операция настоящего
alien fallback сохраняется; native fragment filter по-прежнему исключает
все операции. Тест подтверждает ошибку формирования настоящих render commands.
Он не заявляется отдельным pixel/readback доказательством fogged decal;
GPU регрессии проверяют наш существующий native scene/renderer отдельно.

Проверки:22/22 Windows (9.85 с),19/19 Linux (6.81 с), включая5 native-display,
12 FONT-COMPAT и полные raw без NUL; обе Game и Windows ALL_BUILD.
NetworkSerializer/ClientUI/StopResume3/3 (8.12 с); CI-like278/278
(51.27 с), raw278 полный без NUL. Сохранены общие native medal/camera и
bgfx/selection/low-RAM проверки; на Windows также partial/index/specular.
Свидетельства fog-surface-eligibility: red-{build,test,native-test}.log,
green-{build,test,native-test}.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
network-{test,native-test}.log, ci-{test,native-test}.log.

CPU base371, последний полный checkpoint e356d2db — base370+6=376 обе
платформы; свежий полный377 здесь не заявляется. Статус281:
109 pending/100 imported/22 partial/38 already/3 docs/5 excluded-balance,
2 excluded-own, по1 historical/review.132 исходных коммита ещё разбираются.
Итоговая проверка актуального HEAD в GitHub Actions остаётся обязательной.

## Статический lookup для неподвижного тумана — ad57a2df

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/ad57a2df018f9eda3f33d3b79a0d595786889dea

StationaryFogRuntimeTests запускает наш native SDL/bgfx, настоящий CGScene.Draw,
mesh/material и perspective camera. Начальный quad заполняет цвет/глубину
той же плоскости; затем туман проходит через обычный scene render path.
Положительные контроли до фикса: static fog меняет4096 пикселей; движущийся
dynamic vapour отличается от статического минимум на100 пикселей.
Compiled native RED: при speed0/switchTime0/time0 неподвижный dynamic fog
отличается от static lookup на4096 пикселях (код217).

Перенесён единственный hunk: внутри Draw локальный FOG_DYNAMIC заменяется
на FOG_PERVERTEX при fVapourSpeed==0. Это исключает анимационную ветку с
возможным делением на нулевой интервал. Настройка игрока, SFogParams,
сериализация, качество движущегося тумана и игровые расчёты не менялись.
GREEN: полное совпадение128×128 пикселей со static reference при интервалах
0/2 и временах0/1/7; контроль движущегося пара остаётся отличным от static.
Fixture синтетический, с допустимым нулевым интервалом; это не запуск
конкретной authored zone/light4412 из комментария источника.

Проверки:26/26 Windows (15.44 с, включая3 network) и20/20 Linux (6.91 с),
по6 native-display и12 FONT-COMPAT; raw26/20 полные без NUL.
Собраны обе Game и Windows ALL_BUILD; сцена работает в нашем D3D11/Vulkan.
Свидетельства stationary-fog: red-{build,test,native-test}.log,
green-{build,test,native-test}.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log.
Новая проверка — extended GPU; CPU base371 не изменился. Последний полный
checkpoint e356d2db остаётся376/376 обе платформы, последний CI-like
2c74ae84 —278/278. Свежие full378/CI для этого изменения не заявляются.

Статус281:108 pending/101 imported/22 partial/38 already/3 docs/
5 excluded-balance/2 excluded-own, по1 historical/review.131 исходный
коммит ещё разбирается; all281/final actual HEAD Actions не завершены.

## Сохранённая render topology голов — 8997d1a7, с адаптацией старых save

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8997d1a7c369fd1a90eddda8b2792cf2896c62ab

Наш CHeadAnimator уже правильно выводил первые три индекса каждой smoothing
range. Патч не исправляет формулы мимики или положение вершин: он разделяет
сохранённый render triangle list и данные smoothing и кэширует первый при
создании mesh. До переноса отдельные сохранённые trueIndices/trueTris
игнорировались, хотя их теги8/9 уже присутствуют в SRawHeadMeshInfo.

UpstreamHeadRenderTopology использует настоящий authored head11, его исходные
LifeStudio streams, CFaceGenMeshHolder, CStructureSaver и наш CHeadAnimator.
Compiled native RED: legacy empty-tag save/load даёт818 вершин/1274 треугольника;
ненулевые теги другого допустимого порядка тех же треугольников правильно
сохраняются/загружаются, но renderer honors them=0. Нейтральные координаты
обоих случаев до фикса совпадают: проблема именно в чтении topology.
Файл с прежней сборкой/старое дерево исходников для воспроизведения не создавались.

Теперь CHeadMeshLoader строит отдельный triangle list один раз; native animator
копирует его в CObjectInfo. CFaceGenMeshHolder читает persisted list; реальный
CreateHeadInfo переносит его с base mesh в новую baked голову. Все source hunks
учтены. Адаптация к нашему форку: если ОБА списка пустые, при загрузке holder
строит их из smoothing ranges. Прямой upstream перенос без этого сделал бы
невидимыми старые наши головы, у которых эти поля всегда писались пустыми.
Сериализованные теги и LifeStudio streams не менялись; fallback заполняет только
runtime cache, не переписывает сохранённые исходные пустые поля.

Native GREEN сохраняет legacy geometry, использует persisted alternate ordering
и даёт прежние нейтральные координаты. Дополнительно проверен base-loader cache
и НАСТОЯЩИЙ наш FaceGen producer: authored template94 → CreateHeadInfo,
один baked stream/непустая topology → CHeadInfo save/load → CHeadAnimator с
тем же списком треугольников. Это CPU production pipeline собственного движка,
не вызов стороннего старого LifeStudio DLL; facial math/morph/sequence/scale
код не переносился. Dedicated pixel сравнения говорящего лица здесь нет.

Проверки:20/20 Windows (67.45 с),15/15 Linux (40.83 с), включая native data/
FaceGen/MMTree, downed head idle, medal scene и bgfx/low-RAM/stationary fog;
12 FONT-COMPAT и полные raw без NUL. Windows дополнительно3 network,
NativeHeadResourceTests (все134 heads/136 streams/56462 vertices, прежний
digest8A6A6F3A612C18D0) и NativeHeadSequenceRuntimeTests.
Собраны обе Game и Windows ALL_BUILD. Сборки переиспользованы, HD pack не
генерировался; все round-trip save bytes существуют только в памяти теста.
Свидетельства head-render-topology: red-{build,test,native-test}.log,
green-{build,test,native-test}.log, all-build.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log.
CPU base372; последний полный checkpoint e356d2db —376 обе платформы,
свежий полный379 не заявляется. Итоговый actual HEAD GitHub Actions ещё нужен.

CI-like278/278 (23.79 с), raw278 полный без NUL. Статус281:
107 pending/102 imported/22 partial/38 already/3 docs/5 excluded-balance,
2 excluded-own, по1 historical/review.130 исходных коммитов ещё разбираются;
все281/final actual HEAD Actions пока не завершены.

## Удаление освобождённых звуковых каналов — отдельная часть c8b908f2

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c8b908f27c4c8ecb595225715730fb58b32fe63b

UpstreamReleasedSoundChannelCleanup использует наши настоящие LoadSample2D,
PlaySound, headless PCM mixer и Update. Test-only extern существующей
hashChannel позволяет проверить удержание записей; production diagnostic API
не добавлен. Положительные контроли: обычный звук заканчивается через mixer
и удаляется, отдельный loop остаётся живым. Compiled native RED: после
освобождения временного loop и Update таблица содержит2 записи вместо1,
несмотря на инвалидированный scene handle; контрольный loop продолжает играть.

Исправлен только cleanup hunk: invalid owner и неиграющий channel проходят
через общую ветку StopSound + отложенное удаление записи из таблицы.
Native GREEN повторяет16 циклов create/release/update, каждый раз остаётся
ровно1 живая запись; финальное освобождение очищает таблицу до0. Удерживаемый
декодированный sample и независимый loop остаются живыми во всех циклах.
Первый неудачный fixture-* запуск ошибочно проверял boolean return PCM hook
как число кадров; он сохранён отдельно и не считается RED движка.
После исправления стенда RED повторён до изменения production.

Source остаётся partial: продолжение 2D-звука после loop при освобождении
scene owner — другой дефект, здесь не переносился. Для него требуется
адаптация к нашему NativeFmod: Sample_Free удаляет все каналы данного sample,
поэтому ending holder должен владеть decoded sample; прямой исходный
raw-pointer holder этого не гарантирует. Никаких новых FMOD exports, loop
policy или изменений исходного баланса этот cleanup-коммит не добавляет.

Проверки cleanup:21/21 Windows (21.36 с), включая ВСЕ15 Upstream native-audio,
3 network, registration/bind и rejected-caption cases. На Linux прошли
373/373 CPU cases (306.77 с), включая все15 native-audio. Этот расширенный
Linux набор был выбран из-за сбоя формирования узкого selector; это полный
CPU прогон, GPU extras в него не входили. Windows первоначальный пустой
selector дал No tests; сохранён как selection-*, затем повторён правильный
21-test набор. Оба финальных raw полные, без NUL; Linux и CI имеют по12
FONT-COMPAT. Собраны обе Game/Windows ALL_BUILD. CI-like279/279 (23.89 с),
raw279 полный без NUL. Новая проверка resource-free, помечена native-audio
и отключается при сборке без native mixer, как соседние.

Свидетельства released-sound-channel-cleanup: fixture-{build,test,native-test}.log,
red-{build,test,native-test}.log, green-{build,test,native-test}.log,
all-build.log, selection-{test,native-test}.log, related-test.log,
linux-{configure,build,test}.log, {windows,linux}-native-test.log,
ci-{test,native-test}.log. Сборки переиспользованы, старые копии не создавались.
CPU base373; последний full с GPU extras остаётся e356d2db376/376 обе платформы,
свежий полный380 здесь не заявляется.

Статус281:106 pending/102 imported/23 partial/38 already/3 docs/
5 excluded-balance/2 excluded-own, по1 historical/review.130 исходных
коммитов ещё в разборе: перенос одного hunk pending→partial не завершает source.
All281/final actual HEAD Actions остаются незавершёнными.

## Доигрывание 2D-звука после loop — завершающая часть c8b908f2

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c8b908f27c4c8ecb595225715730fb58b32fe63b

Отдельный UpstreamSound2DEnding генерирует короткий PCM с различными уровнями
loop/tail и прогоняет его через НАШ headless native mixer. Backend control
на исходном коде: manual LoopOFF при живом владельце доигрывает полный tail,
peak0.0863167→0.345267. Контроль нескольких scene owners сохраняет loop до
последнего владельца; отдельный loop того же sample продолжает играть.
Compiled native RED: после release последнего scene owner ending=0 при
independent=1 — канал немедленно оборван. Это отдельный дефект от утечки
таблицы, уже отправленной в6699e500.

Перенесена source логика handoff: CChannel stop-on-delete policy/valid-ID guard/
SetChannel, CSound2D ending token и временный channel holder после release.
Адаптация к нашему движку: holder ВЛАДЕЕТ CObj<CSample2D>, поскольку native
Sample_Free уничтожает его каналы. Token расположен ПОСЛЕ pSample, чтобы
сработать до освобождения sample и передать владение. Сохранён штатный
OBJECT_BASIC_METHODS lifecycle: специальный CSound2D destructor не добавлен.
Первая такая попытка дала compile error из-за автодеструктора макроса,
сохранённый в macro-build.log; это не runtime RED и не финальный вариант.

Дополнительная обязательная адаптация: Done помечает движок неинициализированным
ДО очистки owner map, а ending token проверяет этот флаг. Поэтому деструктор
не вставляет новый holder в очищаемую таблицу при shutdown. Actual native
backend control доказывает, что LoopOFF достигает полного конца sample:
его общие loop points на native backend не изменяются, соседние voices
сохраняют исходный диапазон. Новые Sample_GetLoopPoints/header/export hunks
из источника здесь не нужны. Для legacy FMOD используется уже известный
nStartSamples и существующий SetLoopPoints до sample end; отдельного
runtime/oracle прогона внешнего старого FMOD DLL здесь нет.
Остальные source hunks учтены: nChannel=-1 уже был в нашем constructor.

Native GREEN проверяет слышимый tail по PCM, last-owner semantics, соседний
loop того же sample/его диапазон, прежнюю немедленную остановку loop без ending.
При исчезновении ВСЕХ scene sample references временный holder удерживает
декодированный sample до конца tail, после cleanup sample инвалидирован и
таблица пуста. Отдельно проверены shutdown с owned loop и shutdown уже после
handoff: нет новых channels/retained sample/reinsert. Это native PCM/lifetime
проверки, без субъективного прослушивания и без изменения 3D/audio math.

Проверки:23/23 Windows (21.73 с, включая3 network),20/20 Linux (16.15 с);
на каждой платформе все16 native-audio,12 FONT-COMPAT, полные raw без NUL.
Обе Game и Windows ALL_BUILD; CI-like280/280 (27.69 с), raw280/12fonts полный
без NUL. Сборки переиспользованы, WAV и mix buffers только в памяти.
Свидетельства sound-2d-ending: red-{build,test,native-test}.log,
macro-build.log, green-{build,test,native-test}.log, all-build.log,
related-test.log, linux-{configure,build,test}.log, {windows,linux}-native-test.log,
ci-{test,native-test}.log.

c8b908f2 теперь imported с адаптацией, два независимых исправления — два
отдельных коммита с immediate push. CPU base374; последний Linux CPU checkpoint
до этого изменения —373, последний парный full с GPU — e356d2db376.
Свежий полный381 здесь не заявляется. Статус281:106 pending/103 imported/
22 partial/38 already/3 docs/5 excluded-balance/2 excluded-own,
по1 historical/review.129 исходных коммитов ещё разбираются; все281 и
итоговый actual HEAD GitHub Actions ещё не завершены.

## Сброс неактивной позы после принудительного перехода — часть71bf7730

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/71bf7730c33bf37b19beaab21042f6a39e5b3285

UpstreamForcedMoveItemEligibility создаёт реальный мир и обычного вооружённого
бойца. Native RED: обычный ForcedMove сохраняет CanActivateItem=true; публичный
Climb(false) действительно планирует authored JUMP_START clip (TimeEnd растёт)
и блокирует активацию на время неактивной позы. Затем ForcedMove в обычную позу
по-прежнему оставляет CanActivateItem=0. Приватный флаг в fixture не подменяется.

Перенесён ТОЛЬКО animator hunk: после DefaultAction и восстановления константы
времени bInactivePose синхронизируется с pose конечной команды. Native GREEN
проверяет STAND/CROUCH/LAY, каждый раз нормальная поза снова разрешает активацию.
AP, inventory active item, hand-active state, HasCommand/action и серверная
world position остаются прежними. Границы: проверен настоящий animation/eligibility
контракт, не поездка бойца по пути, natural-clock clip completion или pixels.
Мимика, выбор клетки, fall/height/distance heuristics и AP формулы не менялись.

Проверки:29/29 Windows (37.48 с, включая3 network),26/26 Linux (34.24 с).
Проверены родственные movement callbacks/terminal states/holster/hidden item/
явная и movement activation/corpse carry;12 FONT-COMPAT, raw29/26 полные без NUL.
Собраны обе Game и Windows ALL_BUILD. Свидетельства forced-move-item-eligibility:
red-{build,test,native-test}.log, green-{build,test,native-test}.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log. Сборки прежние, HD pack не генерировался.

71bf7730 остаётся partial: немедленная GridInfoUpdated у двери, AlignTime/
порядок SetPosition/обновление destination passability в server relocation
нуждаются в отдельных runtime RED. Heuristic изменения выбора клетки и Fall
вынесены для сверки оригинала в UPSTREAM-BALANCE-CHANGES.md.
CPU base375; последний CI-like — aef3b6be280/280, последний Linux CPU full —
6699e500373/373, последний парный full с GPU — e356d2db376/376.
Свежие full382/CI для этого изменения не заявляются.

Статус281:105 pending/103 imported/23 partial/38 already/3 docs/
5 excluded-balance/2 excluded-own, по1 historical/review.129 исходных
коммитов ещё разбираются; all281/final actual HEAD Actions не завершены.


## Верхняя граница очень длинной подсказки — часть a2bba0ad

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a2bba0ad86c91dcf5a239adda24792664abf1e43

Native RED использует настоящие CInterface/CToolTip/CText/CFrame, packaged
vector Courier16 и native formatter при dummy SDL viewport1024×768. Обычная
подсказка138×26 размещается корректно в886,741. Подсказка со100 строками
измеряется408×1808, но получает origin616,−1040: её заголовок уходит за экран.
Первоначальная ошибка наблюдателя glyph submission исправлена до окончательного
RED; fixture logs сохранены отдельно. Вид считает реальные glyph submissions,
но не рисует GPU textures: это проверка native layout, не pixel screenshot.

Адаптированы только две финальные Max/Min границы из upstream: в нашем Draw
origin не становится отрицательным, даже когда окно больше экрана. GREEN:
616,0; повторный Draw не сдвигает окно, возврат короткого текста сохраняет его
исходные размеры и позицию. Проверка IsVector подтверждает наши HiRes-шрифты.
Ширина400, font face/size, собственный immediate/sticky SetToolTipOwner pipeline
не менялись. Перенос upstream UpdateToolTip/anchors/width512 сюда не включён;
source остаётся partial. Это не изменение баланса.

Windows15/15 (16.86с), Linux12/12 (10.66с), включая две native display проверки
medal panel/chapter screenshot, родственные dialogue/edit/tooltip проверки и
Windows3 network tests. На каждой платформе12 FONT-COMPAT, raw15/12 полные без
NUL. Собраны обе Game и Windows ALL_BUILD. Свидетельства в прежней лаборатории:
oversized-tooltip-position-{red,green}-{build,test,native-test}.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log; fixture-* сохранены отдельно. Новых копий сборки
нет, HD pack не генерировался.

Статус281:104 pending/103 imported/24 partial/38 already/3 docs/
5 excluded-balance/2 excluded-own, по1 historical/review.129 исходных коммитов
ещё разбираются. CPU base376; свежий full383 не заявляется. Последний CI-like
остаётся aef3b6be280/280, парный full с GPU — e356d2db376/376, Linux CPU full —
6699e500373/373. Все281 и итоговый actual HEAD GitHub Actions ещё не завершены.


## Обработка очереди звуков собственной сцены меню — часть695465bc

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/695465bc189bfe93a657a3f48ff51bc664c5b7ef

UpstreamOwnedMenuSoundPump проверяет настоящий CInterface.Draw, CSoundScene,
Add2DSound и native PCM mixer. Положительный контроль: shared scene остаётся
необработанной после кадра её интерфейса, явный scene.Draw запускает channel,
S2FmodRenderForTest выдаёт ненулевой PCM. Затем независимый owned interface
ставит click в очередь, но Draw оставляет playing=0 — окончательный native RED.
Оснастка лишь останавливает frame на StartNewFrame, до GPU drawing; она не
подменяет звук. Первая partial-field подмена view сбрасывала ownership bool:
fixture logs сохранены отдельно, observer исправлен на полное сохранение полей
с перенаправлением только view reference, flag/scene проверены после восстановления.
Продукционный файл был возвращён к HEAD и окончательный RED повторён до фикса.

Перенесён owned-only pump после sCounter.Advance с identity CTransformStack.
Shared renderer scenes не получают второй pump. Собственные native audio,
асинхронный loader, сохранённые tags/clock и глобальная music continuity сохранены.
GREEN подтверждает реальный ненулевой PCM, неизменный channel/cursor на следующем
Draw, отсутствие дубликата и повторного запуска после естественного конца sample.
Синтетический PCM помещён в настоящий shared sample cache; это runtime queue/
playback proof, не отдельная end-to-end проверка задержки дискового async load.
Баланс, fonts, мимика и HD pipeline не менялись.

Windows ALL_BUILD и Linux Game собраны. FULL: Windows384/384336.03с,
Linux384/384318.38с (CPUbase377+7 display/renderer extras). Все17 native audio,
12 FONT-COMPAT на каждой платформе; raw384 полные без NUL. Дополнительно реальные
menu/options/backdrop/chapter tests: Windows15/1526.37с включая3network,
Linux12/1216.52с; raw полные без NUL. Полный raw сохранён ДО следующего CTest.
Свидетельства owned-menu-sound-pump: red/green-{build,test,native-test}.log,
fixture-{red,green}-{build,test,native-test}.log, all-build.log, full-test.log,
linux-{configure,build,test}.log, {windows,linux}-full-native-test.log,
display-network-test.log, windows-display-network-native-test.log,
linux-display-{test,native-test}.log. Новых копий сборок нет, HD не генерировался.

695465bc остаётся partial: отдельные hotseat player-switch banner и unit diplomacy
не включены в звуковой фикс. Статус281:103pending/103imported/25partial/38already/
3docs/5excluded-balance/2excluded-own, по1historical/review.129 исходных коммитов
ещё разбираются. Последний CI-like до этого фикса aef3b6be280/280, новый CI-like
не заявляется. Actual hosted run7bc3d00340449767003 наблюдался in_progress;
предыдущие b403/aef отменены новой отправкой по штатному cancel-in-progress.
Все281 и итоговый actual HEAD Actions ещё не завершены.


## Порог инстинкта выживания независимо от переносимости ранений — часть7368b4b1

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7368b4b10182ef243135d94b93bc481fd6dd593f

Сначала независимо сверён оригинальный Steam EXE
D:/SS-lab/steam-ui-oracle-01/game/Game.exe, SHA256
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705
(тот же проверенный binary, что для DropItems). В CVP routine0x694fb0 исходный
truncated процент сохраняется в stack local по695054 и копируется в EBX695058.
Perk5 добавляет25 только в EBX695069; perk8 сравнивает исходный stack local
695088 с Param1×100. Таким образом перенос исправляет реконструкцию, не retail.
Адреса upstream другой сборки не использовались как доказательство сами по себе;
G:/SS/Silent-Storm/bin/Game.exe — другой EXE, для этой сверки не использовался.
Дизассемблирование сохранено в perk-health-retail-disassembly.txt лаборатории,
оригинальный процесс не запускался и binary не менялся.

Native RED: авторский persona103, настоящее дерево с perks5/8, параметры8=.25/.1,
VP1/147. Public TakePerk формирует четыре комбинации; acquisition prerequisites
не проверяются. Настоящий IUnitMission.ApplyCritical(C_VP): без perks ICcap8,
с8 ICcap9, с5+8 ошибочно ICcap8. До окончательного RED исправлены две ошибки
оснастки: снятие wound modifiers восстанавливает caps, но не уже уменьшенный
live AP; survival modifier меняет IC cap, live effective value не обязан
автоматически пополняться. Fixture/fixture-live-ic logs сохранены отдельно,
production до окончательного RED не менялся.

Фикс сохраняет исходный процент в отдельной константе только для gate perk8.
Perk5 по-прежнему добавляет25 для прежних пяти wound penalties. GREEN проверяет
VP1/36/37/147 из147: ICcap9 ниже25%,8 на границе и выше; эффект5 не выключает8.
Все остальные skills/XP/progress точно равны5-only control, actual VP/inventory
не меняются, снятие critical возвращает все caps и training fields. Никаких
FreezeToMax flags fixture не подменяет: недостающая fresh-skill часть0ffb568d
остаётся отдельным кандидатом. Это native modifier-cap/cleanup proof, не тест
всей combat pipeline или claim об автоматическом изменении текущего effectiveIC.

Windows21/2118.68с (critical/perk wire, четыре training acknowledgements,
preview/target controls и3network), Linux17/1712.89с. Both Game и Windows
ALL_BUILD собраны; raw21/17 полные без NUL. Font-specific cases в этой узкой
группе не запускались; последний FONT-COMPAT checkpoint —1c98ce64 full384.
Свидетельства health-survival-threshold: red/green-{build,test,native-test}.log,
fixture-{build,test,native-test}.log, fixture-live-ic-{build,test,native-test}.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log. Новых копий сборок/HD generation нет.

Source7368 остаётся partial. Девять явно помеченных ORIGINAL RETAIL BUG FIXED
частей исключены, adaptation6/71 уже учтена поздним revert1bd13094; cover/moving
части нуждаются в независимых native proofs, хотя static oracle найден.
Статус281:102pending/103imported/26partial/38already/3docs/5excluded-balance/
2excluded-own, по1historical/review.129 исходных коммитов ещё разбираются.
CPUbase378; свежий full385 не заявляется. Последний полный парный384 —1c98ce64,
CI-like280 —aef3b6be; all281/final actual HEAD Actions ещё не завершены.


## Действующие значения новых навыков следуют модификаторам — завершение0ffb568d

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0ffb568d67bc6ca389d7ca63a9da4f3978c801a7

Оригинальный Steam EXE проверенного SHA256
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705
в InitSkills0x6bc7d0 перебирает17 cells. Смещения24/28 (=индексы9/10 VP/AP)
пропускаются6bc858/85d; для остальных Skill+2c=1 по6bc868 и Update по6bc86c.
Oracle fresh-skill-flags-retail-disassembly.txt сохранён от начала функции;
EXE только читался. Поэтому source flags восстанавливают исходную механику.

Compiled native RED UpstreamFreshSkillModifierValues: настоящий новый persona103,
public TakePerk8 и IUnitMission.ApplyCritical(C_VP) при20%VP. Native bonus
существует (ICcap9), но mission.GetIC остаётся8. Сначала проходят positive
контроли: увеличение cap VP/AP не пополняет уже потраченные значения. Production
до этого runtime RED не менялся.

Перенесён только9-line loop в конструкторе CUnit с persona:15 статических skills
FreezeToMax+Update, VP/AP остаются расходуемыми. Default load ctor, saved tags,
формулы навыков/XP/perks не менялись. GREEN: effectiveIC9,15 skills следуют
положительным и отрицательным modifiers и восстанавливают прежний effective cap
после снятия; XP/progress неизменны. Native saver сохраняет flags каждой cell.
Старый unfrozen cell value7/cap20 после roundtrip и bonus cap30 остаётся value7.
Это cell compatibility proof, не запуск произвольного старого пользовательского
save. Автомиграция старых сохранённых false flags не добавлена. Первоначальный
GREEN compile private serializer access исправлен вызовом публичного virtual
CObjectBase interface, private-serializer-build.log сохранён; это не runtime RED.

Глобальная инициализация влияет на все навыки, поэтому повторён FULL:
Windows386/386314.33с, Linux386/386317.64с (CPUbase379+7renderer extras).
Все17audio/12FONT-COMPAT на каждой платформе, raw386 полные без NUL. Обе Game
и Windows ALL_BUILD собраны. Дополнительно Win6/6wire/network5.41с (3network+
critical/perk wire), Linux2/2wire0.02с; raw сохранены отдельно. CI-like280/280
24.00с, raw280 полон без NUL/12FONT-COMPAT. Полные raw сохранены ДО следующего
CTest. Свидетельства fresh-skill-modifier-values: red/green-{build,test,native-test}.log,
private-serializer-build.log, all-build.log, full-test.log,
linux-{configure,build,test}.log, {windows,linux}-full-native-test.log,
wire-network-test.log, windows-wire-network-native-test.log,
linux-wire-{test,native-test}.log, ci-{test,native-test}.log, ci-tests.txt.
Сборки прежние, HD pack не генерировался, после проверок тестовые процессы завершились.

0ffb568d теперь imported: totalXP исправлен ранее независимо, fresh flags теперь
сверены с оригиналом и проверены отдельно. Статус281:102pending/104imported/
25partial/38already/3docs/5excluded-balance/2excluded-own, по1historical/review.
128 исходных коммитов ещё разбираются. Все281 и итоговый actual HEAD GitHub
Actions ещё не завершены; локальный CI-like не выдаётся за hosted CI.


## Исходный множитель обычного попадания в голову — частьdfb4e99b

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/dfb4e99b50118531a3f641f46ba22c88ae38314f

Независимый original Steam oracle (SHA256
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705):
switch table0x6c4658 прочитана из PE как6DWORDs
[6c4060,6c4042,6c404c,6c404c,6c4056,6c4056]. Это BODY0=1.0,
HEAD1=IEEE3fc00000=1.5 по6c4042, RH/LH=.7, RL/LL=.85. HL ordinals совпадают
с native aiPosition.h. Адрес source другой сборки6c4092 не принят на веру.
Файл head-damage-retail-lookahead.txt содержит дизассемблирование и проверку
jump table; оригинальный EXE не запускался и не менялся.

Native compiled RED с первого запуска: реальный persona103/нативная mission,
authored human armor и разрешённый damage type, fixed damage40/K100000,
critical0/attackerNULL/targetself/noKO−1. BODY40/RH28/LH28/RL34/LL34 проходят,
HEAD возвращает48 вместо исходных60. Native CalcStructDmg positive control
сам подтверждает base40. Nullworld — поддержанный contract без difficulty
multipliers; не claim о projectile collision или полном world damage dispatch.

Перенесена только одна строка множителя1.2→1.5 в RPGUnitMission.ProcessAttack.
GREEN head40→60,41→61 (прежнее truncation),1→1, остальные parts unchanged.
Возвращённый damage равен потере actual VP, target не погибает/не теряет сознание,
inventory тот же. Для каждого hit clone shared ISAAC подтверждает прежний один
KO draw и4 последующих слова, без лишних draws. Это игровая RPG механика,
собственная лицевая анимация не затрагивается. Повышение относительно текущей
реконструкции возвращает исходный баланс, а не меняет правило оригинала.

Windows23/2320.75с, Linux19/1914.80с: связанные critical/perk/training/preview
проверки, Windows3network. Both Game и Windows ALL_BUILD собраны, raw23/19 полные
без NUL. Font-specific tests в этом narrow profile не запускались; последний
full/fonts checkpoint49154b89:386/386,17audio/12fonts и CI-like280/280.
Свидетельства head-body-damage: red/green-{build,test,native-test}.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log. Новых копий сборок/HD generation нет.

dfb4e99b остаётся partial: level-based critical severity loop также найден
статически в исходном EXE6c40b0..6c40d1, но требует собственного native RED
и адаптации к нашему persisted shared/network RNG. Здесь RNG/difficulty/
critical selection/called-shot rules не менялись. Статус281:101pending/
104imported/26partial/38already/3docs/5excluded-balance/2excluded-own,
по1historical/review.128 исходных коммитов ещё разбираются. CPUbase380;
свежий full387/CI не заявляется. Все281 и final actual HEAD Actions не завершены.


## Броски тяжести критического попадания за уровень атакующего — завершениеdfb4e99b

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/dfb4e99b50118531a3f641f46ba22c88ae38314f

Независимый original Steam EXE того же проверенного SHA2564F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705:
6c40b5 вызывает GetSkillValue(ST_LEVEL13);6c40be/c0 передаёт20/1 в global RNG,
результат прибавляет EBP и повторяет цикл до уровня. Сумма EBP передаётся
по6c417a/4183 в actual critical calculator. Oracle сохранён отдельно в
attacker-level-critical-retail-disassembly.txt; старый EXE только читался.

Compiled native RED с первого запуска: actual persona103/real missions,
body armor, fixed damage1/self target/noKO−1. Контроли без attacker: probability0
сохраняет один KO draw, probability100 логирует severity7 и сохраняет прежние
RollCritical+KO shared draws. Затем реальный attackerLVL1 при probability0
не расходует обязательный level prefix, four-next-ISAAC words расходятся.

Перенесён только local difficulty и один Get(1,20) за уровень через наш
MissionRandom (Winrandom/Linuxs2_game_random). Generic SRand/global backend,
save/network RNG ownership не заменялись; входной portion не переписывается.
Нынешние probability/critical selection/extra RollCritical draw/formулы65558b11
остались прежними. Native GREEN LVL1/3/7 × probability0/100 проверяет точные
prefix draws, прежние последующие draws и4 следующих ISAAC слова. Existing
ILogNotify читает реальный severity argument из diagnostics critical calculator:
он равен7+sum(prefix), не просто ожидаемому расходу RNG. Public DisableCriticals
в fixture изолирует выбор травмы; это severity handoff/RNG proof, не отдельный
oracle итоговой травмы/critical damage. Blocked attack nK0 не расходует RNG,
не наносит damage и не обучает. Base body damage1/actualVP loss/attackerAP+XP/
inventory/inputportion сохраняются. Nullworld — supported без difficulty factor.

Старая vitality acknowledgment regression обновлена ровно для identified
attacker prefix из native GetSkillValue(ST_LEVEL), затем прежний KO draw.
Anonymous branch и полное сравнение обучения с прежним CUnit.UseSkill
сохранены, four-next-words assertions не ослаблены. Targeted GREEN5/54.83с.

Так как изменяется shared random stream при каждом identified hit, повторён
FULL: Windows388/388317.08с, Linux388/388322.14с (CPUbase381+7GPU extras),
17audio/12FONT-COMPAT на каждой платформе, raw388 полные без NUL. Обе Game и
Windows ALL_BUILD собраны. NetworkSerializer/ClientUI/ObjectStages/StopResume
4/46.90с. CI-like280/28023.85с, raw280/12FONT-COMPAT/noNUL. Полные raw сохранены
до следующего CTest, CI raw скопирован самим wrapper после завершения.
Свидетельства attacker-level-critical-rolls: red/green-{build,test,native-test}.log,
all-build.log, full-test.log, linux-{configure,build,test}.log,
{windows,linux}-full-native-test.log, network-test.log,
windows-network-native-test.log, ci-{test,native-test}.log, ci-tests.txt.
Сборки прежние, HD не генерировался, тестовые процессы после проверок завершились.

dfb4e99b теперь imported: head multiplier и level prefix перенесены отдельными
фиксами/коммитами с immediate push. Статус281:101pending/105imported/25partial/
38already/3docs/5excluded-balance/2excluded-own, по1historical/review.127 исходных
коммитов ещё разбираются. Последний observed hosted run3ae77adf40453581256 был
in_progress; он не выдается за success текущего fix. Все281 и final actual
HEAD GitHub Actions ещё не завершены. Соседние critical math/perk rebalance
hunks остаются отдельными, явные ORIGINAL RETAIL BUG FIXED изменения исключены.


## Авторский множитель perk движущейся цели — часть7368b4b1

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7368b4b10182ef243135d94b93bc481fd6dd593f

Повторно проверен SHA256 оригинального Steam EXE:
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705.
Actual GetTMove начинается6b7660; HasPerk14 получает Param1 output6b7798..a6,
после восстановления четырёх registers6b77b3/b7 умножает этот local на ранее
рассчитанный projected movement penalty. Косинус используется ранее при проекции,
а не как perk multiplier. Файл perk-movement-retail-disassembly.txt обновлён
от настоящего начала функции до return; original process не запускался.

UpstreamMovingTargetPerk: actual FixtureWorld, обычный party actor, настоящий
CUnitServer с authored persona130, его моделью/аниматором и реальной mission.
Perk14 берётся из настоящего дерева, Param1=2 из DB не меняется. Public
StartNewTurn задаёт turn-start CP для трёх shot directions, normal native
CUnitToHitCalcer использует настоящий SPosition. Unperked controls3.2 и
2.26274/2.26274 проходят; native compiled RED с первого запуска: после TakePerk14
при поперечном движении actual0 вместо6.4. Это fixture геометрии и калькулятора,
не выполненное перемещение по пути, расход AP, реальный выстрел или AI решение.

Исправлено только получение Param1 и его применение вместо direction cosine.
Проекция, cap-before-multiplier, first-round guard и native движение не менялись.
GREEN три направления6.4/4.52548/4.52548, stationary/firstRound0, прежний порядок
ограничения дальнего перемещения. Сам GetTMove сохраняет AP обоих actors,
position/inventory/XP. Public TakePerk создаёт состояние владения perk;
acquisition prerequisites не являются предметом этой проверки. Это исходное
retail поведение защитного perk, не изменение числовых параметров оригинала.

Windows26/2623.53с, Linux23/2318.44с: родственные accuracy/preview/aim/snipe/
skill checks, Windows3network. Both Game/Windows ALL_BUILD собраны; raw26/23
полные без NUL. Font-specific cases в узком профиле не запускались; предыдущий
full/font/audio/CI checkpoint — e8133062:388/388,17audio/12fonts и280/280.
Свидетельства moving-target-perk: red/green-{build,test,native-test}.log,
all-build.log, related-test.log, linux-{configure,build,test}.log,
{windows,linux}-native-test.log. Прежние сборки используются повторно, HD pack
не генерировался. Собственные facial/SDL/bgfx/network engines не менялись.

7368 остаётся partial: Cover72 последний отдельный reconstruction hunk;
девять явных ORIGINAL RETAIL BUG FIXED изменений исключены, adaptation6/71
уже учтена final revert1bd13094. Статус281 прежний:101pending/105imported/
25partial/38already/3docs/5excluded-balance/2excluded-own, по1historical/review.
127 исходных коммитов ещё разбираются. CPUbase382; свежий full389/CI здесь
не заявляется. Все281 и final actual HEAD GitHub Actions ещё не завершены.


## Авторский множитель покрытия без потери площади цели — завершение7368b4b1

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7368b4b10182ef243135d94b93bc481fd6dd593f

Original Steam EXE SHA2564F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705:
GetCA6b6fd0 сохраняет virtual GetTArea, HasPerk72 пишет Param1 local6b6ff1..fc,
6b7003/07 умножает его на coverage; area остаётся отдельным множителем6b702e.
Oracle perk-cover-retail-disassembly.txt записан от настоящего начала функции.
Параметры оригинального executable не выводились из текста описания perk.

Native probe собран ДО production и с первого запуска дал настоящий RED:
authored perk72Param1=2, persona106 с настоящим деревом и CUnitServer/mission
в реальном FixtureWorld. Unperked шесть контрольных комбинаций tile/unit ×
distance5/20/60 подтверждают ненулевую площадь и линейный вклад coverage.
После public TakePerk72: distance5/tile/cover25 actual.961905 вместо.968190.
Значение2 взято из DB без её изменений; гипотеза об эквивалентности при Param0
не подтвердилась на текущих авторских данных. Probe logs — именно runtime RED,
не fixture/compile ошибка.

Исправлено только чтение multiplier и fTCover*=Param1 вместо fTArea=0.
Сохранены virtual area, native Prepare/equipment/scope/distance, no-cover perk17
и called-shot ветви. Unit fixture использует реального target без17 и HL_ANY,
чтобы изолировать эту часть. GREEN18 combinations двух типов цели, трёх
дистанций иcoverage0/25/100. Expected factor получен из native unperked floor
и area contribution с исходным multiplier; DistanceFunc/весь шанс попадания
в тест не скопированы. AP/XP/inventory/placement при GetCA неизменны. Native
GetCA coverage0 floor не объявляется равным GetToHit coverage0: его прежний
entry guard сохраняется. Это calcer geometry proof, не выполненный выстрел,
collision/путь или AI decision; acquisition prerequisites не тестируются.

Общий GetCA используется разными атаками, поэтому выполнен FULL:
Windows390/390319.51с, Linux390/390318.99с (CPUbase383+7GPU extras),17audio/
12FONT-COMPAT на каждой платформе. Raw390 полные без NUL, сохранены до следующих
CTest. Both Game/Windows ALL_BUILD собраны. NetworkSerializer/ClientUI/
ObjectStages/StopResume4/47.02с; CI-like280/28023.88с, raw280/fonts12/noNUL.
Свидетельства cover-perk-multiplier: probe-{build,test,native-test}.log,
green-{build,test,native-test}.log, all-build.log, full-test.log,
linux-{configure,build,test}.log, {windows,linux}-full-native-test.log,
network-test.log, windows-network-native-test.log, ci-{test,native-test}.log,
ci-tests.txt. Новых копий сборок нет, HD pack не генерировался, тесты завершились.

Повторно сопоставлены все hunks7368 и final1bd13094. Три reconstruction fixes
перенесены по отдельности: health3f240042, moving1ca11284, cover этот фикс.
Девять явно помеченных ORIGINAL RETAIL BUG FIXED правил исключены и подробно
описаны в UPSTREAM-BALANCE-CHANGES.md. Исторический блок6/71 уже заменён final
retail revert1bd: own UseWeapon использует DB multiplier×counter, rate/cap
perks остаются прежними. Source7368 теперь imported с этим разделением,
а не blanket cherry-pick. Собственные facial/HD/SDL/bgfx/network engines и
форматы сохранений не заменялись.

Статус281:101pending/106imported/24partial/38already/3docs/5excluded-balance/
2excluded-own, по1historical/review.126 исходных коммитов ещё разбираются.
Все281 и final actual HEAD GitHub Actions ещё не завершены; локальный CI
не выдаётся за hosted run.


## Идентичность игроков в дипломатии юнита — часть695465bc

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/695465bc189bfe93a657a3f48ff51bc664c5b7ef

Независимый original Steam oracle SHA256
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705:
реальные wrappers CUnitServer7c0160/7c01b0 вызывают World slots b8/c0.
Factory778490 -> ctor76a950 -> final vtable8cbc8c:
b8=7627a0 (unit/unit), c0=7627f0 (unit/player). Последний сначала сравнивает
player pointers (same=ALLY2), затем scenario IDs (distinct/sameID=ENEMY0),
иначе вызывает b0=761f20: negativeID=NEUTRAL1, GetRPG/cast IUnitMission,
индивидуальная SDiplomacy. Gold PDB использован только для поиска opcode
signature; поведение проверено в Steam, не перенесено между версиями.
Адрес7bfaf0 из исходного комментария не соответствует этому Steam EXE.
Оригинальные executable не запускались, не изменялись и не копировались.
Read-only proof unit-diplomacy-retail-{wrappers,world,mask,world-ctor}.txt
и unit-diplomacy-retail-identity-notes.txt.

Сначала compiled native RED: FixtureWorld, два действительно созданных
CGlobalPlayer/AddPlayer/commander и развёрнутых CUnitServer. Оба human
scenarioID0; self-alliance и три individual NPC overrides ENEMY/NEUTRAL/ALLY
работают до RED. При запросе первого юнита ко второму получено ALLY2 вместо
World/original ENEMY0. Это не подставленные private player flags/masks:
проверка индивидуальной маски использует публичный SetDiplomacy и её
восстановление. RED1.21с, после минимального исправления GREEN1.15с.

CUnitServer теперь использует существующий own World overload (unit,player)
и отклоняет отсутствующую цель. Overload (player,player) здесь намеренно
не применяется: он потерял бы индивидуальную дипломатию. GREEN оба направления,
self, null, прежние overrides; запросы не меняют AP/VP/placement/player/mask.
Собственные LAN IDs/командиры/сохранения не заменялись. Это доказательство
native diplomacy queries, не сыгранный полный hotseat матч/AI attack decision.

FULL Windows391/391323.29с, Linux391/391328.61с, CPUbase384+7GPU extras,
17native-audio и12FONT-COMPAT на каждой платформе. Both Game и Win ALL_BUILD.
4Network Serializer/ClientUI/ObjectStages/StopResume6.86с. CI-like280/28024.04с,
raw280/font12. Все raw полные, без NUL, сохранены до следующего CTest.
Evidence unit-diplomacy-identity: red/green/full/network/ci build/test logs,
windows/linux-full-native-test.log, windows-network-native-test.log,
ci-native-test.log; редактировались прежние build directories, HD OFF.

Source695 остаётся partial: hotseat banner отдельный пункт. Его правильная
разметка зависит от отсутствующего в нашем Update вызова Show из46b3ace0;
перенос только constructor не докажет рабочую передачу хода. Все281 и final
actual HEAD GitHub Actions ещё не завершены. Счётчики прежние101pending,
106imported,24partial,38already,3docs,5excluded-balance,2excluded-own,
по1historical/review;126 исходных коммитов остаются в разборе.


## Проверка condition voice preview —46b3ace0, без переноса

Native probe подтвердил, что GetPersVoiceAck на настоящей persona54 и hero
с тремя voice donors уже находит звук и на текущей baseline DB, и на
проверенной original Steam game.db. Печать показывает pCondition ID102 и
совпадающий legacy nConditionID102. Original Steam645170/645279 действительно
читает ссылку pCondition; это независимо найдено через Gold PDB signature.
Но на поддерживаемых авторских данных изменение не дало функционального
различия. RED получен только после искусственной записи numeric=-1, поэтому
не признан доказательством нужного переноса. Production не менялась, probe
убран из репозитория; небольшие compile/runtime evidence voice-preview-condition-*
сохранены. Первые ошибки include/API в probe исправлены до запуска.
Контроль без mutation прошёл на обеих базах; вывод о полной эквивалентности
для произвольных модифицированных DB или всех прочих hunks46 не делается.
Реконструированный комментарий о якобы отсутствующем Steam tag5 не принимается
как факт вопреки runtime данным. Собственный facial engine не менялся.


## Занятое исправное орудие остаётся доступным своему AI стрелку — завершение75341d49

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/75341d49319b927a76537c6021e6e655d635de95
Реализация и native тест: bb776ca10928fa1adbf1fca9ad651fde8b93c1d3.

Независимый verified Steam SHA2564F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705:
IsWorkingCannon452420 вызывает ICannon IsBroken и weapon HasAmmo, без occupancy.
Dock GetInfoInner452c60 отдельно проверяет ICannon IsOccupied452dc5 и пропускает
занятый gun452dca. Gold PDB адреса4518f0/452130 использовались только для поиска
уникальных exact/masked opcode signatures. Адреса из старого комментария
source для этого Steam binary не подходят. Proof mounted-gun-{working,dock,
shoot}-retail.txt регенерирован от настоящих начал функций; original EXE
не запускался/не менялся. Это восстановление правил оригинала.

Первый compiled native RED1.22с: настоящая карта810, её единственное орудие
с100ammo, два настоящих AI units. Подобраны реальные cannon approach/path,
допустимая точка цели и native AI-map body aim/CanAttackWithCannon. Уже первый
candidate attackable. Контроль незанятого gun разрешает shooting/docking и
не требует abandon. Затем public SetCurrentUnit устанавливает занятость;
прежний helper запрещает своему gunner стрельбу (fire0 вместо1).

Минимальная правка: убрать IsOccupied только из working helper и перенести
в docking scan. Не менялись дальности10/5/3м, cone, path/AP, threat filtering,
ammo, firing parameters или порядок команды. GREEN1.13с: occupied gun fire/stay,
запрет docking другого бойца, empty ammo firefalse/leave true; eligibility
queries не тратят AP/ammo и не двигают actor. Public IAIUnit.SetCannon задаёт
анализируемое AI состояние; SetCurrentUnit реальное состояние занятости.
Тест не объявляется проигранной docking animation или выполненным выстрелом.

Both Game/Windows ALL_BUILD. Related23/23 Windows10.76с/Linux10.32с включают
mounted commands/UI, AI/threat/contact/diplomacy/registration и12FONT-COMPAT.
4Network Serializer/ClientUI/ObjectStages/StopResume7.02с. CI-like280/28034.98с,
raw280/fonts12. Все raw полные без NUL, сохранены до последующего CTest.
Evidence mounted-gun-eligibility-{red,green,related,network,ci} build/test,
windows/linux-related-native-test.log, windows-network-native-test.log,
ci-native-test.log. CPUbase385; нового FULL392 здесь не заявляем, последний
полный391/391 на обеих платформах относится к2c44f24f. Прежние build folders
используются повторно, HD не генерировались.

Все hunks753 учтены: command cross-casts были перенесены ранее, occupancy/firing
этим отдельным TDD fix. Source теперь imported. Счётчики281:101pending,
107imported,23partial,38already,3docs,5excluded-balance,2excluded-own,
по1historical/review;125 исходных коммитов ещё разбираются. All281/final actual
HEAD GitHub Actions ещё не завершены. Проверка voice46 выше не считается
переносом: изменения production для неё не сделано. Журнал обновлён отдельным
документационным коммитом: при подготовке bb776ca1 проверка индекса столбца
tracker остановила только обновление метаданных; production и тест уже прошли
нужные проверки и были отправлены. Переписывания опубликованного коммита нет.


## Отдельный шаблон окна передачи хода — завершение695465bc

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/695465bc189bfe93a657a3f48ff51bc664c5b7ef

Первый compiled native RED1.52с: NativeDisplayContext, настоящая CICBeginMission810,
два реально развёрнутых игрока54/103 и их IPlayerTracker. Из настоящей CMissionUI
через штатный сохранённый tag25 прочитан созданный CPlayerSwitchUI. Наблюдение:
root0, size0x0, хотя text/ok dummy существуют. Причина: GetControl(playerswitch)
в HUD template не создаёт самостоятельную нужную разметку. Предварительная
ошибка probe (у нашего tracker отсутствует upstream IsAIPlayer) исправлена
на собственный GetGlobalPlayer()->bAIPlayer до runtime RED; лог сохранён.

Меняется только создание: отдельный initially hidden enabled/topmost root
на CInterface и LoadTemplate авторского UIContainer453. Native GREEN1.44с:
root1, text/ok настоящие, size1024x768, начальный hidden/enabled/topmost.
Далее public Show на обоих игроках при исходной pausefalse/true показывает
локализованное имя активного игрока и ставит паузу; EVENT_NOTIFY ok скрывает
окно и восстанавливает именно прежнюю паузу. Это проверка созданного окна,
реальной разметки и существующих API, а не pixel screenshot или доказательство
автоматического handoff. Последний требует отдельного отсутствующего Update
trigger из46b3ace0 и будет проверяться отдельным RED. Игровая механика,
собственные high-res fonts/adaptive layout/LAN и schema tag25 не менялись.

Both Game/Windows ALL_BUILD; related11/11 Windows10.66с/Linux8.80с: шесть
native display cases (этот dialog, MissionSaveMenu/QuickSave/LoadingInput,
MedalPanelCaption/PreviewCamera), дипломатию/регистрацию/12FONT-COMPAT.
4Network Serializer/ClientUI/ObjectStages/StopResume7.29с. CI-like280/28024.06с,
raw280/fonts12. Все raw полные/noNUL, сохранены до последующего CTest.
Evidence hotseat-dialog-template-{red,green,related,network,ci} build/test,
windows/linux-related-native-test.log, windows-network-native-test.log,
ci-native-test.log. Новый тест NativeHotseatDialogTemplate помечен
extended/native-display и использует собственный isolated user directory.
CPUbase385 не изменился; нового полного прогона здесь не заявляем.
Прежние build folders использованы повторно, HD OFF, тестовые процессы закрыты.

Все695hunks учтены тремя отдельными fixes: owned-menu1c98ce64, unit-diplomacy2c44f24f,
этот template. Source теперь imported. Счётчики281:101pending/108imported/
22partial/38already/3docs/5excluded-balance/2excluded-own, по1historical/review;
124 исходных коммита ещё в разборе. All281/final actual HEAD GitHub Actions
ещё не завершены. Автоматический показ относится к46, не приписывается695.


## Сохранённое старое окно передачи хода — адаптация695465bc к нашему форку

Источник layout logic: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/695465bc189bfe93a657a3f48ff51bc664c5b7ef

Отдельный compiled native RED1.59с доказал необходимость дополнительной
совместимости: предыдущая форма окна (zero-size HUD child) воспроизведена
публичными window APIs в настоящей двухсторонней миссии, затем вся CMission
записана с компрессией и восстановлена из памяти. После восстановления
GetDesktop/tag25 действительно дают старое окно0x0; Show ставит паузу, но
root остаётся0 и разметка не появляется. Это полный native save graph,
без частичной подмены сериализованных полей и без копии старой сборки.

Show теперь восстанавливает старую/отсутствующую разметку после разрешения
всех ссылок: тот же CPlayerSwitchUI переносится на свой CInterface root,
загружается template453, включаются enabled/topmost. Во время RemoveChild/
AddChild объект держит дополнительный CMObj: обычный CObj не предотвращает
инвалидацию при обнулении отдельного mission ownership domain. Никаких новых
сохранённых полей или изменения pause policy; загрузка не запускает Show сама.
Один include первоначально не объявлял GetUIContainer; исправлен на DataFormat
до GREEN, исходный compile log сохранён.

Native GREEN original+restored2/2: оба игрока при prior pausefalse/true,
localized name/пауза/OKrestore, после повторных показов ровно один root dialog.
Both Game/Windows ALL_BUILD. Related Windows12/1212.17с, Linux12/129.91с,
семь native display cases и12FONT-COMPAT;4Network7.04с;CI-like280/28023.82с.
Raw все полные/noNUL;CI raw280/fonts12. Evidence hotseat-dialog-legacy-*.
HD/facial/fonts/баланс не менялись. CPUbase385 и счётчики281 не изменились;
последний полный391 обеих платформ относится к2c44f24f, новый full не заявлен.
Automatic Show из46b3ace0 остаётся отдельным будущим RED. Все281/final actual
HEAD Actions ещё не завершены.


## Автоматический показ окна передачи хода — часть46b3ace0

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/46b3ace0a0354ef6b524216932de59b41d86c65b

Сначала compiled native RED2/2: настоящий HUD.Update в CICBeginMission с двумя
human trackers не показывает окно (visible0/paused0). Свежий вариант уже имеет
правильную разметку root1/1024x768; второй восстанавливает полный legacy graph
с прежним0x0 HUD child и тоже не вызывает Show. Фикс695 constructor/de7 legacy
проверен отдельно, поэтому здесь подтверждено отсутствие именно Update trigger.

Перенос адаптирован к существующим API: GetPlayers.size>1, валидный active/
GetGlobalPlayer и !bAIPlayer, изменившийся сохранённый pLastActivePlayer(tag26).
Затем Show и запись latch. У нас отсутствуют upstream IsSetupMode/IsAIPlayer;
новые virtual APIs не вводятся. Старый собственный setup-интерфейс не создаёт
CMissionUI, а LAN Initialize создаёт ровно один локальный tracker. Это проверка
кода LAN-граничного условия; native тест действительно проверяет однопользовательский
HUD с одним tracker, четыре общих сетевых регрессии не выдаются за end-to-end
проверку popup в реальном LAN-клиенте. Данные bAI соответствуют существующей
UI readiness policy; сам AI commander/алгоритм передачи ходов этим тестом не
восстанавливается. Другие части46 разбираются отдельно.

GREEN4/46.06с: свежая/полностью восстановленная старая миссии, реальный HUD.Update
вызывает named pause dialog, EVENT_NOTIFY ok закрывает/сохраняет prior pause,
тот же active не открывает снова; четыре чередующихся human смены с pausefalse/true.
UI AI flag пропускается без подтверждения latch, возврат к human вызывает Show.
Native запись tag26 содержит active; полный acknowledged mission graph записан/
восстановлен и следующий HUD.Update не открывает/не ставит паузу. Далее настоящая
новая одиночная миссия с одним игроком проходит три UI updates без handoff.
Legacy auto case не вызывает ручной Show перед первым Update: новый trigger
действительно задействует de7 repair. Это проверка обновления UI, а не input
through physical keyboard; клавиатурный путь будет проверен отдельно.

FULL Windows396/396336.44с, Linux396/396329.23с = CPUbase385+7GPU extras+
четыре NativeHotseat cases. На каждой платформе17native-audio/12FONT-COMPAT;
raw396 полные/noNUL и сохранены перед последующим CTest. Both Game/Win ALL_BUILD.
4Network Serializer/ClientUI/ObjectStages/StopResume6.94с;CI-like280/28023.97с,
raw280/fonts12/noNUL. Evidence hotseat-handoff-{red,green,green-save,full,network,
ci} build/test logs, windows/linux-full-native-test.log, windows-network-native-
test.log, ci-native-test.log. Прежние build folders/HD OFF, свои facial/fonts/
SDL/bgfx/LAN системы сохранены.

46 теперь partially-imported: voice condition ранее проверен без функционального
переноса, recruitment facial-idle/group-route/AI merc initialization ещё в разборе.
Счётчики281:100pending/108imported/23partial/38already/3docs/5excluded-balance/
2excluded-own, по1historical/review;124 исходных коммита ещё не закрыты.
All281/final actual HEAD GitHub Actions ещё не завершены.


## Фокус окна передачи хода — адаптация46b3ace0 к нашему вводу

Источник включения окна: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/46b3ace0a0354ef6b524216932de59b41d86c65b

Отдельный первый compiled native RED2/2 (3.00с) подтвердил интеграционный дефект:
свежее и восстановленное legacy окно уже visible1/paused1/root1, но focused0.
Контроль GetKeyForMessage успешно переводит CT_KEY ENTER в VK_RETURN; настоящий
CInterface.ProcessEvent возвращает consumed0 и оставляет окно видимым. До этого
тесты проверяли обработчик OK напрямую, поэтому клавиатурный путь выделен отдельно.

Show использует существующий ShowWindow(SWTYPE_SHOW), чтобы отправить штатный
запрос активации родителю. Константа key event теперь ссылается на EVENT_CHAR,
её числовое значение0x02000022 сохранено; прежний комментарий про base0x20 не
соответствовал нашему UIEvents.h. Native GREEN6/69.27с: обычный и цифровой Enter
чередуются при шести подтверждениях каждого fresh/legacy сценария; focused1,
consumed1, окно закрывается, исходная пауза сохраняется. A потребляется без
закрытия; repeat suppression/AI gate/одиночная миссия/полный acknowledged save
контроли предыдущего теста сохранены. CT_KEY сообщения переданы через реальный
input API/mapper/CInterface; OS keyboard или SDL event queue здесь не эмулируются.

Original Steam Show независимо найден через Gold PDB opcode signature:
62e3c0 вызывает SetStyle7278f0 с visible/true, helper только меняет биты.
Read-only proof hotseat-show-retail-disassembly.txt и -style.txt. Изменение
активации — явная адаптация включённого upstream окна к фокусу нашего UI,
не заявляется идентичным телом Show оригинала. Боевые правила/AP/RNG/баланс
не меняются; собственные SDL key contracts и facial/fonts сохранены.

Both Game/Windows ALL_BUILD; related20/20 Windows19.07с/Linux15.25с,
11native-display cases, SDLPlatformInputTests, edit-keyboard/escape/cursor,
дипломатия/регистрация/12FONT-COMPAT.4Network7.16с;CI-like280/28023.90с.
Raw полные/noNUL, CI raw280/fonts12; evidence hotseat-keyboard-*.
CPUbase385, новых двух NativeHotseatKeyboard cases нет в прежнем full396;
полный398 здесь не заявлен. Последний полный обеих платформ —0690176e396/396.
Все281/final actual HEAD Actions ещё не завершены; счётчики источников прежние.


## Подсказка юнита в принудительной атаке — часть4430374b

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4430374be9ea6be47346d06be41b68005dd7e835

Native воспроизведение использует настоящую CICBeginMission с явным authored
variant810 и его родительским template, выбранного бойца54, штатные GivePlayerTurn,
player.Update/Select/RefreshPointerTarget и paused mission.Step. Проверены три
настоящих юнита, screen320x240 и TestRayInFrustrum для цели. Обычный не-свой
hover создаёт рамку; затем принудительный CStateAttack.Initialize даёт tag7=null
(RED1.63с), хотя general action eligibility и камера уже подтверждены.

Первые fixture assumptions были исправлены до production: после загрузки cache
UA_ATTACK ещё содержал empty-selection result; штатный RefreshPointerTarget
его обновил. Рандомный template810 имел только одного своего юнита, поэтому для
контроля чужого hover задан конкретный variant810. Проекция собственного юнита
была корректной; прежнее имя camera-cache log обозначает предварительную гипотезу,
не подтверждённую причину. После уточнения parent template RED повторён. Все
fixture/runtime logs forced-attack-tooltip-* сохранены; эти первые отказы не
выдаются за воспроизведение бага production.

Изменено только создание caption frame: для обоих режимов, initially hidden,
затем существующий UpdateEnemyStateInfo. Собственный порядок trace/info/cursor
сохранён: UpdateCursor уже повторно обновляет info/caption при смене hit location,
поэтому перестановка upstream не копировалась. GREEN1.65с: реальное имя из
GetEnemyUnitInfo текущего игрока, visible caption над проецируемым юнитом,
правильный client parent, Terminate инвалидирует рамку, новый forced state без
unit target оставляет её hidden. AP/VP/position бойца неизменны. Это native
контроллер/геометрия/caption proof; выполненный выстрел или пиксельная картинка
не утверждаются. Расчёт попадания, fog policy и свои шрифты не менялись.

Both Game/Windows ALL_BUILD; related56/56 Windows52.12с/Linux48.07с включают
attack/hit/target/cursor/reload/acknowledgment cases,10native-display/12FONT-COMPAT.
4Network7.00с;CI-like280/28024.01с. Raw полные/noNUL,CI raw280/fonts12.
Evidence forced-attack-tooltip-{red,green,related,network,ci} и native logs.
CPUbase385 неизменён (новый Native extended case), свежего FULL399 не заявляем;
последний полный0690176e396 обеих платформ. HD OFF/старые build folders повторно.

443 теперь partial: body/emptyPK hover остаётся отдельным пунктом. Три первых
hunks только уточняют комментарии; MakeUnitStateToolTip уже не имел death guard.
При проверке зависимости найден неверный старый PRESENT у5394f920: в текущем
newrepo GetEnemyUnitInfo по-прежнему отбрасывает мёртвых юнитов. Этот source
переоткрыт в pending, без изменения production; нужны original oracle и native
TDD. Остальные PRESENT также требуют сверки с текущим форком до завершения.
Счётчики281 теперь100pending/108imported/24partial/37already/3docs/5excluded-balance/
2excluded-own, по1historical/review.125 незакрытых source entries по tracker;
число выросло из-за исправленного статуса, а не новой upstream истории.
All281/final actual HEAD Actions ещё не завершены.

## Информация о погибшем бойце —5394f920

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5394f92094e7a3e3ad44c5acb66c5bf2230004eb

Старая отметка PRESENT оказалась неверной для текущего репозитория:
CPlayer::GetEnemyUnitInfo прекращал обработку при IsDead. Первый собранный
native RED: у живой чужой цели имя длиной16 и нормальные сведения; после
RPG.Kill и перехода сервера в CUnitStateDeath имя0, unitInfo0, maxHP0.
Это настоящий серверный/RPG снимок смерти, без заявления о проверке всей
анимации смерти и всех связанных событий.

Исправление удаляет только проверку смерти у валидной цели. Правила видимости
точных HP не меняются: чужой труп по-прежнему показывает состояние вместо
закрытых чисел, свой сохраняет полную информацию. GREEN также проверяет
имя/maxHP/категорию смерти, очистку предыдущей информации для null и объекта,
не являющегося юнитом, отсутствие расходов AP/VP и перемещения наблюдателя.

Независимая статическая сверка оригинального Steam Game.exe с SHA256
`4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705`:
GetEnemyUnitInfo находится по7876a0, а не по адресу787108 из комментария
upstream. Gold PDB использован только как указатель для уникальной сигнатуры.
В самом Steam EXE после dynamic_cast есть null/invalid guards, затем проверка
собственного юнита, получение имени и состояния. Call через vtable44 по7877be
проверяет CanFight члена отряда наблюдателя для перков, а не смерть цели.
EXE не запускался и не изменялся. Сохранена ограниченная дизассемблировка.

Проверки: Windows ALL_BUILD/Game и400/400337.56с; Linux Game и399/400331.87с.
Единственный Linux сбой — ранее существующий ChapterClueHeading с ошибкой
границы рисования; это НЕ полностью зелёный Linux прогон. Все400 записей и
12FONT-COMPAT сохранены без NUL. Само новое исправление и связанные проверки
Linux4/4; неизменённый ChapterClueHeading отдельно5/5, нестабильность остаётся
на расследование. Windows4Network6.75с. Дополнительно запущенные12сетевых
диагностик дали interrupt-return failure и timeout EncounterMaps при общем
ограничении120с; их логи также сохранены, успешными они не объявляются.
CI-like280/28023.78с, raw280/noNUL/12FONT-COMPAT. Evidence:
corpse-target-information-{red,green,full,linux-full,linux-related,network-related,ci}
и chapter-heading-isolation-linux. Используются те же две сборки, HD OFF.

5394 полностью imported:99pending/109imported/24partial/37already/3docs/
5excluded-balance/2excluded-own/1historical/1review =281;124 незавершённых
исходных коммита по tracker. Старые37PRESENT ещё требуют сверки нынешнего кода.
Общая работа281, расследование сбоев и финальные Actions не завершены.

## Освобождение действия при полной отмене движения —d8a7f2e1

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d8a7f2e166c54b438f3e2c58de6d81dc3edf9686

В нынешнем репозитории старая отметка PRESENT тоже оказалась неверной:
CExecMove::FullCancel вызывал Finished, который меняет только state, но не
StopAction, освобождающий pAction. Пока очередь или другой владелец держал
завершённый mover, мир мог продолжать считать его активным действием.
Старые проверки сбрасывали сам mover перед проверкой IsAction, скрывая ошибку.

Новый native тест создаёт настоящий многоклеточный путь и executor через
MakeNativeMovementFixture/CreateSimpleMoveExecutor. Run действительно даёт
RUNNING/IsExecuting. Дополнительно удерживается настоящее отдельное действие
мира через публичный GetActiveCounter. Первый собранный RED: после FullCancel
state FINISHED, но retainedMover=1/executing=1. Два более ранних обращения к
неподходящему API fixture дали ошибки компиляции, исправлены до RED и хранятся
отдельно; они не считаются воспроизведением бага.

Единственное изменение поведения — StopAction перед прежним Finished.
GREEN: mover ещё жив, но executing=0; другое действие не сброшено; после его
завершения мир свободен; повторный FullCancel не возрождает действие, не тратит
дополнительные AP и не меняет VP/позицию. Сохранены политика очередей, graceful
Cancel, ожидание пути, конечный state и оригинальные боевые формулы.
Устаревшие комментарии приведены в соответствие с исправленным поведением.

Проверки: Windows ALL_BUILD/Game, Linux Game; связанные24/24 Windows21.61с и
Linux21.42с,5Network включая PartialMovement8.60с, CI-like280/28023.95с.
Raw24/24/280 полные, без NUL, CI12FONT-COMPAT. После удаления временной
диагностики соседнего теста целевой тест повторно собран и проверен: Windows1/1,
Linux1/1.
Ранее упавший NetworkMatch прошёл отдельный контроль15.55с после исправления;
один прогон не объявляется доказательством общей стабильности всех LAN сценариев.
Длительный EncounterMaps и нестабильный ChapterClueHeading остаются отдельными
расследованиями. Заголовок без изменения проверяемого поведения прошёл ещё
30 отдельных запусков и20 с тем же X11 окружением, что в полном прогоне;
это не заменяет устранение причины исходного сбоя. Временный printf в его
тесте удалён, production заголовка не менялся.
Evidence movement-full-cancel-action-{red,green,related,linux,network-related,ci,final}
и chapter-heading-boundary-{probe,display}-linux. Те же сборки, без копий и HD.
CPU387, ожидаемый FULL401 не заявляется как выполненный; последний полный
Windows400/400 и Linux399/400 относится к предыдущему исправлению.

При сверке старых PRESENT дополнительно возвращены в pending четыре источника:
82dfbf13 (обновление курсора всё ещё внутри условного блока), a12d7ee2
(курсор опций2 вместо1), e5443130 (default tile attack всё ещё general failure),
3f78b519 (GetPassability всё ещё расширяет footprint и не проверяет направление
лестницы). Это подтверждение отсутствия конкретных hunks, а не готовый вердикт
об их применимости: нужны native TDD и проверка особенностей нашего форка.
Отдельно повторно подтверждены оба живых merge-пути хеша056a71ab, стрелки946/947
14ee5eae и conclusion/взвешенный выбор1b1cc6d7. Старые FSAA/lighting18df661c
заменены собственной страницей FXAA/FPS и буквально не переносятся.

Tracker281:103pending/110imported/24partial/32already/3docs/5excluded-balance/
2excluded-own/1historical/1review.128 незавершённых исходных entries, включая
четыре вновь открытых; это не число будущих коммитов. Оставшиеся PRESENT ещё
проходят проверку нынешнего репозитория. All281 и финальные Actions не завершены.

## Исчезновение подписи при уходе с юнита — адаптация4430374b

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4430374be9ea6be47346d06be41b68005dd7e835

После разрешения подсказки в forced attack её окно теперь живёт дольше одной
цели. Дополнительный native RED в настоящем CICBeginMission: после успешно
показанного имени SetStateTarget(null) и CStateAttack::Step оставляют visible=1.
Раньше такой переход не проверялся: существовал только контроль создания
нового forced state сразу над пустым местом.

В UpdateEnemyStateInfo убран только IsValid(pUnit) перед вызовом существующего
MakeUnitStateToolTip. Сам helper уже умеет скрывать окно при отсутствии юнита.
GREEN проверяет переходы юнит→null→тот же юнит→не-юнит CWindowDoor, сохранение
одного окна/имени, освобождение в Terminate и неизменность AP/VP/позиции.
Это отдельная адаптация времени жизни окна к нашему интерфейсу, необходимая
для корректного переноса443; не буквальный новый hunk оригинального коммита.
Формулы стрельбы, выбор целей, физический ввод и шрифты не менялись; тест
не заявляет проверку пикселей или движения аппаратной мыши.

Windows ALL_BUILD/Game и Linux Game, связанные58/58 Windows53.78с и
Linux55.77с,10native-display/12FONT-COMPAT, raw полные без NUL. Целевые
NativeForcedAttackTooltip и CorpseTargetInformation2/2 после фикса.
4Network6.94с и CI-like280/28026.98с, raw280/noNUL/12fonts. Evidence:
forced-attack-caption-transition-{red,green,related,linux,network-related,ci}.
CPU387/FULL401 inventory прежние, новый полный прогон не заявляется.
443 остаётся partial: CStateUse body/emptyPK проверяются независимо с учётом
нашего Panzerklein-кода. Tracker и общая работа281 остаются незавершёнными.

## Подпись forced курсора поверх дочернего окна —82dfbf13

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/82dfbf1336676954c8f94949e18923cd5d8aa773

Ещё один ложный старый PRESENT проверен заново: SetCursorInfo всё ещё находился
внутри сравнения значков. Native fixture использует настоящий CICBeginMission,
публичный CommandState с CStateAttack, обычное дочернее CWindow с собственной
SCursorInfo и полный CInterface::Step. При совпадении значков дочернее окно
оставляло свой текст вместо текста игрового действия.

Учтена особенность нашего интерфейса: SetCursorInfo сначала меняет pending
sCursor, а GetCursorInfo читает уже переданный ICursor, то есть предыдущий кадр.
Первая версия теста ошибочно сравнивала их сразу и упала в положительном
контроле; это не RED. После исправления fixture pending проверяется по настоящему
writer-полю6, существующий cursor получается из поля10, а итог наблюдается
после штатного Step и передачи результата в ICursor. Production до этого не
менялась. Первый настоящий RED: sameIcon=1/stateCaption=0/childCaption=1.

Перенесён единственный hunk: SetCursorInfo после дочерних окон выполняется
всегда для FORCED/TEMPORARY; очистка текста при смене значка остаётся условной.
GREEN: тот же значок сохраняет caption состояния, другой очищает его на этом
кадре, возврат восстанавливает caption. Все три проверки идут через настоящий
UI frame, с положительным контролем native CWindow. Сохраняются переходы
подсказки юнит→null→юнит→не-юнит, освобождение окна и AP/VP/позиция.
Нет заявления о физическом движении мыши, аппаратном вводе или проверке пикселей.

Windows ALL_BUILD/Game, Linux Game;59/59 Windows56.97с/Linux52.52с,
11native-display/12FONT-COMPAT, raw полные без NUL;4Network7.19с;
CI-like280/28024.04с, raw280/noNUL/12fonts. Новый Native тест extended,
CPU387 прежний, ожидаемый FULL402 не объявляется выполненным. Полные старые
Windows400/400 и Linux399/400 с ChapterClueHeading остаются отдельной записью.
Evidence forced-state-cursor-caption-{red,green,related,linux,network-related,ci},
а первые ошибки подготовки — fixture-control/fixture-probe-native-test.log.
Сборки переиспользованы; игровые формулы, шрифты, HD и свои backend не менялись.

Источник82df полностью imported. Tracker281:102pending/111imported/24partial/
32already/3docs/5excluded-balance/2excluded-own/1historical/1review.
127 незавершённых исходных entries, плюс сверка оставшихся старых PRESENT.
Общий перенос, расследование нестабильного теста и финальные Actions не завершены.

## Подпись пустого панцеркляйна при использовании — завершение4430374b

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4430374be9ea6be47346d06be41b68005dd7e835

В нынешнем коде пустой PK получает CUnitStateDeath в CWorld::AddUnit даже при
положительной прочности брони. Это представление небоеспособного корпуса;
серверный IsDead здесь не означает разрушение самой брони. Старая проверка
IsUnconscious для подписи поэтому пропускала настоящий целый пустой PK.
Первый правильный собранный native RED: persona93, armorHP700, IsEmptyPK1,
serverDead1, accepted use state, но label0. Настоящая проекция в320x240 и
native GetEnemyUnitInfo успешно прошли положительные контроли до этой ошибки.

Тест использует настоящий CMission::Initialize с авторским variant810 и его
родительским template, мир, player54, HUD, renderer и полный активный
NMainLoop::StepApp. Единственная подменённая граница — вход GetStateTarget:
тест передаёт настоящий объект и признак world, не проверяя аппаратное
наведение/трассировку мыши. Тип PK, состояния, SEnemyInfo, CStateUse, камера,
UI-окно и текст выполняются штатным кодом. Состояния человека и разрушенного
PK задаются публичными native SetState/RPG API; весь процесс смерти, анимации
и сохранения не заявляется проверенным.

Предыдущие ошибки подготовки сохранены отдельно и не считаются RED:
неподходящий конструктор/API fixture, ошибочное ожидание !IsDead у целого PK,
затем устаревшая cached camera matrix после прямого Mission::Step без активного
главного цикла. Диагностика показала directProjection1/cachedProjection0.
Исправлена именно подготовка: полные активные StepApp, штатный scripted
SetPlacement и clock100100 дают direct1/cached1/current1/ready1. Production
до окончательного RED не менялась.

Независимая сверка оригинального Steam Game.exe, SHA256
`4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705`:
Gold PDB CStateUse::Initialize5d5a10 через уникальную54-байтовую сигнатуру
с маской EH и сдвига поля mission10→c сопоставлен со Steam5d8f90.
В Steam5d90a1 вызывается CUnit slot0 и сохраняется отрицание CanFight;
5d90d4 вызывает slot10, затем создаёт подпись со style2c по5d9126.
Идентичная34-байтовая функция Gold CanFight7ba9b0 найдена в Steam7c7280;
её thunk7c7340 и таблица CUnit8ce15c дают slot0. Slot10 указывает на7c72b0,
чьё тело совпадает с Gold PDB IsEmptyPK7ba9e0 и читает PK-флаг [this+84].
Таким образом, названия виртуальных функций не взяты только из комментария
upstream. Некоторые старые адресные комментарии не соответствуют этому EXE:
например7c6840 здесь находится внутри конструктора, а не CanFight.
Оригинальные EXE только читались, не запускались, не копировались и не менялись.

Перенесён выбор IsEmptyPK вместо IsUnconscious и исходно скрытый style2c;
общий helper показывает окно только для валидной проецируемой цели.
Исправлены соответствующие устаревшие комментарии о dead-hover и поле окна.
GREEN: целый и разрушенный настоящий PK получают подпись/PK-only состояние
брони; ограничения точных HP сохранены; обычный живой человек, бессознательное
тело, труп человека, null и non-world target проходят свои контроли. Terminate
освобождает окно. Запросы не меняют AP/VP/позицию наблюдателя и не исполняют use.

Отдельная замена gate IsDead||IsUnconscious на !CanFight не нужна: обе
production реализации CanFight (CUnit и CUnitServer, включая используемые LAN
объекты) определены как !IsDead&&!IsUnconscious. Для всех нынешних типов это
эквивалентный hunk, что дополнено native контролями состояний; искусственного
override ради падающего теста не вводилось. Исходный gate оставлен.

Windows ALL_BUILD/Game и Linux Game;60/60 Windows61.08с/Linux57.84с,
12native-display/12FONT-COMPAT, raw полные без NUL.4Network12.36с;
CI-like280/28030.21с, raw280/noNUL/12fonts. Целевые4/4 после фикса.
Evidence body-hover-labels-{red,green,related,linux,network-related,ci},
body-hover-label-retail-{lookup,disassembly}; ошибки подготовки — fixture-*
и camera-probe. Те же сборки, HD OFF, без дополнительных копий игры.
CPU387 прежний; ожидаемый FULL403 не объявляется выполненным. Последний
полный Linux399/400 с нестабильным ChapterClueHeading остаётся отдельным
неустранённым результатом; длительный EncounterMaps также требует проверки
с подходящим лимитом времени.

443 полностью учтён: forced frame d2f1982f, его native lifecycle адаптация7fb6eda0,
нынешняя подпись PK и комментарии, эквивалентный CanFight gate. Порядок
обновления called-shot/trace/info из своего форка сохранён и проверен, буквальная
перестановка upstream не нужна. Повторно подтверждён эффект06bf8c45 через
native SEnemyInfo.bUnitInfo=false для PK и соответствующий guard общего helper.
Оригинальный баланс, свои шрифты, лицевая анимация и графический backend
не менялись. Tracker281:102pending/112imported/23partial/32already/3docs/
5excluded-balance/2excluded-own/1historical/1review;126 незавершённых исходных
entries плюс сверка оставшихся PRESENT. Вся цель и финальные Actions не завершены.

## Дополнительная проверка всех encounter maps и oracle для3f78b519

На HEAD892c6353 NetworkEncounterMapsTests запущен со своим лимитом600с:
прошли361/361 авторских random-encounter вариантов, размещение обоих отрядов
и применение состояния к клиентской replica. CTest1/1,127.61с (общий127.65с),
raw полный/noNUL. Предыдущий тайм-аут был получен с моим общим ограничением
120с, которого этому набору не хватало. Production, тест и CMake ради него
не менялись; ошибка движка из того тайм-аута не следует. Evidence:
encounter-maps-proper-budget-{test,native-test}.log. Это проверка deployment
и replica обновления по двум segment на карту, не полный бой на каждой карте.
Нестабильный ChapterClueHeading из прежнего полного Linux прогона остаётся
открытым расследованием и не закрывается этим результатом.

Для следующего pending источника
https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3f78b5198af04658670c9abeaf444fc5006f334d
проведена только независимая статическая сверка, без переноса production.
Gold PDB GetPassability(const place)442830/385bytes через54 точных байта
уникально сопоставлен с проверенным Steam443560. Другой Gold overload441840
принимает иные аргументы; upstream адрес443670 не является началом нужного
метода. В фактическом Steam порядок: pose check44368e, затем только anchor
nLocks по4436ae, затем door query4436c2→443410. Цикла GetLockArea нет.
Направление лестницы проверяется4435ee..443609, bounds слоя —443583..443595
до доступа к ladder. Сохранены anchor-passability-retail-{lookup,disassembly}
и обе Gold overload disassembly; Gold только локатор, Steam — oracle.

Источник3f78 остаётся pending: нужны отдельные native RED/GREEN для независимых
ошибок и адаптация к своим overlap locks/PK/LAN. Эта запись не объявляет
изменённую механику исправленной и не заменяет TDD. Количество завершённых
исходных коммитов не меняется. Полная цель281 и финальные hosted Actions
по-прежнему не завершены.

## Классификация опорной клетки отдельно от footprint — часть3f78b519

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3f78b5198af04658670c9abeaf444fc5006f334d

В оригинальном Steam443560 GetPassability проверяет конкретную опорную клетку,
а полная область лежащего бойца/PK проверяется отдельными путями. В нынешнем
коде этот granular query ошибочно вызывал GetLockArea и возвращал LOCKED,
если занята соседняя клетка или иной элемент расширенной области.
Независимая статическая сверка Steam/Gold и точные адреса записаны выше;
без неё перенос мог бы закрепить старое неверное ожидание теста.

Новый resource-free native RED: настоящий CPathNetwork с двумя слоями,
штатные Lock для двух перекрывающихся владельцев на соседней клетке.
Standing/crouch и незаблокированный pose path проходят положительные контроли;
у опоры nLocks0, у соседа1, но prone anchor даёт LOCKED3 вместо YES0.
Старый SimpleCrossLayerPoseEntry тоже упал после исправления одного неверного
ожидания: он считал GetPassability эквивалентом IsPassable. При этом все его
проверки запрета полного занятого footprint и пути оставлены строгими.
Ошибки сборки или настройки здесь за RED не выдаются: оба теста собраны и
падают на ошибочном anchor verdict до изменения production.

Минимальный фикс заменяет только цикл GetLockArea на чтение nLocks самой
опоры. Старые относительные проверки door/pose и ladder пока не менялись:
это следующие независимые части источника и им нужны свои RED/GREEN.
Исправлены комментарии, утверждавшие равенство granular и bool запросов.

GREEN подтверждает: строгие IsPassable, BigLockerPassableState, PK placement
и настоящий FindSimplePath всё ещё запрещают занятую область тела; реально
занятая опора даёт LOCKED. Снятие первого владельца сохраняет блокировку
второго, снятие последнего возвращает доступность пути. В CDumbUnitServer
CheckPassable после anchorYES по-прежнему выполняет BigLockerPassableState
для PK или назначения LAY. Свои overlap locks, bool запросы, размеры footprint,
flying guard и упаковка координат не менялись. Это native unit-level контракт
сетки; полный melee-бой/атака на PK в игровом мире здесь не заявляется.

Windows ALL_BUILD/Game и Linux Game; FULL404/404 Windows346.98с и
Linux344.93с: CPU388 плюс прежние GPU/native-display extras,17audio/12fonts
на каждой платформе, raw404 полные без NUL.5Network включая PartialMovement
8.93с. После изменения361/361 encounter variants снова прошли deployment и
replica,128.56с при лимите600с. В том же расширенном наборе NetworkMatch
повторил известный ранее сбой interrupt return: диагностический отдельный
запуск показал busy=1 в момент EndTurn и отклонённый ответ. Весь сетевой набор
не объявляется зелёным; причина и исправление проверяются отдельно от anchor.
CI-like281/28124.24с включает новый resource-free UpstreamAnchorPassability,
raw281/noNUL/12fonts. Временная failure-only диагностика ChapterClueHeading
в полных прогонах не сработала и удалена; финальные3/3 (anchor, старый pose
path, heading) повторно собраны/проверены Windows1.27с/Linux1.15с. Сама прежняя
нестабильность heading этим не объявляется исправленной.
Evidence anchor-passability-{red,green,full,linux-full,network-related,network-maps,ci,final},
raw native logs и interrupt-return-probe-native-test.log. Сборки прежние, HD OFF.

Дополнительно по актуальным файлам перепроверены восемь old PRESENT:
f7426577 (clue template0), a0d6faf5 (пустой serializer CCmd), d5c745a2
(обнуление удалённого render slot в двух местах),15248279 (реестр только
map-defined групп),5a5c7218 (нулевые scroll axes),8bfee072 (refresh called-shot
до caption),d6406b71 (stationary ChangePose),a4b37de5 (unlock pilot до
восстановления shell). Все конкретные hunks остаются в нынешнем активном коде;
повторный перенос не нужен. Это проверка существующих реализаций, не новые
независимые исправления; сведения о доступных native controls указаны в TSV.
Для c0c1914c исправлено прежнее описание GetPassability, сам pose path fix
и его запрет занятого полного footprint сохранены.

3f78 остаётся partially-imported: bounds/facing/precedence ещё открыты.
Tracker281:101pending/112imported/24partial/32already/3docs/5excluded-balance/
2excluded-own/1historical/1review;126 незавершённых source entries плюс остаток
сверки old PRESENT. Оригинальный баланс не менялся. Общая цель, расследования
сетевого interrupt/heading и финальные actual-HEAD Actions не завершены.

## Корректный момент EndTurn в сетевом integration test

Это самостоятельное исправление нашего тестового сценария после расширенных
проверок d8a7f2e166c54b438f3e2c58de6d81dc3edf9686 и 3f78b519;
новая игровая логика из upstream здесь не переносится. Связанный исходный коммит:
https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d8a7f2e166c54b438f3e2c58de6d81dc3edf9686

Native RED interrupt-idle-red: перед отправкой EndTurn мир занят (busy=1),
сервер закономерно возвращает UCR_UNAVAILABLE=1. Прежний тест игнорировал ответ
и ожидал смену хода. Новый assert проверяет принятие команды и воспроизводит
ошибку непосредственно в её причине. Предыдущий probe дополнительно показал
ровно две записи interrupt stack: основной ход и единственное прерывание.

Тест теперь продолжает настоящую симуляцию/репликацию до native idle, максимум
400 шагов; при превышении лимита падает. Проверяет, что владелец прерывания
сохранился, затем отправляет EndTurn, требует UCR_OK и сохраняет исходные
проверки возврата управления, replica, победы/ничьей. В первом GREEN потребовалось
35 дополнительных шагов, busy=0/result=0, полный сценарий прошёл за18.27с.
Ни action counters, ни серверный запрет EndTurn во время действий не менялись.

Сетевой набор6/6 прошёл за52.28с, включая ещё один полный match (15.37с),
preparation, client UI, partial movement, object stages и stop/resume.
Evidence: interrupt-idle-{red,green,related}-native-test.log. Это тестовая
адаптация своего LAN harness, без изменения баланса, AP, RNG и игровой механики.
CI-like281/28123.74с; raw281 полный без NUL,12font controls. Linux network match target в нынешней LinuxGame конфигурации отсутствует; Linux полный404 подтверждён для предыдущего production fix, для этой Windows test-only правки не заявляется. Tracker source counts не меняются.

## Направление персонажа на лестнице — отдельная часть 3f78b519

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3f78b5198af04658670c9abeaf444fc5006f334d

UpstreamLadderPassabilityFacing использует настоящий CPathNetwork, слой и четыре
native SLadder с восемью ступенями. Ничего не подменяет в запросах проходимости.
Все корректные направления, разрушенная ступень и блокировки проверяются до
RED assert. Первое собранное RED0.08с: LD_RIGHT, step0, facing0 принят как YES,
хотя требуется facing4. Независимый retail oracle уже найден по уникальным
байтам Gold PDB в Steam Game.exe: 4435ee..443609 проверяет (2*eDir-4)&7.
Таблица теста задана отдельными значениями 4/6/0/2 для RIGHT/FRONT/LEFT/BACK.

Production fix — одна строка после прежних проверок геометрии и блокировки
ступени: сравнение направления с (2*eDir+4)&7. GREEN3/3 подтверждён; основной
тест проверяет все4 ориентации x8 ступеней x8 направлений. Занятая ступень
сохраняет LOCKED раньше facing, разрушенная NOT_PASSABLE раньше lock. Старый
bool IsPassable по-прежнему проверяет occupancy без этого facing-контракта.
Native геометрия/блокировки не изменяются запросом. Полный подъём по реальной
лестнице игрового уровня этим unit-level тестом не заявляется.

Windows ALL_BUILD/Game и Linux Game собраны. Связанные21/21 Windows7.61с,
Linux6.93с, сетевые6/6 57.21с (полный match20.24с, с исправленным ожиданием
native idle). Evidence ladder-facing-{red,green,related,linux,network}-native-test.log.
Новые исходники/сборки не копировались, HD не пересобирались, оригинальное
направление на лестнице восстановлено без изменения оригинального баланса.

Повторно проверены все hunks ещё пяти old PRESENT:28c73ff5 (flying guards),
de185dac и2d238a8b (ограниченный stationary inactive AI yield),3eed22d7
(ручное снятие fixed PK weapon),e0eb2a14 (точечное снятие immediate notification).
Их реализации уже есть в активных файлах; сведения внесены в TSV, повторного
переноса нет. Это сверка кода, отдельные новые runtime RED для них не заявляются.
3f78 остаётся partial: bounds-before-ladder и ground verdict precedence открыты.
CI-like282/28224.00с, raw282 полный/noNUL/12FONT-COMPAT. Новый asset-free test одинаково зарегистрирован Windows/Linux; CPU389, полный набор405 ожидается, новый FULL405 в этой проверке не запускался. Tracker101pending/112imported/24partial/32already/3docs/5excluded-balance/2excluded-own/1historical/1review, всего281,126 незавершённых.

## Границы слоя до обращения к лестнице — отдельная часть 3f78b519

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3f78b5198af04658670c9abeaf444fc5006f334d

Native UpstreamLadderPassabilityLayerBounds сначала проверяет настоящие две
лестницы в существующих слоях, прежний отказ ground-запросов для слоёв2/255
и три flying-запроса, в которых layer кодирует высоту. Затем обращается к
nonintegral ladder place в отсутствующем слое. Первый собранный RED0.09с:
процесс SegFault на layer2 при available2; положительные controls уже прошли.

Минимальный fix переносит существующую проверку номера слоя перед лестничной
веткой. IsFinal остаётся самым первым условием, так что flying altitude по-прежнему
даёт YES, включая IsBigLockerPlacePassable. Лестничная и ground ветки используют
один уже проверенный указатель слоя. Новых форматов/параметров, произвольных
ограничений индекса лестницы или её ступеней не добавлено. Independent Steam
oracle443583..443595 подтверждает ровно такой порядок после flying guard.

GREEN4/4 (новый тест, facing, anchor, прежний FlyingRestoration); расширенные
22/22 Windows7.10с/Linux12.74с; Windows ALL_BUILD и обе Game собраны.
Сетевые5/5 37.73с, CI-like283/28327.82с. Raw logs полные без NUL,
CI сохраняет12FONT-COMPAT. Evidence ladder-layer-bounds-{red,green,related,
linux,network,ci}-native-test.log. CPU390; ожидаемый полный набор406,
новый FULL406 здесь не заявляется. Оригинальный баланс, HD и собственные
footprint/overlap policies не менялись. Source3f78 всё ещё partial:
остался независимый порядок ground pose -> lock -> door. Общие tracker counts
не изменились (126 незавершённых source entries).

## Приоритет причины отказа ground-запроса — последняя часть 3f78b519

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3f78b5198af04658670c9abeaf444fc5006f334d

Новый native UpstreamGroundPassabilityPrecedence использует настоящий
CPathNetwork, frozen слой и CWindowDoor. Open-state настоящей двери задан
через её CObjectBase serializer tag2; flipper регистрируется штатным GetFlipper,
closed/open restriction maps и производный lookup проверяются public query.
Алгоритмы GetPassability/IsPassable и их door lookup не подменяются.
Первоначальные fixture compile errors (private IsNotOnDoor и прямой доступ к
private serializer) сохранены отдельно, исправлены через public query и virtual
CObjectBase dispatch, не считались native RED и не меняли production API.

Настоящий собранный RED0.09с: поза STAND, facing0, native nPassable0, lock1,
открытая дверь — возвращается LOCKED3 вместо NOT_PASSABLE1. Перед этим
прошли16 положительных controls для четырёх поз и allowed/locked/door случаев.
Тест отдельно задаёт160 комбинаций пяти blocked-pose cases, восьми направлений,
двух состояний двери и двух состояний блокировки; среди них CANNOT_LAY при
общей возможности лежать без доступного направления. После удаления ограничений
проверяет возврат YES. Строгий bool IsPassable во всех blocked случаях остаётсяfalse.

Independent Steam oracle44368e..4436d3 подтверждает именно pose -> anchor lock
-> door; исходный комментарий с другим адресом не использовался как доказательство.
Минимальный fix переносит прежнюю IsTilePassable/CP_LAY классификацию перед lock
и door и удаляет ставший лишним повторный RefreshSpot/чтение той же клетки.
Anchor-only query, flying guard, bounds и ladder facing из отдельных TDD-коммитов
сохранены; footprint/overlap ownership и bool IsPassable не менялись. GREEN6/6
0.15с включает все четыре новых контракта, старые pose path и FlyingRestoration.
Полный melee/PK gameplay сценарий этот native unit-level тест не изображает.

Итог проверок после всех частей3f78: обе Game/Windows ALL_BUILD собраны;
FULL407/407 Windows346.36с/Linux340.94с,17native-audio,14native-display,
12FONT-COMPAT на каждой платформе.7/7Network185.62с, включая полный
match18.09с и361/361 authored encounter deployment+replica129.70с (лимит600с).
Это два сегмента на map variant, не полный бой на каждой карте. CI-like284/284
23.95с. Все raw логи полные/noNUL; evidence ground-verdict-order-{full,
linux-full,network,ci}-native-test.log. Прежний ChapterClueHeading прошёл оба
полных прогона без временной диагностики; это не доказательство устранения
его прежней единичной нестабильности.

3f78 теперь imported: flying guard был уже реализован; anchor footprint,
ladder facing, bounds-before-ladder и ground verdict precedence внесены
независимыми native RED->fix->GREEN коммитами с немедленным push.
CPU391, полный набор407 актуален. Tracker281:101pending/113imported/23partial/
32already/3docs/5excluded-balance/2excluded-own/1historical/1review;
125 незавершённых source entries. Оригинальный баланс не менялся, HD OFF,
сборки переиспользованы. Весь281-пунктный перенос и final actual-HEAD Actions
ещё не завершены. В TSV дополнена read-only подготовка e5443130 с независимым
retail oracle: реальные TOOL/KEY достижимы в default tile-attack ветке;
для этого источника production fix/нативный RED пока не заявляются.

## Скрытие атаки по клетке для инструмента и ключа — e5443130

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e54431300ecc6da00dfea9991724bd2fd4b3a9c1

UpstreamNonAttackTileOrders расширяет существующий native tool fixture новым
отдельным режимом, не меняя прежний сценарий EquippedToolActionAvailability.
Настоящий FixtureWorld, CUnitServer, native inventory, CToolItem из authored
CRPGTool и ключ через authored CRPGItem successor используются с реальными
CreateActionExecutor и CMission::CanDoCommand. Key Equip/AT_KEY, разрешение
атак в мире и реальная доступность use-tool проверяются до RED assert.

Первый собранный RED1.17с: инструмент item387/AT_TOOL9, executor0,
UCR_GENERAL_FAILURE2. Перед production edit тест дополнен наблюдением native
UI-result и снова RED1.19с: тот же результат2, available1/uiResult2. Эти два
лога сохранены; fixture compile errors или подмена GetActionType не требовались.
Настоящий ключ item389 уже прошёл положительный Equip/AT_KEY control.

Независимый Steam oracle: GoldPDB CreateActionExecutor792190/5657bytes,
первые57bytes с маской только EH/двух RTTI pointers уникально совпадают с
Steam79ddd0. RTTI951490 подтверждает CCmdShootTile branch79de5a. Native
GetActionType793a40 распознаёт IToolItem через RTTI97c648 и даёт10, IKeyItem
через97c6ac и даёт11. Оба индекса jump table79f3d8 идут на79deb1, result5
(retail UNAVAILABLE). Исходный комментарий79d821 не соответствует адресу
нашего проверенного Steam exe. Наши UCR номера и LAN wire отличаются:
использован символ UCR_UNAVAILABLE, числовые enum/wire не менялись.
Evidence nonattack-item-retail-{lookup,disassembly,actiontype,jumptable}.

Minimal fix в default ветке tile attack — UCR_UNAVAILABLE вместо
UCR_GENERAL_FAILURE и удаление ASSERT для штатных неатакующих предметов. NoAttack world
и отдельный запрет мин сохранены. GREEN2/2 2.28с: tool387/type9 и key389/type10
теперь result1/executor0/available0/uiResult1; настоящий use-tool остаётся
доступен. Проверки не расходуют AP/VP/заряды/ключ, не меняют packed position
и не создают приказа. Боевые формулы, стоимость действий, inventory transfer,
HiRes fonts, facial engine и renderer не менялись.

Windows ALL_BUILD и обе Game собраны. Связанные63/63 Windows61.66с,
Linux56.87с, включая12native-display и12FONT-COMPAT на каждой платформе.
Raw63/noNUL.6/6Network52.60с. Evidence nonattack-tile-orders-{red,first-red,
green,related,linux,network}-native-test.log. Сборки прежние, HD OFF.

В текущих активных файлах дополнительно сверены все hunks трёх old PRESENT:
796f6b2d (проверка выполняемого/доступного действия до выдачи path remover,
readiness для скрытого портрета),90479594 (выбранный получатель store, владелец
перетаскиваемого предмета, ручная клетка и fallback),d441feb8 (called-shot через
snipe executor/state, raw CVec3 save positions, HL_ANY и строгий angle guard).
Свой facial PlayAckEvent уже сам выставляет readiness; существующий hidden
fallback не заменялся upstream-аниматором. Это сверка существующего кода,
не новые production fixes или новые runtime RED для этих трёх источников.

CI-like284/28424.27с, raw284/noNUL/12FONT-COMPAT. Новый asset-backed test
исключается из resource-free CI по прежней baseline policy; зарегистрирован
на обеих платформах и реально прошёл в63/63. CPU392, ожидаемый FULL408
здесь не запускался; последний фактический FULL407/407 обеих платформ был
на предыдущем1cfe7cc7. e5443130 теперь imported. Tracker281:100pending/
114imported/23partial/32already/3docs/5excluded-balance/2excluded-own/
1historical/1review —124 незавершённых source entries. Оригинальный баланс
не менялся. Общий перенос и финальные actual-HEAD GitHub Actions не завершены.

## Ожидающий маршрут после отмены движения — 416e09a2

Источник: https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/416e09a2e5661cb128ef963dc0338d860bdaf0e4

Native UpstreamInterruptedMovementRoute проверяет настоящий CWorld, AI- и
human CUnitServer, реальную команду CCmdPath, CExecQueue/CExecMove, штатную
TBS_CANCEL_ACTION и retirement через Segment/animation boundary. Исполнитель,
pathfinding, правила проходимости и командный dispatch не подменяются. Поля7/9
наблюдаются read-only через обычный serializer/external refs. AI определён
реальным CAICommander, человек — CSequenceCommander, не тестовым bool override.

До RED проверяется действующий AI guard: подготовленный реальный mover
отменяется своим native Cancel, Run получает FAILED и штатно освобождает
исполнитель/команду. Это проверка failed-order cleanup, не полный AI-бой.
Персонаж игрока действительно начинает многоклеточное движение. Штатная отмена
заканчивает уже начатый шаг:2,8->2,7 и49->45AP, здоровье137 неизменно.
Начальные ошибочные fixture-предположения «Cancel не завершает шаг/не тратит AP»
исправлены до утверждения о RED; игровая стоимость остановки не менялась.

Первый подтверждённый RED2.07с, затем окончательный baseline RED2.12с после
восстановления ровно HEAD-версии wUnitServer.cpp без временной диагностики:
FAILED исполнитель уже освобождён, команда человека исчезла, target1,4 потерян.
AI control прошёл раньше. Native final-red log фиксирует command0/target1,4 и
падение именно assert сохранения назначения. Проходы fixture-step, clear-path,
ai-retire/cancel-cost, short-route и free-destination не выданы за этот RED.

Independent oracle: GoldPDB CheckCmdExecState7b4b60/242bytes, первые49 EXACT
байтов уникально найдены в проверенном Steam по7c12e0. В7c1328..7c134d только
state FINISHED1 и отсутствие forced-action флага очищают pCurrentCmd; FAILED
его сохраняет. Исходный адрес7c0cb8 для этого exe не использовался как oracle.
Evidence interrupted-movement-route-retail-{lookup,disassembly}.

Minimal fix сохраняет неудачную команду человека, но оставляет существующую
очистку FAILED для AI. Это осознанная адаптация: наш scheduler считает
HasCommand незавершённой работой, и прежний фикс защищает GFirst/RF1 от цикла
Continue мёртвого исполнителя и блокировки очереди решений. FINISHED и forced
action guard сохранены. Reentrant release, prompt FAILED reap, hold-aim,
FullCancel action counters и TBS_STOP обработчик этим коммитом не менялись.

GREEN проверяет native GetCurrentPath до прежней цели, отсутствие движения/AP
расхода при запросах pending-path и четырёх idle кадрах; явный Continue
возобновляет ход. Дополнительный реальный TBS_CANCEL_ACTION4 в1,5/AP35 был
подтверждён временным production printf, после чего диагностика полностью
удалена. Причина этого дополнительного сигнала здесь не утверждается. Драйвер
подаёт новый явный Continue после остановки, максимум8 requests/128 edges;
маршрут доходит до1,4 с31AP и успешная команда удаляется. Первый ошибочный
драйвер ожидал пройти до цели через единственную команду Continue и падал при
повторной остановке; это не объявлено отдельной ошибкой end-move/b4505fb0.
Явный CCmdCancel для нового подготовленного обратного пути по-прежнему удаляет
команду/путь без исполнения, расхода AP и изменения позиции.

Отдельный UpstreamInterruptedMovementRouteSave сохраняет полный native graph
CGlobalGame+CWorld+selected CUnitServer в памяти,1167173bytes на Windows.
Восстановлены независимые world/actor, вызван RestoreRuntimeCaches. Назначение,
human owner,45AP/137VP/позиция и отсутствие выполнения сохранены. Тот же
GetCurrentPath/Continue сценарий завершается в1,4 с31AP. Это полный WORLD graph;
проверка не изображает CMission UI, файловый SaveMission header или SDL mouse
E2E. Исходный мир удерживается в памяти, отдельной копии сборки/исходников/
ресурсов на диске не создаётся. Fresh3/3 4.38с и save+fresh+FullCancel+AIIdle
4/4 6.37с прошли до расширенных прогонов.

Глобальная TBS_STOP_MOVE_AND_CANCEL_ACTION имеет отдельную историю и остаётся
предметом pending890e4474 (deferred TBS events). Его wholesale patch также
меняет enum values, очереди, weather timing, first-turn perks и удаляет свои
reentrant/reap guards. Эти изменения нельзя переносить в рамках одного
retirement fix без собственных native TDD/oracle и проверки наших LAN/save/AI
инвариантов. Оригинальный баланс, формулы AP/RNG, HiRes fonts, facial engine,
SDL/bgfx и HD pipeline данным исправлением не менялись.

Итог: обе Game и Windows ALL_BUILD собраны. FULL410/410 Windows349.66с/
Linux348.81с,17native-audio/14native-display/12FONT-COMPAT на каждой платформе.
7/7Network186.00с, включая361/361 authored encounter deployment+replica129.97с
(лимит600с; не полный бой на каждой карте). CI-like284/28424.53с.
Raw410/410/7/284 полные без NUL, fonts12 сохранены. Evidence
interrupted-movement-route-{full,linux-full,network,ci}-native-test.log.
Новые два asset-backed test зарегистрированы Windows/Linux, resource-free CI
не расширяют; CPU394, FULL410 актуален. Последний проверенный hosted run для
HEAD до этого коммита23a36227 завершился success:
https://github.com/mrartanis/silent-storm-remaster/actions/runs/40487401584
Это не подтверждение ещё не выполненного Actions для нового коммита.

416e09a2 теперь imported с явно описанной адаптацией AI. Tracker281:
99pending/115imported/23partial/32already/3docs/5excluded-balance/
2excluded-own/1historical/1review —123 незавершённых source entries плюс
оставшаяся сверка old PRESENT. Общая цель и final actual-HEAD Actions не
завершены. В TSV890e4474 добавлена dependency заметка: текущий OnPassControl
синхронный, standalone captured-owner bug не доказан; изменение owner API
нужно рассматривать вместе с deferred delivery, не выдумывать для него RED.
Глобальный STOP не менялся. Прежний единичный ChapterClueHeading здесь также
не объявлен исправленным; оба текущих full прогона его прошли.


## Временная подсветка бойца ИИ — визуальная часть0726a489

Источник: [0726a489c66ef5131dc5f336d8539c316ca31573](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0726a489c66ef5131dc5f336d8539c316ca31573).
Рикошеты/clear-distance/RNG из того же исходного коммита не включены.

NativeAIUnitFlash открывает настоящую CMission с authored variant810/party54,
реальными world/player/HUD, скрытым SDL окном и нашим bgfx renderer. Настоящий
CUnitServer принадлежит существующему hostile AI commander. Входная граница
теста — штатная очередь CUICmdAIUnitWillMove, а не выбор бойца AI scheduler.
Мир остановлен впереди presentation clock; боевые Segments не выполняются.

Первая попытка с4 прогревочными кадрами не прошла контроль стабильности сцены;
это не RED бага. После24 прогревочных и4 кадров стабилизации настоящего
выделения GPU выделил525/664 pixels, соседние контрольные кадры совпали.
Подтверждённый compiled RED2.18с: UI command обработан, а повторный Select
возвращает другой объект (preserved0). Текущий обработчик подменял обычное
выделение и при selectionmode2 не создавал ожидаемую вспышку. Evidence
ai-unit-flash-confirmed-red-native-test.log; production до него не менялась.

Независимая сверка оригинала: SHA256 Steam Game.exe
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705.
GoldPDB FlashUnit6c5e40: первые80 байтов с маской только EH-address9:13
уникально найдены в Steam6cc8e0. Gold prune6c5b90:48bytes/mask3:7 ->6cc630;
Gold Recalc6c9870:27bytes/maskfloatAddress23:27 ->6d03d0, константа8c5840
равна0.003. Оригинал добавляет1500 в6cca68 и удаляет при end<now, без
подмены selection. Две точные66byte последовательности Gold5a16e0/5a1727
уникально совпали со Steam5a4070/5a40b7: cyan(0,77/255,78/255,0) для
neutral/ally и red(78/255,20/255,0,0) дляenemy. Исходный6cc930 не выдаётся
за проверенный адрес данного exe. Evidence ai-unit-flash-retail-{lookup,disassembly}.

Минимальный перенос использует существующие AddPostFilter/CPostColorer и tag9
unitFlashes. Добавлены CFlashColorFunc с исходным registrationID02973151,
IRenderGame::FlashUnit, реализация только для rUnits и удаление просроченных
эффектов после Sync. Обработчик миссии выбирает оригинальный цвет по diplomacy.
Собственные SDL/bgfx, facial engine, audio mixers, обычное выделение, HiRes fonts,
HD pipeline, LAN/save schemas, боевые формулы и RNG не переписывались.
Независимый sound-clock test double получил только обязательный throwing override
нового интерфейса; алгоритм проверяемого рендерера ничем не заменён.

Усиленный native GREEN2.39с: выделение сохраняет identity, GPU показывает
271/332 изменённых model pixels даже приselectionmode2; цвет слабеет на300мс.
Native borrowed save fields наблюдают ровно один weak filter, живой при1500мс
и освобождённый при1501мс. Кадр после expiry совпадает с независимым no-flash
контролем при том же1501мс. Сравнение с исходным кадром при0мс было неверной
постановкой: idle-анимация также следует renderer clock; это исправление теста,
не дополнительный игровой баг. Первоначальный Windows max-macro compile error
также не считался RED. Дополнительный fixture с authored hidden enemy не нашёл
подходящего скрытого юнита; его заменил настоящий native FastUpdate path,
который убирает unit render set. Отсутствующая в этом наборе модель и non-unit
world object не получают flash. Это контроль render-set boundary, не обещание
полного fog-of-war/AI-commander/hardware-mouse E2E. AP,VP,позиция и world time
остаются прежними. Снимки/структуры удерживаются только в памяти.

Обе Game и Windows ALL_BUILD собраны. FULL411/411 Windows353.79с и
Linux350.51с, на каждой17native-audio/15native-display/12FONT-COMPAT.
Оба raw411 полные без NUL; новый тест отдельно зарегистрирован Windows/Linux
с изолированным user-data directory. Evidence ai-unit-flash-{full,linux-full}-native-test.log.

Завершена дополнительная сверка оставшихся old PRESENT1bd13094/9ff4893c/
6d189543/51ab89c5 по всем hunks с текущим кодом: familiarity caps,
immediate clue transfers, decal rebuild/projection и scripted PK round trips
уже реализованы. Сохранены собственные RNG/route/validity guards. Уточнения
добавлены в TSV, статусы already-present не менялись; новые fixes для них
не выдумывались. 1b1cc6d7 уже имел текущую сверку conclusion/weighted selection.

6/6Network58.03с и CI-like284/28424.68с также прошли; raw6/284 полные,
без NUL, fonts12 в CI. Последний411 suite — свежий полный прогон на обеих
платформах;361-map sweep не повторялся для визуального эффекта (последний
успешный361/361 остаётся проверкой416e09a2). Tracker281:98pending/115imported/
24partially-imported/32already/3docs/5excluded-balance/2excluded-own/
1historical/1review,123 незавершённых source entries. Оригинальный баланс
не менялся; pending ricochet scope описан в balance Markdown. Общая цель
и final actual-HEAD GitHub Actions остаются активными.


## Перо в меню настроек — a12d7ee2

Источник: [a12d7ee275f1a05762a09f220df520fc243530df](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a12d7ee275f1a05762a09f220df520fc243530df).

NativeOptionsPenCursor создаёт настоящее SDL/bgfx окно, загружает наши native
CICOptions pages и выполняет активный кадр. Положительный контроль подтверждает
доступность разных authored pen1/arrow2 textures и обычный default cursor2.
На границе входного mouse-message тест передаёт EVENT_MOUSEMOVE настоящему
page handler и читает его результат в native CInterface. Первый compiled
RED1.29с: modern video page=1 публикуетrecord2 вместоpen1. Production до этого
не менялась. Обработчик/конструктор не подменяются. Входное сообщение подано
тестом; hardware mouse picking и растеризация bitmap курсора не объявлены E2E.

Исправлена только запись GetUICursor2->1 в CEmptyOptionsUI constructor и
устаревший комментарий. Прежняя стрелка была унаследована от upstreama8c50bf29,
не являлась самостоятельным решением нашего форка. Пять страниц — video,
audio, gameplay, controls, profiles — сохраняют нашу современную video UI,
adaptive layout, собственные HiRes fonts, cursor backend/hotspots и HD assets.
Их данные, размеры текста и текстуры не меняются. Default arrow2 вне настроек
остается прежней; pen handler убирает предыдущую child caption.

Independent oracle: GoldPDB parameterized CEmptyOptionsUI constructor61dae0,
338bytes. Короткий39byte prefix с маскойEH даёт4 Steam candidates, поэтому сам
по себе не доказывает совпадение. Более длинный102byte prefix не совпадает:
в Steam CWindow base вырос с0x68 до0x80, что видно по инструкциям и offsets.
Constructor622bf0 независимо связан с нужным типом через установленную
vtable8c0668, RTTI locator8eccc4/descriptor97e67c с именем
.?AVCEmptyOptionsUI@NUI@@. В нём622c97 movECX,1 и622ca1 call7dade0;
Gold61db7e movECX,1/61db88 call7ec910 (PDB NDb::GetUICursor). Это подтверждает
выбор pen1. Исходный622b57 не принят за адрес этого exe. Evidence
options-pen-cursor-retail-{lookup,disassembly}.

GREEN1.37с проверяет все5 настоящих страниц: submitted cursor1, пустая подпись,
default2; каждая страница закрыта штатно. Обе Game и Windows ALL_BUILD собраны.
Связанные70/70 Windows69.62с/Linux63.88с включают graphics options, три audio
options сценария, screenshot adoption, native cursors, AI flash, save/input,
12FONT-COMPAT и17native-display. CI-like284/28424.68с. Raw70/70/284 полные,
без NUL, fonts12 в каждом. Evidence options-pen-cursor-{related,linux-related,ci}-native-test.log.
Полный412 suite не запускался повторно для одной записи курсора; последний
актуальный полный прогон411/411 на обеих платформах выполнен для8e6e9424.

Tracker281:97pending/116imported/24partially-imported/32already/3docs/
5excluded-balance/2excluded-own/1historical/1review —122 незавершённые записи.
Оставшиеся old PRESENT hunks сверены в предыдущем коммите. Боевые формулы,
оригинальный баланс и HD pipeline не менялись; отдельные старые сборки не
создавались. Общая цель и final actual-HEAD GitHub Actions не завершены.


## Выбранная и исходная сложность игры —98af43bd

Источник: [98af43bdaf252e5a2eac1ad047c92511adb8e565](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98af43bdaf252e5a2eac1ad047c92511adb8e565).

Original oracle: GoldPDB CreateGlobalGame693f60/189bytes; первые34 байта с
маской толькоEH3:7 уникально совпали со Steam699f90. В69a007..69a02d проверяется
валидность переданного record и сохраняется его pointer; fallback передаёт1
в GetDBDifficulty7db190 (Gold PDB7eccc0). Исходный69a0b7 в данном exe относится
к другому коду и не использовался как доказательство. Steam SHA256 остаётся
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705.
Evidence global-game-difficulty-retail-{lookup,disassembly}.

UpstreamGlobalGameDifficulty: первый compiled native RED1.01с показал
factory default2 вместо1. Положительный контроль загрузил разные authored
DB records1/2 и вывел их реальные коэффициенты. Затем восстановлены selected
pointer API/factory и его передача из CICBeginGame. Числа DB не редактировались;
CGlobalGame save fields и scenario construction сохранены. Использований
прежнего явного integer второго аргумента в исходном текущем репозитории нет.
Для LAN это не смена правил: две собственные NetworkMatch factories явно
передают прежнюю DB2. До production fix реальные NetworkMatch/Preparation
с новыми положительными guards прошли2/2 48.94с (оба host bases, replica,
переход к бою и обычная arena выбирают2).

Native GREEN1.03с: default/null/сохраняемый native invalid weak record ->1;
выбранные1/2/3 не заменяются. Существующий ChangeDifficulty2 и native global
save graph194bytes загружаются с2, даже когда default уже1. Это небольшой
полный CGlobalGame graph с unavailable scenario tracker, без world/mission UI.
Дополнительно реальный CICBeginGame для authored maps3/4/6 и четырёх выборов
(null,1,2,3) сохраняет roster, global-map state, выбранную difficulty и исходные
intro zones123/122/124; GREEN1.62с. Наблюдается настоящая очередь CICBeginMission
через существующий pointer-to-member accessor; команда начала миссии не
исполняется. Сохранения создаются только в изолированном тестовом profile,
копии игры/сборки не сохраняются. UI mapping кнопок остаётся отдельным02d85e5b.

Обе Game и Windows ALL_BUILD собраны. Windows расширенный436/436392.51с
включил все game-backed native menu/chapter/save/audio проверки:
34native-display/24native-audio/12FONT-COMPAT. Linux выполнил431 case успешно
за375.40с; ещё5 были NotRun из-за отсутствующих legacy executable. Ни
игрового падения, ни Linux436GREEN это не означает. Расширенный selector
впервые включил NativeMission{Map,RouteMap,UnitRouteMap,LargeMap}Probe и
NativeMissionScriptCorpusTests, которые прежний shared script не собирал.
Попытка собрать их обнаружила независимые старые portable link gaps:
IsLockerUnit из aiGrid и csSystem/CLogStream из lvm. Их portable dependencies
не менялись данным исправлением; их ремонт следует отдельным коммитом.
Evidence global-game-difficulty-linux-{extra-probes,script-corpus}-build.log.

Первый Linux compile отдельно отверг неявное CObj->CPtr copy-init тестового
invalid record; строка переведена на owner.GetPtr(), без production изменения.
Windows затем заново собран и выполнил финальный native case1.43с. Ошибка
сборки не считалась RED игрового бага. Raw LastTest.log у CTest ошибочно пишет
Test Passed даже для отсутствующих executable: для вывода о результате
использованы actual stdout и exit8, а не только raw счётчик436.

Windows7/7Network188.80с, включая361/361 authored encounter deployment+replica
132.70с при timeout600, сохраняют свои difficulty2 guards. CI-like284/28424.49с.
Raw436/436/7/284 полные без NUL; fonts12 в обоих широких прогонах и CI.
Linux raw436 включает5 NotRun, его не выдаём за436 выполненных. Новая asset
регистрация не расширяет resource-free CI. Evidence global-game-difficulty-
{full,linux-full,network,ci,final-portable-fixture}-native-test.log плюс CTest stdout.

Tracker281:96pending/117imported/24partially-imported/32already/3docs/
5excluded-balance/2excluded-own/1historical/1review —121 незавершённая запись.
Исходный баланс, выбранная сложность/старые saves и LAN ruleset сохранены.
HD, facial engine и HiRes fonts не менялись. Общая цель и final actual-HEAD
Actions остаются открыты; следующие шаги — отдельный ремонт portable probes
и оставшиеся difficulty mapping/save-policy hunks02d85e5b.


## Portable Lua: зависимость от журнала послеcc9fbeab

Это отдельный build-integration follow-up к уже перенесённому
[cc9fbeab94c206c59606fb91902c800c5690f0c9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc9fbeab94c206c59606fb91902c800c5690f0c9)
(наш исходный warning commit a4cba8ad). Lua VM пишет missing-global diagnostic
через настоящий csSystem/CLogStream. У standalone s2_game_lua не было link
зависимости от уже имеющегося s2_game_ai_log, хотя full Game эту реализацию имел.

До изменения реальные команды сборки NativeLuaRuntimeTests и
NativeMissionScriptCorpusTests завершились undefined references из lvm.cpp
к csSystem/CLogStream. CTest тех же двух целей вернул exit8/2NotRun. Это
воспроизведение build defect; ошибка линковки не выдана за compiled gameplay RED.
В PortableCore.cmake добавлена только transitive dependency на существующий
logger. Подмен, заглушек журнала и изменений Lua return/dispatch не добавлено.

Обе цели затем реально слинкованы, CTest2/2 GREEN0.95с. LuaRuntime выполняет VM,
ScriptCorpus обрабатывает113 DB scripts (один empty),355575bytes,
digestC2462A66D562BAF6; это не полный запуск миссий. Registration/readonly
inventory2/2 0.18с. Evidence portable-lua-logger-{baseline-build,baseline-test,
green-build,green-test,green-native-test,registration-test}. Ошибка сборки
NativeMissionMapProbe/IsLockerUnit — независимая и остаётся в следующем fix;
данный коммит не объявляет все436 Linux cases завершёнными.

Игровой код, оригинальный баланс, HD/textures/fonts и сохранения не менялись.
Статус cc9fbeab остаётся imported, общий tracker121 незавершённая запись.
Остальные source entries и final actual-HEAD GitHub Actions ещё открыты.


## Portable path lockers: link the real eligibility policy

Отдельный integration follow-up к
[48c244323ed01b3ba955474d8160e7fde2073c8b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/48c244323ed01b3ba955474d8160e7fde2073c8b)
(наш исходный import d9bd202f). Расширенный Linux прогон обнаружил, что настоящий
NativeMissionMapProbe не линкуется: grid ссылается на IsLockerUnit, который жил
в full-server debug translation unit. Четыре map cases были NotRun. Реальная
ошибка сборки сохранена в global-game-difficulty-linux-extra-probes-build.log;
она не объявляется compiled gameplay RED.

IsLockerUnit перенесён в уже связанный aiLocker.cpp и вызывает политику через
CUnit, как соседний IsBigLocker. Новый IsPathLocker по умолчанию false;
CUnitServer делегирует существующему CDumbUnitServer::IsLocker без изменения
его выражения. Виртуальные методы добавлены после прежних методов классов.
Сохраняются server eligibility, worn/empty PK и corpse правила; поля объектов,
serialization tags, lock counts, footprint и движение не менялись. Заглушек
server policy в диагностике и зависимости portable probe от всего Main нет.

После ремонта настоящие четыре map cases выполнились:4/4 GREEN3.81с.
После окончательного расположения новых virtuals заново собраны оба Game и
Windows ALL_BUILD, выполнены расширенные Windows437/437395.34с и
Linux437/437383.77с. В этих прогонах реально запущены NativeMissionMapProbe,
NativeMissionScriptCorpusTests и NativeLuaRuntimeTests; совместно с1db39d28
закрыты все5 ранее отсутствовавших portable diagnostics. Native locker tests
проверяют dead/overlap/moving/selection/flying/removed-player случаи.

Own LAN7/7 GREEN185.33с, в том числе361/361maps130.63с (deployment/replica,
не полные бои). CI-like284/28424.44с. Raw437/437/7/284 полные, без NUL,
Test Failed и Unable to find executable; fonts12 в обоих широких прогонах и CI.
Окончательные CTest stdout и exit0 подтверждают выполнение; прежний CTest raw
Test Passed при NotRun не использован как доказательство GREEN.
Evidence portable-locker-interface-{full,linux-full,network,ci}-native-test.log.

Это ремонт зависимости ранее перенесённой логики, а не повторный перенос
алгоритма. Оригинальный баланс, свои LAN/renderer/facial/HiRes fonts и HD
не менялись. Статус48c24432 остаётся imported; tracker281 и121 незавершённая
запись неизменны. Общий аудит и final actual-HEAD GitHub Actions ещё открыты.


## Side menu: forward the selected original difficulty

Независимый hunk
[02d85e5b939ac897f726293b37f30b55d0f2ff72](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72).
Текущий native menu действительно передавал enum ordinal как DB ID:
normal2 положительный контроль, затем hard1 прибывал в настоящее hero menu
как Easy1, а не Hard3. Compiled runtime RED1.77с сохранён до правки.

Независимый original oracle: GoldPDB CSideMenuInterface::ProcessEvent638da0,
RTTI/vtable slot7 -> Steam63e200 (descriptor97ed84, COL8ed95c,
vtable8c169c). Реальная ветка63e329..350 задаёт0->1/1->3/2->2.
Адрес63e1e3 из source comment для этого Steam exe не использован. База
подтвердила distinct DB1Easy/2Normal/3Hard с разными исходными коэффициентами.
Внесён только switch привязки; ordinal default, faction IDs, DB коэффициенты,
сохранения и LAN factories не менялись. Прозрачное влияние ошибочного hard1
и восстановленного hard3 описано в UPSTREAM-BALANCE-CHANGES.md.

Native GREEN2.15с выполняет реальные CICSideMenu/CICHeroMenu и оба SDL/bgfx
кадра, наблюдает выбранный record в настоящем hero interface. Обе стороны x
normal/hard/easy/default, no-side запрет и реальный modal return проверены.
Тест подаёт EVENT_NOTIFY кнопки и native next binding; физический клик,
полный запуск кампании/сохранение миссии этим тестом не заявляются.

Оба Game и Windows ALL_BUILD собраны. Related72/72 Windows73.70с,
Linux67.80с, по18native-display/4native-audio/12FONT-COMPAT. Свой LAN
NetworkMatch+Preparation2/2 GREEN45.65с с сохранёнными DB2 controls.
CI-like284/28424.51с; raw72/72/2/284 полные без NUL/missing executable/Failed,
fonts12 также в CI. Evidence side-menu-difficulty-{baseline,green,related,
linux-related,network,ci}-native-test.log и side-difficulty-retail-{lookup,
disassembly}. Resource-free CI count не увеличивался для asset/native-display.

02d85e5b остаётся partially-imported: live canSave policy и selected caption
style ещё открыты. Tracker121 незавершённая запись. Свои facial renderer,
HiRes fonts/adaptive UI и замороженный HD pack не изменены.


## Side menu: selected colors with the fork's existing HiRes font

Отдельный визуальный hunk
[02d85e5b939ac897f726293b37f30b55d0f2ff72](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72).
Compiled native RED1.40с: настоящий easy button уже в state3 после уведомления
и native кадра, но его CText использует normal11129 вместо selected17338;
disabled state тоже ошибочно содержит caption.

Особенности форка проверены по настоящим строкам DB:11129/11130/17338 все
Impact36pt, outlinesize2. Цвет текста соответственно FF935712/FFDB821B/
FFFFAB3F; outline FF3E2400/FF5C3600/FF784612. Меняются только выбранный стиль
цвета и пустой disabled caption. Размер/face/HiRes assets, adaptive reflow,
normal/hover, markup-before-brackets и facial renderer сохранены.

Original oracle: GoldPDB CSideMenuUI::ProcessMessage637910/4750,
RTTI descriptor96dbf4/COL8de180/vtable8b24bc slot10 -> Steam descriptor97ed64/
COL8ed900/vtable8c1604 slot10=63cd40. Например easy caption17329at63d902,
selected17338at63d91d, UTF16 bracket literals8c1664/8c165c, empty disabled
8b65b0; настоящая wrapper5c66b0 передаёт state arguments в AddButton5c61f0.
Последний независимо найден из Gold5c3550 first32/maskEH3:7 uniquely.
Evidence side-selected-style-retail-{lookup,disassembly,wrapper}.

GREEN2/2 3.50с: обе реальные SideMenu регрессии. Selected-style проверяет все
пять программных buttons (easy/normal/hard/axis/allies), реальное выбранное
state3 после кадра, CText selected/disabled, сохранённые normal/hover и
одинаковый font face/size. Это проверка native state/text/render path;
попиксельное соответствие цвета или физический клик не заявлены.

Оба Game/Windows ALL_BUILD построены. Related73/73 Windows75.93с,
Linux70.18с, по19native-display/4native-audio/12FONT-COMPAT. CI-like284/284
24.60с. Raw73/73/284 полные без NUL/missing/Failed, fonts12 и в CI.
Evidence side-selected-style-{baseline,green,related,linux-related,ci}-native-test.log.
Для color-only изменения сеть повторно не запускалась: свежие2/2 на f9b02bd0
и7/7 с361maps на f74ab551 уже GREEN. Balance rules не менялись.
Source02d85e5b остаётся partial, теперь открыт только live canSave policy;
tracker121 незавершённая запись, цельALL281 ещё выполняется.


## Mission: recompute original save permissions, preserve own LAN prohibition

Последний независимый hunk
[02d85e5b939ac897f726293b37f30b55d0f2ff72](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72).
Настоящая CMission810 после положительных ALWAYS/realtime controls продолжала
разрешать Save при realtime1/sequence1 и authored REALTIME difficulty.
Compiled runtime RED1.51с captured в final-baseline. Перед правкой настоящая
инициализированная LAN preparation прошла3.98с: difficulty2/savefalse.

Original oracle: GoldPDB CMission::GameStep5fda20/1079, vtable8ab2a4 slot87;
Steam RTTI descriptor97b76c/COL8e8d70/vtable8ba394 slot90=602160. Индекс
изменился с ABI; Steam slot87=696440 и короткий prefix не подошли и не
выдаются за совпадение. Проверена настоящая policy602204..255/60230a..341:
IsBase -> true; CHAPTER0 -> false; REALTIME1 -> IsRealTime && !IsSequence;
ALWAYS2 -> true; invalid/unknown сохраняют предыдущее значение. Вызовы50/44
соответствуют slot20 IsRealTime (Gold59fbe0/Steam5a24c0) и slot17 IsSequence
(Gold5f79d0/Steam5fbf50). Source VA6020f4 для этого exe не использован.
Evidence save-policy-retail-{lookup,disassembly}, save-policy-gold-disassembly.

Минимальный production fix —17 строк в CMission::InternalStep после refresh
current-player, перед обработкой UI commands, как original GameStep. Добавлен
обязательный own guard networkWorld/networkSession: штатный LAN savefalse
сохраняется и в подготовке, и при ALWAYS. Ни save fields/tags, ни gameplay
coefficients, ни facial/HiRes/HD системы не менялись.

Native GREEN2/2 6.40с использует настоящие SDL/bgfx frames/CMission/CWorld,
CUICmdBegin/EndSequence, GivePlayerTurn/EndOfTurn. Solo paused останавливает
физику, сохраняя реальные game-step/UI paths. Policy считается перед UI drain,
поэтому изменение sequence проверяется на следующем шаге. Реальная Save-menu
привязка открывает native CLoadView при запрете в TBS и CSaveView после realtime,
закрытие возвращает ту же mission. Отдельно создан настоящий scenario/base
campaign3 через GetZoneByDBZone и CICBeginMission: HQ разрешает даже во время
sequence и при CHAPTER. IsBase/IsRealTime/IsSequence не подменялись.

Текущая DB содержит только3difficulty: Easy1 canSave2, Normal2/Hard3 canSave1.
CHAPTER0 отсутствует; его ветвь проверяется только на отдельной полной native
serialized in-memory копии Easy, изменён только её canSave, authored DB не
мутируется. Первые fixture failures из-за неверного ожидания chapter record и
неисполненного queued OpenNetworkMission отдельно сохранены; они не gameplay
RED. После их устранения primary RED и LAN positive получены до production fix.
LAN fixture действительно открывает свой CMission/renderer/preparation, проверяет
savefalse при ALWAYS/REALTIME/ALWAYS, затем восстанавливает обычную DB2.
Полные сетевые бои/физический F-key этими двумя cases не заявляются.

Оба Game и Windows ALL_BUILD собраны. Свежие расширенные Windows441/441
405.41с и Linux441/441388.57с: по38native-display/24native-audio/12FONT-COMPAT,
включая обе SideMenu regression и portable map/Lua diagnostics. Own7Network
184.99с, в том числе361/361maps129.57с (deployment+replica, не полные бои).
CI-like284/28424.49с, fonts12. Raw441/441/7/284 полные, без NUL/missing/Failed;
actual CTest exit0 подтверждён. Evidence mission-save-policy-{final-baseline,
final-controls,full,linux-full,network,ci}-native-test.log и соответствующий stdout.
Ранее существующие Save/QuickSave native fixtures также прошли без ослабления.

Source02d85e5b теперь imported: все его независимые hunks завершены. Tracker281:
96pending/118imported/23partial/32already/3docs/5excluded-balance/2excluded-own/
1historical/1review —120 незавершённых записей. Общая цельALL281 и проверка
final actual-HEAD GitHub Actions остаются открыты; не выдаём эту часть за конец.


## Original reload action price: restore perk33 before AI integration

Первый независимый адаптированный hunk
[b875e43ddec65e0375d1335eb8d8726e38b4934a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b875e43ddec65e0375d1335eb8d8726e38b4934a).
Наш AC_RELOAD возвращал только raw weapon cost и не применял perk33.
Compiled native RED1.02с: настоящий CUnit(persona3)/IUnitMission, effective
CWeaponItem65; положительный no-perk40, затем реальный TakePerk33/Param0.3
даёт actual40 вместо исходных12.

Original oracle: GoldPDB CUnitMission::GetActionAP6bac90/3724 first32,
mask только EH9:13 -> уникальный Steam6c0d30. Настоящий jump table6c1a48,
action23 AC_RELOAD ->6c13d2. Выбранное оружие передаётся raw query6bb8a0;
HasPerk33at6c13ef/6c13f3, умножение Param6c1407, truncate6c1416 и прямой
integer return6c1932. Floor1AP/вычитания из1 нет. Source VA6c1443 не соответствует
этой ветке нашего Steam. Evidence reload-action-cost-retail-{lookup,disassembly}.

Добавлены nonvirtual CUnit::GetWeaponReloadActionAP(optional selected weapon)
и именованный constant33, player AC_RELOAD делегирует helper. Он применяет
оригинальный Param1 без clamp. Существующий raw GetWeaponReloadAP и consumers
AI shot forecasts сохранены. Upstream1-Param/min1 не переносились. Влияние
40->12 вместо upstream28 и нулевой край прозрачно описаны в BalanceMD.

Native GREEN0.99с проверяет3poses, effective/explicit weapon, raw price и
юнит AP неизменны от запросов. Край raw1->action0 — полная отдельная native
in-memory копия DB weapon, меняется только её raw reloadAP; authored record
сохраняется. Unarmed0 подтверждён реальным пустым active slot; если у persona
есть исходный предмет, тест снимает и возвращает его через TakeOff/Equip/
Activate с проверкой identity. Это native cost API, не полный reload executor,
анимация, физический ввод или сериализация всей миссии.

Первые fixture compile failures pDefaultClip/NAI::STAND/LAY заменены реальными
pInnerClip/WALK/CRAWL по заголовкам; это не gameplay RED. Сначала unarmed setup
ошибочно игнорировал исходное оружие; Take действует на backpack, корректный
API TakeOff. Затем Linux выявил, что CDBIterator использует unordered_map:
он выбрал persona265/weapon127(raw12->3), с пустым исходным слотом, тогда как
Windows выбрал3/65(raw40->12). Исправлена только подготовка возможного empty
slot; проверка actual empty/current-null/action0 осталась обязательной.
Первый Linux full441/442387.52с с этой единственной fixture failure сохранён
в retail-reload-action-linux-first-full-native-test.log. Он не заявлен GREEN.

Оба Game/Windows ALL_BUILD построены. Windows full442/442407.57с, затем
окончательный portable fixture+registration3/3 1.38с. Linux после исправления
fixture targeted3/3 1.11с и заново полный442/442391.01с. По38native-display/
24native-audio/12FONT-COMPAT. Own7Network189.30с, в том числе361/361maps131.08с
(deployment+replica); CI-like284/28424.59с, fonts12. Raw442/442/7/284 полные
без NUL/missing/Failed; actual exit0 подтверждён. Evidence retail-reload-action-
{native-baseline,native-edge-controls,final-portable-fixture,full,linux-full,
network,ci}-native-test.log; окончательный Linux stdout в
retail-reload-action-final-linux-test.log. Дополнительных копий сборки не создавали.

Sourceb875e43d pending -> partially-imported. Native AI GetInfo/Do consumers
ещё не менялись: их использование выбранной action price требует отдельного
RED/GREEN. Tracker281:95pending/118imported/24partial/32already/3docs/5balance/
2own/1historical/1review —120 незавершённых записей. ЦельALL281/final-HEAD
GitHub Actions открыты; исходные perk coefficients/сохранения/LAN/fonts/facial/HD
сохранены, исключённый ребаланс33 и минимум1AP остаются исключёнными.


## AI reload: use the selected original action price in both planning paths

Второй независимый hunk
[b875e43ddec65e0375d1335eb8d8726e38b4934a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b875e43ddec65e0375d1335eb8d8726e38b4934a)
после52c57831. Два compiled runtime RED: availability1.13с и spend1.14с.
Native FixtureWorld, настоящий AI commander/player, зарегистрированный CUnitServer
с authored persona265, реальный CAIUnit/CAIInventory. Controlled current slot1:
weapon65 raw40/action12; holstered slot2: weapon127 raw12/action3, единственное
reload-eligible оружие с настоящим spare clip. No-perk budget12 positive,
затем настоящий TakePerk33/Param0.3. Бюджет3 ошибочно отвергался; при запасе12
Do списывал12 вместо3, planningAP12->0 вместо9, physicalAP47 не менялся.

Original oracle: GoldPDB GetInfoInner41be10/315, Do41cfe0/369; first32,
maskEH3:7 -> уникальные Steam41c8b0/41da80. Actual GetInfo41c9a2 и
Do41db6e передают action23 и selected item в unit vslot5, calls41c9b0/41db7c;
результат идёт в affordability/SpendAP. Gold named CAIUnit::GetActionAP
4abd20/11 находится в vtable8a6984 slot5, Steam vtable8b5a6c slot5=4ad500,
все11bytes8b01ff50048b108bc8ff22 совпадают: передача в RPG mission.
Source VA41c0d0/41d2a0 здесь не использованы. Evidence ai-reload-action-retail-
lookup.txt и ai-reload-action-{info,do}-retail-disassembly.txt.

Production — только CAIReloadAction GetInfo/Do: guards реального RPG unit и
GetWeaponReloadActionAP с конкретным pWeapon->GetItem(). Do получает цену
до изменения planning hand. Собственные ownership/reentrant guards, raw wrapper
GetReloadAP/shot forecasts, world executors и численные параметры не менялись.
Используется уже отдельно проверенная original Param formula/no1AP floor,
а не upstream1-Param/min1. Сохранения/HiRes/facial/HD не менялись.

Native GREEN8/8 5.63с включает оба новых случая, player price, AIReloadLog
wire/rollback/ownership, ReloadActionWire и MountedReloadWeapon. Availability3
принимается,2 отвергается; Do planningAP12->9, planning selected hand и full
clip обновлены. Физические AP, active weapon и ammo остаются исходными.
Это native planning/инвентарное зеркало; полный физический AI turn, animation,
input и whole-save proof этими cases не заявляются. Первый fixture compile
error вызвал private AddUnit(path) overload; исправлено на штатные публичные
CUnitServer constructor + world.AddUnit(server), как существующий native path;
он не выдаётся за gameplay RED.

Оба Game/Windows ALL_BUILD построены. Fresh444/444 Windows409.72с и
Linux390.77с, по38native-display/24native-audio/12FONT-COMPAT. Own7Network
184.66с, включая361/361maps129.88с (authored deployment+replica, не полные бои).
CI-like284/28424.46с, fonts12; raw444/444/7/284 полные, без NUL/missing/Failed,
actual exit0 подтверждён. Evidence ai-selected-reload-{native-baseline,green,
full,linux-full,network,ci}-native-test.log и соответствующий stdout.

Sourceb875e43d partially-imported -> imported, ADAPTED к оригиналу. Исключённые
70%/minimum1 по-прежнему не переносились, BalanceMD обновлён. Tracker281:
95pending/119imported/23partial/32already/3docs/5excluded-balance/2excluded-own/
1historical/1review —119 незавершённых записей. Общая цель и final actual-HEAD
GitHub Actions остаются открыты.


## Empty models: original nonblocking geometry instead of a sphere

Source [1b9664486352df69fc5e893213d6c4e552ac051b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1b9664486352df69fc5e893213d6c4e552ac051b).
Compiled native RED1.20s uses the actual authored game.db, not a synthetic
replacement: Models table1 contains30 null GeometryID variants. Variant433,
template351, imported as sphere6/AItraffic23 instead of default4299.
Explicit valid variant5/geometry1/template identity and the existence of4299
were independently checked before the failing assertion. The production
CRndModel::Import is active in native portable/legacy database deserialization
on both platforms, not only an old editor path.

Original oracle: immutable Steam Game.exe SHA256
4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705.
GoldPDB CRndModel::Import7edfe0/304 first32, mask EH3:7, uniquely maps to
Steam7dc560. Actual null/ref-invalid gate7dc649..653 leads to immediate4299
at7dc657, GetGeometry7f1550 call7dc65c and assignment. Exact and incorrect
EH9:13 signatures do not match; source VA7dbf67 is not trusted for this binary.
Evidence empty-model-fallback-retail-{lookup,disassembly}.txt.

Minimal production change only replaces the missing-model fallback6 with4299.
Actual authored database records, explicit sphere6 and all explicit valid
geometries stay intact. Native GREEN4/4 4.41s includes registration, readonly
inventory, PartialReferenceImport and the new fixture. All30/30 authored null
variants receive4299 with original source-template identity. Real default
AIgeometry2728 has flags0/no secondary; linked NWorld::GetMask yields0x8,
while explicit sphere6 yields0x8710. Checked blocking mask0x8720 covers
movement/weapon/item/cover/vision; it is absent for every empty variant using
its actual RPGArmor argument. Explicit variant5/geometry1 remains unchanged.
This proves native database/model/object-mask behavior, not full physical
ray tracing, path walking, rendered pixels or whole-save compatibility.
No font/facial/HD/backend, numerical rule, RNG or save-field changes.

Both Game/Windows ALL_BUILD built. Fresh445/445 Windows401.55s and
Linux390.43s, each38native-display/24native-audio/12FONT-COMPAT. Own7Network
186.22s including361/361maps130.38s (deployment+replica). CI-like284/28424.60s,
fonts12; raw445/445/7/284 complete with no NUL/missing/Failed, actual exit0.
Evidence empty-model-fallback-{baseline,native-mask-controls,full,linux-full,
network,ci}-native-test.log and corresponding stdout. Shared builds reused;
no extra game/build copies or HD texture generation.

Source1b966448 pending -> imported. Tracker281:94pending/120imported/
23partial/32already/3docs/5excluded-balance/2excluded-own/1historical/1review,
118 unfinished entries. Overall ALL281 goal/final actual-HEAD hosted CI remain
open; this individual source is complete.


## Blocked walking: retain the scheduled trajectory after an earlier turn

Source [ff91745d9aa43573ee04444745b1f9770578ba7f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/ff91745d9aa43573ee04444745b1f9770578ba7f).
Our StartMove does not set bStandIfRecalcCommand, contrary to the upstream
comment; Rotate does, and EndRotate leaves it until EndMove. Applicability was
therefore reproduced using an actual preceding native rotation, not assumed.

Native FixtureWorld, real FindPath from the opposite facing, actual CExecMove
and an unarmed CancellingMoveActor using its base CanFight behavior reach a
real walking CCmdMove. Another actual human unit has a running nonwaiting
native movement executor. Ordinary SetPosition releases the walker's old
future reservation and force-places the blocker at the pending destination.
This models occupancy after route construction; no CanDoGameMove stub is used.
The real query returns CMR_LOCKED and identifies that blocker. Deterministic
clock1000+AlignTime and actual native animation-finished callbacks are used.

Compiled runtime RED initially reproduced the extra300ms. The strengthened
final test was rerun against the complete original production wUnitMove.cpp
from HEAD: RED1.17s total, native animationEnd1635->1935 and the actual
CSkeletonAnimator/CASequence scheduled animator replaced. The mover remained
waiting with the same current command, logical position, AP49 and VP. Native
GREEN19/19 24.23s includes cancellation, released-owner, wait, completion,
item visibility, holster policy, camera and carried-corpse cases. Both final
platform runs confirm1635->1635, identical scheduled animator, extraDelay0.
This is native movement/animation graph and timing proof under forced live
occupancy, not a rendered battle, physical input or whole-save E2E claim.

Original oracle: GoldPDB CheckCanDoMove7c0420/245 first32, masking only
rel32 call operand14:18, uniquely maps to Steam7ccf20. Actual7ccf47 calls
vslot14; only true reaches stand-flag+c2 and EndRotate73dcf0 call7ccf6e.
Gold CExecMove RTTI descriptor9740c4/vtable8be784 slot14=7b0710 named
IsRotating; Steam descriptor985354/vtable8cdd6c slot14=7bcf50. Actual32-byte
body casts current command+34 from CObjectBase RTTI94f860 to
CCmdRotate98525c and returns nonzero. Source VAs are not trusted. Evidence
blocked-walking-retail-{lookup,disassembly}.txt and
blocked-walking-rotation-retail-disassembly.txt.

Production only adds the rotation query and gates the existing EndRotate.
Own DoCommand owning holds before reentrant move checks, failed-AI cleanup,
interrupted human route behavior, full locker-chain walk, AP rules and saved
fields/IDs remain intact. Existing legacy end-command interpretation stays.
No font/facial/HD/backend, numeric balance, RNG or asset changes.

Fixture development failures were kept distinct from gameplay RED: SPathPlace
has equality but no !=; initial path had no rotation; a merely prepared mover
has no conflicts remover in TBS, so it must actually run; EndOfTurn alone did
not establish real time in this fixture. The final fixture uses a running
blocker, actual rotation and an explicitly aligned clock. Earlier exploratory
clock values are not presented as deterministic timings. No production fix
was inferred from these fixture setup failures.

Both Game/Windows ALL_BUILD built. Fresh446/446 Windows401.94s and
Linux394.42s, each38native-display/24native-audio/12FONT-COMPAT. Own7Network
186.76s including361/361maps131.13s (deployment+replica); CI-like284/28424.55s,
fonts12. Raw446/446/7/284 complete, no NUL/missing/Failed, actual exit0.
Evidence blocked-walking-{final-native-red,native-green,full,linux-full,
network,ci}-native-test.log and corresponding stdout. Shared builds reused,
no per-bug game/build copies or HD generation.

Sourceff91745d pending -> imported. Tracker281:93pending/121imported/
23partial/32already/3docs/5excluded-balance/2excluded-own/1historical/1review,
117 unfinished entries. Overall ALL281 goal and final actual-HEAD hosted
GitHub Actions remain open.


## Movement collision: reveal another owner's hidden blocker immediately

Source [1d260f7faa7d44b32cc35c952447adb5157f21a9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1d260f7faa7d44b32cc35c952447adb5157f21a9).
Compiled native RED1.11s: actual CExecMove reaches a real CCmdMove, native
CanDoGameMove returns CMR_LOCKED and identifies an actual other-player unit
force-placed into its future route cell. The blocker remains hidden, emits0
CEventOnUnitUnhide and does not force visibility. Both independent positive
controls already pass before the production fix: same-owner hidden unit stays
hidden/event0, other-owner visible unit stays visible/event0. Native baseline
3 tests total3.36s includes that single real failure, not a compile failure.

The native fixture uses FixtureWorld/FindPath and the ordinary unarmed
CancellingMoveActor base behavior, real SetPosition/EnableHide/Hide, a live
NGlobal::CEventRegister receiver counting only its target's actual unhide
event, and the real world/RPG game. It restores the world's existing saved
coalescing flag53 via the public CObjectBase virtual saver API; no production
visibility override is used. A deliberately stale night mirror in native
CGame is read through its real saved field5. Ordinary UpdateVisible(false)
first demonstrably defers, retaining that mirror. Only the revealed collision
must force the native sweep and resynchronize it to the actual world TOD.
This verifies immediate native refresh execution under restored coalescing
state; it does not claim an actual complete CWorld::Segment, full AI turn,
rendered enemy visibility/cache, physical input or whole-save E2E proof.
The initial private CWorld::operator& fixture access failed to compile and
was corrected to the existing public base virtual API; it is not gameplay RED.

Original oracle: GoldPDB CheckLockerState7c0190/646 first32 EH3:7 uniquely
maps to immutable Steam7ccc80. Actual IsHiding7cccf1 precedes GetPlayer
comparison7ccd05/7ccd0f; unequal ownership calls Hide750260 with false/true
at7ccd1b, then world vslot65 with true at7ccd2e. Gold named Hide744c70/205
first32 EH9:13 uniquely maps to750260. Unit RTTI offset332/vtable Gold8beb74
slots3/13 name IsHiding7b3600/GetPlayer7b3930; Steam8ce15c slots3/13 are
7bfd80/7c00b0. Gold World primary vtable8bc74c slot65=75bb50 names
UpdateVisible, Steam primary8cbc8c slot65=7673d0. That callee's first32
signature did not match, so its mapping is through RTTI/vtable rather than a
false unique-signature claim. Evidence collision-hidden-retail-
{lookup,disassembly}.txt. Source addresses are not trusted. Ownership, rather
than hostile diplomacy alone, is the actual original condition.

Production only holds the locker with CPtr, unhides with its ordinary event,
and calls the existing forced visibility refresh before walking the locker
chain. Our deliberate full cycle walk, movement lifetime safeguards, saved
fields/IDs and own network/visibility implementations remain. No AP/range,
perk coefficient, RNG formula, database, asset, font/facial/HD or backend edit.
The restored sweep uses the existing normal perception pipeline.

Native GREEN30/30 32.24s includes all3 new cases, movement, hiding, explicit
Hide state and AI event collection locks. Final Windows and Linux both report
other-hidden -> hidden0/event1/forced1; own-hidden -> hidden1/event0/forced0;
other-visible -> hidden0/event0/forced0. Both Game/Windows ALL_BUILD built.
Fresh449/449 Windows405.27s and Linux397.12s, each38native-display/
24native-audio/12FONT-COMPAT. Own7Network184.26s including361/361maps129.74s
(deployment+replica); CI-like284/28424.62s, fonts12. Raw449/449/7/284 complete,
no NUL/missing/Failed, actual exit0. Evidence collision-hidden-
{native-baseline,native-green,full,linux-full,network,ci}-native-test.log and
corresponding stdout. Shared builds reused; no extra game/build copies or HD
generation.

Source1d260f7f pending -> imported. Tracker281:92pending/122imported/
23partial/32already/3docs/5excluded-balance/2excluded-own/1historical/1review,
116 unfinished entries. Overall ALL281 goal and final actual-HEAD hosted
GitHub Actions remain open; this source is complete.


## Cannon approach: play the original body entry clip with stance-only flags

First independent hunk of [f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86).
Native FixtureWorld810, actual CCannon/GetApproaches/CanDoIt and CExecCannon.Run:
ordinary armed persona838/personal weapon30 has body flags0x1001; actual
cannon weapon65 uses DShK12.7mm. Original stance-only lookup resolves authored
START_ATTACK clip3947; native resource loader independently loads833ms.
Compiled RED1.17s: no cannon entry clip in the real body skeleton graph and
actual duration0. The fixture accepts both authored clips in that cannon's
START_ATTACK/name family, rather than assuming roulette always picks the
same variant. Earlier RED1.88s had the same missing-clip cause. Initial bad
IUnitMission.IsUsingCannon fixture API was corrected to GetCannonItem; that
compile failure is not gameplay RED. No DB fields/assets were changed.

Original oracle: GoldPDB CUnitAnimator::EnterCannon731cf0/472 first32,
maskEH9:13, uniquely maps to immutable Steam73ca30. Actual pose jump73cae9,
table73cbf8 maps CRAWL0->73cafe(flag4), CROUCH1->73caf5(flag2), WALK2/RUN3
->73caf0(flag1 via EBP=1 at73ca5f). Pose enum is not guessed. Explicit mounted
name is passed at73cb1d/73cb2b, START_ATTACK type6 lookup call7dc4f0 at73cb2f.
Evidence cannon-entry-retail-lookup.txt and
cannon-entry-animation-retail-disassembly.txt; source VAs are not trusted.

Production is only the stance-only switch in CUnitAnimator::EnterCannon.
Current Run already supplies the mounted name, so no public API/header,
AP/state/occupancy timing or completion changes are bundled here. This is
body CSkeletonAnimator, not our LSHead facial system. Save fields/IDs,
HiRes fonts, HD assets/build rules, RNG formula and balance numbers remain.

Native GREEN Windows40/40 44.68s and Linux40/40 43.78s include movement,
mounted UI/reload, corpse/ragdoll, hidden body and torso cases. Both native
runs play clip3947 for833ms; the test also checks stance-only field1 and a
finite actual hip pose at the clip midpoint, then completes and leaves the
actual mounted state with no active action retained. This is native executor,
resource/animation graph and sampled skeleton proof, not GPU pixels, complete
AI turn, physical input or whole-save compatibility proof. Both Game builds
pass; CI-like284/28424.23s (12FONT-COMPAT). Raw40/40/284 complete with no
NUL/missing/Failed, actual exit0. Evidence cannon-entry-animation-
{final-native-red,native-green,linux-related,ci}-native-test.log and stdout.
Focused checks suffice for this isolated body selection change; no new full450
or361-map run is claimed. Shared builds reused, no old build copies/HD generation.

Sourcef2ed3ca5 pending -> partially-imported: completion/AP/item/state/
occupancy timing and unconscious/dead exit still require separate native TDD.
Tracker281:91pending/122imported/24partial/32already/3docs/5excluded-balance/
2excluded-own/1historical/1review,116 unfinished entries. Overall goal and final
actual-HEAD hosted CI remain open.


## Cannon entry: commit the original action only after approach completion

Second independent hunk of [f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86), afterde0bccf6.
Native compiled RED1.15s: actual833ms approach begins, but AP46->42/cost4,
RPG mounted item, cannon occupancy and state3->7 commit immediately. A separate
actual historical already-paid graph already passes its baseline completion
control1.01s, AP42->42. No fixture compile/setup failure is used as this RED.

Original oracle: named Gold CExecCannon::Run79a4e0/357 maps via class RTTI
9735a8/vtable8bd63c slot6 to Steam descriptor984818/vtable8ccc24 slot6=7a6990.
First32 Run signature does not match because of RTTI operands; no false unique
claim. Gold named AnimationFinished79a650/296 slot8 maps to7a6b00 and its
first32 also matches uniquely. Actual Run7a6a23 calls EnterCannon73ca30 with
explicit name, cannon and position, then starts the action without mounted
commit. Actual completion pushes action25 at7a6b5f, calls DoAction74f500 at
7a6b61, sets RPG cannon item at7a6b75 and then name/occupancy/state. Evidence
cannon-entry-retail-lookup.txt and cannon-entry-{run,completion}-retail-
disassembly.txt. Source VAs are not trusted; no numerical AP rule is replaced.

Production Run now schedules approach with an explicit mounted animation
name; completion commits the existing action25, RPG item, stored name,
occupant and using-cannon state. The original cost4 in this fixture is retained.
Entry field IDs and save types stay unchanged. An adaptation detects the
complete already-mounted phase of our older saves (matching RPG item, actual
occupant and state) and finishes without charging their already-paid entry.
It does not skip new unmounted entries. Existing dead/unconscious exit behavior
is still pending its independent fix. Own LSHead/fonts/HD/backend are untouched.

One actual pre-fix native fixture1182638bytes was emitted by historical Run
with productionde0bccf6 before modifying completion. SHA256
05c3307daa22fbb9799b1acd68e73a23db82ffb3f6b943ab43bf25e4aca549d1.
The generator requires actual already-paid/mounted/running state before it
opens output, so the new implementation cannot silently regenerate/bless it.
Details in fixtures/cannon-entry-before-deferred.md. This single1.13MiB
regression graph is not an old game/build copy; the same builds were reused.

Native GREEN Windows42/42 47.03s and Linux42/42 45.78s. New entry stays
AP46/state3/unoccupied during approach. A full independently restored
CGlobalGame/world/actor/cannon/executor graph retains that pending phase;
both loaded and original live executors commit AP46->42/state7 only on the
actual AnimationFinished callback. The unchanged historical Windows graph
also loads on Linux and completes AP42->42, retaining item/occupant/state.
This is native graph restoration and executor callback proof, not save-slot
UI loading, full physical input or a completely scheduled game frame.
Both Game builds pass; CI-like284/28424.27s with12FONT-COMPAT. Raw42/42/284
complete, no NUL/missing/Failed, actual exit0. Evidence cannon-entry-completion-
{legacy-generator,native-baseline,native-green,linux-related,ci} logs. No fresh
full452 or361-map run is claimed for this focused phase; broad regression follows
the remaining exit hunk. No HD generation, numerical DB edits or extra builds.

Sourcef2 remains partially-imported until dead/unconscious exit is tested.
Tracker281 unchanged:91pending/122imported/24partial/32already/3docs/
5excluded-balance/2excluded-own/1historical/1review,116 unfinished. Overall goal
and final actual-HEAD hosted CI remain open.


## Cannon exit: preserve the downed actor's animation context until world sync

Third independent hunk of [f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86), afterde0bccf6/f47b06f6.
Two compiled native REDs: unconscious1.16s and dead1.13s; living exit positive
control1.16s already passes. After a real completed mounted entry, a pending
native CExecCannon exit crosses the actual RPG-to-world seam: fully initialized
CAttackPortion + native MakeClickOfDeath/ProcessAttack produces the mission's
unconscious result with1 hit or dead result with2, while world state is still
using the cannon. This is not a mocked IsUnconscious/IsDead or a private flag
patch. The normal exit then wrongly adds300ms of standing, replaces the active
CASequence animator and clears the mounted animator context. Exit AP stays
42->42 or37->37 after the RPG damage result; no exit price change is involved.

Original oracle: actual Steam Run7a6990 exit calls RPG vslot77 at7a6a44,
then vslot38 at7a6a56; either true skips to7a6a7b, only a living actor calls
LeaveCannon73ffe0 at7a6a76. The IUnitMission interface is at offset76(0x4c),
not its CObjectBase primary table. Gold vtable8b63cc slot38=6b8ae0 named
CUnitMission::IsDead/11, slot77=6bf5e0 named IsUnconscious/7; Steam interface
8c54e4 slot38=6beed0/slot77=6c5d70. The unrelated primary slots were explicitly
discarded during lookup. Evidence cannon-entry-retail-lookup.txt and
cannon-entry-run-retail-disassembly.txt. No source VA is assumed correct.

Production only gates the existing LeaveCannon call on the two real RPG
states. Logical item/occupant/state detachment remains; no death/critical,
ragdoll, face, AP/VP formula, weapon stats, save fields or own renderer changes.
A downed actor keeps its active animator, end time and mounted marker until
actual SyncConscious. That native synchronization then installs a real
CParticleSkeleton at1833/1833 (no intermediate death clip), with actual unit
owner verified through field11. Existing own ragdoll/impact handling is used
unchanged. Living exit still transitions to standing for300ms and clears the
marker. The test checks health, AP and position at the exit boundary before
world synchronization; it does not claim a whole combat turn/input/rendered
frame or save-slot UI proof.

Final native related45/45 50.95s. Initial fixture CRay two-argument constructor
was corrected to the actual field API; no compile error is claimed as RED.
An initial post-fix43/45 run passed the new graph/context/ragdoll assertions,
but its cleanup incorrectly demanded no world action after SyncConscious.
The native synchronization creates its own world action window; exit-counter
release is now checked before that phase. No production counter was altered
and no exit invariant was dropped. That failed run remains separately saved.

Both Game/Windows ALL_BUILD built. Fresh full455/455 Windows421.13s and
Linux400.85s, each38native-display/24native-audio/12FONT-COMPAT. Own7Network
184.95s including361/361maps130.78s (deployment+replica); CI-like284/28424.59s,
fonts12. Raw455/455/7/284 complete, no NUL/missing/Failed, actual exit0.
Both full runs also verify source and restored deferred entry, immutable old
Windows saved phase on Linux (AP42->42), actual body clip, and the three exit
controls. Evidence cannon-exit-incapacitated-{native-baseline,final-native-
green,full,linux-full,network,ci}-native-test.log and stdout. Shared builds
reused; no old game/build copies or HD generation.

All functional sourcef2ed3ca5 hunks are now imported, including explicit entry
name, stance flags, completion and downed exit. The legacy-save guard is our
necessary adaptation; an unused local removal in the source carries no
behavioral change. Tracker281:91pending/123imported/23partial/32already/3docs/
5excluded-balance/2excluded-own/1historical/1review,115 unfinished entries.
The overall ALL281 goal and final actual-HEAD hosted CI remain open.


## b4505fb0 — original EndRotate save command (consumer)

Source b4505fb0b183854ea69f680ff770759c5245cebb, independent first hunk.
The original EndRotate type72312170 was unregistered; loading an actual file
CStructureSaver game/world graph lost that command. Compiled native confirmed
RED: original wire failed1.18s; the old fork null-EndMove control passed1.17s
(total2.35s). The test rewrites only the selected native Cancel descriptor and
wraps its identical CCmdUnit payload in original tag1. It retains every native
wire ID and the full strong game/world/actor/mover roots, and subsequently
executes the loaded command in the actual mover, rather than checking only a
factory/type registration. The legacy control writes its actual null tag2.

Add the original class, tag1 base serializer, registration and animator dispatch.
Keep the previous fork's null-EndMove branch and saved ID12541120. Neither
producer queues nor SetNewPath transition logic is changed by this commit;
those independent hunks still require their own failing behavioral tests.
Native GREEN: both files load with the actor reference intact, schedule300ms
stand, finish, preserve AP53 and position, and release the action when retired.
This exercises native graph loading and executor callbacks, not the save-slot
UI or a complete scheduled game turn. No balance, action price, font, face,
renderer, audio or HD changes; no per-bug game/build copies or new binary fixture.

Independent original evidence: GoldPDB initCCmdEndRotate0x72312170 @9b7bbc;
Steam registration literals8ac7d2/8ac7e5. Original RTTI9853e8/vtable8cdc1c
slot3 is serializer7bd420: only tag1 CCmdUnit. Gold DoCommand7ac9c0/1244
maps uniquely to Steam7b91b0 with only EH operand3:7 masked. Its call7b945e
to cast7bb530 uses the EndRotate descriptor9853e8; successful branch7b9463
calls animator73dcf0 at7b9473. Gold EndRotate732fb0/44 maps uniquely to
73dcf0 with only first call operand13:17 masked (other40bytes identical).
The initially mistaken EH mask9:13 found no match and is not proof.
Evidence move-end-rotate-{retail-lookup,type-retail-disassembly,
wire-retail-disassembly,cast-retail-disassembly,dispatch-retail-disassembly,
animation-retail-disassembly}.txt under the shared evidence directory.

Fresh related native Windows25/25 31.19s and Linux25/25 30.74s; both Game
builds. Both original and legacy format execute successfully on both platforms.
An early test-only queue replacement omitted its saved FINISHED result (the
saver zeroes missing primitives), so the legacy control reported FAILED despite
correct300ms animation. That fixture was corrected before the confirmed RED.
The first related run also caught missing Linux CMake registrations; both new
cases now match across platforms and the registration check passes. No engine
change was made to conceal either test setup issue. Evidence movement-end-rotate-
confirmed-red-{test,native-test}.log, final-raw-native-test.log, linux-{test,
native-test}.log and corresponding build logs. Tracker281 now90pending/
123imported/24partial/32already/3docs/5excluded-balance/2excluded-own/
1historical/1review;115 unfinished entries. Whole ALL281 goal remains active.

CI-like284/284 24.14s, FONT-COMPAT12. Raw25/25/284 are complete with no NUL,
failed or missing executable; actual runner exits0. This is local validation;
final actual-HEAD hosted GitHub Actions remains separately open.


## b4505fb0 — keep the last walking step when rerouting to a pose/turn

Source b4505fb0b183854ea69f680ff770759c5245cebb, independent movement
transition hunk. An actual native mover reaches its pending next step, then
native FindPath(PF_USE_POSEDIR) requests crouch on that destination. Compiled
RED1.19s (case1.12): the callback moves tile2,1 ->3,1 and spends AP49->45,
but a newly queued null-EndMove is interpreted as EndRotate. Only stand687
is added (distance0/time0), duration300ms, no final movement trajectory.

Keep the interrupted CCmdMove in the ender for both pose and turn branches.
A strong hold before a waiting queue can clear pCurCmd prevents introducing a
dangling reference when constructing that owning ender. The test does not
claim a separate UAF reproduction; it holds snapshots for trajectory inspection.
Native GREEN1.17s (case1.13): actual new clip686 distance1.514869/time710,
combined step+stand573ms, same tile/AP49->45, followed by real completed crouch.
The comparison retains old clip objects and excludes them from its new-clip
assertion, so earlier walking animation cannot produce a false positive.
This is native planner/executor/skeleton-graph proof, not a GPU/full game-turn
capture. Actual callbacks and state completion are checked; no mock path or
replacement animator is used. Two initial fixture failures were fixed first:
normalize the transient moving bit, and request PF_USE_POSEDIR instead of
PF_DEFAULT, which legitimately ignores the desired final pose. They are not RED.

Independent original: Gold SetNewPath7ae8e0/754 uniquely maps to Steam7bb0c0
with first32/EH3:7. Actual native move descriptor98523c and rotate98525c verified.
Old-move branch7bb27e passes that move to ctor7bbb80 at7bb293, then inserts
it at7bb2b6; turn-start branch calls the same ctor at7bb236. Ctor7bbb80 reads
the move's unit, AddRefs the move at7bbbae and stores it atender+0x10 at7bbbb8.
Its vtable8cdc04 resolves to CCmdEndMove RTTI9852bc. Current-command hold is
also present at7bb0ed. Evidence move-reroute-{retail-lookup,retail-disassembly,
ender-retail-disassembly}.txt. No balance formulas, animation code, own heads,
renderer, audio, fonts or HD assets changed. Shared builds reused.

Source remains partial: waiting reseat/extra ender, direct movement continuation
and new EndRotate producers require separate native TDD. Counts unchanged:
281 entries,115 unfinished. Windows related26/26 31.88s and Game build passed.

Linux related26/26 31.12s with Game built; CI-like284/284 24.38s. Raw26/26/284
complete, no NUL/missing/Failed, actual runner exits0. Evidence movement-reroute-
final-step-{confirmed-red,green,related,linux,ci} logs in shared evidence folder.
