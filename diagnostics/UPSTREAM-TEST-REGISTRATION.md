# Регистрация регрессионных тестов в CI

Локальная ошибка CMake скрывала 31 тест, которому не нужны игровые ресурсы,
за условием наличия установленной игры. В GitHub Actions эта переменная пуста.
Сравнение исходных списков Windows и Linux проходило, поскольку ошибка была
одинаковой на обеих платформах.

Сначала расширена существующая UpstreamRegistrationTests проверкой реального
списка CTest. В конфигурации с пустым S2_GAME_DIR получен runtime RED:
31 resource-free engine regressions are absent from configured CTest.
Затем эти 31 регистрации и свойство виртуального SDL-драйвера вынесены из
условия на обеих платформах. Тесты, требующие базы и ресурсов, остаются
условными. Код движка, игровые параметры и HD-сборка не меняются.

Проверка конфигурации включается только при наличии цели UpstreamBugTests;
легковесные проекты сохраняют прежнюю проверку паритета исходных списков.
Она читает сгенерированный JSON-инвентарь CTest и проверяет наличие всех
122 тестов движка без аргументов игровых ресурсов. Новый тест внутри такого
же ошибочного условия теперь вызовет падение проверки.

В существующем Windows-каталоге сборки временно очищен S2_GAME_DIR:
125/125 проверок прошли, включая все ранее скрытые случаи. Затем восстановлен
G:/SS/lab/baseline, проверка регистрации прошла 1/1. Linux Game собран,
полный upstream-набор с ресурсами прошёл 238/238. Режим без полной игры
сохранил проходящую проверку паритета. Отдельных копий сборки не создавалось.

Это локальная проверка конфигурации, не результат запуска GitHub Actions;
реальный workflow на окончательной ревизии всей задачи ещё предстоит проверить.

Логи находятся в G:/SS/lab/evidence/upstream-remaster-20261024:

- no-data-registration-red-configure.log и no-data-registration-red-test.log;
- no-data-registration-green-configure.log и no-data-registration-green-test.log;
- no-data-registration-restored-configure.log и no-data-registration-restored-green-test.log;
- no-data-registration-linux-configure.log, no-data-registration-linux-build.log,
  no-data-registration-linux-test.log.

## Происхождение локальных тестов

### Проверки звука и сборка со средствами только для линковки

