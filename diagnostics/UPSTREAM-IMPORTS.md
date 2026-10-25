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
