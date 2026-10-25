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
баг нет; сохраняются только небольшие текстовые свидетельства RED/GREEN.

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

## Продолжение

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