В реальном [GitHub Actions 40200095412](https://github.com/mrartanis/silent-storm-remaster/actions/runs/40200095412)
сборка прошла, но четыре новые проверки звука упали при инициализации:
`UpstreamLateLoopingSoundOffset`, `UpstreamHeadlessMusicStream`,
`UpstreamTracklessSceneMusic`, `UpstreamReloadedMusicRecord`.
Этот workflow собирает конфигурацию без `S2_ENABLE_NATIVE_SFX` и нативной музыки:
совместимые заглушки обеспечивают линковку, но не проигрывание PCM.

Звуковые проверки теперь имеют метку `native-audio`. В Windows-конфигурации
без нативного SFX они остаются видны в CTest с явным статусом `Disabled`.
При наличии нативного движка, включая Linux Game, они включены и обязательны.
Проверка регистрации сверяет эту доступность с фактическим наличием цели
`s2_native_fmod`, поэтому случайное отключение в нативной сборке тоже является ошибкой.

TDD: новая проверка регистрации сначала упала в той же рабочей сборке,
перенастроенной без нативного SFX: `native=0, disabled=False`.
После исправления проверка проходит, семь звуковых проверок с игровыми данными
явно отключены. Это проверка конфигурации; этот шаг не выдаётся за пересборку
движка с заглушками. Затем нативная конфигурация восстановлена, и все звуковые
проверки снова включены. Реальный Actions окончательной ревизии проверяется отдельно.
Восстановленный нативный профиль прошёл 254/254 на Windows и Linux; Linux Game
собран. Отдельно без игровых данных и со всеми тремя нативными media-опциями
OFF проверка регистрации прошла, сохранив пять доступных только нативному звуку
проверок явно отключёнными. Свидетельство: `native-audio-ci-registration-no-data-green-test.log`.
Свидетельства: `github-40200095412-failed.log`,
`native-audio-ci-registration-{red,stub-green,green}-test.log`.

Это локальное исправление тестовой регистрации. Исходные коммиты:
`22c58cf2ae1d946dd934a65736c3836b8bf66104`,
`c41c04a58f96ce3e9b03880062ddd4625294e0c3`,
`1f9490af00042d0639fb3d5115b5c7e581c968cc`,
`beaaa605ff079a8bcc71767b8b2a3c02579e2e06`,
`b217ccb0c23895a8b115b7e77b4826edfa19c813`,
`3423193c17cafab5f77e57dcd13a1573e9ce0926` и `6e5e0dd04c40d48b2dc5d92176f340b4d920650b`.

### Шрифты при отдельной сборке `UpstreamBugTests`

Усиленная проверка редактируемого текста использует настоящий комплект
`assets/fonts`. Раньше его копировали только `Game` и `BgfxRendererTests`;
отдельно собранный `UpstreamBugTests` случайно зависел от их предыдущей сборки.
Теперь у тестового приложения есть собственный `s2_copy_fonts` на Windows
и Linux. Это копирование существующих TTF, без генерации HD-текстур.

TDD на Windows: один собранный шрифт временно переименован после проверки
совпадения SHA-256 с репозиторием; заново скомпонован только тестовый exe.
До исправления target не восстановил шрифт, и настоящий тест упал с
`the test application is missing its packaged high-resolution fonts`.
После исправления тот же target восстановил идентичный шрифт, временный файл
удалён и профиль прошёл 261/261. Проверка наличия font provider выполняется
перед рисованием, поэтому нехватка ресурса даёт понятный FAIL.

Linux `Game` и профиль также прошли 261/261. После этого повторена отдельная
сборка только тестового exe без одного шрифта: target восстановил идентичный TTF,
проверка всех 12 сочетаний шрифта/масштаба прошла, время изменения `Game`
осталось прежним. Временный файл удалён. Свидетельства:
`standalone-font-test-assets-linux-solo-green-{build,test}.log`.

Это локальное исправление комплектации теста, а не новый upstream-перенос.
Исходные тестовые коммиты: `3cf392b945550460e7982f3bce7fdba61ad287e9`
и `e50eba4abd7d811f15d8aa69ea38a137b4f3f44a`. Свидетельства: `standalone-font-test-assets-{red,green}-test.log`,
`standalone-font-copy-control.json` и соответствующие build-логи.

### Первоначальное исправление отсутствовавших проверок

Это исправление нашей регистрации, а не перенос игровой логики из upstream.
Первоначальная проверка паритета добавлена в
[486028c3](https://github.com/mrartanis/silent-storm-remaster/commit/486028c39abde987b7379bfd0c85a37e25d66722).
Исходные коммиты строк регистрации совпадают на Windows и Linux:

| Тест | Коммит исходной регистрации |
| --- | --- |
| UpstreamRestoredPortraitSelection | [1dd31be5](https://github.com/mrartanis/silent-storm-remaster/commit/1dd31be58f77e5dfb93d59be980cdde60b0d0e58) |
| UpstreamRestoredEmptyPortraits | [1dd31be5](https://github.com/mrartanis/silent-storm-remaster/commit/1dd31be58f77e5dfb93d59be980cdde60b0d0e58) |
| UpstreamNativeCursorPosition | [6e093b8a](https://github.com/mrartanis/silent-storm-remaster/commit/6e093b8ad4201f4a36352579d23a9ab467580f70) |
| UpstreamDisabledCursorInput | [9fb22f12](https://github.com/mrartanis/silent-storm-remaster/commit/9fb22f12d508ee4fa9d82228d426420ec6bbf9db) |
| UpstreamMissingLuaGlobalWarning | [a4cba8ad](https://github.com/mrartanis/silent-storm-remaster/commit/a4cba8ad5c22324727198443d8eb121ebba250ec) |
| UpstreamPrecalcSweepBounds | [c48e1325](https://github.com/mrartanis/silent-storm-remaster/commit/c48e13259824fc1ddd8700f3caa9483cfc1e7cfd) |
| UpstreamPronePoseChangeFacing | [39653768](https://github.com/mrartanis/silent-storm-remaster/commit/396537686d2616bcdec413f25f4841c1188ef97f) |
| UpstreamSimplePronePosePath | [915c1ea4](https://github.com/mrartanis/silent-storm-remaster/commit/915c1ea4f54aa7809ef647c81d4ab134aa48f09c) |
| UpstreamSimplePoseFacingPreference | [faf57a7c](https://github.com/mrartanis/silent-storm-remaster/commit/faf57a7c6f961ce0fa507325ac1584d47c3797ea) |
| UpstreamSimpleCrossLayerPoseEntry | [ded2382c](https://github.com/mrartanis/silent-storm-remaster/commit/ded2382c00517c40cbbb76c05cc34c1a4cee2399) |
| UpstreamConnectedAliasMoves | [6599d936](https://github.com/mrartanis/silent-storm-remaster/commit/6599d93651306332224ad17e01586f896cd14308) |
| UpstreamIsolatedSourceMoves | [fbb212d4](https://github.com/mrartanis/silent-storm-remaster/commit/fbb212d4a20fbb6a2ac9b7b267b3c918e6e0eecf) |
| UpstreamLockedAliasMoves | [312608a8](https://github.com/mrartanis/silent-storm-remaster/commit/312608a83d5dcdb1fdf60f7cbb1f09e819835051) |
| UpstreamIsolatedSourcePath | [74b46ee0](https://github.com/mrartanis/silent-storm-remaster/commit/74b46ee0fefa1176cbbf7256e4646e53927addda) |
| UpstreamIsolatedTransitionLinkMatch | [fd836686](https://github.com/mrartanis/silent-storm-remaster/commit/fd836686b26f998903615298c6e7f6b40babf66a) |
| UpstreamIsolatedTransitionShortcut | [cbfb21a6](https://github.com/mrartanis/silent-storm-remaster/commit/cbfb21a6a549ab84954ae815f36497d3dae0114f) |
| UpstreamIsolatedTransitionEntry | [63c1fcb8](https://github.com/mrartanis/silent-storm-remaster/commit/63c1fcb82b99feccc01d6a0ac7e012fbc3caaa9a) |
| UpstreamConnectedDirectTransitions | [33245114](https://github.com/mrartanis/silent-storm-remaster/commit/33245114ba6edc765979265de57fe04f470abe6f) |
| UpstreamNearbyConnectedDestination | [ca23b8bb](https://github.com/mrartanis/silent-storm-remaster/commit/ca23b8bb9c351c8426abe0ffc620e34b0a637e35) |
| UpstreamIsolatedSourceAliasVector | [7723159b](https://github.com/mrartanis/silent-storm-remaster/commit/7723159b237baf15fc0c558f1e2a2183ac7ca09f) |
| UpstreamIsolatedSourceAliasArray | [7723159b](https://github.com/mrartanis/silent-storm-remaster/commit/7723159b237baf15fc0c558f1e2a2183ac7ca09f) |
| UpstreamNavigationAliasVector | [6e1a4ce7](https://github.com/mrartanis/silent-storm-remaster/commit/6e1a4ce79779bae8f27dc4f47756954e8ce18c9c) |
| UpstreamNavigationAliasArray | [6e1a4ce7](https://github.com/mrartanis/silent-storm-remaster/commit/6e1a4ce79779bae8f27dc4f47756954e8ce18c9c) |
| UpstreamFlyingShootDirection | [90b909dc](https://github.com/mrartanis/silent-storm-remaster/commit/90b909dcff25d20e5c8a9b32de711899e9a3360c) |
| UpstreamFlyingDynamicLocks | [ebace437](https://github.com/mrartanis/silent-storm-remaster/commit/ebace437c84475aafa77521e2c4c9b988f0e3878) |
| UpstreamRemovedSelectedEnemy | [83bf1f66](https://github.com/mrartanis/silent-storm-remaster/commit/83bf1f66e834f9f70a442528777040af0343aa26) |
| UpstreamRemovedSelectedPossible | [83bf1f66](https://github.com/mrartanis/silent-storm-remaster/commit/83bf1f66e834f9f70a442528777040af0343aa26) |
| UpstreamRemovedSelectedAlly | [83bf1f66](https://github.com/mrartanis/silent-storm-remaster/commit/83bf1f66e834f9f70a442528777040af0343aa26) |
| UpstreamOwnedAIActionEnemy | [ea3ef71a](https://github.com/mrartanis/silent-storm-remaster/commit/ea3ef71a8c027731f02b7eb3fa596ec5f5bfc8b8) |
| UpstreamOwnedAIPlaceEnemy | [ea3ef71a](https://github.com/mrartanis/silent-storm-remaster/commit/ea3ef71a8c027731f02b7eb3fa596ec5f5bfc8b8) |
| UpstreamMountedAILogCommands | [084eeb02](https://github.com/mrartanis/silent-storm-remaster/commit/084eeb02e8a9e8e23c22fac092c855b04a881e68) |

## Контракт списка выбранных юнитов в заглушке attack-location

В исходной тестовой заглушке `AttackMission` отсутствовал `GetSelectedUnits`:
унаследованный метод обращался к неустановленному `pActivePlayer`.
Новая проверка выбранной цели законно запросила этот список и обнаружила
падение старого `UpstreamAttackLocation` на Windows и Linux.

Перед исправлением добавлена явная проверка пустого списка в worldless-заглушке.
Она воспроизвела падение также без нового игрового исправления. CDB подтвердил
`CMissionBase::GetSelectedUnits`, `iMissionBase.cpp:189`, из
`AttackLocationRefresh`. Заглушка теперь возвращает пустой список;
`AttackCursorMission` по-прежнему предоставляет свой настоящий список выбора.

Источник тестовой заглушки:
[671ebe975b1a5a077899483ad03d3ea56f5a91a2](https://github.com/mrartanis/silent-storm-remaster/commit/671ebe975b1a5a077899483ad03d3ea56f5a91a2).
Это локальная поправка теста с `Original-Test-Commit`, а не перенос production-кода.
Свидетельства: `worldless-attack-fixture-selection-{red,green}-{build,test}.log`
и `worldless-attack-fixture-selection-red-cdb.log`.

## Подтверждённый hosted GitHub Actions

[Запуск 40246010705](https://github.com/mrartanis/silent-storm-remaster/actions/runs/40246010705)
на `00af3fcb6f2d26d9d8af296b22a971ecdd078990` завершил и `Build`, и `Test`
с результатом `success`. В hosted runner прошло 270/270 исполняемых тестов.
Пять native-audio случаев остались зарегистрированы, но отключены при
используемом link-only backend: LateLoopingSoundOffset, FailedSoundChannelLifetime,
HeadlessMusicStream, TracklessSceneMusic и ReloadedMusicRecord. Это соответствует
проверяемому условию наличия native SFX; локальный профиль с native SFX по-прежнему
обязан исполнять соответствующие проверки. Само число 275 в hosted CTest включает
эти пять отключённых случаев. Тесты, требующие game.db/res, проверяются локально.

Прямые свидетельства: `github-40246010705-result.json` с успешными шагами и
`github-40246010705-test.log` с CTest summary. Это подтверждает исправление
прежнего hosted audio-registration отказа на указанной контрольной точке.
После завершения всех переносов всё ещё нужен успешный запуск на окончательном
HEAD; более ранний зелёный workflow не подтверждает ещё не отправленные изменения.


## Hosted checkpoint `07e24b81`

[GitHub Actions41013866830](https://github.com/mrartanis/silent-storm-remaster/actions/runs/41013866830)
на `07e24b816cf0fadb683cb9aee23e8c52a6f2d32f` завершился успешно.
По фактическому job123925715523 шаги Configure CMake, Build и Test имеют
conclusion success. Прямое свидетельство: github-41013866830-success.json
в G:/SS/lab/evidence/upstream-remaster-20261024. Это подтверждение указанной
контрольной точки; после следующих переносов нужен успешный workflow на
окончательном HEAD. Локальный CI для текущего corpse-cancel фикса прошёл305/305;
его будущий hosted запуск этим более ранним результатом не подменяется.


## Asset-free регрессии активации окон

UpstreamWindowActivationFlags0..5 зарегистрированы в общем наборе Windows и
Linux, без проверки наличия игровых ресурсов. Они используют настоящее дерево
CWindow и native ProcessMessage/SendMessage. Случаи0..3 воспроизвели четыре
ошибки на каждой платформе до фикса;4/5 проверяют NOACTIVATE и обычные события.
После фикса все6/6 проходят. Текущий локальный asset-free манифест содержит
311 исполняемых тестов и включает все шесть новых имён; прошло311/311.
Это локальное свидетельство, отдельное от будущего hosted workflow.


## Asset-free очистка слабых ссылок скрипта

UpstreamScriptMiscWeakCleanup0..3 зарегистрированы в Windows и Linux без
условия наличия игровых ресурсов. Это native CScript, существующая слабая
коллекция tag4 и реальные типы команд звука/эффекта. RED3/4 до production,
GREEN4/4 после очистки; контроль живых ссылок/владения проходил до исправления.
Текущий локальный CI-манифест содержит315 тестов, включая все четыре новых;
315/315 успешны. Это локальный результат; окончательный hosted workflow
необходимо проверить отдельно на финальном HEAD всей задачи.


## Публичный запрос вокселя видимости

UpstreamVisionVoxelBoundary зарегистрирован без игровых ресурсов в Windows/Linux:
настоящий default tracker из native class factory, coordinate round-trip,
шесть запросов вне сетки. NativeVisionVoxelGeometry в ресурсной группе проверяет
реальный RPG tracker и независимо растеризованную геометрию карты. На обеих
платформах RED2/2 timeout непосредственно в GetVision -> GREEN2/2.
Локальный asset-free CI316/316 включает boundary case; native geometry остаётся
extended. Это не подтверждение hosted workflow на окончательном HEAD.


## Кеш точечных запросов видимости

UpstreamPointVisionCache0/1 зарегистрированы без ресурсов в Windows/Linux:
настоящий default tracker, FOV/range/key/multiplier, strict15000 limit и ordinary
полный save -> NEW tracker. NativePointVisionCacheGeometry использует настоящий
map4517/door439 и проверяет сброс stale false/true результатов при смене состояния.
До production RED3/3 на каждой платформе; initial GREEN3/3, затем save control
добавлен и успешно исполнен в финальных related наборах16Win/14Linux.
Локальный asset-free CI318/318 включает оба новых CPU теста. Hosted результат
для окончательного HEAD всей задачи по-прежнему требуется отдельно.


## Конечная ячейка луча видимости

NativeVisionDestination0..6 зарегистрированы в ресурсных Windows/Linux группах,
LABEL extended: оба направления по трём осям для NONE->SOLID реальной карты и контроль
start/interior/zero-length/FOV/range. Native RED6/7 -> GREEN7/7 на каждой системе.
Финальные related23Win/21Linux проходят, включая предыдущие pointCache/save,
voxel/script/door/AI проверки. Локальный asset-free набор остаётся318/318;
новые семь ресурсных случаев не выдаются за исполняемые в hosted без game DB.


## Подсказки меню стороны и сложности

NativeSideMenuTooltip0..6 зарегистрированы в Windows/Linux ресурсных группах,
extended;native-display, с отдельными S2_USER_DATA_DIR. Реальные menu/button/
CToolTip/CText и native frame проверяют семь локализованных attachment/text
сценариев; normal/hover markup и прежние SelectedStyle/DifficultyMapping controls
сохранены. RED6/9 -> GREEN9/9 на каждой системе. Связанные21/21 each и локальный
asset-free CI318/318 проходят; новые семь resource cases не считаются частью318.


## Выключенное автоматическое слежение камеры

UpstreamFollowCameraCommandPolicy0..4 зарегистрированы без ресурсов в обоих
наборах. Actual CMissionUI/CMission queue/native camera и реальные executors
проверяют блокировку прокрутки follow-off unit/point commands, follow-on
и scripted interpolation/completion/unrelated-command controls. Fixture getter
предоставляет выбранную PC camera, без подмены её реализации или live READ.
RED2/5 -> GREEN5/5 each. Related34/34 each включает24 существующих native-display
world-camera cases и прочие фильтры/producer/serializer. Локальный CI323/323
включает все пять новых CPU случаев; hosted результат подтверждается отдельно.

## Отсутствующий аниматор носильщика

NativeCorpseNullTaker0..3 зарегистрированы в Windows/Linux ресурсных группах,
extended, timeout120. Настоящие world/body/skeleton/animation sequence: null
с carrier/без carrier/уже carried и контроль обычного nonnull attachment.
RED3/4 -> GREEN4/4 на каждой системе, с сохранением текущего native clip и
head identities/AP/health/death/positions. Это defensive public API regression;
в текущих штатных caller paths taker nonnull. Related40 Win/39 Linux и локальный
CI323/323 проходят; новые четыре ресурсных случая не входят в asset-free323.

Последний подтверждённый hosted checkpoint: head093320b11d8b2dfb0a07d702750f359c4ca9f2ba,
[run41063705086](https://github.com/mrartanis/silent-storm-remaster/actions/runs/41063705086),
actual Configure/Build/Test success. Это предыдущий HEAD, без утверждения, что
текущий null-taker фикс или окончательный HEAD уже прошёл hosted CI.

## Звук стадии разрушения

NativeObjectSoundStage0..3 зарегистрированы на Windows/Linux, extended,
timeout120: обе смены стадии без render, render-first, unchanged-stage.
Compiled native RED2/4 -> GREEN4/4 each. Ресурсные случаи не входят в323
asset-free теста. Итоговые related33/33 each включают16 native-display;
Linux исполняется с Xvfb/openbox. Текущая стадия берётся из реального RPG
объекта, записи stage audio — из authored CRndObject831.

## Временные метки стадии разрушения

NativeObjectSoundStage4..7 добавлены к ресурсной extended группе Windows/Linux:
effect/permanent sound, с повторным неизменяющим SetDestroyStage и без него.
RED4/4 -> GREEN4/4 each; actual authored831/563, aimtime1375/world9000,
ожидаемый возраст стадии125. Всего stage/clock8 случаев, вне asset-free323.
Related45/45 each и CI323/323 успешны, Linux display через Xvfb/openbox.

## Контекст мира разрушаемого объекта

NativeObjectAttackWorld8..11 — ресурсные extended случаи Windows/Linux:
null/foreign/own context и no-stage-change. RED2/4 -> GREEN4/4 each;
реальные RPG damage/clock/action tracker проверки. Related49/49 each и
asset-free CI323/323 проходят;4 новых случая вне323.

## Команды носильщика тела

NativeCarrierAdmission0..7: extended resource tests Windows/Linux, timeout120.
Четыре новых forwarding случая и4контроля; actual corpse handoff, nativefactory
parity, positive teleport/custom body animation завершение, сохранность carried
state/AP/HP/head identities. RED4/8 ->GREEN8/8 each,related48Win/47Linux,CI323.
Passage/Talk проверяют прохождение к штатной ошибке цели; новые8 внеasset-free323.

## Локализованные строки журнала

NativeLocalizedLogStyle0..2: extended resource Windows/Linux, SDL dummy,
timeout120. Actual CTextDraw->CML quads проверяют цвет/outline и прежние texture/
glyph dimensions Impact20 при scale1/2, newest-first9-of12 ring. RED3->GREEN3
each,related6Win/5Linux и CI323. Набор новых3 внеasset-free323.

## Бюджет скрытых шагов мира

NativeHiddenSkipBudget0..3: resource extended Windows/Linux,timeout120.
Actual world/event delay50ms, new30ms budget; owner/null viewers и LAN host/replica
ранние guards. RED1/4 ->GREEN4/4 each; повторный вызов проверяет reset бюджета.
Related13/13 each,CI323/323; новые4 внеasset-free323. Быстрое прохождение >5
сегментов отдельно не измерялось, clock/hiddenTime/50ms формулы сохранены.

## Начало отсчёта анимации ножа

NativeKnifeTimeRebase0..3: resource extended Windows/Linux,timeout120.
Actual published native animator: future-start firstsample/rollback/normalforward/
exactstart. RED2/4 ->GREEN4/4 each, fullsave->newanimator/motion/rotation/wire
controls and action release; related11Win/10Linux,CI323. Новые4 внеasset-free323.

## Unit-target команда метательного ножа

NativeKnifeUnitDispatch0..4: resource extended Windows/Linux,timeout120.
ANY/head/body и point/targetless controls; actual native eligibility/factory/
queue metadata/native aim/facing path/fresh executor restore/AP/HP/item checks.
RED3/5 ->GREEN5/5 each,related16Win/15Linux,CI323. Сам полёт/попадание не проверялось.

## Unit knife preview geometry

NativeKnifeUnitPreview0..5: resource extended Windows/Linux,timeout120.
Authored equipped knife ANY/head/body use actual native high/low geometry and
forwarding map observations; true melee, firearm and LAN replica controls.
RED3/6 each ->GREEN6/6 each. Chance clamp2% alone is not used as proof of failure.
Related22Win/21Linux, full Game builds, CI323; new6 outside asset-free323.

## Unit preview physical origin

NativeUnitPreviewShotOrigin0..2: resource extended Windows/Linux,timeout120.
Actual snap weapon/actors/cover queries and unclamped ANY/head/body probabilities.
RED3/3 ->GREEN3/3 each; AP/HP/ammo/pose/action/RNG controls. Extra standalone
area-only attempts without observable differences are excluded from RED evidence.
Full Game builds,related25Win/24Linux,CI323; new3 outside asset-free323.

## Test weather world placement

NativeWeatherWorldPlacement0..3: resource extended Windows/Linux,timeout120.
Real mission rain/snow templates and renderer clock; CPU view observer/native
camera fixture, identity forward/inverse, replacement/final handle retirement,
world time/action/RNG controls. RED3/4 ->GREEN4/4 each; no GPU screenshot claim.
Related8Win/7Linux,Game builds,CI323. New4 outside asset-free323; initial fixture
camera-selector crash and unbuilt Linux helper run are excluded from pass evidence.

## Chapter Panzerklein deployment gate

NativeChapterPanzerkleinGate0..6: resource/display extended Windows/Linux,
timeout120; Linux Xvfb/Openbox. Native chapter action/queue/log behavior with
authored zone flags, real hired mercs and suit persona; dry transition only.
RED2/7 ->GREEN7/7 each, five valid/dead/reserve/empty-player controls retained.
No camp fallback/duplicate warning; money/interface/travel/time/RNG checks.
Full Game builds,related10/10 each,CI323. New7 outside asset-free323.
Initial fixture type/center/container assumptions are excluded from RED evidence.

## Main-menu version/date lifecycle

NativeMainMenuVersionCaption: resource/display extended Windows/Linux,timeout120,
isolated user directory; Linux Xvfb/Openbox. Actual menu/template caption and
repeat completion, localized wide text/date, camera/world/queue controls.
RED1->GREEN1 each. Intermediate Microsoft wide-format truncation on Linux is
archived; final adapter preserves Unicode. Full Game builds,related8Win/7Linux,
CI323. New1 outside asset-free323; no compile-date equality across translation units.

## Person-clue death reserve

NativeClueDeathReserve0..3: resource extended Windows/Linux,timeout120. Established
real slot-distribution/spawn fixture with in-memory slot flags, current authored
difficulty, raised level, zero coefficient and ordinary non-slot controls.
Initial RED2/3 each ->GREEN4/4 after adding the non-slot control. Full RPG-unit
save/fresh load and inclusive IsDead boundary checks; no asset edits.
Game builds,related10/10 each,CI323; new4 outside asset-free323.

## Debug explosion producer routing

NativeDebugExplosionRouting0..2: resource extended Windows/Linux,timeout120.
Actual world/map TraceRay with empty/populated unit roster and no-hit control;
capture existing virtual blast-call boundary, exact point/preset/null ownership,
time/action/RNG. RED2/3->GREEN3/3 each. No actual blast-damage/visual claim.
Game builds,related7Win/6Linux,CI323; new3 outside asset-free323.

## Immediate VP overdose

NativeVPImmediateOverdose0..6: Windows/Linux resource extended, timeout120.
Native deployed hero, real AddVPBoost; zero/excess/exact/safe/dead boundaries,
duration clearing, modifier retention, VP/consciousness, AP/XP/time/position/RNG.
RED4/7->GREEN7/7 each. Related24 each; Game builds; asset-free CI323.
Periodic duration countdown remains a separate pending hunk.

## Periodic VP drug duration

NativeVPDurationCountdown0..8: resource extended Win/Linux, timeout120.
Actual deployed hero/native RPG periodic consumer; safe/exact/overdose/mixed/
empty trajectories, AP-refresh non-consumption, replica gate and native turn
callback consciousness synchronization. RED7/9->GREEN9/9 each, related33 each,
Game builds and asset-free CI323; new9 outside323.

## Explicit script grenade chance

NativeScriptGrenadeChance0..6: resource extended Win/Linux, timeout120.
Real projectile initial physics, seeded random52 versus chance0/51/52/100/-1/101,
plus null inputs. RED3/7->GREEN7/7 each; AP/HP/inventory/time/action controls.
No complete flight/damage or actual Lua execution claim. Related17Win/17Linux,
Game builds, CI323; new7 outside323.

## Columnar launcher import dependency

NativeWeaponImportOrder1/92/93/98/99: standalone native DB loader, resource
extended Win/Linux,timeout120. Actual type/launcher classification after load.
Linux RED4/5, Windows5/5 baseline controls -> GREEN5/5 each. Related10Win/9Linux,
Game builds,CI323; new5 outside323. No invented pre-fix Windows failures.

## Night inputs in retained preview dispatchers

NativeNightPreviewInputs0..4: resource extended Win/Linux, timeout120.
Actual firearm/launcher tile and dedicated launcher/grenade dispatchers;
day controls, night chances and script override37. RED4/5->GREEN5/5 each after
the separate loader prerequisite; AP/HP/time/position/RNG unchanged. Related
50Win/50Linux, Game builds, CI323. New5 outside323. No rendered-pixel claim.

## AI accuracy night construction

NativeNightAIInputs0..2: resource extended Win/Linux,timeout120. Actual actor/AI
wrappers, integer/float cover constructors and GetLight/GetToHit; day control.
RED2/3->GREEN3/3 each, AP/HP/position/time/RNG retained. Related53Win/53Linux,
Game builds,CI323; new3 outside323. No full planner/attack simulation claim.

## Grenade executor night input and native scatter

NativeGrenadeThrowNight0..2: resource extended Win/Linux,timeout120. Native
factory/day control plus real Run trajectory/seeded CheckToHitAndDelay consumer
through a diagnostic subclass, read-only grenadeParams. RED2/3->GREEN3/3 each.
No complete flight/damage claim. Related31Win/31Linux, Game builds,CI323; new3 outside323.

## Per-unit first-turn grenade input

NativeGrenadeUnitFirstTurn0..3: resource extended Win/Linux,timeout120. Native
world/unit state mismatch, public movement counter, day/night input cases and
real Run/seeded scatter consumer. Query-side-effect control passes; RED3/4 ->
GREEN4/4 each. Related35Win/35Linux,Game builds,CI323; new4 outside323.
Earlier night-only regression references the current per-unit input and is retained.

## First-turn penalty ability

NativeFirstTurnPenaltyPerk0..5: resource extended Win/Linux,timeout120. Actual
deployed actor, authored tree/node23 and native TakePerk; concrete tile calcer
GetFRMult, first/later/no-perk and restored runtime coefficient1.2 boundary.
RED2/6->GREEN6/6 each; point budget/AP/HP/XP/time/position/RNG controls.
Related41Win/41Linux,Game builds,CI323; new6 outside323. No full attack/planner claim.

## Perfect-engineering charged-tool eligibility

NativePerfectEngineeringTool0..5: resource extended Win/Linux,timeout120.
Actual RPG persona3/perk50 grant/tool1; skill0/required20 and qualification,
native SpendOneCharge empties with outer stack1. RED1/6->GREEN6/6 each; point
budget/AP/HP/XP/skill/RNG/charges retained. Direct RPG eligibility API scope,
not world action/placement. Related28Win/28Linux,Game builds,CI323; new6 outside323.

## Best shooting pose perk12

NativeBestShootingPose0..10: resource extended Win/Linux,timeout120. Native
authored persona/perk grant, active weapon1, concrete tile stance consumer.
Actual three poses, tie/nonpositive variations, ordinary and unarmed controls.
RED4/11 -> GREEN11/11 each; point/AP/HP/XP/RNG and cached pose stable. Isolated
stance query scope. Related58Win/58Linux, full Game builds, CI323; new11 outside323.

## Zeroing bonus85 repeated-shot consumer

NativeZeroingBonus0..14: resource extended Win/Linux,timeout120. Actual RPG
persona/grant85, native completed-shot counters and tile-calcer GetRS. Authored
bonus2/cap5, cap/above-cap and fractional/negative/zero parameter controls.
RED6/15 -> GREEN15/15 each; no repeat/AP/HP/XP/RNG/point mutations. Related
73Win/73Linux, Game builds, CI323. New15 cases outside323; no whole-shot claim.

## Difficult traps49

NativeDifficultTraps0..15: resource extended Win/Linux,timeout120. Actual RPG
persona/grant49, mine/grenade difficulty APIs, authored/zero/fractional/negative
and no-perk/null-argument cases. RED10/16 -> GREEN16/16 each; skill/AP/HP/XP/
points/RNG stable. Query scope; no placed trap/disarm claim. Related89Win/89Linux,
Game builds, CI323. New16 outside323.

## Classless perk lookup

NativeClasslessPerk0..6: resource extended Win/Linux,timeout120. Actual authored
personas887/888 with no class/tree, flag and optional-output lookups; classed
native untaken49/acquired49/acquired50 controls. RED4 crashes/7 -> GREEN7/7
each; AP/HP/XP/RNG and point budget stable, false retains output sentinels.
Related96Win/96Linux, Game builds, CI323. New7 outside323.

## Disarm traps51

NativeDisarmTraps0..15: resource extended Win/Linux,timeout120. Native authored
RPG grant and CanClear, fixed RNG boundary rolls, fractional/negative/zero
parameters, cap95/probability0 and ignored modifier controls. RED7/16 -> GREEN
16/16 each; skill/AP/HP/XP/points stable, exactly one RNG draw. Related112Win/112Linux,
Game builds, CI323. New16 outside323; success-query scope only.

## Detection baseline precision

**Superseded precision inference:** the earlier double-product claim and
distance15/30 baseline22/0 below are incorrect under observed original PC24/RN.
See the later complete317 original-runtime audit; coefficient/truncation remain.

NativeUnhideBasePrecision0..7: resource extended Win/Linux,timeout120. Actual
RPG observer/target, no active25/75, distance0/5/15/30/40 and skill/clamp cases.
RED3/8 -> GREEN8/8 each; both units' skill/AP/HP/XP and RNG stable. Related
120Win/120Linux, Game builds, CI323. New8 outside323; direct probability-query scope.

## Better spotting25 ownership and rounding

NativeBetterSpotting0..13: resource extended Win/Linux,timeout120. Actual RPG
observer/target native grants, authored.25, fractional/negative/zero parameters,
zero skill/clamps and target-only/both ownership. RED6/14 -> GREEN14/14 each;
both units' skill/AP/HP/XP, points and RNG stable. Related134Win/134Linux, Game builds,
CI323. New14 outside323; direct detection-query scope.

## Observer75 scales target stealth

NativeObserverStealth0..13: resource extended Win/Linux,timeout120. Actual RPG
observer/target grants75, authored.25, target-only/both ownership, zero stealth,
clamps and fractional/negative/zero controls. RED6/14 -> GREEN14/14 each; skills,
AP/HP/XP/points/RNG stable. Related148Win/148Linux, Game builds, CI323. New14 outside323;
query scope only, no complete visibility/world day-night scenario claim.

## PK unsupported pose AP queries

NativePKUnsupportedPose0..15: resource extended Win/Linux,timeout120. Native
RPG SetPanzerklein/suit1/positive shell HP, four move actions in CRAWL/CROUCH,
worn WALK/RUN, ordinary body and pose-independent controls. RED8/16 -> GREEN
16/16 each; AP/HP/shell/XP/points/equipment/RNG stable. Related164Win/164Linux, Game
builds, CI323. New16 outside323. Query scope; pose34/35/corpse42 are untaken.

## Prone costs crouch34

NativeProneCostsCrouch0..15: resource extended Win/Linux,timeout120. Actual RPG
Take34, four movement costs, ordinary/other-pose/early-action and actual PK
controls;35/42 untaken. RED4/16 -> GREEN16/16 each; AP/HP/shell/XP/points/
equipment/RNG stable. Related180Win/180Linux, Game builds, CI323. New16 outside323;
cost-query scope. Joint34/35 awaits35.

## Crouch costs walk35

NativeCrouchCostsWalk0..15: resource extended Win/Linux,timeout120. Actual
person2/tree5 native Take35, four body movement costs, worn PK and early-action
controls. RED6/16 -> GREEN16/16 each; AP/HP/shell/XP/points/equipment/RNG stable.
Native inventory shows34/tree6 separate, so failed mixed-owner setup is archived
and no synthetic joint tree is used. Related196Win/196Linux, Game builds, CI323.
New16 outside323; direct AP-query scope, no native joint34/35 claim.

## Corpse-tracker42 AP and PK precedence

NativeCorpseTrackerAP0..23: resource extended Win/Linux,timeout120. Actual
person34/native Take42, four body poses, actual suit1, no42/normal/early-action
controls and fractional/negative parameters. RED14/24 -> GREEN24/24 each;
unsupported-PK1 retained,5.75 diagonal8 and-2.75 diagonal-4, no AP/HP/shell/XP/
points/equipment/RNG mutation. Related237Win/237Linux includes native carrier admission,
null-taker/cancel and carried-body stability; Game builds and CI323 pass. New24
outside323. Direct AP-query scope; no new full movement/animation claim.

## Cheap-melee32 AP

NativeCheapMeleeAP0..11: resource extended Win/Linux,timeout120. Actual person2,
native Take32 and equipped authored item5; natural/0/140 melee skill, fractional
and no-floor parameters, no32/throw/open controls and stable state/RNG. RED7/12
-> GREEN12/12 each. Game builds, related249Win/249Linux and CI323 pass; new12 outside323.
Native cost-query scope; no new attack execution/animation claim.

## Throwing-knife base AP

NativeThrowKnifeBaseAP0..9: resource extended Win/Linux,timeout120. Native
person2/item5, throwing0/27/70/140, opposite melee skills, acquired32 scope,
melee/open/none controls and stable AP/HP/XP/points/skills/item/RNG. RED7/10 ->
GREEN10/10 each. Game builds, related44Win/44Linux and CI323 pass; new10 outside323.
The existing native knife preview/dispatch/time/feedback/handoff and animation
checks are included. No new complete knife-flight or actual AP-spending claim.

## Cheap-throw43 knife AP

NativeCheapKnifeThrowAP0..11: resource extended Win/Linux,timeout120. Actual
person2/Take43/item5; authored0.25, skill0/140, fractional/zero/free/
negative parameters, no43/melee/open controls and stable state/RNG. RED7/12 ->
GREEN12/12 each. Game builds, related56Win/56Linux including prior knife/melee AP,
knife preview/dispatch/time/feedback/handoff, animation, reload33 and network,
plus CI323 pass. New12 outside323; cost-query scope.

## Grenade AP table5

NativeGrenadeTableAP0..8: resource extended Win/Linux,timeout120. Native person2,
runtime-owned RPGAP5 restored after temporary24/7/0/-3, authored20 and four
non-grenade controls. RED4/9 -> GREEN9/9 each; state/RNG stable. Full Game
builds, related51Win/51Linux with previous knife/melee AP and existing grenade
execution/handoff/trajectory, and CI323 pass; new9 outside323. Steam table-driven
query scope; Gold's fixed20 distinction is recorded, no actual AP-spend claim.

## Cheap-throw43 grenade AP

NativeCheapGrenadeAP0..13: resource extended Win/Linux,timeout120. Native
person2/Take43, authored.25/table20 ->15, fractional/free/negative parameters,
table7/0/-3, no43/table24, melee/open and already fixed knife43 controls.
RED7/14 -> GREEN14/14 each, state/RNG stable, runtime values restored. Full
Game builds, related65Win/65Linux and CI323 pass; new14 outside323. Cost-query scope;
existing grenade execution/handoff/trajectory checks included.

## Better-evasion40 IC

NativeBetterEvasion0..11: resource extended Win/Linux,timeout120. Native
person41/Take40, authored0.15, skill0/21/23/140, fractional/negative/>1/
zero parameters and nearest-even tie cases. RED8/12 -> GREEN12/12 each; four
pre-fix controls, stable AP/HP/XP/points/skill/RNG. Game builds, related31Win/31Linux
and CI323 pass; new12 outside323. Native GetIC query only, no new attack roll,
practice, explosion39/aura or damage-reduction claim.

## Explosive modifier min/max precision

**Precision clarification:** the earlier wide-product inference and large-half
examples below are superseded by the later original-runtime audit. Integer
truncation remains correct (13.5 ->13); product significand is PC24/RN.

NativeExplosiveModifierMath0..14: asset-free extended Win/Linux,timeout120.
Direct actual FillAttackModifiers with typed native records; fixed expected
fractional/zero/neutral/negative and safe large-integer cases, structure/critical
controls, unrelated attack-field and RNG preservation. RED10/15 -> GREEN15/15
each. Full Game builds, related24Win/23Linux and CI323 pass; new15 outside323. No
authored owner/perk fixture or new full explosion/trace/application claim.

## Live current-item familiarity contribution

NativeLiveFamiliarity0..9: resource extended Win/Linux,timeout120. Native
person2, authored firearm1/melee5 and actual unarmed fallback; actual UseWeapon
with declared rate/cap/multiplier stimuli and real equipped item instances.
RED6/10 -> GREEN10/10 each, four item-identity/untrained controls; AP/HP/XP/
points/equipment/live-counter/RNG stable during query. Full Game builds,
related44Win/44Linux and CI323 pass; new10 outside323. Query input isolates the
missing familiarity term; no actual firing or full sum-order/targeting claim.

## Original additive float order

NativeAdditiveOrder0..11: resource extended Win/Linux,timeout120. Actual
person2/firearm1/native terms and counters; fixed literal golden result/input
bits from independently decoded original stores and Fraction crosscheck.
RED8/12 -> GREEN12/12 each, four pre-fix controls reject upstream D2/Stance
swap; no fixture prerequisite failures. State/counters/equipment/ammo/skill/
query inputs/live familiarity/RNG stable. Full Game builds, related56Win/56Linux
and CI323 pass; new12 outside323. Temporary discovery mode removed, discovery
and provisional wrong-order scout explicitly are not RED evidence. Shared
additive-query scope, not final chance or actual shooting.

## Retail1.2 extra-AP additive base

NativeTargetingBase0..13: resource extended Win/Linux,timeout120. Actual
person14/firearm17/SNAP/Aimed/Careful/Snipe, native capped movement, no73,
no-weapon and declared controlled fractional-movement inputs. RED7/14 ->
GREEN14/14 each; seven pre-fix controls, state/counters/equipment/ammo/skill/
inputs/RNG stable. Game builds, related70Win/70Linux and CI323 pass; new14 outside323.
Raw additive-query scope. Goldolder-Max/Steam1.2 distinction recorded, no
Prepare, AP-spend, final chance or shooting equivalence claim.

## Faster-targeting73 Snipe contribution

NativeFasterTargeting0..13: resource extended Win/Linux,timeout120. Actual
person14/Take73/firearm17/authored1.25, extra1/2/10, capped and fractional
authored movement, temporary direct parameters, no73/Careful/Aimed/SNAP and
separate-truncation controls. RED6/14 -> GREEN14/14 each; eight pre-fix controls,
state/counters/equipment/ammo/skill/inputs/RNG stable, parameters restored.
Game builds, related84Win/84Linux and CI323 pass; new14 outside323. Shared additive
query only; no new Prepare, AP-spend, final-chance or actual shooting claim.

## Late incidental damage evasion base

NativeIncidentalDamage0..7: resource extended Win/Linux,timeout120. Real native
RPG ProcessAttack/person103/authored body armor/constant40, intended/incidental/
IC0/20/100/140/failed-roll/actualPK-bypass and actual MakeClickOfDeath pair.
Corrected RED4/8 -> GREEN8/8 each, four pre-fix controls. Damage/HP/receiver/
shared RNG and click practice checked; worn shell/inventory/input portions
preserved. Initial overstrict AP fixture archived NOT final RED; existing
wound-driven AP recomputation observed. Full Game builds, related40Win/40Linux and
CI323 pass; new8 outside323. No39/aura0 noncritical native RPG application
scope; no new full projectile/FX/AI/death-loot/critical-pipeline claim.

## Original unsigned IC chance

NativeUnsignedICChance0..7: resource extended Win/Linux,timeout120. Actual
person41/40/native TakePerk and restored temporary parameters; actual CheckIC
with IC21/24/-10/-21/0/140, plus real incidental base40 ->43 damage atIC-10.
RED3/8 ->GREEN8/8 each, five pre-fix controls. One-check RNG/IC practice and
unrelated state checked; damage case validates HUMAN/HP/RNG/death state.
Full Game builds, related48Win/48Linux, CI323; new8 outside323. No39/aura0 scope.

## Typed explosion evasion39

NativeExplosionEvasion0..16: resource extended Win/Linux,timeout120. Actual
person2/39/authored.1, native TakePerk and real ProcessAttack/body armor/flesh
fixed40/100 with runtimeVP500. RED9/17 ->GREEN17/17 each, eight pre-fix
controls. Separate chance-only/percentage-only rolls, blast/fragment/normal,
nearest-even rounding1.5/34.5, restored zero/fractional/negative parameters,
unsigned negativeIC, intended/wornPK scope, HP/receiver/RNG/practice/baseIC/
points/inventory/shell/input checked. Full Game builds, related65Win/65Linux,
CI323; new17 outside323. Aura0; no full world/critical pipeline claim.

## Aura player-pointer ownership

NativeAuraPlayerOwnership0..9: resource extended Win/Linux,timeout120. Actual
world810/person3/authored66 and native28 donor, real passable near/far points,
SetPlayer restored after detached-pointer probes, production aura preview
methods. Corrected RED3/10 ->GREEN10/10 each; seven controls. Bothnull-owner
own66/neutral donor exclusion/ordinary same-player sum/fractional/untaken/
absent/outside controls; HP/AP/points/RNG preserved. Scoped donor28 diagnostic
label avoids pre-existing Linux CP1251 log exception and restores exact text;
initial encoding-fixture failures explicitly NOT final RED. Full Game builds,
related47Win/47Linux, CI323; new10 outside323. No victim damage scope claim.

## Existing stored aura evasion term

NativeStoredAuraEvasion0..12: resource extended Win/Linux,timeout120. Actual
native mission existing tag43 restored by full snapshot, every other field
and reference compared unchanged; real person2/39/ProcessAttack/body armor/
fixed40/100. RED9/13 ->GREEN13/13 each, four pre-fix controls. Chance-only draw,
normal/blast/fragment, 21.5/22.5 nearest-even and39+aura before final rounding,
negative/zero stored result, intended/wornPK scope, HP/receiver/RNG/baseIC/
field/points/inventory/shell/practice checked. Full Game builds, related78Win/78Linux,
CI323; new13 outside323. World application/reset remains pending.

## World victim aura application/reset

NativeWorldVictimAura0..11: resource extended Win/Linux,timeout120. Actual
world810/person3/native66, actual world server damage/hit locator and complete
native stored43 restoration, no fake world/API stub as RED. RED9/12 ->GREEN12/12
each, three controls. Authored +10/base7 ->5, chance-only draw, stale overwrite
and reset for normal/intended/blocked calls, absent/zero/distinct second-output/
negative/fractional/IC0 controls; HP/hit/RNG/practice/baseIC/points/inventory/
input/alive checked. GREEN damage<=5 avoids unrelated blood-particle RNG;
its legitimate old >5 path explains extra RED draws. Full Game builds,
related100Win/100Linux, CI323; new12 outside323. All037c functional hunks now accounted
with original arithmetic corrections and compatible own API adaptations.

## Native terrain grass room floor

NativeTerrainGrassFloor0..1: resource extended Win/Linux,timeout120. Actual
worlds810/4517/terrain/grass trackers/animators/CSetRender; recording scene sink
retains inputs and captures metadata, no GPU screenshot claim. RED2/2 ->
GREEN2/2 each; all4+44 sectors -100 ->0,85 terrain controls remain0, room groups
null and actual texture/pose/animator present, shared default-100 preserved.
Original633byte helper/typed room and floor0 arguments independently mapped.
Full Game builds, related33Win/33Linux, CI323; new2 outside323. No generated assets.

## Native carrier objective notification on death/removal

NativeScenarioCarrierDestroyed0..8: resource extended Win/Linux,timeout120.
Actual world810/person103/authored campaign clue/item pairs Win5442/314,5452/303 and Linux5480/315,
5479/338, with existing goals/tasks, real inventory/kill/remove/pickup/native objective-state
query. Corrected authored-only RED5/9 ->GREEN9/9 each; four controls. Normal/
silent removal/unconscious-death/two items/carried body complete only carrier
tags; live/missing/precompleted/uncarried controls and identities/other tags
checked. Invalid positive-quantity search initially rejected native quantity0
wrappers; initial logs NOT RED, unused fallback removed. Full Game builds,
related43Win/43Linux, CI323; new9 outside323. Actual attack KO remains separate.

## Native attack-induced knockout scenario notification

NativeScenarioAttackKnockout0..9: resource extended Win/Linux,timeout120.
Actual world/person clue/authored STUN goal, real RPG/world attack, raw task
and objective-state query. Corrected RED5/10 ->GREEN10/10 each, five controls.
Known/unseen/incidental/carrier/first-repeat cases and blocked/already-KO/
other-person/precompleted/script-only controls. Initial lethal repeated-click
fixture logs are NOT final RED. HP500, no IC practice, visibility/identities/
other tags preserved. Full Game builds, related53Win/53Linux, CI323; new10 outside323.

## Impact-order fixture after original late evasion

Existing UpstreamBulletImpactDodgeOrder now uses native IC50/aura0/base4 ->3,
with seed55 first roll0/second>=90 and positive particle/sound/decal checks.
Old actual3/expected-1 failure reproduced on both OS with pending fragment
speed reverted0. Corrected test passes each, CI323; no production behavior
change. Full35 related cases rerun in the separate fragment-speed fix.

## Original speed in real fragment bullets

NativeFragmentSpeed0..4: resource extended Win/Linux,timeout120. Native
world810/tracker ctor/authored grenade16 and engineer1, skill160, zero damage
modifiers. Two near bursts cover targeted+random, two remote bursts random
only, no-record control creates0. Read-only actual CBulletServer snapshots
verify speed3/tag3, nonempty trails, null model/shooter/effect-1; counts24/32
unchanged. RED4/5 ->GREEN5/5 each, full Game builds, related35Win/35Linux, CI323.
Linux display-labelled decal regression uses Xvfb/Openbox. New5 outside323.
These tests do not claim changed null-model flight timing or GPU tracer pixels.

## Native melee miss dispatch and success-event gate

NativeMeleeMissGate0..3: resource extended Win/Linux,timeout120. Real world810/
party54/opposing person103 units/free standing places/AI Sync/blade2/CanDoIt/
Start/cover/OnLabel. Observing executor delegates actual PerformAttack to its
native base; native event listener. Corrected RED2/4 ->GREEN4/4 each, two
passing nonlethal controls. Chance2, seed2/draw61 gives miss; existing native
forced-hit flag/check1 gives positive ratio2 with ordinary attacker skill0.
Miss dispatch/event1->0; HP500 unchanged, actual positive HP losses bounded
in both phases (own clock-seeded static ray roulette retained), attacker HP/
AP/item quantity retained. No-enemy/placement/
unsynced-hull/zero-draw/extreme-control/overstrict-HP logs NOT final RED.
Extreme skill2000/VP20000 controls superseded because of invalid HP reading.
Full Game builds, related23Win/23Linux, CI323; new4 outside323.

## Native plain-ray routing and retained explicit-hit fallback

NativePlainRayRoute0..4: resource extended Win/Linux,timeout120. Real world810/
party54/unit838/body/materials/map Sync/Trace, real CWorld overload and bullet
snapshots/Segment. Zero kinetic preserves HP/AP/life/action. Normal/fragment
loose rays consume one eligible probability0 ricochet draw and one incidental
chance100 draw; ISAAC777/four-word fingerprint. Ignored/empty/explicit true-
empty-cache fallback controls consume0 and preserve correct entries/metadata.
RED2/5 ->GREEN5/5 each; three controls. Initial failed ray searches NOT RED.
Full Game builds, related40Win/40Linux, CI323; Linux display decal uses Xvfb/Openbox.
New5 outside323. Closes the last aa functional hunk with own hit fallback kept.

## Native melee executor retains world/difficulty context

NativeMeleeWorldContext0..3: resource extended Win/Linux,timeout120. Actual
world810/party54/native geometry/units and Unit/Tile direct PerformAttack,
fixed intended damage2/kinetic positive. Temporary native in-memory difficulty
multiplier2 restored exactly: null/world literal2/4 controls; real HP+hit2->4.
RED2/4 ->GREEN4/4 each, two controls (anonymous, multiplier1) remain2. Actor
HP/AP/life and one receiver event checked. IsAIPlayer is0 for both actual
selected missions; no new AI-flag/enemy-multiplier claim. Full Game builds,
related27Win/27Linux, CI323; new4 outside323. No proxy world/map/callback is used.

## Native unit melee target filter with retained float API

NativeMeleeTargetFilter0..5: resource extended Win/Linux,timeout120. Actual
world810/party54/two person103 units/free standing tiles/Sync/two native hulls/
real Unit or Tile existing PerformAttack; no proxy callbacks or new API stub.
RED3/6 ->GREEN6/6 each, three passing controls. Select back/front damages only
selected3; unselected godmode gets0 callbacks, while null/Tile retains both
and godmode Tile retains current two -1 per-piece callbacks with0 HP loss.
HP500/IC0, no critical/KO, actor HP/AP/life preserved. Old native miss observer
delegates immediate unit class before RED. Full Game builds, related33Win/33Linux,
CI323; new6 outside323. PDB prototypes correct the old vector-ABI interpretation;
original is single-portion, own vector/float API is retained by an override/core.

## Native godmode melee receiver deduplication

NativeMeleeGodmodeDedup: resource extended Win/Linux, timeout120, outside323.
Actual Tile executor/world810/party54/two native bodies, requires >1 front
body interval. RED1/1 ->GREEN1/1 each: protected front HP unchanged, callbacks
2/-2 ->1/-1; back remains one/3 HP. Actor HP/AP/life retained. Both Game
builds, related34Win/34Linux including all six filter controls and CI323 pass.

## Canonical native melee attacker and retained cannon alias

NativeMeleeAttackerIgnore0..2: resource extended Win/Linux timeout120. Actual
cannon entry/animation queue/hulls/body and separate cannon-bound rays, no
fabricated animator pointer. Final RED1/3 ->GREEN3/3 each with2 passing controls:
mounted self HP3/hit1 ->0/0, unmounted self0/0; cannon HP/stage/fields unchanged.
Actor AP/position/life/mount retained. Full Game builds, related37Win/37Linux, CI323.
Fixture compile/joint-ray failures archived separately, not counted as RED.

## Native knife/fist command critical matrix

NativeMeleeKnifeCritical0..7: resource extended Win/Linux timeout120, outside323.
Actual world/actor/authored blades/native gun/empty hand, native CCriticalsBan
object/tile commands. RED2/8 ->GREEN8/8 each with six controls: knife5/4->3/2
removes fist-only motionless/patient, fists5/4, throw/firearm3/2 unchanged.
AP/HP/position/cost/order/RNG/quantity retained. Existing cursor regression
moves old gameplay-invariance assertion to this original native matrix,
retaining cursor/empty/equip/unequip/AP controls. Full Game builds, related
45Win/45Linux, CI323. Appended action ordinal preserves every existing own value.

## Native shared attack awareness portion routing

NativeBackstabAwareness0..9: resource extended Win/Linux timeout120 outside323.
Actual world/owners/person103, authoredblade2/gun, native contact-list/visibility
state, access-only real executor subclass/shared CreateAttack, no fake portion.
RED2/10 ->GREEN10/10 each, eight controls. Audible unseen blade28..46/crit59
->38..46/107 and flag0->1; gun flag0->1 with unchanged current damage. Visible,
unheard and null controls retain formulas/flags; AP/HP/position/items/ammo/
contacts stable. NativeSEEALL only arranges visible controls, no naturalLOS
claim. Invalid pathCP/max-rise assumptions archived, not RED. Game builds,
related55Win/55Linux, CI323 pass.

## Native critical type selection shared RNG

NativeCriticalSeverityDraw0..5: resource extended Win/Linux timeout120 outside323.
Actual world/mission/flesh armor, crit100/0 head/body/zero-energy portions,
critical application disabled to isolate selector. RED2/6 ->GREEN6/6 each with
four controls: four-word shared RNG fingerprint matches original one KO draw
for positive damage and no draw for blocked; actual body2/head3/blocked0 HP
unchanged, AP/position/life/input severity stable. Previous attacker-level
regression updates only old extra selector-draw expectation, other controls
retained. Fixture API/energy-guard assumptions and truncated invalid metadata
log are archived, not RED. Complete final logs and Game/related69Win/69Linux/CI323
pass. Probability SRand remains separate; no damage multiplier audit claimed.

## Native critical damage factors across body regions

NativeCriticalDamageFactor0..9: resource extended Win/Linux timeout120 outside323.
Actual world/newperson103/armor/mission, authored bleeding-only selection, no
resistance/severity perks, gamecap1000 restored. RED8/10 ->GREEN10/10 each with
2 controls. Body14->5/head13->7/hands7->2/legs10->3, fractionalbody149->50/
head137->70 actual HP/received damage; zeroCrit2/zeroenergy0 unchanged. Bleeding
stillone; GREEN AP/pose/life/inputDC stable. Legacy injury DC scaling remains
separate. Initial base40 head crossed existing wound/AP threshold; production
restored exact68 baseline before accepted base20 RED, patch then reapplied.
Both Game builds, related79Win/79Linux, CI323. Original reciprocal300 float stores
and named-interface direct helper calls audited without whole-helper claims.

## Native applied injury severity across body regions

NativeCriticalSeverityApply0..7 invoke existing real factor fixture modes10..17;
Win/Linux resource extended timeout120 outside323. Real ICriticalInfo difficulty
observes injury, no new API. RED8/10 ->GREEN10/10 each incl2 prior no-critical/
blocked controls. Body/head642->300, hands428->300, legs535->300, fractionalbody/
head645->301; real damage/HP5/7/2/2/3/3/50/70 remains identical. Bleeding type/
count and GREEN AP/pose/life/inputDC retained. Both Game builds, related87Win/87Linux,
CI323. Earlier10 factor cases pass. Original rawfloat store/FISTPL truncate/
actual ApplyCritical call verified separately in each version; no extreme
float-int overflow or broader cap/perk/forced rule claim.

## Native maximum critical severity order with verified game binding

NativeCriticalCapOrder0..7: resource extended Win/Linux timeout120 outside323.
Two real native receivers compare real cap versus explicit-clamped input under
neutral1000. Zero-energy actual server attack binds game; read-only tag45 proves
cached identity. Bleeding-only after deployment/warmup. RED4/8 ->GREEN8/8 each,
four controls: cap50/input300 damagebody50->25/head70->30/hand24->15/leg37->20,
selected native bleeding grades now match reference (headnone). Equality/below/
zeroCrit/low input, HP/AP/pose/life/limits and four-word RNG preserved. Fixture
operator/deployment reset/unbound context errors archived, not RED. Prior69/70
neutral worldcap did not prove cached binding and is not cap-order evidence.
Full Game builds, related95Win/95Linux, CI323. No new cache-binding production change.

## 2026-10-31: NativeCriticalPKResistance0..5

Registered in both CMake lists, extended/120s. Actual mounted suit2 resistance.7,
person103, native bound-game entry/tag45 and direct RPG bypassPK flesh processing.
Complete RED4/6 ->GREEN6/6 each, two controls. Body/head/hand/leg damage
41/58/21/31 ->29/42/17/23; naked50 and zero-critical20 controls unchanged.
Injury severity300 and shellHP700/mount/AP/pose/input invariants retained.
Full Game builds, related101Win/101Linux, CI323 pass. Logs/original oracle and final
verification JSON use prefix critical-pk-resistance. Probability10000 isolates
damage factor, not ordinary frequency; full UI/penetration is outside scope.
Invalid inventory API compile logs are calibration, not RED.

## 2026-10-31: NativeCriticalResistancePerk0..7

Both CMake lists, extended/120s. Actual native TakePerk46/authored.1/point budget,
world deployment/game binding and mounted suit2 combination. Native RED5/8 ->
GREEN8/8 each, three controls; damage50/70/24/37 ->47/66/23/35, mounted29->28.
Unacquired50, zero-critical20, unacquired mounted29 stable; injuries DC300 and
mount/shellHP700/AP/pose/inventory/points/inputs retained. Full Game builds,
related109Win/109Linux, CI323. Evidence prefix critical-resistance-perk. Probability10000
isolates factor, not ordinary frequency/fullUI/penetration. Shim candidate8-way
ambiguity resolved by named RTTI/vtable, not an asserted unique byte pattern.

## 2026-10-31: NativeCriticalSeverityPerk0..7

Both CMake lists, extended/120s. Two native authored-class missions; actual
TakePerk47/parameter1.25 vs unacquired explicit reduced/capped reference, real
world/game binding. Final RED5/8 ->GREEN8/8 each, three controls. Severity301->240;
body/head/hand/leg HP50/70/24/37 ->44/62/22/33; cap270 case47->44, DC270->240.
Unacquired50, bindingcap15035, zero-critical20 stable. Injury grade/DC/location/
duration, four RNG words, native AP/pose/life/input/points/cap checked. Full Game
builds, related117Win/117Linux, CI323; evidence prefix critical-severity-perk. Preliminary
cap200 passing control archived, finalcap270 RED rerun before production. No
new ordinary-frequency/fullUI/zero-divisor/overflow claim.

## 2026-10-31: NativeCriticalForced0..7

Both CMake lists, extended/120s. Actual native portion flag and two receivers,
world/game binding, difficulty-fallback disabled/restored. Seed710 local SRand
ctor/Get4 then12/71 verifies rejecting chance1 conditions. RED5/8 ->GREEN8/8 each,
three controls: forced HP20/30/14/17 ->50/70/24/37 and BLEED/DC300; forced-1 body
20->50. Unforced1/forced0/unforced-1 body20/no injury stable. Four RNG words,
native injury/AP/pose/life/chance/flag/input/cap verified. Full Game builds,
related125Win/125Linux, CI323. OriginalSteam1.2 gates and typed field2d/tag14 prefix
mapping recorded; Gold's different trueflag zero-return is not target behavior.
Evidence prefix critical-forced; full UI/producer flows remain outside scope.

## 2026-10-31: NativeCriticalDifficultyFallback0..10

Both CMake lists, extended/120s. Four actual native units, actual difficulty flag,
level20/reference0 attackers and authored damage coefficient retained (base27),
native binding/tag45 and real46 grant controls. RED5/11 ->GREEN11/11 each, six
controls: d20 prefix527 resetsd5sum47 ->HP27->33/DC47; cap5 stillDC47, nullattacker
HP20->50/DC300, validlevel0HP27samebutBLEED/DC0, negative chance same d5 behavior.
Difficultyoff/resisted/passed/zero/forcedresisted/nullworld controls stable. Four
RNG words and native receiver/attacker HP/AP/pose/level/input/cap/difficulty checked.
Full Game builds, related136Win/136Linux, CI323. Evidence prefix critical-difficulty-fallback.
Separate float probability-threshold boundary remains unverified/pending; no UI claim.

## 2026-10-31: NativeCriticalProbabilityPrecision0..8 / source65558b11 closure

**Superseded precision inference:** the wide-significand claim and boundary
rejection below are incorrect under the observed original PC24/RN runtime.
See the later original-address critical comparison audit and correction.
Historical measurements are retained; they do not establish original behavior.

Both CMake lists, extended/120s. Actual authored46/.1/native grant/two world units,
exact native SRand boundary seeds and real zero/guaranteed reference attacks.
RED4/9 ->GREEN9/9 each, five controls. P100draw90/P50draw45/P30draw27 and resisted
AlwaysCritical boundary: HP47->20, wrongBLEED/DC300->none; adjacent89/91,
unacquired/zero/forced controls stable. Four RNG words, injury/AP/pose/perk/input/
cap/difficulty checked. Full Game builds, related145Win/145Linux, CI323; evidence prefix
critical-probability-precision. Exact original FILD/FMULS/FCOMPP has no FLOATstore;
ordinary authored boundaries certified, no extreme/custom/nonfinite/fullUI claim.
All functional source65558b11 hunks now have independent native/original evidence.

## 2026-10-31: NativeDBOverlaySentinel0..9

Both CMake lists, extended/120s. Actual authored DB/native Serialize READ overlay,
single-cell in-memory edits with native chunk lengths and real CP1251 Script56 /
UTF16 CString25. Accepted RED5/10 ->GREEN10/10 both; five controls. Int/bool/float/
narrow/wide markers now preserve output; ordinary0/true/0/Lua-return8/localized
control still apply. Record/table identity/count and another CP1251 script stay
exact. No new disk/game-tree copy/write or mock importer. Invalid include/eight-
character fixture attempts archived separately, final RED both beforeproduction.
Full Game builds, related156Win/155Linux, Windows existing NativeModManagerTests, CI323.
Evidence prefix db-overlay-sentinel; data-loader checks do not certify full UI/
script execution. Linux manager target unavailable, native overlay cases pass.

## 2026-10-31: NativeDifficultyCanSaveOverlay0..2

Both CMake lists, extended/120s, actual DB overlay fixture modes10..12. Accepted
RED1/3 ->GREEN3/3 each, two controls. Actual Difficulty1.CanSave marker previously
changed always2->invalid3 despite scalar false; now preserves2. Chapter/realtime
controls still apply2->0/1. Pointer/count3/CP1251 control stable; original wire
chunk parent lengths rebuilt in memory, no new game-file write/copy. Exact target
TESTAL/JZ before resolver/member24 and enum/name table verified. Full Game builds,
related159Win/158Linux, Windows manager regression, CI323. Data-loader state, not full
save-menu/UI or separate missing-column/unknown-name certification. Evidence
prefix difficulty-cansave-overlay.

## 2026-10-31: NativeMeleeCalledShot0..14

Both CMake lists register15 extended/120s actual world melee cases. Accepted
production RED9/15 ->GREEN15/15 each, six unchanged controls. Real CanDo/Start/
OnLabel/ray/native damage and actual borrowed-reference network/ordinary save/
legacy network roundtrips, not future-field/stub RED. Four region groups test
severity and low-probability acceptance; two hand/leg sides share independently
verified original switch branches. Actual level20*d20 prefix retained. Ordinary
save/old messages reuse live portions and must drop transient requested metadata.
Native HP/injury fields and actor/item/rule controls; no equality assertion for
post-world RNG versus a direct RPG call, no full peer/UI/resume claim. Invalid
fixture calibration logs archived separately. Full Game builds, related174Win/173Linux,
Windows manager regression and CI323. Evidence prefix melee-called-shot.

## 2026-10-31: NativeRangedCalledShot0..5

Both CMake lists, six extended/120s cases. Actual native CanDo/Start/generated
portion/carrier saver/native cover+physical trail/real bullet Segment+HP/injury,
not a future-field assertion. Accepted first-run RED4/6 ->GREEN6/6 each; rule-off
and ANY controls. All four region groups exercise the actual producer and the
existing consumer; actual38 grant and original level20*d20 prefix retained.
Real trail is bounded to first flesh entry, keeping one physical receiver;
no full firing animation/timing/travel/peer/UI or post-world RNG parity claim.
Actor AP/HP/pose/perk/level/ammo8->7 controlled. Full Game builds, related180Win/179Linux,
Windows manager regression and CI323. Evidence prefix ranged-called-shot.

## 2026-10-31: NativeToHitAwareness0..14

Both CMake lists,15 extended/120s actual native world/person/item/cover/to-hit
queries. Accepted RED2/15 ->GREEN15/15 each,13 controls; unseen/audible melee
33->50 and firearm34->41. Five awareness/null contexts per melee/firearm/knife.
Existing native cover snapshot/GetHitCover/GetToHit reference, no new engine API
or copied probability formula. Actual SelectTargetHLs, stored SEEALL visibility,
public audible list and day isolation. Four shared RNG words, AP/HP/pose/items/
ammo/contact state stable. Initial private-cover compile fixture invalid/archive;
corrected native saved-field reader before RED. Full Game builds, related195Win/194Linux,
Windows manager regression and CI323. No natural LOS/full cursor/UI/night claim.
Evidence prefix tohit-awareness.

## 2026-10-31: NativeToHitNight0..11

Both CMake lists,12 extended/120s actual native world/unit/cover/night queries.
First-run production RED2/12 ->GREEN12/12 each,10 controls. Firearm31->28 and
knife3->2 only for ordinary night inputs; day/melee/native21/script37/null
retained. Actual untaken21 actor/tree grant, saved cover source and existing
GetHitCover/GetToHit explicit reference; ordinary native skill-boundary search
0..100, no copied formulas/DB edits. Four RNG words/AP/HP/pose/items/ammo/
awareness/night/perk/script state stable. Full Game builds, related233Win/232Linux
including earlier night previews/incidental night, Win manager regression and
CI323. Current knife fixed range intentionally retained; no full knife accuracy/
physics or world lighting/LOS/cursor/firing/UI claim. Evidence prefix tohit-night.

## 2026-10-31: NativeKnifeWorldContext0..3

Both CMake lists,4 extended/120s actual native projectile/animator/world-clock/
physical flesh collision/HP/hit-locator cases. First-run RED2/4 ->GREEN4/4 each,
two controls. World difficulty2: damage2->4; anonymous/coefficient1 remain2.
Prime launch has no damage; actual500ms advances5m and retires on one real hit.
Authored model/item and saved carrier refs, actor AP/HP/item1/living receiver
stable. Existing stored trajectory supplied; no current-direction/OnLabel/full
flight/range/velocity/AI classification or whole action/drop parity claim.
Own debris removed, clock restored. Full Game builds, related277Win/276Linux including
knife and earlier night/incidental regressions, Windows manager and CI323.
Evidence prefix knife-world; complete original147-byte helper/typed call verified.

## 2026-10-31: NativeKnifeImpactDirection0..3

Four extended/120s cases in both CMake lists. Actual native crouched-head knife
collision/clock/animator and actual world CDecal mapping center, not metadata-only
assertions. Accepted RED2/4 ->GREEN4/4 each; stale UP/SIDE now produce one real
lower-surface blood decal. Correct DOWN high/low controls unchanged1/0. Damage
15/3, one physical hit/retirement,1m, actor/item/pose controls. Original normalized
current-segment dataflow and complete86-byte normalizer verified independently.
Unused nonserialized trajectory diagnostic removed and RED repeated before fix.
First complete related run exposed an old preview reference assuming day and
hearing-based awareness after fixes82/83; corrected direct reference inputs and
added environment/contact diagnostics, archived failure and repeated full gates.
Full Game builds, related281Win/280Linux, Windows manager and CI323. No full launch/
range/velocity/rendered frame/UI/post-world RNG claim. Evidence knife-direction.

## 2026-10-31: NativeKnifeLaunchTarget0..4

Five extended/120s cases in both CMake lists. Actual legal CanDo/Start/OnLabel ->
one real native knife/item/actor -> saved actual carrier -> real BODY trace and
existing native throwing callback/HP versus native target-aware reference.
First fixture RED3/5 ->GREEN5/5 each; two unchanged free-point controls. Selected
IC100 damage1->37 (visible and unseen/audible); IC0 keeps37 but target retained.
Unseen backstab0->1, bounds18..29->24..29 and critical52->100. Free-point1/37
unchanged. Native AP51->42->42/cost9, actor/item controls; cap1 and disabled
injuries/headshot/forced difficulty isolate damage; no full critical frequency,
flight/velocity/range/timing/drop/UI or post-world RNG parity. Rules restored and
own objects removed. Original named label/private producer/member28/callee
verified after RED. Full Game builds, related286Win/285Linux, Windows manager and CI323.
Evidence prefix knife-launch-target; no failed fixture/calibration.

## 2026-10-31: NativeKnifeCalledShot0..5

Six extended/120s native legal throw Start/OnLabel/projectile/item/actual carrier,
real HEAD/BODY trace, native throwing callback/HP and real bleeding DC/value/part
versus independently bonus-adjusted native reference. Actual perk37 grant,
level20 prefix175, cap1000 and native game-binding warmup/bleeding-only states.
Accepted RED2/6 ->GREEN6/6 each, four outcome controls. Head90->123/DC185->335;
body63->66/DC185->205. Flag-off head90/DC185; ANY/free/mismatch body63/DC185.
Raw100/10, AP57->48->48/cost9 and actor/item/grant/level/rule controls retained.
First all-criticals-disabled fixture was unobservable/all-pass, archived and
replaced BEFORE genuine RED/audit/fix. Original eHL30->portion54+3c and same
solver handoff verified. Full Game builds, related292Win/291Linux, Windows manager and
CI323. No full launch/flight/range/velocity/UI, ordinary probability frequency,
bleeding duration or post-world RNG claim. Evidence knife-called-shot.

## 2026-10-31: NativeKnifeChanceTruncation0..5

Six extended/120s cases in both CMake lists: actual native calcer/skill0..100,
authored knife/cover/contacts, actual unit helper AND player composite preview,
then actual CheckToHit at seeded next-int roll. First RED2/6 ->GREEN6/6 each,
four fully unchanged low-fraction/day-night/minimum2/script37 controls. Native
raw2.73483658/day and2.91429806/night return3->2; roll3 ratio1->2/3 hit->miss.
No query game-RNG draw; Check exactly1, subsequent4 words verified. Native AP/
HP/pose/item/contact/night/perk/script controls. Named actual target6b8a10 and
COMPLETE39-byte conversion tail/41-byte float clamps match originals without
masks. Full Game builds, related298Win/297Linux, Windows manager and CI323. No failed
fixture or full range/velocity/launch/flight/whole chance-formula parity claim.
Evidence knife-chance-truncation.

## 2026-10-31: NativeKnifeBallisticRange0..9

Ten extended/120s native range getter + actual admission cases, real equipped
authored blade450g, actual perk88/89 grants and point budget, native healing,
actual worn PK and native CSkillModifier changing only VP cap. Initial RED4/8,
extended RED5/10, then decoded original literals/fixed points repeat FINAL
RED7/10 ->GREEN10/10 each, three unchanged cases; no invalid fixture. Original
ranges11->19 skill,15->18 perk88,32->33 perk89,15->19 healing,15->32 STR; actual
fixed boundary denied->admitted. Worn PK pilotSTR12->22 stays14, cap-only VP500->
300/current300/theory500 stays18. Night/cover/actual launch15 controls retained.
Actual CanDo/Start/OnLabel one knife/item and saved deadline/velocity9.375m,
AP57->46/cost11; prevents fists/null context after tear-off. Query0 game RNG,
4-word/AP/HP/pose/item/perk controls. COMPLETE original285/48/49,557 blade suffix
and182 VP factor/59pow dispatcher audited with only verified relocations/data.
Full Game builds, related308Win/307Linux, Windows manager and CI323. No full speed/
continuous-distance/flight/accuracy-range/UI or cross-CRT extended80 bit claim.
Evidence prefix knife-ballistic-range.

## 2026-10-31: NativeKnifeAccuracyRange0..5

Six extended/120s native calcer/helper/player composite/CheckToHit cases with
real authored blade, real perk88/89 grants, real native cover and boundary rolls.
RED5/6 ->GREEN6/6 each; script37 public outcome unchanged. Reference supplies
only independently decoded literal min/max19/28,32/48,33/49,18/27,19/28,11/16
to native chance code. Public chance29->69,2->75,2->76,2->64,37->37,2->29;
affected boundary rolls miss->hit. THROW0 is affected, not a clamp control.
Queries0RNG/check1draw, four-word verification, native AP/HP/item/pose/contacts/
day/quality0/grant points controls. COMPLETE171 named original virtual function
and exact117 _ftol2 checked before production. Full Game builds, related314Win/313Linux,
Windows manager and CI323. No full flight/shared-skill/UI claim. Evidence prefix
knife-accuracy-range.

## 2026-10-31: NativeKnifeLaunchSpeed0..7

Eight extended/120s actual legal label ->native knife velocity/deadline/item/AP
cases. Initial6 genuine speed failures plus stale preheal HP control archived;
correct health control and repeatRED7/8, pin original wide-perk speed literal.
Archive unseeded RED/GREEN/full runs after review exposed occasional old scatter;
restore owned production to90 and seed actual GameRandom120 just before label.
Repeat FINALseededRED7/8 before production ->GREEN8/8 each and full gates. One lifetime/AP/item outcome
control does not assert a new speed; actual speed changes there too. Original
speed literals16.6124363/16.2141476/21.854887/16.6124363/21.4804249/14.7401333/
14.7401333/14.7401333 versus fixed5. Real native perks/healing/STR/night/cover,
one actual blade/knife and native vector norm1e-5 relative tolerance; lifetime
coherent within native clock rounding. Query0RNG/four words and AP/HP/pose/item/
grant controls. Actual original named label/direct velocity call/1.5f/store and
named world virtual receiver/typed signature audited. Full Game builds, related
322Win/321Linux, Windows manager and CI323. No post-world RNG, original absolute lifetime,
solver/cache/full collision-flight/drop/UI or exhaustive x87/CRT claim. Evidence
prefix knife-launch-speed.

## 2026-10-31: NativeKnifeFractionalRange0..5

Six extended/120s actual CanUnitThrowKnife cases, native authored blade5/450g,
range15. Offsets+.125,+.875,+.0009765625 wrongly allowed ->denied; inside-.125,
equality and outside+1.125 unchanged. Initial RED3/6 ->GREEN6/6 each; full gates
exposed two older perk89 probes incorrectly expecting33.125 inside range33.
Repair ONLY those fixed points to32.5 between before32/after33, restore ONLY92
production and repeat FINAL RED5/8 ->GREEN8/8 each: six new cases and both real
earlier native perk89 cases. Gate0->0 becomes11->0 at32.5; range32/33 unchanged.
Three identical measured controls, query0RNG/four words/AP57/HP500/pose/item/perk controls.
COMPLETE175 Gold795d50 ->uniqueSteam7a1b70 only two decoded E8 and verified1.6f
pointer masked; actual distancefloat store and integer-range comparison/order
verified. Direct current-position helper exact23 and prior complete physical
math retained; own enum0/11, no ABI/ordinal equivalence claim. Full Game builds,
related328Win/327Linux, Windows manager and CI323. Double adaptation, no exhaustive
extended80 or full flight/UI claim. Evidence prefix knife-fractional-range.

## 2026-10-31: actual knife lifetime in fourteen existing cases

Reused NativeKnifeLaunchSpeed0..7, NativeKnifeBallisticRange7 and extended actual
NativeKnifeLaunchTarget0..4; no new registrations. Initial pointRED9/9, then
selected-unit/freepoint coverage repeat FINAL RED14/14 ->GREEN14/14 each BEFORE/
AFTER production. Actual legal CanDo/Start/OnLabel ->real projectile/item/actor
->saved deadline/velocity versus original integer range, nine point cases plus
three selected units/two freepoints. Point ranges19/18/33/19/32/15/15/15/15 now
deliver integer distance instead*.625; selected/freepoint nominal18 now18 rather
than11.25. Clock tolerance1.1ms*actual speed; point speed literals/AP/item controls
kept; original selected target HP/native body callback continues after assertion.
Named world/factory/ctor chain, exact38/31 forwarding and complete44 clock block
only verified1000f pointer masked; actual allocationF8/FC difference not masked.
Full Game builds, related328Win/327Linux, Windows manager and CI323. Existing norm/float
clock adaptation, no exhaustive clock bits/extended80/full flight/drop/UI claim.
Evidence prefix knife-lifetime-distance.

## 2026-10-31: NativeKnifeSkillContext0..5 and original runtime/kernel audit

Six extended/120s real calcer/helper/player preview/CheckToHit contexts, actual
perk89 grant, actual VP-cap-only native modifier, healing and script37. Rejected
direct-perk60 and PC53-derived89 hypotheses archived BEFORE production. Exact104
original override +actual main-thread PC24 kernel prove raw100/current300/
theory500 ->skill90 for cap500 AND300; diagnosticPC53 alone89. Native FINAL
RED1/6 ->GREEN6/6 each: cap300 wrongly skill100/chance68 ->90/67, native roll68
hit->miss. Five identical controls: ordinary wounded90/67, heal75/64, healthy
50/56, real89 raw50/range16/chance59 and script37. Query0/check1draw/four words,
AP/HP/item/pose/grant/contact/day controls. Original probe used ONLY owned
process, restored full thread context/config and normal exit. Full Game builds,
related334Win/333Linux, Windows manager and CI323. Main-menu/kernel precision proof,
no live gameplay breakpoint/whole9f3/full flight claim. Precision follow-up on
earlier wide assumptions required; evidence prefix knife-skill-context.

## 2026-10-31: critical probability original-runtime regression correction

Existing9 real native modes now use actual original-address PC24/RN kernel
admission: boundaries100/90,50/45,30/27 and resisted AlwaysCritical accept.
Fresh production RED4/9 ->GREEN9/9 EACH, five identical controls. Boundary
damage20->47 and BLEED/DC300 match native guaranteed-critical reference; four
RNG words/AP/HP/pose/perks/input/cap/difficulty exact. Original comparison AND
branches executed at original addresses with controlled stack/chooser before
injury selection; all21 CW/operand modes retained, context/config restored,
owned original normal exit. Full Game builds, related334Win/333Linux, CI323.
Old instruction-only wide inference explicitly superseded. Evidence prefix
critical-probability-runtime; no whole helper/live-gameplay breakpoint claim.

## 2026-10-31: late incidental IC runtime precision regression

Accepted FINAL RED13/56 ->GREEN56/56 EACH: NativeIncidentalDamage8,
StoredAura13, WorldVictimAura12, ExplosionEvasion17, actualKnifeLaunchTarget5
and BulletImpactDodgeOrder1. The43 controls retain39 core outcomes plus four
actual knife-label outcomes; passing rolls are selected per process and each
run verifies four following RNG words against its own predicted sequence.
Actual HP/receiver/hit/reset/practice/perk/pose/item/input assertions remain.

InitialRED9/GREEN48 stale signed/aura controls and subsequentRED11/GREEN50 with
two downstream full-suite failures were archived. Production was reverted
before each accepted expanded RED. Negative world aura uses base2 to keep its
corrected3HP below existing blood-effect threshold5; original kernel also
verifies base4 giving6HP. Explicit knifeIC100 zero and bulletIC50/D4 two-HP
expectations now fail against old production and pass after the fix.

Exact original69-byte arithmetic/RC chronology and actual original-address
kernel30 control/input records with six CW/FX007f samples support the correction.
Full context/config restoration and normal owned-process exit verified;
incomplete timed-out probes excluded. Main-menu/kernel scope, no gameplay
breakpoint or full original-method claim. Full Game builds, related396Win/
395Linux and CI323 pass. Evidence prefix incidental-runtime-precision.

## 2026-10-31: explosive modifier original-runtime regression

NativeExplosiveModifierMath0..17, extended/120s on both platforms:3 new ordinary
.7/.9 signed cases plus corrected2 large-half expectations, all BEFORE RED.
FreshRED5/18 ->GREEN18/18 each,13 identical controls; actual native linked
function/typed records, structure/critical/other fields/trajectory/owner/RNG.
Original COMPLETE87 EXACT0 masks and actual-address36 operand/CW records confirm
product PC24/RN before CHOP conversion. Guarded Game-code/empty-x87 injection,
full context restore, six originalCW samples, normal WM_CLOSE/exit0/cfg restored;
early-exit/timeout attempts excluded. Full Game builds, related27Win/26Linux, CI323.
Isolated modifier math and existing grenade consumers; no new authored-perk
ownership or full explosion propagation/gameplay-breakpoint claim. Prefix
explosive-runtime-precision; prior wide/large-half explanations superseded.

## 2026-10-31: detection distance original-runtime regression

Existing UnhideBase8/BetterSpotting14/ObserverStealth14: fresh RED4/36 ->GREEN36
each,32 identical controls. Literal15/30 baselines23/1, trained63 and actual25
fixture19 replace22/0/62/18. Other skill/perk ownership/scalar/clamp/AP/HP/XP/RNG
assertions retained; only distance baseline production changed, with explicit
float product barrier against FMA contraction. COMPLETE original317 with only
four verified constant-pointer relocations, actual-address72 operand/CW records,
six observedCW007f samples, guarded context restore and normal exit0/cfg restore.
Separate24 actual live RPG getter/AP/weapon calls also showPC24/RN, but no live
perception/critical/lateIC capture is claimed. Full Game builds, related71Win/71Linux,
CI323. Evidence prefixes unhide-runtime-precision and original-live-fpu;
prior31 wide-precision explanations superseded. Direct probability API scope.

## 2026-10-31: fractional-cap skill training core

NativeFractionalSkillTraining0..23: actual person2/CUnit/dynamic cells, authored
record63 per-skill rates/caps, resource-backed extended Win/Linux timeout120,
outside CI323. FINAL RED14/24 ->GREEN24 each,10 identical controls. Fixed
progress literals checked within absolute1e-7, independently rechecked by
complete actual original151/helper75/initialized DB/Upgrade,24 operands x2 CWs
=48 records. Fractional capacity, used progress, rates20/60/10, min.00001/max.1,
LEVEL/zero/exhausted/legacy-offset cases; no promotion so all integer cells,
caps/other cells/XP/points/RNG unchanged. Original owned context/config restored
and normal exit0. Initial incomplete registration/stale LEVEL prerequisite
archived; accepted final native RED24 precedes production. Shared base method
is reached by existing live unit compatibility entry at factor1. Final full
build/related112Win/112Linux includes core24 plus native ack/impact/aura/critical/knife/
classless controls; CI323. Attribute/learning-perk wrapper remains pending;
no full action-flow or bit-exact log claim. Evidence prefix training-rates.

## 2026-10-31: training progress and cap truncate to whole points

NativeDynamicSkillUpgrade0..15: actual native public dynamic-cell method,
binary-exact fixed state/result literals, extended Win/Linux timeout120; full
RPG harness, no runtime DB argument, outside asset-free323. Half/fractional/zero
increments, whole/multiple points, fractional cap clamp, guard equality/above/
zero, negative progress, hurt raw current and frozen effective values; bool
quirk true on positive increment before cap clamp retained. Four RNG words equal.
24 prior base-training goldens unchanged. FINAL RED8/40 ->GREEN40 each,
32 identical measured controls. COMPLETE142 Gold/uniqueSteam EXACT zero masks,
actual original32 operand/CW records including real Update; AL/raw fields/frozen
match every literal exactly. Derived effective figure uses audited getter rule,
not an executed original getter. Six CW007f samples, guarded context/cfg restore,
normalexit0. Full Game builds, related128Win/128Linux, CI323. Associated-attribute WIP
was parked after original disproved its prior half-point promotion assumptions;
its wrong original outcomes were not accepted/committed. This independent fix
has a fresh native RED40 before production. Evidence prefix dynamic-skill-upgrade.

## 2026-10-31: associated primary-attribute practice

NativeAssociatedAttributeTraining0..21, actual units/all17 skills/all17 cells, offset7/-7
and primary-only/selected-only/both whole-point boundaries. Separate native base-call
reference checked by actual original full-unit code156+table36/
recursive/core/DB/Update/GetBaseStat,22 inputs x2CW=44 records. Exact integer/
base/raw-current/cap/frozen/bool and progress tolerance1e-7. Known raw current uses public
temporary unfreeze/restore, no private fields. Correct threshold1: STRcap1/rate100
atXP1m/progress.999999; no half-point promotions.16 existing Upgrade and24 base
controls/goldens unchanged. FINAL RED: 14 failures among 250 Windows / 249 Linux tests;
GREEN: 318 Windows / 317 Linux tests passed, 62 identical measured controls and374
identical initial/reference cell rows. Six original CW007f, guarded context/config restore
and normalexit0. Prior rejected half-point oracle led to separate Upgrade fix; this final
RED was on corrected baseline BEFORE associated producer. Full Game/related 318 Windows /
317 Linux, CI323. Extended native Win/Linux timeout120/outside323. Learning perks/live
mission flow remain pending; Old combat and knife controls keep their
damage/probability/velocity/range/clock/AP/RNG assertions with canonical
base+XP/held-modifier setup stable through real refresh; final RED250Win/249Linux preceded
producer reapplication. Broader critical/called-shot final gate covers that affected path;
evidence prefix training-attributes.

## 2026-10-31: authored learning-perk factors

NativeLearningPerkTraining0..15: real units/authored tree nodes and TakePerk grants
for11/60, owned/unowned category pairs, passive9 and LEVEL 13 guard. Independent audited
base-call reference; all 17 cells and four following RNG words. Native RED: 4 among 266
Windows / 265 Linux; full GREEN 334 Windows / 333 Linux, exact RED-test inclusion
verified.88 identical measured controls,272 learning and374 association initial/reference
cells unchanged; all legacy golden assertions retained. Full Game builds and CI 323;
extended tests remain outside323.

Actual original Unit156/table36 and real constructor/LoadTree/Take/Has/flags0 destructor
lifecycle,16 inputsets x2CW=32 records/full17cells. Exact integer/current/cap/freeze/bool,
progress 1e-7; points before/after practice, other learning category unowned, destructor
clears map and writes base vtable. Accepted real-stack scratch bytes/context/config
restored, six CW007f and normal exit 0. Earlier prefix/detached-stack attempts rejected.
Runtime is an isolated initialized main-menu kernel, without combined-perk or live mission
frequency parity claims. Evidence prefix training-perks.

## 2026-10-31: nearest-even rounding of modified skill caps

NativeDynamicSkillRounding0..15: actual production Update with real RAII modifier,
explicit public controlled input setters, fixed nearest-even literal caps and
signed/odd/even/non-half/XP/hurt/frozen controls. Initial constructor effects are reset
before the measured Update; no native private-offset access. Native RED: 8 among 282
Windows / 281 Linux; full GREEN 547 Windows / 546 Linux, exact RED-test inclusion verified
and no duplicate focused GREEN.100 identical measured controls and unchanged old272
learning/374 association reference rows. Shared-cap regressions additionally cover
wounds/perception/to-hit/AP/grenades/familiarity; full Game builds and CI 323. New tests
are extended/outside323, timeout120.

Complete actual original 178 at independently bound6bbb80, unique normalized178 with only
two verified cleanup calls and typed operands.32 original-address binary-exact input/CW
records, exact cap/current/base/XP/progress/freeze and stable live
modifier/vector/references. Effective getter result is derived, not executed; original
constructor/dead-entry/GC/ScaleForDifficulty are not runtime-proved by this matrix. Six
CW007f, real-stack bytes/context/config restored and normal exit 0. Unchanged Upgrade
truncation and old difficulty helper are explicitly checked. Evidence prefix
dynamic-rounding.

## 2026-10-31: medical and engineering action practice

NativeMedicalEngineeringPractice0..27 exercises real RegisterAction->mission->Unit
practice for7 restored medical/engineering actions, fractional and wholepoint cases,
plus14 existing/noop controls. All17 real cells compare with cloned native Unit UseSkill;
one/no acknowledgment and no duplicate, unchanged XP/points/RNG/movement/repetition
checks. Native RED14/91 both; full GREEN 374 Windows / 375 Linux, strict RED-name
inclusion,76 identical measured controls,476 initial/reference rows and old272/374 cell
rows preserved. Full Game builds and CI323. Tests remain extended/outside323, timeout120.

Actual original Register227/table80 and realwrapper196 are bound by
namedGold/typedRTTI/VTslot80 plus unique27 prefix with only verified Unit-call
relocation.28 fractional original-address input/CW records check476 cells and actual
action state; original acknowledgment/log truepath and live command frequency not
runtime-covered by this sparse fixture. Real-stack byte restoration/readback,
fullcontext/config restoration, normalexit0. All source156 hunks now accounted for
including earlier mounted reload; no obsolete hypothesis accepted as a regression.

Source: [156ccd173ff6c9671904e6d94e709397fc701d66](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/156ccd173ff6c9671904e6d94e709397fc701d66).

## 2026-10-31: Stealth practice on real Hide entry

NativeHidePractice0..9: actual World/persona54/player-turn CCmdHide factory/executor,
legacy/explicit entry, unchanged requests, exits, cooldown and repeated explicit request;
fractional and selected-Stealth wholepoint conditions. Native reference uses real Unit
UseSkill with AP charged first. All17 cells, one/no acknowledgment, four RNG words,
XP/points/position/inventory/ammo/action lifetime. RED5/43 both ->full GREEN 384 Windows /
385 Linux, exact RED-name inclusion and no duplicate focused GREEN.33 identical measured
controls,170 new and476 old initial/reference cell rows,16 unchanged action ACK events,
old explicit Hide rows unchanged. Full Game builds/CI323; Linux existinghelper targets and
owned display wrapper. New10 are extended/outside323.

Original Run123/DoAction160 uniquely bound by typed RTTI and only verified call
relocations, GetPose43 and hiding getters exact.6 actual original Register HIDE fractional
input/CW records check102 cell states; original AP stays52 because pure Register excludes
native command cost14/final38. This is a declared scope boundary. Original
executor/world/trueACK lifecycle is not executed in the sparse original probe; native
real-world integration tested separately. Original real-stack/context/config
restored/readback, normalexit0. DEX remains fractional in these fixtures; two integer
improvements concern selected Stealth.

Source: [2e15a10299e0d5e4a90558e39b34cc150ce51382](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2e15a10299e0d5e4a90558e39b34cc150ce51382).

## 2026-10-31: Original additional-round AP and short-burst perk

NativeExtraRoundAP0..15: actual native mission/tree10/gun/inventory/query, authored RoF25
and factor0.15; mode/ownership/floor/invalid/explicit/fallback/spent/max/remainder
controls. All17 cells, four RNG words, XP/points/ammo/item/mode/selection/authored data
unchanged. Original-backed literal costs replace initial hypothesis bounds before fresh
exact RED9/54 both ->full GREEN 555 Windows / 556 Linux.45 unchanged measured controls,170
Hide/476 medicine reference rows unchanged; full Game builds/CI323, all54 RED names in
GREEN. New16 extended tests remain outside323.

Actual Steam method504 at6bb6a0 bound via original mission action22 caller, not donor
midbody6bb6f0.28 actual original explicit-weapon records include real original104-byte
weapon virtual-base constructor/getters/destructor(flags0) and real
tree1/Take10/Has10/cleanup. PC24 costs17 vs diagnosticPC53 cost16 for
quotient20/factor0.15, original multiply/subtract before CHOP. Unit/AP and controlled
typed record unchanged. Native fallback10/13 integration tested separately; original
inventory/DBrecord/world/ammo construction and live shot frequency not claimed. Original
stack/context/cfg restored/readback, normalexit0, six observed007f samples.

Source: [24a00fd151a39d6c696c2ee8ab6deeb9d862f875](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24a00fd151a39d6c696c2ee8ab6deeb9d862f875).

## 2026-10-31: Original single-precision XP threshold calculation

NativeXPThresholdPC240..5 exercise the real native public Unit::GetXPForSkill query with
authored NPC838/cap15 at levels 0/1/2/3/5/6. Six literal original float bit patterns, 17
cells, XP, perk points, four RNG words and unchanged authored/live caps. RED 1/54 on
Windows/Linux -> full GREEN 573 Windows / 574 Linux; 53 identical measured controls,
strict RED-name inclusion, full Game builds and 323 asset-free checks. The six new cases
are extended tests outside those 323.

Complete original GetXPBySkill64 uniquely bound with only six verified constant operands
and a verified CRT call. Twelve actual original records cover six inputs under observed
PC24 and diagnostic PC53. Pure math inputs only: no original unit constructor, live
progression or mission execution claim. Original real-stack/context/config
restored/readback, six observed 007f samples and normal exit 0. Existing XP coefficient
and recipient dispatch remain separate work.

Audit context: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed); not a donor hunk or source completion.

## 2026-10-31: Original base XP reward coefficient

NativeKillRewardQuote0..13 use the actual native mission/Unit838 and authored cap15.
Effective level/party inputs cover ordinary values, signed and upper party clamping and
controlled negative-level zero guards. Fourteen exact original PC24 float bit literals;
all17 cells, XP/points, four RNG words and unchanged caps. Initial property RED12/41 both
archived; final exact RED12/47 both -> full GREEN 587 related + 11 supplemental Windows /
588 related + 11 supplemental Linux, 24 identical measured query controls and11 unchanged
periodic scenarios/measurement rows, full Game builds and323 asset-free checks. New14 are
extended tests outside323.

Complete original wrapper128 and helper70 independently bound through named symbols/typed
mission slot124 and only verified operands/calls. Actual28 original records at
observedPC24 and diagnosticPC53; all14 reference quote bits and current/next thresholds
match native GREEN. Kernel input operands cap/level/party are unchanged by the earlier
threshold correction; prior printed threshold values were not injected operands. Pure math
scope only, no original world/death/recipient AddXP/constructor execution claim. Original
real-stack/context/config restored/readback, normalexit0, six007f samples. Coefficient fix
is separate from pending recipient dispatch.

Audit context: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed); not a donor hunk or source completion.

## 2026-10-31: XP distribution after a fatal periodic pass

NativeFatalBleedingXP0..10 execute actual native World/TBS periodic callbacks, NPC838
death and real player rosters/AddXP. Fatal/nonfatal, neutral/ally, KO exclusion, double
callback, already dead, replica, two distinct hostile parties sharing scenario0, empty
roster and clue-KO suppression. Existing AddXP reference uses deep native Unit save/load
clones; compare recipient XP/LEVEL/progress/points, HP/death, unchanged input/reference
data and four following RNG words. RED4/58 both ->full GREEN 609 Windows / 610 Linux;7 new
scenario controls,36 measured query controls and11 old periodic scenarios/37 measurement
rows unchanged. All58 RED names in full selection, full Game builds/CI323. New11 extended
tests are outside323.

Original actual800-byte periodic function independently pinned by exact97 parent and51
clock/caller blocks. Typed primary/secondary RTTI, GetTBS+8, unit/player diplomacy,
CanFight/list, GetXP124 and UnitAddXP0 delegation are statically verified. Original
world/constructors/recipient AddXP are not executed by the pure original math probe.
Actual original GetXP128/helper70 has28 records; native cap15/level0/party0/1/2/3 quotes
and thresholds match its PC24 records. Native parties/recipients retain multiplicity
despite duplicate scenario/persona IDs. Source aa724 complete;56 pending/partial remain.

Source: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed).

## 2026-11-01: hostile-party XP at direct-attack incapacitation

NativeDirectAttackXP0..18 use actual native World, damage, KO, death, removal and party
rosters. References are deep native Unit save/load clones using existing AddXP. Two-phase
scenarios validate each phase separately. RED10/85 both includes nine reward assertion
failures and a real null-lookup crash; full GREEN 636 Windows / 637 Linux, full Game
builds andCI323. All85 RED names are selected,66 old controls/nine new controls
unchanged;21 input/reference phase records and20 comparable damage/state/attribution/RNG
records unchanged. The lethal removed-attacker case now passes. Real combat gib and
existing golden-shot removals retain weak server ownership and held logical RPG data.

Typed original DAMAGE+8 dispatch maps named Gold2058 to independently decoded Steam2122
at7c3dd0. Original static evidence proves pre-damage hostile roster/quote, removed-target
unconditional prepared payout and retained-target first-incap payout. Pre-KO removal
intentionally pays, matching the original early branch. Actual original quote128/helper70
has28 records; applicable native cap/level/party inputs match PC24 reference bits. No
original World/constructor/recipientAddXP runtime claim or fake providers. Sourcec923
complete;55 pending/partial sources remain.

Source: [c923fe9eaa63ed62aee9178976661a74d4daefd9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c923fe9eaa63ed62aee9178976661a74d4daefd9).

## 2026-11-01: movement-immunity and range accuracy perks

NativeAccuracyMovementRange0..9 use actual native authored missions/perk nodes, equipped
gun17 and TakePerk19/86; existing CTileToHitCalcer consumes shared Prepare/GetSMove/GetD1.
Movement0/3/13-cap10, first round, range owned/unowned/no-weapon and aimed/snap controls.
Real range50*1.25 truncates62, D1 skill30/min22 changes28 ->30.571428298950195
(bits41f49249, scale140). Initial abstract-fixture compile and rejected extra
scale100/D1=34 hypothesis are archived separately; final native RED3/60 both precedes
production. Full GREEN 674 Windows / 675 Linux/Game builds/CI323, all60 RED names included
and50 old measured controls unchanged. Seven new numeric controls pass; owned
zero-movement now returns original positive zero.
HP/AP/ammo/movement/points/skill/min-range/RNG checks remain intact.

Original typed Prepare256 is UNIQUE EXACT Gold/Steam. SMove105 masks only independently
verified positive-zero data pointer; D1 38 masks only equal float1/140 pointer, HasPerk
wrappers/actual callers/typed fields are bound. Original runtime construction/grants/query
execution is not claimed here. Other-input native D1 division versus original reciprocal
multiplication is an unproved precision candidate retained separately. Source9f3 remains
partial,55 sources unfinished.

Source: [9f3c03712f2a86f83fb7739891827847b2f9f645](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9f3c03712f2a86f83fb7739891827847b2f9f645).

## 2026-11-01: shared item skill and original accuracy arithmetic

NativeSharedAccuracy0..29 cover23 firearm and7 live-world grenade contexts through
existing native calcers. NativeSharedMeleeSkill0..3 add real authored nonthrowing/throwing
melee item and TakePerk70/89 API controls. Actual TakePerk/point budgets, stable native
skills, wounded/cap-only contexts, real turn counter, correct explicit AI grenade,
RNG/HP/AP/item/ammo/active slot and metadata invariants are checked. Genuine
existing-calcer RED17/90 both precedes production; added raw API and melee assertions are
GREEN controls only. Full708 Windows /709 Linux andGame builds/CI323 pass, all90 RED names
included,60 old measured controls exact. All34 PC24 raw return words match68 actual
original runtime records. Firearm consumers execute original Prepare256; grenade/melee
conversion uses a wrapper and does not claim original gameplay/physics/Prepare execution.
Original actual-vtable sparse buffers and controlled typed DB are explicitly scoped
separately from real item/tree constructors/virtuals/destructors. PC53 diagnostic
differences and rejected setup/compiler artifacts remain retained. Source9f3 complete,54
unfinished.

Source: [9f3c03712f2a86f83fb7739891827847b2f9f645](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9f3c03712f2a86f83fb7739891827847b2f9f645).

## 2026-11-01: original distance-skill interpolation precision

NativeD1Precision0..3 execute actual native Prepare/GetD1 on equipped authored gun17 and
real TakePerk86, with skill13/unowned and13/30/140 owned. Genuine RED1/98 both ->full712
Windows /713 Linux/Game builds/CI323. All98 RED names included,94 old measured controls
exact,3 new controls exact, all4 captured inputs/reference/HP/AP/ammo/points/RNG
unchanged. Mode1 old41cdb6db changes to original41cdb6dc, a one-ULP correction.

Typed D1 Gold6b0af0/38 ->actualSteam6b6be0 unique, only decoded/verified float1/140
operand relocated. Actual GetD2 calls1c and storesfloat. Eight actual original
Prepare256+D1 queries with real item/tree lifecycles and sparse actual-vtable
Mission/Unit/calcer buffers verify4 PC24 references; diagnostic PC53 recreates the old
mode1 word. Original D2/world/shot/DB-record construction is not claimed.
Stack/context/CW/config restored, normal WM_CLOSE/exit0. Source226 prerequisite only, its
own3 hunks still pending;54 unfinished.

Source: [226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4).

## 2026-11-01: grenade accuracy interval, stance perk and truncation

NativeGrenadeRangeStance0..15 use actual authored missions/Take92 and live-world calcers:6
interval cases regular/engineer/explicit backpack,6 owned/unowned stance cases,2 fixed
fractional realGetToHit cases and2 clamp controls. Unsupported initial positive-bonus
fixture and exploratory helper composition are archived separately; neither is canonical
RED. Actual stock bonuses-4/-2 mean ability92 selects WALK0. Canonical RED10/114 both
->full728 Windows /729 Linux/Game builds/CI323, all114 RED names included.91 old
measurements are exact;7 old shared grenade controls adapt only maxRange to1.5 while
retaining every other field/raw API word/RNG.16 original PC24 min/max/stance/D1 references
match. Fractional distances8/9 exercise the actual return conversion both before and after
range enlargement; bounds100/2 stay exact.

Typed complete Gold/Steam Stance200/Fill252/GetToHit191/BestPose73 plus exact
Min41/Max41/_ftol2117 helpers are verified.32 actual original
SharedSkill/Fill/Stance/D1/MaxVelocity records plus4-record lifecycle smoke use real
item/tree constructors and sparse actual-vtable Mission/Unit/calcer/controlled typed DB
buffers. Original GetToHit/grenadePrepare/world/flight/DB-record constructors are not
runtime claims. Stack/context/CW/config/refcounts/objects restored, normal WM_CLOSE/exit0.
Source226 complete,53 unfinished; source3cfc physical/perk/PK/Hide/executor work remains
separate.

Source: [226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4).

## 2026-11-01: live grenade velocity and physical distance

NativeGrenadePhysics0..14 target existing public velocity/range methods with real Take88,
regular/engineer/backpack items, raw70/140, fractional wound/cap/healed inputs, worn PK,
night and post-Prepare damage. Initial PK setup/override failures and input-only
exploration are archived separately. Final PK60/27/10 inputs are measured and pinned.
Canonical RED13/129 both ->185 Windows /182 Linux affected tests, full Game builds/CI323.
All129 RED names selected;114 old measured controls and2 new controls exact, all15 inputs
and query invariants fixed. All15 PC24 velocity/range words match30 actual original
records; PC53 diagnostics expose PK one-ULP and cap-only extended-return conversion
differences. Real item/tree lifecycles and sparse typed contexts are distinguished from
unexecuted original world/PK-wear/cache/flight/DB construction. The Windows flight/save
fixture independently failed on clean115 with default script chance0 and passed1/1 after
its harness explicitly supplied100. The final affected-suite rerun includes that corrected
fixture. Source3cfc remains partial,53 unfinished.

Source: [3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a).

## 2026-11-01: grenade fuse, PK launch and concealment completion

NativeGrenadeFuse0..15 covers actual executor/script fuse90 ownership, strict99,
fractional71.0999985/roll71 and52.5/roll52, exact delay words and8 continuation draws.
Exact native44-byte params and2064-byte input/reference ISAAC states are retained for64
actual original957 records, with16 lifecycle-smoke records; all PC24 reference and PC53
diagnostic outputs match in this matrix. The original global RNG test input is completely
restored, along with actual stack/context/CW/cfg/lifecycles. Native script construction's3
later physics draws are accounted separately. Initial record-selection/constructor-draw
assumptions are archived, not canonical RED.

NativeGrenadeHideLaunch0..2 exercises real Run, saved flag6 and deferred completion
visibility; NativeGrenadePKLaunch0..2 exercises actual worn shell/collider/solver and
script projectile launch origin. Initial consumed-save-stream fault was corrected by
rewind before canonical RED. A later initial full-suite PK HP invariant137->157 was
isolated to pending worn-PK derived-skill refresh; real UpdateSkills is now part of
fixture preparation, with a fresh6-case RED on116 before restoring117. The HP invariant
remains asserted. Genuine RED is12/145 +4/6 both,16/151 unique by union, followed by207
Windows /204 Linux affected tests, full Game builds and CI323. All151 RED names,129 prior
measured controls and6 new controls are included/exact. Native consumers versus
original957 runtime and original PK/Hide static blocks are bounded separately; no original
World/trajectory/Run runtime claim. Source3cfc completes;52 unfinished.

Source: [3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a).

## 2026-11-01: held grenade priority and engineering fallback

NativeHeldGrenadePriority0..8 uses actual Lua/commander/items and exact
SetActive/Exchange/Continue payloads, with both ordinary hands, mixed hands, engineering
fallback and both active slots. Native RED5/13 both ->79 Windows /76 Linux selected tests,
full Game builds/CI323. All13 RED names are selected,4 new priority controls/3 prior draw
cases/4 prior ordinary held measurements are exact, all9 inputs fixed. The old held
fixture's4 engineer-policy expectations intentionally change to exact queued exchanges;
AP/HP/hands/backpack/position/time/RNG remain unchanged before queue consumption.
Completed exchange/animation execution is outside this producer proof.

12 actual original143 classification/Get/GetActive/realGrenadeGetDBItem/CRT records plus6
smoke records distinguish empty/ordinary/engineering at both active slots in PC24/PC53.
Sparse actual-vtable Inventory64/borrowed slots8 are not original
InventoryCtor/Equip/World/Lua/queue. Actual Steam getter reads slots30 versus Gold34;
actual complete27 bytes and typed slots bind it without offset/branch masks. Full RNG2064
read-only equality, lifecycles, buffers, real stack/context/CW/cfg and normalclose0 are
checked. Actual Lua registry/ordered calls/fallback are statically bound separately,
including rejection of a whole43-byte comparison with changed branch distances. Source14ff
completes;51 unfinished.

Source: [14ffbcc83ea0fff503aa22386c98274a5aadd9ca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/14ffbcc83ea0fff503aa22386c98274a5aadd9ca).

## 2026-11-01: camera framing floor gates and authored PK concealment

NativeCameraFraming0..9 exercises real public framing, strict35 boundaries and independent
single-point frozen/clamped/saved-floor cases. NativePKHideGate0..7 exercises actual worn
authored allowed/forbidden records, factory acceptance/cooldown and zero-cost
unhide/toggle Run; entry LOS/AP is outside scope. NativeCameraFramingBridge0..2 uses
actual concrete CMission factory/accessor/forwarding with a real initialized view/world
dependency attachment, no behavior override, and temporarily null camera-world handle; it
does not claim full tactical initialization. Initial menu no-op forwarding assumptions are
archived and excluded from canonical proof. Native RED9/10 +3/8 +2/3 both is a14/21 unique
union. Seven controls, inputs and hypotheses remain exact; search-angle preservation is
intentionally pending1b85 candidate order, not an original full-pose reference.

10 original base-floor records/4 smoke execute real220-byte base-subobject
ctor660/non-pure43-byte setter/getter/range/Freeze/Lock/dtor in PC24/PC53, with exact5
floor references, read-only full RNG2064 equality,
lifecycles/stack/context/CW/cfg/normalclose0. Actual original rod and PK gates are
independently typed/unique/static. Original full World/framing/LOS/bridge/PKCanDo were not
executed; null-world full framing was rejected before runtime, including detection of
actual9/5-argument signatures and changed factory sizes/addresses. No
branch/opcode/frame/size masks or fake provider. Full Game builds/86 Windows /85 Linux
selected regressions/CI323 include all21 RED names. Sourcea675 complete,50 unfinished;
source1b85 next.

Source: [a6753bf56be2d09c3b84412b4285db2ff54d6306](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a6753bf56be2d09c3b84412b4285db2ff54d6306).

## 2026-11-01: camera candidate order and local probability window

NativeCameraCandidate0..7 exercises existing public real-camera framing and serialized
desired pose in supported no-world contexts:4 changed yaws at rod40 ->rod15/current yaw,4
exact base/opposite/fast-path controls. NativeCameraSloMoWindow0..5 uses real
campaign/world and cheat option with existing local-seed harness: count3/divisor1.6/seed1
changes window2->1, count3->4 and retained lease0->4000, with5 exact controls.
Deliberately all-failed framing resets final ratio1; initial invalid final-ratio3 premises
are archived, corrected canonical RED5/14 both before producer changes. Captured
inputs/live pose/world time/next gameplay ISAAC stay fixed. Later candidate groups and
strict tie are statically audited, not separately isolated native runtime here. Forced
local extra roll is static-only because native temporary dies without another use on the
forced path; no invented global RNG effect/test hook.

Actual typed Steam926/474 functions differ from Gold921/458 by extra9th/5th argument;
own8arg API remains. Unique actual36-byte truncation block, exact raw23-byte tie branch,
independently decoded array/call/yaw operands and unique46-byte unconditional-roll block
with typed whole43-byte SRand.Get bind original behavior without opcode/frame/branch/field
masks. No new original runtime records or full original World/framing/LOS claim. Full Game
builds/100 Windows /99 Linux selected checks/CI323 include all14 RED names. Three119
search-yaw preservation expectations intentionally follow restored120 current-yaw-first;
remaining119 camera measurements exact.49 unfinished sources remain.

Source: [1b85f23ac337fb5f2f4d6acde5341cfe66bb757a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1b85f23ac337fb5f2f4d6acde5341cfe66bb757a).

## 2026-11-01: PK pilot exit owns the forward placement

NativePKPilotExit0..7 runs actual authored shell93/human54/world/path/wear and existing
public callback in eight directions, measuring real simulation and current animation-graph
standing placements. Canonical RED8/10 both ->zero pilot/shell/animation placement error,
with fixed reference inputs/AP/healed wounds/shell HP/inventory
identity/time/callback/poses. NativePKPilotExit8/9 retain two exact
worn-ignore-target/unworn-invalid-command query controls, including next RNG word. Initial
invalid border-footprint assumptions are archived; corrected actual geometry selection and
independent post-wear reference precede canonical RED and producer change. No fake
provider or new game helper. Blocked-abort runtime, completed action/AP-spending/timeline
and full post-callback RNG are not claimed.

Actual typed Steam7a64a0/1058 via RTTI8ccbe4/slot1c uniquely matches Gold799ff0 after
decoded E8 operands and independently checked float/namedRTTI/EH absolute operands only;
original opcode/frame/field/branch/size bytes remain exact. Original clear-link
snap/passability-before-detach and distinct shell/pilot simulation/animator argument flow
are audited. Callee names are comparative in-method PDB mappings, not separate
whole-callee equivalence; no original World/callback/animation runtime or patched original
state. Full Game builds/116 Windows /117 Linux related checks/CI323 include all10 RED
names. Sourced44 complete,48 unfinished.

Source: [d44bba0e1f5ecb91f589ef70e205eb0cacfc6285](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d44bba0e1f5ecb91f589ef70e205eb0cacfc6285).

## 2026-11-01: PK collapse, stun and ordinary corpse refresh gate

NativePKCollapse0..6 exercises actual
world/wear/death/KO/scripted-KO-animationfalse/nonstanding-death/STUN ProcessAttack and
exact ordinary/no-STUN controls. Five changed cases produce one positive-duration
authored2484/2466 clip, actual idle-ban hold policy and world16584 sound;
logical/worn/shellHP/time and captured AP/HP/consciousness outcomes stay fixed. Hold
policy is tested, not future bone frames. NativePKCorpseGate0..3 exercises actual base
Segment/GetCorpseHLs: downed wearers6HL/moved->0HL/unchanged, human corpse6 and living
wearer0 exact controls. Canonical RED7/11 both,4 controls/inputsfixed; no fixture failure
relabeled as RED. No fake provider or DB changes. Bounded segment/cache component, no full
turn/AI/render claim.

Actual Steam223 helper74fa60 via typed attack751570 and Fall750ec0 has different
behavior/optimized register+stack ABI from Gold137 helper7444c0; whole-helper comparison
rejected without frame/register/opcode/size masks. Exact AlignTime19, actual PlayCustom
third-bool idleban and real typed MakeSound slot130/clip/sound constants bind the helper.
Whole original Segment615 matches uniquely7510b0 with decoded E8/EH/exactfloat2 operands
only; exact26-byte worn gate skips ordinary corpse refresh. Original full
World/clip/audio/Segment runtime not executed. Full Game builds/127 Windows /128 Linux
related regressions/CI323 include all11 RED names and prior placement tests; all10
prior121 pilot-exit measurements exact. Sourcec941 complete,47 unfinished.

Source: [c94140922b034b3e11dd9e2a1d740d32c559e488](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c94140922b034b3e11dd9e2a1d740d32c559e488).

## 2026-11-01: hearing probability, quiet practice and crawling noise

NativeHearingProbability0..5 uses actual authored perks20/76 and independent unperked
mission references, integral/fractional SPOT, human/PK sensor branches and valid null/deaf
source cases. NativeHearingPractice0..3 checks real CanHearSound against separately
trained unit skill cells and4-word d99 continuation: loud progress->0, quiet success
unchanged, misses unchanged. NativeMovementNoise0..3 primes prior hearing through actual
World, clears prior event/progress and checks real SetPosition heard
event/marker/AP/HP/time: crawl1->0, walk1/stance0/turn0 exact. Initial missing DataAI
include and invalid TBS-visible-list premise are archived setup failures. Canonical
RED5/14 both is4 behavioral assertions +valid null-source SIGSEGV;9 exact
controls/inputsfixed. No DB changes or fake provider.

Typed actual probability517/step285/position742 match whole Gold bodies uniquely with
decoded E8/EH/equalF32 operands only, preserving opcode/frame/field/branch bytes.
Actual179 hearing caller differs from Gold182's training call; separately audit
quiet-output/d99/quiet-gated mission SPOT6, no forced whole match. Native positive77
low-coefficient boundary is not separately isolated; original exact285 positive0
test/reciprocal supports it statically. Original World/hearing/RNG runtime not executed.
Full Game builds/220 Windows /221 Linux selected checks/CI323 include all14 RED names;21
prior PK measurements exact. Source242 complete,46 unfinished.

Source: [242f6dfd827d93014c4210980f9d1c4af2a45b67](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/242f6dfd827d93014c4210980f9d1c4af2a45b67).

## 2026-11-01: heard unseen AI camera and live RPG ownership identity

Register NativeRPGOwnershipAI0..10, NativeRPGOwnershipDamage0..9 and
NativeHeardAICamera0..11 for both native platforms, extended/120s. Real map/party owners
and authored units exercise original-spawn/new-merc/owner-change/null/snapshot/tag46
consumers. Damage uses actual CAttackPortion and untouched distinct authored difficulty,
fixed100 flesh damage, independent coefficient/nearest-even reference and4 ISAAC
continuation words. Hearing uses actual loud record, blind/see-all visibility,
deaf/null/dead-source controls, prior real hearing, two listeners, ownerless sequence and
saved flags diverging from current commanders; exact native point/floor/priority payload
is drained from the actual World UI queue.

Corrected canonical RED15/33 both ->33/33 final new tests; unchanged production bytes also
pass416 Windows /414 Linux related tests. The initial broad run passes416/412 related
names; the2 missing Linux portable camera targets are then explicitly built/rechecked in
final35/35 batches both. Combined449/447 distinct names, Game both and CI323. The broad
run's sole actual failing assertion is the unasserted mode2TBS1 premise, archived and
replaced by the genuine asserted sequence control; no game-condition change was made to
satisfy it. Related-only counts exclude all new cases, and no single all-green449/447
CTest batch is claimed. All35 prior123 hearing/noise/PK samples exactly match. Original
support is static only; no full World/mission timeline/RNG runtime claim.

An intermediate restore copied the correct producer bytes with older timestamps, so both
incremental builders retained RED objects. That35-case attempt is archived; the final run
refreshes both source timestamps and verifies wMain/wUnitServer recompiled before
accepting any GREEN result.

Source: [1715d48307f8282d20192425599c5b4253c335ea](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1715d48307f8282d20192425599c5b4253c335ea).

## 2026-11-01: whole-hull cover projection, extended raster bounds and normal range caps

Register NativeCoverWholeCenter0..7, NativeCoverProjectionExtent0..5 and
NativeCoverRangePolicy0..7 on both native platforms, extended/120s. Actual map/hull
delegation checks selected-part versus whole-hull optical axis and whether geometry just
past an8/40-unit target remains inside the producer's real native frustum. Actual
CFastRenderer closed-box input plus the existing cover walk checks40-unit human/AI
targets, near positives, tile0.625 pullback and30-unit ally-clearance controls. Query
invariants capture AP/HP/poses and4 shared RNG continuation words; framing also preserves
world time.

Canonical RED15/22 both ->22 new cases pass. The affected final38/38 also cover all8
existing projection and8 existing range controls after replacing superseded
uncapped33-unit references with actual29-unit near controls and the old30 plane
with29.625. Other371Win/372Linux related tests passed in the broad run on identical game
bytes, giving409/410 combined distinct names plus full Game both/CI323. All33 prior124
native RPG/camera/difficulty samples match. Initial broad4 old-reference failures are
archived; no single all-green409/410 batch is claimed. The original933/540/FPTAN17/caller
evidence is static only; no fake interval/score provider, original executable patch or
original full World/firing runtime claim.

Source: [01eb873b41cd2c8b9ce2d7b81835d2e00d492118](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/01eb873b41cd2c8b9ce2d7b81835d2e00d492118).

## 2026-11-01: particle half-texel margins and grass cell insets

Register NativeParticleAtlasMargin0..5 and NativeGrassAtlasMargin0..7 on both native
platforms, extended/120s, with distinct isolated profiles/headless audio. Each uses an
actual hidden SDL/bgfx display, real RGBA8 atlas texture and current-container checks;
normal cleanup closes the owned display. Rain emits a real front-facing quad with literal
vertex/color/depth controls. Grass emits every cell of1/2/3/4 grids, checks fixed
cell-boundary tables and independent fixed sprite-order tables, captures native quad
output and records geometry hashes. Both compare4 RNG continuation words and preserve
cache region/owner and source identity.

Canonical9/14 RED both ->32Windows/32Linux selected names, all14 new names selected, full
Game both and CI323. Five exact HD controls and all geometry hashes remain. Both
actual32-test suites include bgfx rendering, HD graphics options, stencil, resource
cleanup, texture filtering/sampling and portable particle wire. No synthetic
UV/interval-return helper or PNG generation; the resource node only supplies the actual
allocated texture. Initial fixture compile errors and Linux missing3 executable preflight
are archived rather than counted as RED. Original481/678 and independently
named/default/zero call evidence is static only, with no original full renderer or
rendered-pixel claim.

Source: [24c1eca77dbcf2eff5feaa8ef603f390df93f7fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24c1eca77dbcf2eff5feaa8ef603f390df93f7fb).

## 2026-11-01: global-map desktop ownership, draw refresh and globe selection

Register NativeGlobalMapDesktop0..9 on both native platforms, extended/native-display/120s
with isolated profiles. Actual authored Axis map3/campaign/native world/UI constructs
normal and optional globe desktops, checks the real child view and XP/stack identity.
Draw-only tests select an actual authored polygon input, then observe selected state and
base flash through existing serialized object fields. Native complete graph saves
reconstruct normal/globe maps. The legacy variant rewrites only known historical flat
UI-base/empty desktop-list fields, retains all actual object records/children, reads fresh
objects and restores twice to verify one desktop and intact window fields/client identity.
No replacement hover/flash/output provider.

Canonical native RED7/10 both,3 outcome controls ->10/10 both and208Win/181Linux distinct
coverage/Game both/CI323, including all14 existing normal/modal global script, hint,
saved-world and legacy-world cases. Windows selected batch is all green. Linux combines179
initial passes with two unchanged-code targeted retry passes after startup/resource
timeouts; explicit headless audio/owned runtime directory, unchanged-source hashes and
original raw logs are retained. No single all-green181 Linux batch or unique timeout cause
claim. Update that older suite's explicit null-desktop premise to real ownership; its
world/script/XP/return assertions remain. No original runtime/rendered-globe screenshot
claim. Archive initial private typed serializer compile error, static rectangle hit-test
fixture and missing Linux registration; exclude those preliminary runs from canonical RED.
Sourceb91 complete.

Source: [b91ea228f2a5443af74d0ae728e24410f7d90fad](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b91ea228f2a5443af74d0ae728e24410f7d90fad).

## 2026-11-01: shared fixed raster edges with original projective guard planes

Register NativeGenericRasterFast0..7, NativeGenericRasterFog0..7 and
NativeRasterGuardPlane0..19 on both native platforms without database/display
dependencies,60s. These36 native CPU cases belong to asset-free CI, increasing323 ->359.
Fast cases trace real closed hulls through CFastRenderer into sorted interval grids, with
literal pixel rows, source/user identity, flat1..3 depth and4 RNG continuation checks. Fog
cases use the existing native CBilinearTexture input node and actual ISW2DScene
CreateSpot/DrawFog, with literal pixel rows; no replacement raster/coverage provider.
Half-pixel ties, adjacent untied/integer boundaries and clipping controls isolate6/16 RED
with ten exact outcome controls.

Guard cases trace actual native projective closed hulls at radii5..50000, four entirely
external and four half-frustum boxes. Independent double ray/box slab intersections use
unchanged native projection directions and literal1024/0/512 cell counts; compare every
native cell's presence, source/interval chain and depth plus RNG state. Unguarded fixed
edges yield15/20 RED both; five geometric controls pass. Near+four guard planes restore
all20 and the combined36. Final439Win/435Linux selected suites and full Game both pass,
CI359 includes every new case. All14Win/11Linux old failures without guard recover without
changing their fixtures/assertions. Initial unguarded full logs and Windows oracle macro
compile errors remain archived; compile failures are not RED. No original UI/renderer
runtime or rendered-image claim.

Source: [70371f39f6fb4a8cc7cdc15b117f1c704cc81aeb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70371f39f6fb4a8cc7cdc15b117f1c704cc81aeb), [3bd9a53bf0cdd3a2e70c6418029c80f8e11325f5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3bd9a53bf0cdd3a2e70c6418029c80f8e11325f5).

## 2026-11-01: original AI cover trace and transient-lock-free flanking

Register NativeAICoverDepth0..7, NativePathWaveFlags0..3 and NativeAIFlankWave0..3 on
Windows/Linux as database-dependent extended cases, isolated profiles, headless
audio,120s. Cover modes call actual CalcCovers/GetCoverForAIUnitPrecise and observe the
real native frustum while delegating world geometry. Distances8/40 and probe
distances9/20/29/41/43 separate AI trace0 from human trace30 and preserve far-target
expansion. Path modes call the real world/table wrapper with script pose-lock and16/0
budgets, explicitly asserting the independently decoded original flag-shift defect. Flank
modes use real factory AI wrappers and CAIAttackPlaceSource, an actual navigable5x3
clearing, authored RUN cost2, eight native allied units and independent
respect/ignore-lock reference floods; test a perpendicular unoccupied goal outside the
allied ring. No replacement raster intervals, AI state providers, path costs or scores.

Canonical RED4/16 both ->16/16 both,12 exact controls; final455Win/451Linux in
separate447/443+8 all-green batches/Game both/asset-free CI359. AP/HP/physical position
and4 RNG continuation words match before/after every query. Initial missing-header compile
failures, over-wide7x7 clearing assumption, WALK budget failure and wrong expectation of
correcting original pose-lock behavior are archived separately. These are not canonical
RED. Collector/safety sourcefc remains pending.

Source: [9cf3fd3dae155de3b0c0e1de443cf54a7f494426](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9cf3fd3dae155de3b0c0e1de443cf54a7f494426).

## 2026-11-01: original ally-ray classification and AI geometry safety

Register NativeAllyRayClassification0..5, NativeAllyRasterSafety0..8 and
NativeAllyCoverCache0..1 on Windows/Linux, extended, isolated profiles, headless
audio,120s. Controlled closed collision hulls carry actual native unit ownership and
authored flesh material through CFastRenderer and the live cover walk; no supplied
intervals/classifications/scores. Classification covers ally-before-target (AI/human),
target-only, enemy interposition, ally after target and minimum-clearance controls. Native
factory AI wrappers, actual commander/state enumeration and a real CAIAttackPlaceSource
current-place consumer separate collision geometry from the old centreline, cover allies
behind the target, off-axis/enemy/absent controls and independently proven
missingenemyfalse/state-nullweaponpriority. NativeAllySafetyAccess only exposes the
existing protected method; it supplies no AI result.

Canonical RED7/15 both ->17/17 both. New live-API tests are post-fix integration controls,
not an invented compile-failure RED: hit/loose objects are collected, obstacle-only
objects excluded, prior map keys cleared, READ invalidates runtime identities, fresh
native CalcCovers rebuilds them and legacy saved bytes/fields are exact. All8 original
controls and AP/HP/ammo/position/4 RNG invariants remain. Final506Win/502Linux single
all-green/Game both/CI359. Every17 new and16 prior AI-depth/path/flank case appears in the
actual final selected manifests. Initial Linux registration/constructor/incorrect donor
no-enemy expectation logs retained separately.

Source: [fc2836a521913dafcb7737659c6912f178dc5ef1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fc2836a521913dafcb7737659c6912f178dc5ef1).

## 2026-11-01: original integer shadow and occlusion depths

Register NativeIntegerShadowDepth0..18 on both native platforms, resource-free CPU,60s,
inside asset-free CI (359 ->378). Input nodes provide real CObjectInfo AssignFast meshes,
native IPart objects and conservative bounds to the actual public
MakeInvisibleElementsList/Fast/GeneratePartList and IHZBuffer queries. No
depth/refcount/visibility/ignore result is supplied by the input provider. Real
TransformPart, fixed edges, near/guard clipping, part span ownership and HZ hierarchy run.
MakeMatrix(origin,+Z) is explicit; verify actual GetCoverRect premises rather than the
engine's identity-camera+Y convention. Initial wrong-camera exploratory output is archived
and excluded from canonical RED.

Canonical RED10/19 both ->19GREEN,9 exact outcome controls. Flat reciprocal-depth ties
retain first owner after quantization; raw word wrap and the >65535 sphere shortcut
preserve original behavior; quantized sphere comparisons, order/separated planes, odd
sizes, six-view spherical bounds, two fractional-X/Y sloped planes and repeated Fast
object identity are checked. Mesh positions/source-node identity and4 RNG words are
unchanged. Full Game both/final527Win523Linux/CI378, all19 actually selected in final and
CI manifests.16 prior generic raster rows are exact, all20 guard cases pass. Original
evidence is static with complete typed functions and explicitly bounded caller blocks; no
original runtime/rendered-image claim.

Source: [acf8dc8198a56959358dc0f226cf2613d8d56b4e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/acf8dc8198a56959358dc0f226cf2613d8d56b4e).

## 2026-11-01: original game icon in the existing SDL window

Register Windows-only NativeGameWindowIconTests as extended/native-display,60s, using real
s2_platform and the same Game/Game.rc as Game. The observer creates its own hidden SDL
window, reads native class/window HICON, loads resource101 through actual Win32 resource
APIs and rasterizes32px BGRA with DrawIconEx. Original fingerprint is bound independently
to all7 original Steam resource payloads and unchanged donor ICO. Loaded and native
resource-selection controls pass before the fix; only image identity fails (RED1/1
->GREEN1/1). The default-size class/resource comparison permits the native DPI-selected
icon; explicit32px resource comparison verifies original pixels. Close the owned window
through S2Platform::Done; do not destroy SDL/shared icon handles.

Final14Win single all-green; Linux12 distinct as11 initial passes plus1 unchanged-code
invalid-driver retry without the inherited x11 environment override. Initial Linux failure
is archived and not a source regression or fresh RED. Full Game both/CI378 unchanged, new
platform window test excluded from asset-freeCI. Separately inspect actual final Game.exe
as a resource image and confirm all7 embedded RT_ICON payloads plus32px native
fingerprint; this is not an original Game.exe process/runtime or rendered-game parity
claim.

Source: [65b2d080d3a95f9bf713fbda814c2b39c68c74a4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/65b2d080d3a95f9bf713fbda814c2b39c68c74a4).

## 2026-11-01: wounded AI fear and live nearby support

NativeWoundedAIScared0..17 are extended DB cases on Windows/Linux, resource-freeCI
excluded. Real FixtureWorld/NativeHearingOwners, authored personas, actual native
commander/factory units and SAIUnitState::Update consume the state. Cases check VP49/50/51
of100, healthy2/3 contacts, live neutral/own support despite stale team cache, enemy/KO
support, actual positions below/above9, stale zero enemy planningHP,
help-called/already-scared/missing-enemy controls and real pilot/suit ownership. Modes6/7
repeat neutral support input; exact9 distance is not claimed. No substitute fear predicate
or fake roster outcome. Freeze derived enemy-list refresh to isolate the actual
CheckScared consumer, retain actual world support query, verify Modified and untouched
state/physical/planned AP/HP/positions/contacts plus four RNG words.

Canonical RED8/18 both ->GREEN18/18 both and10 exact unchanged controls. Discovery failure
from impossible exact9 on this uneven map and unregistered added support is retained
separately; canonical fixture uses actual8<d<9 /9<=d<10 and registers support through
CAICommander::OnUnitAdded. Full Game both/389 Windows/392 Linux related cases single
all-green per platform/CI378 pass. Source partial; combined contact-list and
reachable-retreat/timing cases pending.

Source: [98871d0f0def4f966e097b984c18e57a8f437d7b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98871d0f0def4f966e097b984c18e57a8f437d7b).

## 2026-11-01: combined threats and deferred reachable retreat

Register NativeWoundedAIContacts0..5 and NativeWoundedAIRetreat0..7 as extended DB cases
on both platforms, outside asset-freeCI. Native factory identities, actual threat
vectors/const-reference caller shape, actual SAIUpdateTracker/Normal/Retreat/Guard and
borrowed saved destination fields exercise the consumers. Contact cases cover
empty/confirmed controls, suspected-only, possible-before-confirmed order, duplicate/null
preservation and copy lifetime after source vectors clear. Retreat cases use real authored
navigation and units: genuine suspected-contact destination deflection, empty/retainedKO
contacts, stale planningHP0, actualAP0/fixed30 wave, incumbent/no-route timing. Compare
seeded existing native helper only for caller routing; independently check native wave
membership/cost<=30 and distance>=5. Native goal selection is checked against each
platform's own helper; no cross-platform selected-point identity is claimed. No substitute
path/fear outcome. Contact fourRNG unchanged; retreat fourRNG equals original-path
consumption, plus physical/planned AP/HP/position/contact/wish-pose invariants.

Canonical RED12/14 ->GREEN14/14 both; two exact contact controls. Initial implicit
pointer-vector compilation and unavailable far-grid fixture are archived before
canonicalRED, with no distant-grid coverage claim. Existing confirmed-promotion test now
names the confirmed vector directly. Old alarm reference had11/12 initialWindows because
Human assumed deterministic fallback; adjust only expected fallback RNG to actual5/30
helper, retaining threshold/40percent/command/contact assertions. Final 403 Windows/406
Linux related single all-green, all12 prior alarm cases and all18 prior fear cases
explicitly selected; prior18 rows exactly unchanged. Full Game both/CI378. Source988
complete,34 unfinished.

Source: [98871d0f0def4f966e097b984c18e57a8f437d7b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98871d0f0def4f966e097b984c18e57a8f437d7b).

## 2026-11-01: tooltip options and original hover deadline

NativeTooltipDelay0..11 are extended DB/dummy-SDL cases on both platforms, outside
resource-freeCI. Genuine CInterface, CWindow, CToolTip and existing shared sound are used.
Replace only the saved render-view reference with a literal frame-boundary observer that
throws from StartNewFrame; actual interface Draw must execute the clock before that
boundary. Packaged-font formatter remains available for the next measurement stage. Native
Step handles disabled-tooltip teardown. No fake visibility, clock result or owner
lifecycle. Trace every visibility transition and verify four RNG words, owner geometry and
shared-sound identity. Empty-tooltip and cleared-owner modes5/6 are exact unchanged
controls.

Canonical first valid discovery RED10/12 both, byte-identical red-file aliases,
thenGREEN12/12 both and 141 Windows/133 Linux related single all-green/full Game
both/CI378. Include strict .1->1000 and zero/equality, same/switch/clear/re-enter, option
changes/disable, .7 fractional7099 at time100 and highclock80000000 signed conversion.
Independent native x86 actual original function33 calls/3CW modes with only3decoded
data-address relocations establishes the default027F numerical references; no original
Game.exe process/fullUI or all-FPU-mode claim. Earlier preferredVA host allocation487 is
transcript/metadata only, not preserved raw or a nativeRED.

All12 System/Courier/Times/Arial vector-font rows at scales1/1.5/2 are equal between
platforms; ordinary/oversized native tooltip metrics and positions remain138x26/886,741
and408x1808/616,0. Existing own menu sound, font wire, desktop/UI/tooltip/render tests
pass. Measurement/anchor/width sourcea2 still pending;34 unfinished.

Source: [360344ca1679d8bc7f4706d69e1270ee0caa3387](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387).

## 2026-11-01: measured tooltips and authored screen anchors

NativeTooltipPlacement0..19 are extended DB/dummy-SDL cases on both platforms, outside
asset-freeCI. Literal ICursor positions enter native Interface::Step without OSwarp.
Complete borrowed native owner snapshots patch only existing wire scalar fields14
anchor/15 type; restore the fixture's explicitly fixed layout mode after its native
postload hook. No replacement placement/visibility/font metric. Actual CText512 uses
packaged Courier and current pixel/logic transforms; frame-boundary observer proves
placement before StartNewFrame, then actual widget Draw submits vector glyphs. Actual
owner geometry/parent/tooltip/shared-sound and fourRNG remain valid.

Canonical RED17/20 ->GREEN20/20 both, exact controls7/13/19. Cases cover512 softwrap
(408x44->448x26), before-frame measurement, all four raw screen anchors/nested parents,
below+32, edge/negative/oversize clamp, same-ownerX, moving and reanchored owners, text
grow/shrink/empty, stable repeatedframes,1792wide and200percent. Initial20 fixture
failures and discovery2's unbroken-word/softnewline limitations are retained and excluded
from canonicalRED; br forces a genuine1826-high control. Existing oversized regression now
uses actual interface measurement/placement and standard Frame draw, preserving
ordinary138x26/886,741 and glyph/repeat/shrink checks; intentional new long518x1214/506,0
replaces old408x1808/616,0.

161 Windows/153 Linux related single all-green, full Game both/CI378. Explicitly select
all20 placement/all12 prior clock cases, owned menu sound, oversized and scaled edit
tests. All12 clock traces exact versus136; all12 vector-font rows exact versus136 and
between platforms. Static actual145 measure/typed56save/inheritedVTs/five exact relative
switch targets;0 new original-function runs or Game.exe processes. Sourcea2 complete,33
unfinished.

Source: [a2bba0ad86c91dcf5a239adda24792664abf1e43](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a2bba0ad86c91dcf5a239adda24792664abf1e43).

## 2026-11-01: AI-controlled demonstration squads and level preparation

Register NativeAutoPlaySquads0/1 as extended native-display DB/resource cases with
isolated user directories and headless audio. Actual console registration/queued
command/mission Initialize/world tracker links and live CGlobalPlayer mercs are the
consumer. Two person968/970 fixtures each verify four squads of4..7, all four AI flags,
level30 for every merc, and untouched separately created ordinary human flag/level.
Existing tick-seeded counts vary; only their original cardinality is asserted.
NativeDisplayContext closes owned hidden SDL/bgfx state and interface commands. No user
game is opened or closed.

Windows UpstreamBugTests now links WHOLE_ARCHIVE Main like Game, so the unreferenced
self-registering autoplay compiland participates; Linux already uses whole-archive.
Initial Windows discovery failed before reaching the consumer and is excluded from
behavioralRED. Windows discovery2 and Linux discovery actual rows show two missing
AI/level cases each, then GREEN2/2 both. Full Game both/CI378 and related 416/417 Windows
and 386/386 Linux; one pre-existing intermittent Windows
NativeCampaignSequence-nested-save movement timeout reproduces on previous-head source,
with byte-exact native .text and retries0/0/8 on both versions. Current source is restored
exactly, full Game rebuilt and both actual demo cases retested; no full-Windows-all-green
claim. All12 vector-font traces exact versus137. Original static typed
command/CreatePlayer/AI-byte/merc-loop/literal30 binding is recorded without whole-body
field/frame masks or original runtime claims. Demo lifecycle and automatic menu entry are
separate unfinished hunks.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: one-shot demonstration map transitions

NativeAutoPlayLifecycle0..8 are isolated extended native-display DB/resource cases. Invoke
the actual registered autoplay command and native mission pump. Consume only its existing
unrelated mission-start CICSaveFile; no snapshot is written. A test-only
explicit-instantiation member accessor writes a literal to the existing typed
CWorld::nTurnID field, then the real GetTurnID getter and pre/post-frame winner invariants
verify the input. This avoids deserializing a live world or guessing a byte offset. The
original world counter corresponds to existing save field38; no production setter,
alternate getter or simulated turn gate is added. Existing serialized demo counter3
supplies its actual shared CCTime node for literal watchdog input. Real MakeUnconscious on
opponents creates one winner, preserving the last tracked squad or the active squad for
the music case; do not substitute a fake winner or diplomacy table for same-id party
hostility. The renderer uses an owned hidden SDL/bgfx window and normal cleanup.

Canonical RED4/9 both ->GREEN9/9 both; exact controls0/1/3/6/7, strict100/101
and1200000/1200001 boundaries, any/last winner, key/time latch, original demo music
retention0->2, stable actual graph and repeated-frame one-shot behavior. Execute the
actual resulting container and require a fresh registered demo modal. Initial
snapshot/diplomacy fixture attempts, missing-getter compile errors and a failed
full-live-world-snapshot GREEN remain in evidence and are excluded from
canonicalRED/GREEN. Final literal-field inputs preserve winner conditions before and after
each frozen native frame; restore prior-head production and re-establish final-fixture RED
before restoring prepared production for GREEN. Original static typed actual59d7a0/168 and
common cursor/callback order are separately verified; no original runtime claimed.

Full Game both/425 Windows/395 Linux selected related all pass/CI378/12 exact vector-font
rows. Both prior squad cases remain selected. Exclude by exact name only the138
prior-head-proven Windows campaign nested-save movement flake; retain its failed suite
and0/0/8 current/baseline evidence, no fix claim. Interactive mission logic, including LAN
guards and UpdateSound ownership, is moved verbatim into the default callback. Sourcefdd
still partial;33 sources unfinished.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: publisher-logo demonstration and UI-clock rendering

NativeAutoPlayPresentation0..5 are isolated extended native-display DB/resource cases. Use
the real registered autoplay command and initialized mission/overlay, consume only the
unrelated start snapshot command without saving, then pause simulation through public
PauseGame. Read actual existing saved flags and UI counter into independent wrappers,
changing only its real CCTime leaf. A passive native CWindow child records the actual Draw
time while normal hidden SDL/bgfx rendering, original flash widget, images, camera and
shared sound run; no replacement visibility/font/clock/viewport implementation.

CanonicalRED6/6 both ->GREEN6/6 both; combined prior2 squad+9 transition+new6
presentation17/17. Cases cover original sequence/lose/HUD/scriptTB/hidden cursor/logo
state, actualUI123 versus rawInput4000, full normalized viewport,1792wide
canvas,2048px/200percent logical1024x768 and loaded flash Draw/morph123 with coefficient
in0..1. The actual ui_scale option is scoped/restored so real StepApp retains the intended
HiDPI setting. Earlier compile-only and ui_scale0 discovery attempts are retained and
excluded from canonicalRED. Producer whitespace partial-apply stopped before AutoCpp write
and was completed before any tests; initial missing-interface-method compile errors use
retained logs and a checked existing native accessor. Shell-driver line endings were
corrected before Linux build/test.

Full Game both/178 Windows/170 Linux UI/menu/font/sound/renderer/network selected related
all pass/CI378. All17 demo cases selected;9 prior transition rows and12 vector-font
compatibility rows remain exact versus139. Existing ordinary-menu source is byte-exact
after class declaration/include extraction; class ID/save fields/old flash methods remain
unchanged. No original runtime/new PNG or HD/face/font regeneration. Sourcefdd still
partial,33 unfinished.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: main-menu idle demonstration, input and return focus

NativeMenuIdleDemo0..7 are extended native-display DB/resource cases using actual menu,
SDL/bgfx, UI clock and registered commands. Canonical RED controls0/7 verify exactly120000
does not launch and actually minimized owned SDL window suppresses launch. Cases1..6
expose strict idle launch, saved2 timestamp, focus/input reset, nonresetting time event,
UI150ms catch-up and pre-consumed-UI reset. Require real nonexclusive SDL input and
consumed F12 before timestamp evaluation. Synthetic-minimize/uninitialized-input
discoveries retained separately, excluded from canonical RED.

RED6/8 ->GREEN8/8 both, combined25/25. Threshold tests execute real demo and return to
original menu without relaunch; isolate start snapshot without saving. Read saved wrappers
without live-world reload, mutate only actual UI clock leaf; preserve root/world/audio
identities. Focus case raw1000->6000 with UI1234 must yield1384, excluding donor extra
reset. Full Game both/186 Windows/178 Linux UI/menu/font/sound/renderer/network
tests/CI378 allpass;9 prior lifecycle+6 presentation+12 font rows exact. Command exact
except unchanged class/include extraction. No original runtime or asset rebuild.
Sourcecomplete32unfinished.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: cached point lights honor their shadow flag

NativeCachedPointShadowFlag0..7 are isolated extended native-display DB/resource cases.
Use actual CLightmapTracker, actual saved light-state codec, actual fresh CGScene and
hidden SDL/bgfx renderer. Test-only typed member access configures existing cache
state/renderer and calls the real private RecalcStep without production API or offset
changes. A passive CRenderWrapper observer counts and forwards every native
FormDirOccludersList request. Cases cover shadow true at stages0/1, false
positive/zero-color, two false points, mixed false/true neighbors and empty cache. Actual
stage completion and depth-view counts are checked; controls0/4/6 stay exact.

Case7 adds a bounded actual receiver fragment inside the non-shadowing point and observes
its positive native color traversal at DG triangle/material extraction. A sentinel stops
before buffer upload, so no pixel-output equivalence is claimed. Old code generates2
shadow views without reaching that receiver; new code reaches it with0 shadow views. This
checks that avoiding shadow work does not silently skip lighting.

Initial7-case RED4/7 andGREEN7 retained separately. After adding receiver proof, restore
production exactly141 and establish canonicalRED5/8 both ->GREEN8/8 both with three exact
controls. Full Game both/269 Windows/260 Linux selected
light/shadow/renderer/menu/UI/network suites/CI378 allpass. Existing
UpstreamCachedPointLighting, UpstreamCachedPointShadowed, point shadow-flag and old/new
save-layout tests selected.23 prior menu/demo and12 font rows exact. Only six production
lines changed; source mixed history/reprojection/shader work remains partial,32
unfinished. No original runtime or asset rebuild.

Source: [f6f6011fc301ace82317b9007ec6dd36b57f3369](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f6f6011fc301ace82317b9007ec6dd36b57f3369).

## 2026-11-01: stable map-probe centimetre midpoint keys (validation143)

Broad camera/deployment validation exposed a diagnostic-only map digest boundary. All52
scenario root maps built with matching counts/routes/behavior. Root5066 item432 has
Windows y9.375 bits41160000 versus Linux9.37499905 bits4115FFFF, one input-float ULP.
Direct Float2Int(y*100) produced938 versus937, so the nominal-centimetre digest differed
despite unchanged raw geometry. Keep native Float2Int rounding, but snap only a midpoint
reachable within one representable input-float step before forming diagnostic x/y keys. Z
hashing and all game/placement code remain unchanged.

NativeMissionMapProbe --check-item-quantization checks positive/negative midpoint
neighbors, ordinary values and distinct centimetre bins:11 invariants plus the
distinct-bin check pass both. Collect52 roots plus3 small representative maps
independently before adopting any expected digest. All52 per-platform raw item-coordinate
multisets, every non-item detail field, counts, routes and behavior stay exact. Fresh
Windows/Linux summaries agree
on4f3d72fcd190fa788e8fc168b9b903bf78d0c0034d80ac6e6284204ebb4d6b82. Update only
affected3830/4526 diagnostic item hashes and the52-root summary baseline; retain
count/script/route/behavior guards. No production code, physical coordinates, collision
data, save/LAN format or assets change in this commit.

Windows6/6 map checks and Linux9/9 refreshed-target checks pass, including all52 roots.
Retain original broad716/717 Windows and681/682 Linux logs; Linux also recorded an
initially missing nested NativeMapDatabaseTests helper. Laboratory preflight now rebuilds
selected existing targets and nested executable inputs. Hashing finds only4 changed Linux
binaries and reruns their9 affected cases. Game/UpstreamBugTests and all other selected
native binaries remain unchanged during this refresh. CI378 is green for accompanying
camera work, recorded separately. This validation-only fix advances no donor ledger
status; camera source completion follows in a separate commit.

Validation source: [6b4f7b8f67f26adb20506bef1015f89aba9f29dd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6b4f7b8f67f26adb20506bef1015f89aba9f29dd).

## 2026-11-01: selected-camera floor ownership and initial handoff

NativeCameraFloorOwner0..13 are extended native-display actual initialized
demo/mission/camera/view/UI cases. Isolate the existing mission-start snapshot without
disk writes and pause simulation. Read independent saved camera18/locator21 wrappers
without reloading the live world; retain root/world/audio graph identity. The no-world
framing context is supported native input used only to isolate framing from terrain/LOS. A
real StepApp with the selected camera frozen separately proves view-floor publication.
Real loaded missionUI/unitpanel/levelswitch, class0xB0241943, consumes the actual floor
notifications.

Cases0baseline,1camera-owned read,2camera-owned write,3renderer drift,4native frame
publication,5deferred framing,6freeze,7clamp,8initial acting player,9idle
deploy-pose/floor handoff,10up,11down,12camera-range slot and13frozen button produce
canonicalRED13/14 both ->GREEN14 both/control0 exact. Discovery3 expanded14 but4 UI
lookups used a root-only search and failed before behavioral rows; discovery4 fixes the
real hierarchy and provides all14 valid rows. Original source is checked independently
through typed camera APIs, original common/init/caller/widget routing, with rejected
whole-selector comparisons documented.

Three earlier bridge-only cases retain clamp/freeze and now require unchanged view1 before
actual mission publication, replacing the removed provider's original-inaccurate immediate
side effect. New actual StepApp case4 covers the deferred scene update. All50 initial
combined cases pass both. Add NativeAutoPlayLifecycle9, actual virtual override dispatch
with a real non-active winner and no common-player switch before the consumer; its typed
protected member access is test-only. The previous9 full-frame cases remain unchanged and
all51 combined cases are in final related selection.

Full Game both/717 Windows/682 Linux
camera/floor/deploy/mission/save/scenario/UI/light/network checks covered/CI378;23 prior
demo/menu,8 cached-point and12 font rows exact. Original full runs retain716/717 Windows
and681/682 Linux, only root maps failing. Separate validation143 fixes a diagnostic
one-ULP midpoint boundary and laboratory nested-helper/freshness setup;52 raw map
geometry/non-item/gameplay fields remain unchanged. Windows6/6 and Linux9/9 affected
reruns cover all selected tests, including all52 roots and three newly refreshed
standalone test binaries. Keep original failures and separate passing retries, no root-map
exclusion. Only known prior NativeCampaignSequence-nested-save movement flake excluded
as139, not fixed. No original runtime or asset generation. Sourcecomplete31unfinished.

Source: [6b4f7b8f67f26adb20506bef1015f89aba9f29dd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6b4f7b8f67f26adb20506bef1015f89aba9f29dd).

## 2026-11-01: saved object-melee executor foundation (145)

NativeObjectMeleeSave0..7 are native real-world legacy payload consumers, not a new
class-name/source-text check.0 keeps normal point melee;1/2 target/null original-field
roundtrip into a fresh registered decoder;3 real inherited Start/AP cost+7;4 retained
pocket target without hull;5 null target, both real one-shot label consumption;6/7 actual
selected/null saved filters across two native human hulls, exact damage and callbacks. No
live-world deserialization; only fresh executor decode with external references. Canonical
RED7/14 both includes all six unchanged native unit/tile/godmode filter controls, all
pass; GREEN14 both and seven controls exact.

Retain initial enum compile failures, old artificial-health fixture failures and
byte-exact parent reproduction separately. The old fixture changed native VP to500 before
lifecycle updates, which later normalized caps to140 and contaminated its damage delta.
Measuring pre-attack HP fixes five controls; removing that unstable artificial cap and
retaining actual healthy native receivers fixes all six, with the exact literal
damage/callback/godmode/actor guards preserved. The canonical RED after fixture
calibration has only seven missing-factory failures. Restore unchanged production
implementation after that RED.

Typed original serializer53/registry and independently decoded label/selector/default HL
info/filter accepted; reject whole Gold499 label comparison. Native full Game both/372
Windows/361 Linux related/CI378 pass,12 FONT-COMPAT rows exact both. Parameter
constructor/live route remain deferred; source still partial31unfinished, no original
runtime or whole retail-save-file interchange claim. Known prior campaign nested-save
movement flake remains tracked, not fixed.

Source: [4cdc1eada6c182c0a9093364e95c750a6166d4c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4cdc1eada6c182c0a9093364e95c750a6166d4c8).

## 2026-11-01: fresh object-melee routing and requested AP (complete146)

NativeObjectMeleeRoute0..6 exercise actual native fresh-command creation, queue traversal,
real authored door/current hull point/close approach, original executor kind, requested
AP, direct Start spending and native fresh decoder roundtrip.6 equips a real nonthrowing
authored knife, with existing named AT_KNIFE12 and held-item preparation; unarmed uses
default inactive-item policy.4 direct-point/5 missing-target controls remain exact. Query
health/AP/position/orders/item state/RNG and knife quantity guarded. Only newly allocated
executor is deserialized; no live-world read or original runtime.

Canonical RED5/7 both ->GREEN7 both, combined21 with all14 prior native
saved-executor/filter cases exact. Original unarmed14 versus expected21 and knife18
versus25 now include requested7; same native approach succeeds on first real candidate.
Fresh object roundtrip requires finite existing native hull point, saved extra7 and NULL
SS filter. Own runtime point seed explicitly preserves the previous fork input and does
not claim parity with original unchecked null-lookup coordinates. Original complete426
classifier, unique typed63 factory prefix/named command/actual tables/constructor args and
item rule independently audited; reject whole5657 factory and whole196 getter equivalence.
Do not transplant Sentinels retained-object producer.

Full Game both/379 Windows/368 Linux related/CI378 all pass. Prior14 raw case rows and
separate godmode-dedup row exact;12 FONT-COMPAT rows exact each. Linux preflight rebuilds
selected existing executables and nested helpers before tests. No native backend, asset,
serialized-field or LAN changes. Source complete30unfinished; known prior campaign
nested-save movement flake remains tracked, not fixed. Full damaging
mission-loop/original-save-file interchange not claimed.

Source: [4cdc1eada6c182c0a9093364e95c750a6166d4c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4cdc1eada6c182c0a9093364e95c750a6166d4c8).

## 2026-11-01: initial cinematic floor range (147)

NativeCinematicFloorRange0..2 are extended native-display actual initialized
mission/sequence camera/view consumers. Temporarily set only real template4412 variant
min/max before initialization and restore original values afterward. Control raw0/0
defaults[-3,4] exact; bounded raw-1/2 requires[-1,1] on player and distinct selected
cinematic cameras; single-floor raw0/1 requires[0,0] and a real frozen-camera StepApp
keeps camera/view0 together. Actual mission/world/view/UI/audio identities and no extra
interface command are guarded. Retain the native pending snapshot without writing it; no
live-world deserialization. RED2/3 both ->GREEN3/combined54,51 prior cases green,46 fixed
row controls plus the non-active-winner marker exact. Actual game.db file hash unchanged.

Original typed caller, unique exact13 range producer and typed42 setter audited; whole4324
initializer and changed-register guard parity not claimed. Actual camera70 setup is not
misidentified as mission70. Only one production line in tactical Initialize. Full Game
both/278 Windows/251 Linux related/CI378 pass; prior melee/filter/godmode and12 font rows
exact both. Known prior campaign nested-save movement flake is the sole exact exclusion,
not fixed. Source remains partial for deployment/foot-support/light remainder,30
unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: live and saved foot support (148)

NativeFootSupport0..6 use actual world/map/navigation, a deployed actor and registered
height node11461141. Four saved-node recalculation cases: ground control, empty allowed
floor4 fallback, distinct restored special height, full two-floor inclusion differential.
Three actual animator producers: constructor, PlaceUnit, StartMove/Move; keep node
identity and compare every cached sample to an independent fresh node decoded from its
saved fields. Borrow map references only; never decode the live world. Native ground
trace743 and floor4 trace0 prove real geometry/filter controls. Existing ground sampling
tolerance0.002 is unchanged; no exact row claim for its small interpolation difference.

Initial peak0.5 GREEN60/61 both retained; existing center-preserving CheckGradient lowers
such a peak. Input corrected to0.25 within its slope, with no production
adjustment/tolerance relaxation. Add actual full-surface floor4/0 vs0/0 and4/4
differential. Same final diagnostics against exact parent three-file source give RED6/7
both; GREEN7/combined61 both.54 prior camera/cinematic/demo/menu/light cases and49 fixed
rows plus non-active-winner marker exact. Full Game both/376 Windows/348 Linux
related/CI378, prior melee/filter/godmode and12 font rows exact, game.db unchanged. Typed
original Move288/both terrain79/85/height ctor108/terrain body132/animator caller43 plus
exact support/nearest/fallback blocks audited; whole995 sampler rejected, no unsafe
masking or original runtime. Source remains partial for deployment/light remainder,30
unfinished. Sole exact campaign nested-save flake exclusion unchanged since139.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: authored point-light shadow propagation (149)

NativeAuthoredPointLight0..6 verify native constructor, real game-view/scene creation,
actual world4517/CSetRender lamp dispatch and zero-radius rejection. Toggle and restore
only the existing authored flag on44 runtime lamp models, retaining real
geometry/color/position/radius. Every actual CPointLight must agree with its saved tag12
and native radiance; a fresh independent registered01961170 node must reload the same
flag. No live-world deserialization or DB file writes. Canonical RED3/7 both
->GREEN7/combined68 both; false0/2/4 and zero6 exact; all44 true world lamps formerly lose
the flag, now preserve it. The unchanged seven new tests support native
predecessor/current API shapes without generating a fake light/flag. Existing three
radiance constructors preserve prior false explicitly; three diagnostic virtual sinks only
accept the native signature.

All61 prior foot/camera/demo/cached-point cases and fixed58 rows remain exact. Full Game
both/452 Windows/423 Linux related/CI378; prior melee/filter/godmode and12 font rows
exact, database hash unchanged. Original unique exact world53/28, named typed render52
E8-only, view exact20 suffix/decoded zero guard, scene103 E8-only, constructor115
namedVT-only and exact77 field stores audited. Whole384/30/165/449 comparisons rejected
where writable/BSS/EH operands differ; no unsafe masks or original runtime. Known campaign
nested-save flake is the sole exact exclusion, unchanged since139. e825 deployment
remainder still pending;30 unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: authored subzone passage deployment (150)

NativeStorePassage0..4 cover actual ETStore2291 Unit1_l/Unit1_g crouch arrivals,
companion2292 ordinary passage facing, exact non-passage control and actual two-merc
formation with consumed saved passage metadata. Load real campaign/DB/variant geometry and
call native AddPlayer; no fake passages/waypoints, live-world read, frame/clock warmup or
geometry replacement. Initial GREEN72/73 both exposed a raw expected formation target
inside a wall. Native GetNearestPosition before party locks now supplies the strict
full-bit expectation, with pose/direction preserved and valid/passable checks. Initial
evidence retained; exact-parent replay of final tests gives RED4/5 both, non-passage3
exact, then byte-identical audited production gives GREEN5/combined73 both. Production was
not adjusted for this fixture refinement.

All68 prior cases and65 fixed rows retained; Game both/471 Windows/442 Linux
related/CI378, melee/godmode/fonts12 exact. Separate12 fresh LAN gates each platform
include all7 actual NetworkMatchTests modes. Linux lacks their CTest registration in this
configured build, so record actual executable commands, exits and per-mode logs directly;
first5 infrastructure gates use CTest. Windows all12 CTest. Native formation full
bits4c011d09+4c011e0a and same native snap row are verified in RED/GREEN; prior formation
already occupied2 cells. Database unchanged. Original named typed getters/slots, full
equality E8-only, five exact blocks and independently decoded actual changed
guard/frame/register operands audited without unsafe normalization or runtime execution.
Existing sole nested-save exclusion retained. All15 donor files now accounted for
across144/147/148/149/150; e825 complete,29 unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: native Lua implicit-end scheduling (151)

UpstreamLuaImplicitEnd0..3 execute native compiled Lua through Script.ExecuteThreads.
Verify implicit call first-update trace1 then12, exact explicit-return control12,
two-level trace1/12/123, and independent fresh Script-only save/reload of a suspended
caller that resumes exactly once to12 alongside the live state. Canonical unchanged RED3/4
both ->GREEN4 both. No fake interpreter/instructions, live-world decode, extra game frame,
timing warmup or tolerance. Actual Steam823080 dispatch823e80 and opcode0 tail823e50
audited; Gold1.1 lacks the yield and is not treated as equivalent. Unique typed23/10 exact
blocks and return helper30 E8-only establish caller/helper context; actual changed
lua_State fields and opcode branch decoded separately, never normalized. Correct the
donor's stale addresses in the production comment.

Windows initial624/625 fails only the old5247 fixedtick50 flyer-phase precondition before
any save. Controlled exact parent passes both old/refined5247 while new4 retain RED3.
Refine only phase observation inside unchanged220-update budget, require native flyer
event, same survival/grid/0.001 position/fullsave/endturn/secondsave checks. Parent
event50/GREEN52, no production/tolerance/frame/clock change. Fresh ALL_BUILD then
refined5/5 and CI382; initial failure retained and624 unaffected passes reused,625
distinct aggregate coverage rather than a single625/625 run. Linux full590/590 with
identical production remains valid; probe case is Windows-only here.

Full Game both/625 Windows/590 Linux distinct related coverage including all150,
prior/new77 cases, CI382,71 fixed rows/melee/godmode/fonts12 exact, database unchanged.
Separate12 fresh LAN gates each platform including361 maps. Linux5 CTest+7 direct native
game modes, Windows12 CTest; all commands/exits retained. Existing single nested-save
exclusion retained. An initial Windows-only target name in the Linux builder was corrected
without dropping tests; original build log retained. Source6face remains partial for its
other four files,29 unfinished.

Source: [6facee6dbea4189b0d817f37d4f62f9318212152](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6facee6dbea4189b0d817f37d4f62f9318212152).

## 2026-11-02: deferred outgoing subzone termination (152)

NativeSubzoneTransition0..2 cover real initialized scenario1 ETStore forward2291->2292,
reverse2292->2291 and exact first-entry control. Use actual world passage group1, real
global merc54, native UI command drain and deferred CICBeginMission execution. Before the
deferred command the old party/interface/world remain intact and no snapshot exists.
Afterwards the old world loses its one human actor, its isolated native linked-zone
snapshot is nonempty, and the real replacement contains the same one living global merc
with passage metadata consumed. RED2/3 both ->GREEN3 both, first-entry2 exact. Tests do
not fake worlds/passages, override mission behavior, modify DB/geometry, execute extra
frames or deserialize a live world. Snapshot reentry/deserialization and a831 historical
footprint cleanup are not claimed. Initial diagnostic filename compile error retained
separately, not RED; final three cases unchanged during production.

Original named CICBeginMission Exec slot10->60c350, unique exact19 termination
block60c3cf/slot28 before actual Init60c50b, named CMission Terminate slot28->5fc560,
unique exact typed47 reference/field block60c146 at constructor60c0f0, actual
producer5feb69/constructor call5feb95 and deferred5febab audited. Typed constructor
mission is native arg5/field2c; own API appends an optional input to preserve preceding
bool/image callers. Actual slot134/frame/register/field changes decoded independently
rather than normalized, no unsafe masks or original runtime.

Game both/640 Windows/604 Linux related in complete runs/CI382, previous/new80 cases and80
previous fixed rows, melee/godmode/fonts12 exact, database unchanged;12 fresh LAN gates
each including361 maps. The151 refined5247 phase probe passes within this Windows full
run, initial151 failure still retained. Sole old nested-save exclusion retained.
Other6face route/reservation files remain pending;29 unfinished.

Source: [6facee6dbea4189b0d817f37d4f62f9318212152](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6facee6dbea4189b0d817f37d4f62f9318212152).

## 2026-11-02: authored unaware-critical parameters (153)

NativeUnawareCritical0..5 exercise real person2, native authored perk74/prerequisite
acquisition, actual equipped melee/snap-gun factories and native CreateAttack. No-perk0,
non-backstab melee2 and gun5 are exact controls; affected melee1/cap3/gun4 give canonical
pinned RED3/6 both ->GREEN6 both. Independent expected chance99/100/56/100/48/39 and
difficulty49/73/28/169/27/18, authored Params0.25/1.5, base chance99/99/56/227/39/39 and
severity49/49/28/113/18/18 fixed before final RED. Preserve no-ammo-spend/no-adaptation,
RNG purity, AP/HP/position/item/ammo/perk points, damage bounds/unconscious probability
and native pointer identities. Initial compile correction/formula discovery retained
separately; final six tests unchanged during production. No fake combat, private modifier
hooks, DB edits, world decode or extra frames/ticks.

Actual original helper6bebd0/133: unique exact40 prefix, complete87 suffix with only
decoded Min E8 relocation, complete Min41 unique exact, independently read writable IEEE1
operand never masked. Named secondary4c HasPerk slot78 and both output pointers, actual
chance10/difficulty14, cap100 immediate, two RC-chop FISTPs/CW restoration and
incoming6c268f/6c2b95 calls audited. Baseline DB equals verified original Steam DB
SHA314d7aa9e6339f0e6826bf2a0c71fdad992aec1d6074a2ed763a6f6fa62d62df. Whole133/2606 parent
identity not claimed at unmatched global/parent layout; no unsafe masks/original
execution. No full projectile/post-damage RNG/exhaustive x87 claim.

Game both/1377 Windows/1340 Linux broad combat+all152 related/CI382, prior/new86 cases
and83 fixed rows/melee/godmode/fonts12 exact, DB unchanged;12 fresh LAN gates each
including361 maps. Linux aliases resolve actual generated CTest command paths, all helpers
freshly built/validated. Existing sole nested-save exclusion retained. Other884
healedVP/damage-share/KO hunks pending;29 unfinished.

Source: [884f91f42db3aa840f729fb5196a18133d49917b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b).

## 2026-11-02: native health depletion and knockout probability (154)

NativeHealedDamage0..4 and NativeHealthKnockout0..11 use actual native
world/person103/cap147, authored party death reserve through CalcDeathVP, native HealVP
and death critical, actual fixed noncritical body attacks. Final independent literals and
actual seeded64/65 probability boundary pinned BEFORE production. Initial three invalid
default-death-threshold KO fixture failures archived, corrected before valid15 discovery
RED9/15 and final17 canonical RED10/17 both. Final17 unchanged during fix ->GREEN17 both;
seven control rows exact. Verify finite nonnegative bandaged health, raw health depletion,
total health and original rounded health-scaled KO/zero probability/forced-click
semantics, native four-word RNG tails and wound AP clamp/pose/inventory/cap/input
controls. No fake fields, live-world read, extra frames or clock changes.

Named original ProcessAttack slotA4/GetHealedE8/GetTotalEC, unique complete
setter22/getter16/Modify36/GetTotal60/Min41/Max41 and range70 with only decoded E8
relocation audited. Generic11 thunk uniqueness rejected; named slot and decoded tail
target used. Actual share/depletion and KO blocks decoded without
field/frame/register/branch/writable constant masks; whole1992 equality rejected. No
original runtime or x87 exception-status/full-flight claim. Initial damage clamp and
legacy getter protection additionally static-proved, no invalid-negative-input RED.

Game both/1400 Windows/1363 Linux related/CI382; prior86+new17=103 cases,89 old fixed
rows/melee/godmode/fonts12 exact,12 LAN gates each/361 maps, DB unchanged. Source884
all4files now imported;28 unfinished.

The initial full1394/1357 related runs retained six fixture failures per platform:
NativeDirectAttackXP0/4/5/18 and
UpstreamAIFirstIncapacitationUnconscious/FinishUnconscious. Their raw logs remain
archived. These fixtures used ordinary input8/probability100 on a healthy target as a
forced KO; original health scaling correctly suppresses it. Only these six KO preparations
change: an actual fixed-input8 CAttackPortion::CalcStructDmg probe with unchanged source,
body armor and difficulty pins damage11 for the party,8 anonymously and5 for the enemy.
Existing native Skills(VP).Modify prepares living HP12/9/6 at authored cap80, so the
unchanged hit leaves HP1 and original probability caps100. The previous strict
KO/death/XP/level/progress/perk/roster/killer/time conditions remain; HP1 is additionally
asserted. No forced state, disabled injuries, fake fields, extra frames or clock. The XP
preparation precedes native reference clones and the AI preparation precedes its first-hit
baseline. Controlled exact parent Main plus corrected fixtures freshly runs
XP19+AI8+new17: all27 old cases pass and unchanged new17 retain canonical RED10. Restore
production Main byte-exact and freshly run all44: all pass. All21 XP inputs/summary rows
including four RNG words and every recipient row match parent versus production exactly,
as do all7 AI metadata summaries and its corpse contact event. Final full scope
additionally includes all8 AI cases, giving1400 Windows/1363 Linux: a SINGLE complete
passing full run each, with the paired rows retained exactly. Initial six failures are
never rewritten/reported as a pass and no new exclusion is added. Paired proof and both
fixture byte snapshots are saved.

Source: [884f91f42db3aa840f729fb5196a18133d49917b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b).
