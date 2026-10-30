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
