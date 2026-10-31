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
