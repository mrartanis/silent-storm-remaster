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
