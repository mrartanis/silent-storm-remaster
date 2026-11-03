# Изменения upstream, которые затрагивают оригинальный баланс

Этот файл относится к переносу в `mrartanis/silent-storm-remaster`, ветка
`develop`. Условие переноса: сохранять механику Silent Storm retail v1.2,
включая числовые особенности оригинала. Исправление реконструкции до поведения
retail допустимо; изменение самой retail механики не переносится.

## Подтвержденные изменения правил — исключены

### [405ed861](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/405ed86144c0340c1ac36f0775d989123e7d8010) — минимальная стоимость действия 1 AP

Upstream устанавливает минимум 1 AP после скидок на выстрел, перезарядку,
бросок, melee, смену позы, перемещение, перенос тела и лечение/ремонт. Также
отрицательная стоимость превращается в расход 1 AP вместо прибавления AP.
Явно бесплатные действия оставлены отдельно. Это меняет экономику AP у
высокоуровневых персонажей и некоторых сочетаний perks. В diff автор прямо
помечает отличие от retail как `ORIGINAL RETAIL BUG FIXED`.

**Решение:** не переносить эти ограничения и минимумы в текущий режим игры.

### [7368b4b1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7368b4b10182ef243135d94b93bc481fd6dd593f) — соответствие описаниям perks вместо поведения retail

Коммит смешанный: независимые ошибки реконструкции нужно проверять отдельно.
Следующие изменения самой оригинальной механики исключены:

| Perk | Что upstream меняет | Влияние |
|---|---|---|
| 88 | Множитель скорости броска заменяет квадратным корнем множителя | Меняет реальную дальность ножей и гранат. |
| 75 | Проверяет ночную маскировку скрытой цели ночью вместо perk наблюдателя | Меняет обнаружение скрытых юнитов днем и ночью. |
| 70 | Дополнительно проводит бонус melee через GetSkillValue | Меняет точность, защиту, урон и стоимость melee. |
| 22 | Подготовка умножается на параметр вместо вычитания параметра | Нулевой параметр делает подготовку бесплатной. |
| 33 | Reload умножается на 1−Param1 вместо retail Param1 | При Param1=0.3 расход становится 70% базового вместо 30%. |
| 19 | Дополнительно убирает штраф движения в weapon-skill расчете | Меняет точность после движения. |
| 30 | Полностью компенсирует соответствующую потерю burst accuracy | Меняет эффективность стрельбы очередью. |
| 47 | Уменьшает итоговый critical damage вместо severity input | Меняет выбор травмы и полученный урон. |
| 52 | Добавляет бонус engineering до picklock roll, не к итоговому score | Меняет вероятность вскрытия. |
| 6/71 | Разделяет familiarity learning rate и combat bonus | Историческая часть позже отменена [1bd13094](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1bd1309408ef97e286f70483a9544f2abcf8a47a); исправленная retail формула уже перенесена ранее. |

**Решение:** перечисленные изменения правил не переносить. Все три отдельные
ошибки реконструкции — VP threshold, moving-target multiplier и cover multiplier —
независимо сверены с оригинальным Steam EXE и исправлены отдельными native TDD
(health-survival-threshold, moving-target-perk, cover-perk-multiplier в
UPSTREAM-IMPORTS.md). Аудит источника завершён с этим разделением; девять
исключённых изменений оригинала и исторический блок6/71 не перенесены.

### Производные коммиты

[b875e43d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b875e43ddec65e0375d1335eb8d8726e38b4934a) связывает AI reload с общей стоимостью выбранного оружия,
но наследует измененный perk 33 и минимум 1 AP. Общая цена для AI и игрока
полезна. Оба независимых шага выполнены с отдельными native RED/GREEN:
общая цена действия/player AC_RELOAD и использование этой цены выбранного
оружия в AI GetInfo/Do. Исходная retail формула Param1 с integer truncation
сохранена, без минимального1AP.
Скидка1-Param и минимум из чужой текущей версии остаются исключёнными.

## Требуют сверки с retail; не объявлены ребалансом заранее

### Классификация инженерных гранат в `14ffbcc8` — требует сверки

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/14ffbcc83ea0fff503aa22386c98274a5aadd9ca)
проверяет в руках только DB successor CRPGGrenade. Наш прежний CGrenadeItem
также представляет инженерные гранаты CRPGEngGrenade. Полная замена предиката
означала бы: при инженерной гранате в руке UnitSwitchToGrenade больше не
останавливается и может извлечь другую гранату из рюкзака. Это меняет выбор
предмета сценарным кодом; соответствие retail отдельно ещё не подтверждено.

Переносится независимый фикс активации обычной гранаты, уже лежащей в другой
руке. Прежнее подавление fallback при инженерной гранате сохраняется;
native controls проверяют оба SLOT, active/inactive и нетронутую запасную
обычную гранату в рюкзаке. Изменение классификации пока не переносится.

Ниже перечислены отмеченные аудитом изменения расчетов. До сравнительного
теста они остаются отложенными. Название `Restore retail` само по себе не
доказывает правильность, но восстановление штатной функции не является
автоматически изменением баланса.

| Источник | Что проверять |
|---|---|
| [2de89e04](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d) Restore incidental bullet impacts and periodic integer bleeding | Пропуск нулевого целого кровотечения уже подтверждён оригиналом и перенесён отдельно; случайные попадания, ночная точность, формулы и периодичность ещё требуют oracle. |
| [5780c0be](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5780c0be34f000c316e1239cd749ae96ccbea027) Restore retail throwing knife targeting, flight and facing | Метательные ножи: штатная дальность, скорость, попадания, направление. Это восстановление формул, не готовый безопасный cherry-pick. |
| [c1ab9336](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c1ab93366661d3a4e25a0effe62625bae92c2d0c) Restore retail melee damage dispatch and armed attack cursor | Native UI-различение кулака/экипированного оружия адаптировано в GetState; action dispatch и critical bans сохранены. Target-filter/damage dispatch остаётся для проверки реальных попаданий и original damage. |
| [7f4f0ea8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7f4f0ea872937e4dcb4a887ff6a136d7dce1fcd5) Match retail nearest AI contact selection | Nearest-contact selection использует другие позиции/фильтры; отдельно сверять поведение AI. |
| [cc0fd988](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf) Match retail AI firing range and target reach checks | Перенесены точка выстрела, достижение юнита, предел AI30 и проекция дальности; маски трассировки и связанный обычный путь ещё проверяются отдельно. |
| [66fc9e9a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718) Match retail AI snipe weapon and cover evaluation | AI snipe должен считать выбранным оружием и не терять дробную cover precision. |
| [0ffb568d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0ffb568d67bc6ca389d7ca63a9da4f3978c801a7) Restore retail level XP persistence and fresh skill maximum flags | Повторная native-проверка нового репозитория выявила потерю общего XP в SetXPLevel; сохранение существующего расчёта исправляется отдельно. FreezeToMax для новых skills независимо сверён с original Steam EXE и перенесён: см. fresh-skill-modifier-values в UPSTREAM-IMPORTS.md. |
| [08c4bb5a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/08c4bb5a6b9a94337d88bd8241a2e51b6984cfce) Restore retail weapon preparation AP costs | Подтверждено Gold/Steam и native TDD: исходная цена типа оружия, вычитание перка22, усечение и0..8. Перенесено; см. проверенный раздел08c4bb5a ниже. |
| [98871d0f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98871d0f0def4f966e097b984c18e57a8f437d7b) Restore wounded AI fear and reachable retreat selection | Раненый AI, fear и достижимое retreat место — требуют retail-сверки, не переносить изменения difficulty вслепую. |
| [069451e7](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/069451e7e61dc0c720812b2f939339f7732fdaf3) Restore retail explosive target groups and friendly clearance | Explosive target groups и safe distance allies — сверять original AI, без новых порогов от себя. |
| [bf54ea44](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/bf54ea44a50f7e9ab7095a05491ec289105c84d0) Restore VP boost duration and overdose processing | VP boost duration/overdose — медицинские формулы original, отдельные oracle-тесты. |
| [65558b11](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/65558b11623917d5634c55a1b718fa2cb588ef66) Restore retail critical severity and damage calculation | Critical severity/damage и perks 46/47: сверять исходные формулы игры. |
| [32f730ac](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/32f730acfe6015bd712e897c4bd076a6f6503642) Refresh wound penalties after healing and respect critical cure gates | Обновление wound penalties после HealVP и условия остановки кровотечения: проверить original medical rules. |
| [285f6a0c](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/285f6a0cff9737d2789396a2543bc75d0ab11da7) Restore retail transient attack-session bookkeeping | Attack-session и medal bookkeeping: проверять исходные правила наград и завершения атаки. |
| [23630bfd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/23630bfd0c6924b748f79cca9ded910e0f4df35e) Match retail backstab awareness and melee called-shot damage | Backstab visibility и wanted hit location в melee: сверить original combat rules. |
| [9f3c0371](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9f3c03712f2a86f83fb7739891827847b2f9f645) Restore shared retail weapon skill and accuracy perks | Weapon skills/accuracy perks, range, movement, burst и snipe: оригинальные формулы требуют oracle. |
| [eb45f18a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/eb45f18a9787d7994bcdb224b198818009786445) Truncate skill progress at full-point boundaries like retail | Целочисленное продвижение skill вместо SkillRound: проверить исходные границы роста. |
| [2893fa84](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2893fa8470599905a5de221be3d8057a52c6d479) Restore retail skill training rates, attributes and learning perks | Training rates, attributes, learning perks и UseSkill: полное восстановление формул требует original oracle. |
| [2e15a102](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2e15a10299e0d5e4a90558e39b34cc150ce51382) Train Stealth when entering Hide like retail | Рост Stealth при Hide: сверить исходные правила обучения. |
| [242f6dfd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/242f6dfd827d93014c4210980f9d1c4af2a45b67) Match retail movement noise and hearing rules | Movement noise/hearing и quiet training, plasma/PK: сверять исходные правила. |
| [24a00fd1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24a00fd151a39d6c696c2ee8ab6deeb9d862f875) Apply retail short-burst perk to additional-round AP costs | Short-burst perk discount для дополнительных пуль: сверять original AP formula. |
| [aa724eca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed) Restore retail XP distribution after fatal bleeding | Распределение XP после смерти от bleeding: проверить original rewards. |
| [1dc30007](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1dc300073e7da9192b99a933d7e041fefbda73c5) Reject tile attacks in sniper fire mode like retail | Перенесён отдельно после native RED: в нашем held-sniper state tile уже запрещён; устранено принятие той же tile команды до входа в held aim. Snap tile и Snipe unit controls сохранены, RPG формулы не менялись. |
| [01eb873b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/01eb873b41cd2c8b9ce2d7b81835d2e00d492118) Restore retail cover projection extent and sampling center | Cover projection, target center и range caps: проверить исходные combat/LOS правила. |
| [853fbb71](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/853fbb7189bfdc0a4fa3c16781107d752840fa8e) Clear temporary bleeding suppression on new bleeding criticals | Null guard перенесён через native SEGFAULT RED/GREEN. Наш nBleedingStopAmount сейчас не используется runtime; соседний сброс не копировался, подробности ниже. |
| [dfb4e99b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/dfb4e99b50118531a3f641f46ba22c88ae38314f) Restore retail attacker-level critical severity and head damage | Обе части независимо подтверждены original Steam EXE и перенесены раздельными native TDD: head-body-damage и attacker-level-critical-rolls. Множитель1.5 и один исходный бросок1..19 за уровень восстанавливают retail; сохранён наш общий сериализуемый RNG. Это не исключённый ребаланс; соседняя critical-formula часть65558b11 остаётся отдельной проверкой. |
| [1d260f7f](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1d260f7faa7d44b32cc35c952447adb5157f21a9) Restore hidden-unit reveal on movement collision | Раскрытие скрытого противника при столкновении в движении: проверить original visibility behavior. |
| [226b2ddb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4) Restore retail grenade accuracy range and stance perk | Grenade range/stance perk формулы: проверить original accuracy. |
| [e6a660f7](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e6a660f7b69c986528b8d04c86f8bdd4ba6b763e) Restore retail death reserve for mission clue targets | Death reserve персонажей-улик из difficulty fClueDeathCoeff: исходные survival coefficients. |
| [c923fe9e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c923fe9eaa63ed62aee9178976661a74d4daefd9) Restore retail direct-attack XP eligibility and incapacitation timing | XP eligibility только по врагам и правильный момент incapacitation: original reward rules. |
| [037c41d9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/037c41d91ec01f29848f346787aa7a8a1d471c43) Restore missing retail perk effects and combat formulas | Missing retail perks/combat formulas: большое восстановление original, требует отдельных reference tests. |
| [2f5a3c75](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2f5a3c753d9673bfc9aa24b2852a7fe2e1bbf60f) Match retail Panzerklein pose-dependent movement costs | Pose-dependent PK movement costs: изменение AP только после проверки original. |
| [c8c4e225](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c8c4e225df115072ce176598c1324aaf7bd7a7c0) Allow safe approaches to off-grid mines when disarming | Доступ к off-grid mine для disarm с reach 1.0: новое ограничение дальности проверить по original. |
| [9cf3fd3d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9cf3fd3dae155de3b0c0e1de443cf54a7f494426) Restore retail AI cover depth and flanking path search | AI cover trace depth и flank paths: поведение original AI требует сравнительных тестов. |

## Числовые части смешанных коммитов — отдельная сверка

### [884f91f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b) — критические попадания и временные VP

Из этого источника переносится независимая проверка отсутствующей униформы.
Следующие расчёты пока не перенесены и требуют сравнения с оригиналом:

- Для unaware critical сложность умножается на Param2 перка 74 вместо
  ограниченного значения шанса; преобразование шанса и сложности меняется
  с округления на усечение. Это меняет результат критических попаданий.
- Временные вылеченные VP и вычитаемая доля урона ограничиваются снизу нулём;
  отдельно обрабатывается нулевой общий VP. Это влияет на распределение
  урона между временным бонусом лечения и базовым здоровьем.
- Вероятность потери сознания после попадания масштабируется оставшимся
  здоровьем; бросок меняется с 0..100 на 1..100, а Click of Death получает
  отдельное условие. Это влияет на частоту потери сознания.

Эти пункты не объявлены ребалансом автоматически: upstream описывает их как
восстановление retail. До проверки оригинала числовые изменения отложены.
Выгрузка всех предметов рюкзака подтверждена статическим анализом оригинального
Steam EXE и перенесена отдельно: правило очистки записей без модели также
сохранено. Приоритет сценарных реплик также перенесён отдельным UI-исправлением;
перечисленные выше боевые формулы не менялись.

## Статус

### Условия повторного входа в зоны из `5cdf627b` — требуют сверки

Источник: [5cdf627bb6ce601188e39945b93742515216655a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5cdf627bb6ce601188e39945b93742515216655a).

Перенос текста подсказок существующих состояний маркера отряда отделён от
оставшихся условий выбора действия. Upstream добавляет `CanEnter()` для
сектора: зона должна быть видимой и либо ещё не посещённой, либо разрешённой
сценарным флагом `reenter`. Этот результат затем участвует в выборе режима
маркера и подсказки входа. Независимое различение выхода с карты и входа в
миссию перенесено отдельно: меняются режим отрисовки и текст подсказки,
существующая проверка попадания отряда в сектор остаётся прежней.

Числовые параметры боя этот diff не меняет, однако доступность действия для
пройденных зон связана с правилами кампании. Условия `CanEnter`/`CanReenterZone`
пока не перенесены: требуется проверить их на оригинальной игре и согласовать
с уже существующей обработкой нажатия маркера. Исправление трёх обычных
подсказок само по себе не меняет возможность входа или повторного входа.

Связанный [2955475691a5096b3c534fb0f71f7a86f2fd1e51](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2955475691a5096b3c534fb0f71f7a86f2fd1e51)
добавляет тот же CanEnter в обработку нажатия маркера. Это уже не только вид
подсказки: посещённая зона без reenter пропускается при выборе назначения;
при отсутствии другого подходящего сектора возможен прежний fallback в лагерь.
Без переноса условий из 5cdf627b этот hunk также неприменим к нашему классу
сектора. Оба eligibility hunk исключены из автоматического переноса как
изменение доступности миссий и лагеря. Решение пользователя после просмотра
этого файла и сверки retail остаётся отдельной работой. UI-части этих двух
коммитов (подсказки, heading, курсор и snapshot медали) проверены независимо.

Файл дополняется по мере переноса. Пункты со сверкой retail не считаются
завершенными или исключенными до проверки. Новые свидетельства и решение
записываются здесь, а RED/GREEN и частичные переносы — в
[UPSTREAM-IMPORTS.md](UPSTREAM-IMPORTS.md).

## Практика медицины и инженерии из смешанного `156ccd17`

Источник: [156ccd173ff6c9671904e6d94e709397fc701d66](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/156ccd173ff6c9671904e6d94e709397fc701d66).

Исправление выбора установленного оружия для перезарядки переносится отдельно.
Оставшаяся часть добавляет вызовы `UseSkill` при регистрации действий:

- `Medicine` — первая помощь (`AC_FIRSTAID`).
- `Engineering` — установка ловушки на объект, разминирование ловушки,
  установка/разминирование мины, вскрытие замка и ремонт панцеркляйна.

Это добавляет источники практики и может изменить скорость развития персонажей.
Часть пока не перенесена: нужна отдельная проверка оригинального `RegisterAction`
и правил роста навыков. Обозначение upstream «retail» само по себе не считается
подтверждением сохранения исходного баланса.
## Условия поражения из смешанного `9af844d6`

Источник: [9af844d6b62ed800bb59a64057b03e0eafc83878](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9af844d6b62ed800bb59a64057b03e0eafc83878).

Помимо исправлений доступности действий, коммит меняет завершение матча:
для продолжения игры требуется именно боеспособный герой; боеспособный
не-герой при недееспособном герое больше не спасает игрока от поражения.
В hotseat поражение проверяется сразу по числу оставшихся игроков,
независимо от владельца текущего хода; сценарный `bScenarioGameOver` также
включается в условие открытия экрана поражения.

Эти части пока не перенесены: меняются условия и момент окончания игры,
поэтому нужны сверка оригинала и согласование с действующей реализацией LAN.
Исправления доступности инструментов и кэша действий переносятся отдельно.

## Предварительный melee-фильтр из `f6455c9f`

Источник: [f6455c9f92eb6d4c66efebc74face61d1e1065bb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f6455c9f92eb6d4c66efebc74face61d1e1065bb).

Отдельный запрет направлять курсор атаки на бойца из выбранной группы
переносится независимо. Другой hunk добавляет ранний возврат `-1` из
`CGame::GetCompositeToHit`, когда melee не достаёт до центра корпуса цели
из текущей позы. Это меняет общий результат расчёта, которым у нас пользуются
не только подпись курсора, но и Lua `UnitGetToHitUnit`, и сетевой preview.

Общий расчёт не изменён: для его изменения нужно проверить контракт оригинальной
Lua-функции. Полезная часть адаптирована в `CStateAttack::UpdateCursorInfo`:
для недостижимой из текущей позы melee-цели остаётся подпись AP, процент не
выводится; для доступной цели процент сохраняется. Тест проверяет и неизменный
результат общей числовой функции. Комментарий upstream о retail сам по себе
не подтверждает сохранение поведения сценариев при переносе общего фильтра.

## Физика, perks и скрытность в смешанном `3cfccbee`

Источник: [3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a).

Ссылка на выбранную гранату и восстановление временного расчётного объекта
переносятся отдельно: это исправляет падение продолжения сохранённого броска.
Следующие части пока не перенесены и требуют проверки оригинальной механики:

- Новый `GetGrenadeThrowSkill` добавляет perk `0x5b` и использует дробный
  результат после VP-штрафа. Текущий расчёт хранит подготовленный навык как
  целое число. Меняются точность и вход навыка в скорость броска.
- Общий `GetMaxThrowVelocity` применяет силу, навык, массу и множитель perk
  `0x58` непосредственно к скорости; для PK берёт его grenade strength.
  Замена текущего `GetMaxImp`/`GetRelWeight` и промежуточного округления меняет
  скорость, дальность и доступность траектории. Увеличение скорости влияет на
  баллистическую дальность квадратично.
- Perk `0x5a` отключает изменение времени запала и соответствующие случайные
  выборки. Меняются задержка взрыва и дальнейшая последовательность RNG.
  Скриптовый бросок дополнительно переводится с локального на общий gameplay RNG.
- Для PK меняется перебор точек начала броска и выбранная сторона анимации.
  Это влияет на возможность бросить гранату рядом с препятствием.
- Бросок снимает `Hide` и устанавливает `bUpdateVision`. Меняются обнаружение
  бросающего и момент запроса пересчёта; эти условия требуют сверки оригинала.

Это не утверждение, что все перечисленные части являются ребалансом: upstream
заявляет восстановление retail. До сравнительной проверки этого недостаточно
для включения числовых и игровых правил в перенос с сохранением баланса.
При исправлении загрузки сохранены текущие функции физики, расчёт навыка,
запала, стоимости и последовательность RNG; ссылка на старую активную гранату
используется только для прежних локальных сохранений с пустым полем выбора.
Обслуживание уже сохранённого запроса `bUpdateVision` при завершении перенесено
отдельно: проверяются native-флаги отложенной очереди мира. Новые условия снятия
`Hide` этим не вводятся и по-прежнему не перенесены.

## Начальное состояние навыков — `0ffb568d` сверён и перенесён

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0ffb568d67bc6ca389d7ca63a9da4f3978c801a7)
смешивает сохранение общего опыта при `SetXPLevel` и начальные флаги навыков.
Потеря опыта отдельно воспроизведена в текущем репозитории: локальная переменная
затеняла сохранённый `CSkilledObject::fXP`. Исправление записывает результат
прежнего `GetXPForSkill` в это поле, сохраняя формулы порогов, навыков и обычные
множители начисления опыта от интеллекта/perks.

Оставшийся hunk независимо подтверждён original Steam EXE
(SHA256 `4F417593A9F73E2BFDE12D83CDBD694AE47EECB92812140D67B8B343E4A95705`):
InitSkills `0x6bc7d0` исключает VP/AP, для остальных15 навыков записывает флаг
и пересчитывает cap (`0x6bc858..0x6bc86c`). Native RED подтвердил потерю
действующего бонуса IC8 при cap9, несмотря на корректную установку perk8.
Отдельный фикс инициализирует флаги только новых персонажей; GREEN проверяет
действующий бонус,15 характеристик с +/-модификаторами, расходуемые VP/AP и
сохранение записанных старых unfrozen flags. Это восстановление оригинала,
не ребаланс. Формат save и существующие записанные флаги не меняются;
автоматическая миграция старых false flags не добавлялась.

## Проверено: распределение повреждений PK и пилота — `432b64c8`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/432b64c8bbe97a9f347a6c9468e7c69173d68394).
Боевые правила проверены непосредственно в Gold/Steam и восстановлены:

- Вероятность критического попадания в пилота и его min/max урона используют
  `1 − fCriticalResist`. Принудительное значение critical2000 заменено исходным
  рассчитанным значением с целым усечением.
- После урона корпусу пробитие уменьшается на1590, с нижней границей0.
  Только оставшееся положительное пробитие продолжает обработку урона пилоту.
- Результат отдельно содержит урон и получателя: отказ, пилот или корпус.
  Нулевой урон корпусу сохраняет свою классификацию; отрицательные VP корпуса
  обнуляются. Восстановлены исходные ветки GodMode, обхода PK и сломанного корпуса.

Это меняет ошибочное поведение реконструкции и возвращает исходные правила.
Таблицы оружия, материалов, сопротивления, коэффициенты сложности и собственные
сетевые правила не перенастраиваются. Тип результата действует только во время
вызова; формат сохранений и сетевых данных не меняется.

Оригинальные функции найдены через ранее проверенную таблицу методов миссии,
уникальные точные блоки и прямые вызовы. Steam6bfb20 содержит обработку PK;
6bfc31 — вычитание сопротивления из1.0,6bfead — константу1590;
6bfdcb..6bfdd3 — обнуление отрицательных VP. Подробности независимой проверки
и нативных RED/GREEN приведены в UPSTREAM-IMPORTS.md.

Crash отступления AI, перенос экипировки, локализация hit labels и групповое
Hide завершались отдельными исправлениями. Цвета и затухание подписей урона
проверены по оригинальному Draw: урон PK голубой, красный канал обычного урона
затухает вместе с прозрачностью. Перенесены отдельно после нативного RED;
проверены реальные глифы растрового и векторного шрифта. Наши HiRes-шрифты,
размеры и семейства не менялись; на игровой баланс эти два правила не влияют.

## Melee dispatch и ограничения от ранений — оставшаяся часть `c1ab9336`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c1ab93366661d3a4e25a0effe62625bae92c2d0c)
содержит самостоятельный UI-дефект курсора, перенесённый через GetState.
Полный патч дополнительно вводит AT_KNIFE и перенаправляет melee damage в
PerformMeleeAttackPortion с явными world/AI map/target filter. Для атаки по
бойцу filter=pTarget ограничивает выбор получателя; меняется также ignore-list
(сам боец вместо возможной пушки). Это требует native проверки реальных
попаданий/отсечения и соответствия original dispatch, а не только курсора.

В текущей legacy CCriticalsBan новый AT_KNIFE без дополнительной адаптации
попадает в CR_DEFAULT_PARAM. В отличие от прежнего AT_MELEE, у object/tile
команд тогда не запрашиваются ограничения C_MOTIONLESS/C_PATIENT. Такой
побочный сдвиг eligibility не включён в UI-фикс: действующие dispatch, цена
и обе таблицы запретов проверены отдельно и сохранены. Это не доказательство
того, какие именно ограничения ножа должны быть в retail; они требуют сверки.


## Object-melee dispatch — оставшаяся часть `4cdc1ead`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4cdc1eada6c182c0a9093364e95c750a6166d4c8)
содержит независимый дефект выбора активного hull двери: он перенесён отдельно
через существующий CUserHullsTracker, после native RED на открытой двери.
Формулы атаки и melee executor при этом не меняются.

Не перенесены добавление CExecMeleeObject и переключение object attacks для
AT_MELEE/AT_KNIFE на этот executor. Новый путь удерживает объект-цель, получает
его текущий hull, выбирает CalcCovers вместо tile covers при живой цели,
использует CheckTileToHit/PeekRay и передаёт target filter в PerformAttack.
Это влияет на геометрию попадания и выбор получателя повреждения.

Upstream-комментарий прямо связывает сохранение target с Sentinels и отмечает,
что SS v1.2 передавал null при сходном object executor. Это указание самого
upstream, не независимая проверка оригинала. До native/retail сверки этот
боевой путь не переносится. Успешный тест активного hull не подтверждает
правильность новой melee-логики; её следует сопоставить также с оставшимися
изменениями c1ab9336 и действующими ограничениями ранений.

## Часы перемещения на карте главы — часть `70d12946`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778)
передаёт GetUITime вместо GetTime в обновление и отрисовку интерфейса главы.
Для текущего форка эта подмена не применяется: собственный ChapterTravel
считает перемещение с частотой 60 шагов/с по времени, переданному в Draw.
CTimeCounter, лежащий под GetUITime, ограничивает прирост одного обновления
150 мс. При интервале кадров 1000 мс такая подмена дала бы 9 шагов вместо 60
за кадр и замедлила бы существующее перемещение при низком FPS.

В переносе часов поддерживаются базовые игровые/UI-счётчики и интервал
кадра, но в Step/RenderFrame интерфейса оставлен прежний GetTime.
Сам расчёт ChapterTravel, скорость и маршрут не меняются. Его существующий
тест проходит на Windows и Linux для интервалов от 1 до 1000 мс, включая
маршруты и откат часов. Это проверка расчёта и сохранённого вызова с прежним
временем, а не доказательство GPU-отрисовки при каждом FPS.


## Выход из зоны при сценарном переходе главы — адаптация `70d12946`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70d1294640babb485804aa9fe7be998a3fb82778)
выделяет общий BeginZone handler и передаёт bEmulateChapter=true также из главы.
В нашем форке этот флаг запускает лечение, обновление сценария и медалей,
а также обработку оставляемого в зоне переносимого тела. При ручном входе
с карты главы уже используется false: обработка выхода была выполнена ранее.

Перенесена сама обработка сценарной команды, с сохранением двух путей:
из тактической миссии — true, с карты главы — false. Для карты сохраняется
также её собственный pPWLImage, как при ручном входе в сектор. Это адаптация
к действующему поведению форка; дополнительное лечение и повторная обработка
выхода из исходного общего handler не перенесены. Native-тест исполняет
BeginZone('ffight') в настоящей активной главе и проверяет созданную команду
перехода; отдельно проверяет прежний тактический путь. Сам переход в новую
миссию в этой проверке не исполняется.

Меню отряда требует обновлённых player trackers до обработки команд:
native RED показал, что закрытие меню на первом кадре иначе повторно добавляет
уже развёрнутого бойца. Обновление списка перенесено перед dispatch. Тест
открывает и закрывает настоящий интерфейс, продвигает мир и требует сохранения
исходных объектов бойцов и их количества; параметры и правила найма не меняются.

## Проверено: разрешения сохранения и выбор сложности — `02d85e5b`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72)
смешивает применение готового permission в интерфейсе и изменение вычисления
самого permission. Все независимые части затем перенесены с native TDD
и сверкой оригинала. Кнопки/shortcuts сначала исправлялись без изменения
вычисления permission; его live policy проверена отдельно, см. раздел ниже.

Маршрутизация Save в Load при уже имеющемся false permission перенесена
для игрового меню и настоящей тактической миссии с независимыми native RED/GREEN.
Перехват запрещённого QuickSave также перенесён с отдельным runtime RED/GREEN:
применяется уже готовый permission, его вычисление остаётся прежним. В нашем LAN
CMission использует собственное network menu и перехватывает save/load menu
bindings; старую hotseat политику вместо него не возвращаем.

## Боевые preview и выбор позы — оставшиеся части fcc994bc

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fcc994bc87c9dad2c90e6c3fd4288bbce281b094)
содержит также два изменения, пока оставленные для отдельной проверки оригинала:

- GetCompositeToHit запрашивает доступные части тела у hull цели вместо hull
  атакующего. Это выглядит как ошибка адресата, но затрагивает общий числовой
  preview, которым у нас пользуются UI, Lua и LAN, и выбор частей тела при melee.
  Нужно проверить доступность/недоступность реальной цели и контракт уже
  перенесённой подписи melee reach; одного совпадения параметра с diff мало.
- Directed attack places ограничиваются текущей позой stand/crouch/lay вместо
  PM_ALL. Это меняет автоматический выбор позы/пути перед выстрелом или броском
  и может изменить фактические AP подготовки. Наш mover переделан; до
  сравнительной проверки это нельзя выдавать за перенос без влияния на бой.

Это ещё не утверждение о ребалансе источника: его retail comments требуют
подтверждения. Модель ножа, связность invalid regions, видимость активного
предмета и отдельная остановка у двери перенесены независимыми RED/GREEN;
перечисленные боевые изменения вместе с ними не копировались.

## Повторная проверка четырёх отдельных изменений игровых правил

Эти четыре diff прочитаны целиком и сопоставлены с текущим кодом.
Они меняют правила действующего форка; независимого подтверждения указанных
автором retail-адресов у нас пока нет. Поэтому в этом переносе без изменений
баланса они оставлены для вашего просмотра. Это **не утверждение**, что автор
намеренно изменил баланс оригинала: возможное восстановление оригинала нужно
сверить отдельно. Код игры по этим четырём источникам не изменён.

| Источник | Что изменится при переносе |
| --- | --- |
| [eb45f18a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/eb45f18a9787d7994bcdb224b198818009786445) | CDynamicSkill::Upgrade переводит накопленный прогресс в целые skill points усечением вместо округления. Первый рост сдвигается с progress≥0.5 на progress≥1.0; отрицательный остаток после раннего округления исчезает. Так же усекается fractional cap вместо округления. Это затрагивает общий темп/порог обучения; требуется отдельная сверка обоих x87 conversions. |
| [2e15a102](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2e15a10299e0d5e4a90558e39b34cc150ce51382) | Каждый RegisterAction(AC_HIDE) начинает тренировать Stealth через mission wrapper. Сейчас такого начисления нет. После нашего переноса явного состояния Hide нужно отдельно проверить вход, выход и повторный запрос уже установленного состояния: нельзя добавить рост Stealth на другое множество действий. Это отдельное правило обучения; перенесённый c55ed688 восстанавливает только уведомления при уже существующем росте. |
| [24a00fd1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24a00fd151a39d6c696c2ee8ab6deeb9d862f875) | Дополнительные пули short burst получают скидку CheapShortBurst по Param1. Например, при базовой цене10 и скидке0.2 получится8 AP. Одновременно вводится минимум1 AP **для всех** valid weapon modes: прежний результат0 при maxAP/RoF<1 становится1. Для отсутствующего оружия/нулевого RoF возвращается10000 вместо0. Старый код уже предотвращает деление на ноль, поэтому отдельного аварийного исправления без изменения AP policy здесь нет. |
| [aa724eca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed) | Смерть от периодического урона/кровотечения начинает раздавать XP всем боеспособным бойцам каждого враждебного погибшему игрока. Доля считается через GetXP(число бойцов), затем действуют обычные INT/perk множители AddXP. Это меняет награды и рост отряда, включая случаи без непосредственного убийцы; нужно проверить оригинальное распределение и пустые отряды. |

Статус этих четырёх источников — excluded-balance в рамках текущего
переноса; это зафиксированное решение оставить игровые правила для отдельного
рассмотрения, а не незавершённое обещание незаметно применить их позднее.

## Сброс подавления кровотечения — неиспользуемая сейчас часть853fbb71

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/853fbb7189bfdc0a4fa3c16781107d752840fa8e)
также присваивает nBleedingStopAmount=0 при новом bleeding critical,
ещё до проверки слияния/отбрасывания более слабого эффекта.

В нашем текущем CUnitMission это private поле только инициализируется и
сериализуется в tag35: рабочего чтения или установки подавления пока нет.
Поэтому сам этот hunk сейчас не исправляет игровой сценарий; его не копировали.
Если впоследствии восстанавливать весь связанный medical pipeline, сброс будет
означать прекращение временного подавления кровотечения даже от более слабого
нового bleeding injury. Тогда нужны отдельные проверки лечения и оригинала.

Независимая защита ApplyCritical от null перенесена через native SEGFAULT
RED/GREEN. Она не включает медицинский pipeline и не меняет сохранённое tag35.

## Выбор места при принудительном перемещении — ещё не перенесённая часть71bf7730

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/71bf7730c33bf37b19beaab21042f6a39e5b3285)
также меняет CUnitServer::ForcedMove: отбрасывает проходимые клетки без выходов,
разрешает подъём до0.05 вместо0.01, добавляет штраф0.04 на основании направления
бойца/вертикальной разницы и меняет классификацию Fall. Ранее вертикальный
кандидат с горизонтальным расстоянием²<0.01 выбирал Fall независимо от того,
попал ли он в допустимый предел падения; новый bCandidateFall устанавливается
только внутри fZDist<fMaxFall. Это может изменить конечную клетку и ветку
обработки падения при вытеснении бойца препятствием.

Эти части НЕ применены. В источнике они заявлены как retail restoration;
без отдельной сверки не называем их намеренным изменением оригинального баланса.
Они оставлены для проверки original parity и runtime сценариев, поскольку
эффект выходит за пределы синхронизации визуального состояния. Независимый
animator bInactivePose fix перенесён с native RED/GREEN; стоимость действий
и перечисленные placement/fall правила в этом коммите не менялись.


## Проверено: оригинальная трассировка и рикошеты —0726a489

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0726a489c66ef5131dc5f336d8539c316ca31573)
перенесён отдельными исправлениями после сверки оригинальной игры и native TDD.
Восстановлены границы трассы, завершение остановившейся пули, оставшаяся
энергия, отражение по нормали и передача остаточной дальности/clear-distance.

Рикошет использует существующие порог угла и вероятность из записи брони.
Значения брони, навыков и оружия не изменены. Если угол подходит, оригинал
делает один GetFloat(0,1), в том числе при нулевой вероятности; неподходящий
угол не расходует этот бросок. Native-проверки сравнивают следующий ISAAC word
с независимым генератором и проверяют равенства на границах угла/вероятности,
перенос энергии и clear-distance. Новых правил баланса здесь нет: дополнительные
броски восстанавливают исходный порядок, а не вводят новую вероятность.

Оригинальный Steam: сегмент691ca0, wrapper6924a0, границы691dc4, завершение692185,
угол691e81, RNG691e97/вероятность691e9c, перенос локальной атаки6923c3 и
рекурсивный вызов6923eb. Полные оригинальные disassembly, проверенная привязка
Gold/Steam и границы доказательства native-тестов описаны в UPSTREAM-IMPORTS.md.
Тесты используют настоящую функцию трассировки пули с аналитической геометрией
на входе; это не заявление о полном прогоне боя на реальной карте.

Визуальная подсветка бойца ИИ из того же коммита перенесена ранее и повторно
проверена отдельно. Сам алгоритм RNG и формат его сохранения не менялись.
Случайные попадания в посторонних бойцов относятся к другому источнику2de89e04
и этим закрытием0726a489 не объявлены готовыми.


## Проверено: выбранная и исходная сложность —98af43bd

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98af43bdaf252e5a2eac1ad047c92511adb8e565)
перенесён после независимой сверки оригинального Steam Game.exe. Factory699f90
сохраняет валидную переданную запись CDBDifficulty и выбирает DB1 при её
отсутствии/недействительности. До исправления наша фабрика всегда брала DB2.
Записи действительно различаются: уDB1/DB2 fAPCoeff0.4/0.7, fVPCoeff0.5/0.75,
enemyDamage0.66/1, ourDamage1.33/1, AI level adjustment-1/0.

Восстановлен оригинальный fallback1 для новой игры без выбора, включая tutorial.
Переданные1/2/3 сохраняют identity; реальные CICBeginGame для карт3/4/6 передают
их в исходные intro123/122/124. Коэффициенты и таблицы DB не редактируются.
Сохранённый global-game graph с выбранной2 загружается с2. Собственные LAN
базы/бой явно получают прежнюю DB2; это проверено до и после исправления,
включая реплику и переход из подготовки в бой. Таким образом, оригинальная
сложность восстанавливается, а самостоятельные правила LAN сохраняются.

Factory fix независим от hunks02d85e5b. Соответствие кнопок записям затем
проверено и восстановлено отдельно (раздел ниже); живое вычисление canSave
также независимо восстановлено (раздел ниже). Самостоятельного изменения
числового баланса в98af43bd не выявлено.

## Проверено: кнопки сложности —02d85e5b

Original Steam ProcessEvent63e200 (CSideMenuInterface, RTTI/vtable slot7,
GoldPDB638da0) содержит switch63e329..350: EASY ordinal0 -> DB1,
HARD ordinal1 -> DB3, NORMAL ordinal2 -> DB2. Текущая база независимо
подтверждает имена Easy/Normal/Hard для DB1/2/3. До исправления реальное
меню передавало ordinal как ID: normal работал, hard выбирал Easy1,
easy/default передавали null. Это ошибка привязки, а не новая настройка баланса.

Восстановлено оригинальное соответствие. Числа готовых записей не менялись:
для DB1/2/3 AP coeff0.4/0.7/1, VP coeff0.5/0.75/1,
enemy damage0.66/1/1, our damage1.33/1/1. Фактический выбор hard теперь
использует исходную Hard3 вместо ошибочной Easy1; easy/default явно несут
Easy1 (factory fallback1 уже восстановлен независимо). Пользовательский
выбор normal и собственный ruleset LAN DB2 сохранены. Прежние saves не
переписываются; mapping применяется при новом выборе в меню.

Native RED1.77с подтверждён через настоящее side menu и инициализацию hero
menu после положительного normal2. GREEN2.15с: обе стороны, три кнопки и
default, no-side guard и возврат к тому же меню. Related72/72 Windows/Linux,
NetworkMatch+Preparation2/2 и CI284/284. Live policy затем проверена отдельно.


## Проверено: доступность сохранений — оставшийся hunk02d85e5b

Живая политика независимо подтверждена в оригинальном Steam GameStep602160:
HQ разрешает сохранения, вне HQ CHAPTER0 запрещает, REALTIME1 разрешает только
IsRealTime && !IsSequence, ALWAYS2 разрешает. Вызовы оригинальных методов
установлены по RTTI/vtable и GoldPDB; адрес6020f4 из upstream comment не
использовался. Это восстановление правил оригинала, без изменения базы.

Текущие authored records проверены: DB1Easy canSave2 (ALWAYS), DB2Normal и
DB3Hard canSave1 (REALTIME). Записи с canSave0 в базе нет. До исправления
native Normal-миссия продолжала разрешать Save во время настоящей UI sequence
и в TBS. Теперь её следующий игровой шаг обновляет флаг: в бою/sequence Save
недоступен, при возврате в real time снова доступен. Easy/ALWAYS и исходное
HQ override сохранены. Неизвестный policy/invalid difficulty не переписывает
предыдущий flag, как в оригинале; формат и теги сохранений не менялись.

Своя LAN-партия сохраняет самостоятельный запрет Save: retail singleplayer
policy пропускается при networkWorld/networkSession. Это проверено настоящей
сетевой подготовкой до и после фикса, даже с ALWAYS; её обычная DB2 сохранена.
Ни один численный коэффициент сложности, стоимость действия, AI modifier или
RNG не менялся. Старые сохранения не мигрируются: live flag вычисляется на
очередном игровом шаге по исходной сложности/режиму.

Native RED1.51с — действующая CMission810, REALTIME1, realtime1/sequence1,
actual permission1 вместо0. GREEN2/2 6.40с: настоящие Begin/EndSequence,
GivePlayerTurn/EndOfTurn, Save binding -> реальный Load при запрете и Save
после возврата в realtime, реальный campaign3 HQ, own LAN prohibition.
CHAPTER0 проверен только на отдельной полной native in-memory копии записи;
это тест enum-контракта, не новое доступное пользователю правило/изменение DB.
Windows/Linux441/441,7Network/361maps и CI284/284 GREEN. Source02d85e5b
завершён; его перечисленные изменения соответствуют исходной механике игры.


## Проверено: исходная цена перка перезарядки — первый hunkb875e43d

В собственной реконструкции AC_RELOAD вообще не применял perk33. Original
Steam GetActionAP6c0d30 независимо найден из GoldPDB6bac90/3724 по уникальному
first32/maskEH9:13. Switch action23 ->6c13d2; HasPerk33 затем умножает исходную
цену на Param1 (fmul6c1407), integer truncate6c1416, return6c1932 без floor.
Это исходное поведение, даже если описание перка читается как обычная скидка.

Восстановлена именно original формула. Пример native Windows: authored
weapon65 raw40, perk33 Param1=0.3, AC_RELOAD раньше40, теперь12 как в retail.
Upstream предлагал28 (1-0.3) и минимум1AP: обе эти перемены оригинала исключены.
На Linux другая допустимая authored weapon127 raw12 даёт3, с тем же Param1.
Числа базы, perk parameter и raw weapon price не переписывались. Нулевая
цена после исходного integer truncation сохраняется; край1*0.3->0 проверен
на отдельной полной native in-memory копии записи, без изменения game.db.

Новая общая функция принимает конкретное оружие и используется player
AC_RELOAD. Обычный GetWeaponReloadAP остаётся raw; AI shot forecasts не
переведены на новую формулу. Сам AI reload GetInfo/Do исправлен следующим
независимым TDD-коммитом (раздел ниже), b875e43d завершён в адаптации к original.
Существующие saves сохраняют свои поля/числа; следующий запрос стоимости
действия у персонажа с уже взятым perk33 теперь восстанавливает исходную цену.

Native RED1.02с -> GREEN0.99с;3poses, реально взятый perk, explicit/current
weapon identity, raw prices/юнит AP неизменны от запроса, empty-hand0 и
исходный zero edge. Полные Windows442/442 и итоговые Linux442/442,7Network/
361maps и CI284 GREEN. Проверяется native API стоимости, а не анимация
полной перезарядки. Собственные LAN ruleset, facial/HiRes/HD не менялись.


## Проверено: AI reload использует исходную цену выбранного оружия —b875e43d

В оригинальном Steam CAIReloadAction::GetInfoInner41c8b0 и Do41da80 (обе
независимо найдены из GoldPDB по уникальному first32/maskEH3:7) передают
AC_RELOAD23 и выбранное оружие в CAIUnit::GetActionAP, а не читают raw price.
Unit forwarder4ad500/vtable8b5a6c slot5 совпадает всеми11bytes с Gold named
GetActionAP4abd20 и передаёт параметры в RPG mission. Это исходные правила ИИ.

После отдельного восстановления player/helper цена selected weapon127 равна3,
а held weapon65 —12 (raw соответственно12/40, authored Param1=0.3). Наша
реконструкция AI по-прежнему отвергала бюджет3 и списывала12 вместо3.
Исправлены только оба consumers: GetInfo проверяет3, Do списывает3, выбирая
конкретное оружие до изменения planning hand. Raw prices для shot forecasts,
параметр перка, floor/округление, стоимости смены оружия и физический исполнитель
не менялись. Upstream70%/minimum1 остаются исключены.

Два native RED1.13/1.14с -> GREEN8/8 5.63с: реальный AI unit/commander/world,
настоящее held/holstered оружие, spare clip и perk, planningAP12->9, физический
AP47->47; availability3 принимается,2 отвергается. План меняет только свою
inventory mirror, реальные ammo/held item не затрагивает. Это проверка
планировщика, а не полной анимации перезарядки/законченного AI turn.
Windows/Linux444/444,7Network/361maps и CI284 GREEN. Sourceb875e43d завершён
с сохранением original механики; новых численных правил не добавлено.


### f2ed3ca5 — восстановлен оригинальный момент оплаты занятия пулемёта

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f2ed3ca5ba3d4d0d3002f02f2aa0fb7f0f9a5f86):
оплата AC_APPROACH_CANNON и установка mounted state/item/occupant теперь
происходят после подхода, как в Steam Run7a6990/AnimationFinished7a6b00
(action25/call74f500at7a6b61). Сам расчёт цены и данные не менялись:
в нативном случае цена остаётся4 AP,46->42; до completion46/без mounted state.
Это восстановление оригинала, а не перенос иной экономики AP. Наш старый
уже оплаченный сохранённый граф42AP завершается с42, без второй оплаты;
поле фазы/новые save IDs не добавлялись. Выход мёртвого/бессознательного
пулемётчика также восстановлен отдельно: пропускается только переход в стойку,
сохраняется mounted context для штатного SyncConscious/падения. Дополнительной
оплаты, изменения HP/AP, урона или вероятностей при выходе нет. Все функциональные
части source теперь завершены; native455/455 на обеих платформах подтверждены.


## Проверено: нулевое целое кровотечение не создаёт атаку —2de89e04, часть1

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Отдельно восстановлен положительный барьер перед периодической атакой:
сила0.5 после преобразования в0 не создаёт ложное попадание0; положительный
контроль3.5 наносит3урона. Этот барьер подтверждён точным уникальным блоком
Gold/Steam97байт, Steam7c3209. Последующий перенос прохода и исходной формулы
описан в части5 ниже; доступ к рассчитанному индикатору и пулевые/ночные ветки
этого смешанного источника остаются отдельными пунктами.

## Проверено: независимые часы состояния и регенерации —2de89e04, часть2

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Восстановлены оригинальные интервалы: состояние обновляется через6000мс,
регенерация через30000мс; каждая ветвь обновляет только собственную отметку.
В реконструкции обе ветви ошибочно запускались вместе через3000мс.
Точные числа и порядок подтверждены единственным совпадением51байта Gold/Steam,
без масок, участок Steam7c3b51. Это восстановление исходной частоты игры.

Нативные проверки воспроизводят преждевременный вызов и сброс обоих часов,
затем проверяют границы интервалов и разнесённые фазы. Они наблюдают реальный
Segment и сериализованные часы, но не величину восстановления здоровья.
Тело регенерации, формулы, длительности эффектов и сетевые правила этим
коммитом не меняются. Последующий перенос самого прохода и целого расчёта кровотечения описан
в части5 ниже; это исправление относится к часам.


## Проверено: длительности эффектов обновляются в конце периода —2de89e04, часть3

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Существующий обработчик длительностей перенесён из обновления AP в проход
регенерации: конец собственного хода или30секунд реального времени.
В оригинальном StartNewTurn вызова старения нет (полное совпадение67байт),
а периодическая регенерация Steam6c49ec вызывает SegmentCriticals6c47c0.
Это восстановление исходного момента обработки; длительности и формулы
эффектов не изменены. Нативные RED4/5 -> GREEN5/5 проверяют конечный,
бессрочный и приостановленный эффекты на реальных игровых объектах.
Последующий перенос расчёта и старого события описан в части5 ниже.


## Проверено: признак первого хода стрелка в случайном луче —2de89e04, часть4

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Конструктор луча переносит исходный признак первого хода из миссии стрелка:
счётчик меньше2. Это подтверждено полными13байтами оригинального IsFirstTurn
и28байтами конструктора Gold/Steam с проверкой настоящей таблицы методов.
Раньше конструктор всегда оставлял false, что проявлялось в MakeAccidentalShot.

Это исправление данных луча. Четыре нативных проверки покрывают счётчики0/1/2
и отсутствие стрелка; обычные выстрелы нашего форка, которые позже задают
свой признак по миру, здесь не меняются. Расчёт шанса и новый обработчик
случайных попаданий ещё не перенесены; изменение точности этим коммитом
не заявляется. Численные коэффициенты, RNG и формат сохранений не меняются.


### Контроль кровотечения: воспроизводимый тестовый RNG

После воспроизведения нестабильности существующего теста положительного урона
ему задан детерминированный ввод RNG: seed2, первый бросок61 при IC5.
Реальные проверка уклонения, атака и очередь попаданий продолжают выполняться.
Производственные RNG и правила уклонения/урона не меняются. Зависимая обработка PK из432b64c8 перенесена отдельно; сам периодический
проход описан в части5 ниже.


## Проверено: целое периодическое кровотечение —2de89e04, часть5

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Восстановлен исходный проход в конце собственного хода и через30секунд
реального времени. Старое событие смены игрока больше не наносит этот урон.
Сначала обновляются сроки эффектов, затем считается целая потеря VP:
усечённая сила критического эффекта плюс десятая часть недостающих до половины
теоретического максимума VP, с повторным усечением. Подавление кровотечения
вычитается, результат ограничивается снизу0. Бессознательный носитель улики
защищён; обычный бессознательный юнит обрабатывается. Сохранён запрет повторного
применения на сетевой реплике. Обход брони PK использует уже исправленную432ветку.

Удалено прежнее автоматическое создание постоянного критического кровотечения
при любом попадании ниже половины HP. Оно срабатывало даже на заблокированный
удар0 и превращало временный эффект в постоянный при периодическом уроне.
В оригинале недостающее здоровье учитывает периодическая формула отдельно.

Числа и порядок подтверждены Steam6c49a0: константы0.5/0.1, два целых усечения,
подавление, положительный барьер и обновление штрафов от ранений. Метод мирового
повреждения отдельно найден через именованную RTTI-таблицу: Steam751570;
автоматического создания C_BLEEDING в нём нет. Это восстановление исходных правил,
без перенастройки данных оружия, сопротивления, AP и сетевой схемы.

Нативные проверки наблюдают реальное здоровье, очередь попаданий, сохранённую
величину и длительность эффекта. Доступ `GetBleeding` для индикатора исправлен отдельно: он читает
существующую сохранённую величину, без изменения формулы урона. Случайные
попадания и ночная точность остаются в работе.
Истечение VP-boost и CheckOverdose не объявляются восстановленными этой частью.
Собственные шрифты, мимика и HD-конвейер не меняются.

Сохранённые эффекты загружаются как записано в сохранении. Исправление
предотвращает новые ошибочные постоянные эффекты; старые постоянные кровотечения
автоматически не удаляются, поскольку их источник и утраченный срок отдельно
не сохранены и неотличимы от допустимых сценарных эффектов.


## Проверено: первый задетый юнит в уведомлении AI о пуле — `2de89e04`, часть7

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Существующее поле первого задетого юнита не заполнялось, поэтому уведомление
о пуле приходило с пустой целью даже после реального контакта. Оригинальный
Segment746ba0 запоминает первого живого получателя, включая остановленный удар,
и сохраняет его при последующих контактах. Восстановлено это присваивание;
визуальный пролёт без получателя не заполняет поле, ранее сохранённая цель
остаётся прежней. Урон, вероятность попадания, рикошет, AP и параметры оружия
не меняются. Нативные тесты наблюдают реальное событие и поле объекта.
Наше закрепление декалей на экипировке, кровь, мимика, шрифты и HD не меняются.


## Проверено: случайные попадания в посторонних юнитов — `2de89e04`, часть8

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Промахнувшаяся пуля пересекала постороннего юнита без отдельного броска
точности и без назначения получателя урона. Восстановлено поведение оригинала:
исходная цель промаха остаётся защищённой; другой юнит получает независимый
бросок на каждом прямом участке, включая новый участок после рикошета.
Повторные пересечения одного юнита в пределах участка не добавляют бросков.

Сохранены оригинальные границы: результат генератора0..99 сравнивается через
`<=`, поэтому нулевая заявленная точность допускает попадание при броске0.
Это подтверждено исходной функцией, не заменено новым правилом вероятности.
Существующая формула точности теперь получает сохранённый ночной флаг:
штатный множитель0.9 без perk0x15 уже был реализован и не изменялся. Контрольный
нативный расчёт даёт79 днём и71 ночью; новые значения коэффициентов не вводились.
Другие вызовы и расчёты тайлов сохраняют прежний вход; их ночной режим эта
часть не объявляет исправленным. Данные оружия, сложности, AP и повреждений
не менялись. Реальная пуля наносит рассчитанные штатным провайдером4HP ровно
один раз; тест учитывает исходные броню и сложность вместо предположения3HP.

Проверка опирается на оригинальные Trace691ca0/Check6b59f0/GetBullet6b5830,
таблицу методов CGame и GetLight, а также нативные RED/GREEN. Наши границы
трассы, рикошет, привязка декалей к экипировке, мимика, шрифты и HD сохранены.
Обработка эффектов из этого смешанного коммита остаётся отдельной работой.


## Проверено: порядок обработки и эффекты попаданий — `2de89e04`, завершающая часть

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d).
Частицы и звук создавались до реального расчёта повреждения. При одном seed55
визуальный RNG забирал бросок0, после чего юнит получал3HP вместо штатного
уклонения. В оригинале сначала выполняется ProcessAttack746df0, а случайный
поворот частицы — позже,746f8f. Восстановлен этот порядок: нативный контроль
даёт отклонённый удар-1, VP137 остаются137. Это влияет на порядок боевых бросков,
поэтому изменение не объявляется исключительно косметическим; формулы, исходные
коэффициенты, алгоритм RNG и параметры оружия не менялись.

Оригинальные условия также убирают эффекты у остановленного/отклонённого удара,
визуального выхода из юнита и чужой поверхности; учитывают выключенную кровь,
тип эффекта и служебный этаж100. Декаль связывается с реальной поверхностью.
Наше GetDecalsRef и сохранение декали на модели PK после выхода пилота сохранены.
Старые проверки привязки переведены с искусственного0HP на реальный небольшой
положительный урон, с точным контролем HP пилота/брони, AP и геометрии; новый
отдельный тест подтверждает отсутствие декали при0HP. Изменения баланса данных,
мимики, HiRes-шрифтов и HD-текстур отсутствуют. Все части этого исходного коммита
теперь учтены; подробные RED/GREEN и независимая проверка оригинала — в отчёте.


## Проверено: доставка событий AI и флаги контактов — `53c51af4`, часть `2eee1b62` / `81d0e839`

[Исходный маршрут](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/53c51af44c685ba15680bd228a5b667a1fb12e09).
Мировое обнаружение врага и правки текущей реакции ошибочно использовали один
маршрут блокировок. Одновременно запись флагов игнорировала блокировки и тем
самым маскировала проблему: после исправления только записи новые противники
переставали выбираться. Восстановлена оригинальная связка: мировое событие
доставляется сразу, а реакция правит контакты без новых флагов обновления;
накопленные раньше флаги сохраняются. Это исправляет уведомления и повторные
реакции AI, поэтому не описывается как чисто техническая правка без поведения.
Числа точности, агрессии, дистанций, здоровья и правила дипломатии не меняются.

Нативные RED/GREEN проверяют реальный мировой SeeNewEnemy, выбор противника,
союзника, вложенность, исключения и загрузку владельца обработчика. Маршруты и
блокировки независимо подтверждены PDB/RTTI и инструкциями оригинальной игры.
Наш отдельный адаптер Notify, владение объектами и совместимость сохранений
сохранены. Полный кэш поз контактов и остальные части смешанных коммитов пока
остаются отдельной работой. Мимика, HiRes-шрифты и HD не затронуты.


## Проверено: полный кэш поз контактов и правила включения — `81d0e839`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/81d0e8395d8ac04977fdc80e29ec0722d94b31cb).
Восстановлены оригинальные записи полных поз контактов: координаты/направление,
стойка, сеть навигации и признак бега. Неизменившийся контакт больше не помечает
список изменённым; сменившийся обновляет кэш, удаление очищает и список, и карту.
Пустая карта из старого сохранения восстанавливается при следующем уведомлении.

В оригинальные списки врагов включаются боеспособные враждебные юниты,
предполагаемые исключают уже подтверждённых, а союзники — самого себя и врагов.
Невраждебные нейтралы по-прежнему допускаются в союзный список. Это восстановление
поведения AI по оригиналу, подтверждённое PDB/RTTI/инструкциями и12 нативными
RED/GREEN, без новых коэффициентов или изменений данных дипломатии. Нельзя
описывать его как правку, совсем не влияющую на решения AI.

Наши живые координаты AI и отдельная плановая копия, обработка мирового
обнаружения и адаптер Notify сохранены. Новый виртуальный метод интерфейса
не понадобился: эквивалент проверки дипломатии локален для этих потребителей.
Формат сохранённых полей прежний; мимика, HiRes-шрифты и HD не затронуты.


## Проверено: обновление боевых снимков AI — часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Восстановлено оригинальное обновление снимка: у недееспособного юнита завершаются
боевой расчёт и оба маршрута; у активного обновляются навыки, поза, оружие и
непосредственно известные противники. Подозреваемые, союзники и дальнейшая
цепочка контактов не обновляются рекурсивно. Человеческий юнит обновляет себя.

Наши старые служебные поля и их интерфейсы сохранены; обновление больше не
перезаписывает их выдуманными сбросами. Активных игровых потребителей этих
геттеров сейчас нет, новую логику начала хода не добавляли. Очистка маршрутов,
как в оригинале, не увеличивает счётчик смены планов. Живые координаты и
отдельная плановая копия нашего форка сохранены.

Это исправляет планирование AI по оригиналу, без изменения коэффициентов,
параметров AP/HP, дипломатии и данных баланса. Нативные RED/GREEN проверяют
KO/смерть, активный и человеческий контроль, цикл контактов и боевой планировщик;
маршруты независимо сверены по PDB/RTTI/инструкциям. Остальные части смешанного
коммита пока остаются в работе. Мимика, HiRes-шрифты и HD не затронуты.


## Проверено: удаление обработанных контактов реакциями — часть `abbe4777` / `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abbe477726bba9b4bc50913b2d0d7a3f7cce720d).
Охрана, отступление, страх и обычная реакция удаляли обработанный контакт,
обходя блокировки уведомления. Теперь эти шесть мест используют оригинальный
путь Notify: новые флаги обновления не возникают, накопленные флаги сохраняются.
Проверены реальные реакции, очередь обработки, планы и живые цели; команда
тревоги, последовательность случайных чисел, AP/HP и позиции в этой части прежние.
Путь независимо подтверждён по PDB/RTTI и вызовам оригинальной игры.

Изменения страха из того же коммита — сохранение маршрута и резерв AP для
поворота — пока не перенесены: это отдельная проверка оригинального правила.
Текущий тест доставки намеренно контролирует прежний AP этой стадии; при переносе
правила AP он должен получить проверенный оригинальный контроль, без ослабления
остальных проверок. Числовые коэффициенты и данные баланса этой правкой не меняются.
Живые координаты нашего форка, мимика, HiRes-шрифты и HD сохранены.


## Проверено: страх сохраняет прежний маршрут — следующая часть `abbe4777`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abbe477726bba9b4bc50913b2d0d7a3f7cce720d).
Реакция страха ошибочно удаляла прежний маршрут перед выбором новой логики.
В оригинале такого удаления нет: маршрут приостанавливается и возобновляется
после снятия логики страха. Убрано только лишнее удаление; использованы уже
имеющиеся механизмы нашего форка. Отдельный маршрут сцен, вложенные паузы,
сохранённые команды и признак циклического патруля сохранены.

Нативные RED/GREEN проверяют спокойную ветку, подозреваемого, известного врага
и отсутствие прежнего маршрута. Порядок независимо подтверждён полным телом
оригинального метода и цепочкой установки/паузы/возобновления. AP/HP и данные
баланса этой правкой не меняются. Резерв AP поворота из того же коммита ещё
не перенесён и остаётся отдельной работой; мимика, HiRes-шрифты и HD сохранены.

## Проверено: AP при страхе и подозреваемом контакте — завершение `abbe4777`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/abbe477726bba9b4bc50913b2d0d7a3f7cce720d).
После выбора направления взгляда оригинал оставляет живому персонажу стоимость
поворота. Реконструкция сохраняла все AP; восстановлен вызов существующего
`SpendAP(живые AP - стоимость поворота)`. Планировочная копия AP не меняется.
Стоимость поворота остаётся прежней: 4 лёжа, 2 в других позах, с действием
существующего перка. Тест проверяет соответствие реальных записей RPGAP24/25
нашей стоимости; данные и формулы не переписываются.

Особенность оригинала сохранена: если AP меньше стоимости поворота, включая
ноль, отрицательный аргумент SpendAP может поднять AP до стоимости поворота.
Это подтверждено исходной машинной реализацией и отдельными тестами; новый
ограничитель для этого случая не вводится. Наши существующие режимы без расхода
AP и scripted sequence продолжают работать через тот же SpendAP.

13 новых вариантов настоящей реакции и два обновлённых контроля прошлых этапов
сначала подтвердили отсутствие резервирования (9 падений из 23); после фикса
23/23 прошли. Проверяются также перк с нулевой стоимостью, расход из живого
состояния при другом значении в планировщике, отсутствие лишнего RNG, сохранение
HP, позиций и команд. Это восстановление исходного правила, а не новая настройка
баланса. Сюда же относится исправление ошибочного комментария Assassin:
в полной оригинальной Update нет заявленных скобок AI-событий; игровой код
Assassin не менялся. Наши шрифты, лицевой движок и HD-текстуры не затронуты.

## Проверено: передача хода не перезаписывает планы — следующая часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
При передаче хода оригинал обновляет групповое состояние AI, а снимок отдельного
персонажа обновляет его собственная логика. Реконструкция дополнительно
перезаписывала планировочные AP/HP всех своих персонажей и известных контактов.
Этот лишний вызов удалён; реальные AP/HP и стоимость действий не меняются.

Тесты сначала воспроизводят перезапись на AI, человеческом CSequenceCommander
и повторной передаче; передача чужой стороне служит положительным контролем.
После фикса сохраняются отдельные планировочные значения и поза, обновляются
группы, очищается очередь команд и ровно один раз вызывается настоящий Lua
OnStartTurn. Ограничение на обычный AI-маршрут у человека и вложенная структура
его сохранения сохранены. Это восстановление оригинального момента обновления
плана, без новых правил расхода AP или подстройки баланса.

## Проверено: очистка состояния конца хода — следующая часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Список логик, закончивших ход, очищается при передаче хода действительному
владельцу. Если игрок ещё не назначен командиру при создании/восстановлении,
оригинал сохраняет этот список. В реконструкции он очищался раньше проверки
игрока; очистка перенесена под ту же проверку, что и Lua OnStartTurn.

Тесты воспроизводят потерю двух записей на настоящих неназначенных командирах;
действительный владелец и чужая сторона служат положительными контролями.
Очередь команд по-прежнему очищается в каждом случае, обновление групп остаётся
перед проверкой владельца. Формат сохранения, реальное состояние персонажей,
AP/HP и баланс не меняются. Это восстановление порядка операций оригинала.

## Проверено: получатели тревоги — следующая часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Тревога проверяет реальную боеспособность источника, цели и получателей.
Планировочный HP не определяет, жив ли персонаж для доставки события. Получатели
AI-событий и собственная отметка помощи проходят проверку управления AI, как
в оригинальном Unit.OnAIEvent. Вызов существующего немедленного обработчика
сохраняет эти правила и не добавляет блокировки реакции Notify.

До исправления тесты подтверждают обе стороны ошибки: живые персонажи с нулевым
HP в планировщике не участвуют в тревоге, а бессознательные с положительным
планировочным HP и человеческие записи старого списка получают события.
14 настоящих игровых сценариев сначала дают8 ожидаемых падений; после фикса
проходят вместе с5 существующими проверками. Проверяются блокировки/отметки,
реальная позиция контакта, AP/HP и неизменная последовательность RNG.

Радиус5м, цены действий, формулы и игровые данные не меняются. Это восстановление
оригинальных проверок по независимо сопоставленным методу команды, виртуальным
методам боеспособности и двум типам событий. Выбор союзников/нейтралов по миру
и ближайшего адресата ещё предстоит перенести; этот этап проверяет доставку
через существующий адаптер списка. Шрифты, лицевой движок и HD не затронуты.

## Проверено: тревога выбирает получателей по миру — следующая часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Оригинальная тревога использует текущие боеспособные юниты мира, а не старый
список только своей команды. Союзная выборка включает всех не-врагов, в том
числе нейтралов, по глобальной дипломатии сохранённого AI-игрока. Индивидуальная
враждебность персонажа остаётся отдельным правилом для записи контакта. Люди
могут попасть в выборку, но существующая проверка доставки не выдаёт им
AI-события. Радиус остаётся строго меньше5м.

Восстановлены метод выборки и его применение в тревоге. Проверены настоящие
новые команды, смена дипломатии без обновления кэша и передача юнита другому
игроку с заменой AI-обёртки. Сохранённый игрок может отличаться от текущего
владельца командира; догадка с подстановкой владельца не вводится. Защитный
выход при ещё не связанном мире сохраняет поведение предоставленного upstream
варианта для нашего пути восстановления, не изменяя правила исправного мира.

Сначала10 сценариев подтверждают неверных/пропущенных получателей; все14 прежних
проверок доставки и4 положительных новых контроля проходят. После фикса28/28
проходят вместе с проверкой обоих флагов и порядка выборки. Игровые данные,
цены действий, AP/HP, позиции, RNG, владение командами и формат сохранений не
меняются. Это восстановление исходной выборки, подтверждённой оригинальным
методом и типизированными вызовами мира. Ближайший адресат тревоги, подсчёт
поддержки и старые адаптеры списков ещё остаются отдельной работой.


## Проверено: ближайший адресат тревоги — часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
AI теперь ищет ближайшего текущего боеспособного AI-не-врага в мире, исключая
себя, вместо старого списка команды. Люди, потерявшие боеспособность и ставшие
врагами записи кэша больше не определяют направление тревоги. Смена владельца
и дипломатии учитывается сразу. Это правило подтверждено оригинальным методом,
включая обычную трёхмерную дистанцию, предел65535 и порядок равных расстояний.

Сначала11 из13 настоящих игровых сценариев воспроизводят ошибку; два контроля
проходят. После фикса проходят все42 целевые и прежние проверки. Не меняются
радиусы, цены, шанс тревоги, данные, пять команд маршрута и их параметры.
Подходящая цель сохраняет один выбор точки через RNG; без цели нет маршрута
и расхода RNG. Состояние планирования, владение, сохранения, наши шрифты,
лицевая анимация и HD не затронуты. Подсчёт поддержки остаётся отдельным этапом.


## Проверено: актуальная поддержка перед тревогой — часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Решение поднять тревогу теперь считает текущих боеспособных AI-не-врагов мира,
включая самого персонажа. Старый кэш мог пропустить нейтральную поддержку или
посчитать людей, потерявших боеспособность и врагов. Без настоящей поддержки
больше не происходит лишний бросок RNG. Оригинальные порог «больше одного»
и шанс40% сохранены и независимо подтверждены оригинальным кодом.

Сначала10 из12 сценариев воспроизводят неверную ветку или расход RNG; контроль
верного кэша и отказ по шансу проходят. После фикса проходят все54 проверки.
Маршрут тревоги, отступление, цены и данные не меняются. Наши шрифты, лицевой
движок и HD не затронуты; формат сохранений и владение командами сохранены.


## Проверено: тактический обзор и собственные привязки — часть `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Наш старый адаптер списков теперь использует ту же актуальную выборку мира,
что и восстановленные запросы оригинала: все боеспособные не-враги, включая
нейтралов и людей, отдельно от врагов. Владение остаётся собственным: чужие
юниты не регистрируются у командира и не получают его состояние. Собственные
юниты после восстановления получают привязку состояния даже без боеспособности.

Это адаптация к оставленным у нас старым потребителям списков; в оригинале
сам Synchronize только передаёт управление обновлению групп. Алгоритм групп,
радиусы, цены, данные и формат сохранений не меняются. До фикса12 из14 игровых
проверок воспроизводят неверные списки или потерянные собственные привязки;
контроли правильного списка и чужого состояния проходят. Шрифты, лицевая
анимация и HD не затронуты.

После явного задания сохранённого игрока в прежнем human-тесте повторный RED
на коде до фикса даёт12/22 ожидаемых падений, все8 прежних PassControl-контролей
проходят. После того же минимального фикса проходят84/84; итоговые связанные
проверки Windows179/179 и Linux177/177 также проходят. Пустой сохранённый игрок
по-прежнему проверяется отдельно, подстановки владельца в рабочем коде нет.


## Проверено: события хода доставляются юнитам — завершение `2eee1b62`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2eee1b6280f55129eb2cca681fbbbeb1c27f1e18).
Событие игрока больше не запускает лишнюю ходовую работу командира: управление
AI и служебная память меняются через существующий OnPassControl. Пустые ссылки
в восстановленном списке юнитов безопасно пропускаются. Обновление AP самих
юнитов и отметки завершения хода/перехода в реальное время сохраняются.

Сначала4 из6 реальных сценариев воспроизводят неверные флаги/потерю записи или
падение на пустой ссылке; два контроля проходят. Стек отладчика подтверждает
падение перед IsDead. После минимального фикса проходят56/56 проверок, включая
критические состояния и кровотечение. Оригинальная функция целиком совпадает
между Gold и Steam: все133 байта, включая проверку ссылок и отсутствие вызова
командира. Порог смерти, обработчики событий, формулы и данные не меняются;
наш формат сохранений, шрифты, лицевая анимация и HD сохранены.

Итоговые проверки: Windows213/213 связанные, Linux211/211, локальный CI290/290;
реальные игровые результаты совпали. Этим закрыт весь исходный коммит2eee1b62
с его14 файлами и перечисленными адаптациями нашего форка. Общий перенос всех
полезных исправлений и итоговая проверка GitHub Actions ещё продолжаются.


## Проверено: ближайший AI-контакт и HP планирования — `7f4f0ea8`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7f4f0ea872937e4dcb4a887ff6a136d7dce1fcd5).
Поиск ближайшего союзника или предполагаемого врага больше не отбрасывает
реального живого персонажа из-за нулевого/отрицательного HP в отдельном состоянии
планирования. По оригиналу здесь проверяется действительность ссылки, а состав
контактов меняют события. Действия сохраняют свои проверки боеспособности.

Наше получение текущей позиции уже правильно; оно сохранено. Общий поиск
использует прежний оригинальный предел65535 и порядок записей при равенстве,
две старые копии поиска заменены вызовом этого метода. Уведомления о смене цели,
удаление контактов и RPG-правила не меняются. Сначала10 из20 сценариев подтверждают
ошибку,10 контролей проходят; после фикса проходят46/46 связанные проверки.
Проверки включают настоящие игровые позиции, смерть/потерю сознания, события
удаления и неизменный RNG. Контакт умершего проверяется в отдельном снимке
состояния до обработки его удаления; глобальное событие смерти не подавляется.
Шрифты, лицевая анимация, HD и формат сохранений не затронуты.

Итоговые связанные проверки: Windows205/205, Linux203/203, локальный CI290/290.
Все20 результатов выбора и20 результатов после удаления совпали между
платформами. Это завершает исходный7f4f0ea8; общий перенос и итоговая проверка
GitHub Actions ещё продолжаются.


## Проверено: AI считает линию огня от точки выстрела — часть cc0fd988

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf).
Расчёт укрытия для AI использовал глаза вместо точки выстрела оружия, поэтому
оценка могла отличаться от правильной (например,90 вместо87). Теперь вызывается
наш существующий GetAttackOrigin с переданными позицией, стойкой и направлением.
Его код, включая путь стационарного оружия, анимация, модели, лицевая система,
HiRes/HD и данные оружия сохранены.

Оригинальный вызов и false для правой руки подтверждены по именованным таблицам/
методам и фактическим аргументам. Сначала7 из8 сценариев воспроизводят неверную
точку/оценку, контроль без цели проходит. После фикса проходят54/54 проверки.
Настоящие карта, геометрия, оружие и расчёт укрытия работают; проверены также
предполагаемая позиция без перемещения персонажа и штатная передача юнита человеку.
AP/HP, патроны и RNG не расходуются. Дальность, маски трассировки и достижимость
цели из того же исходного коммита остаются отдельной работой.

Итоговые связанные проверки Windows228/228 и Linux226/226 проходят, локальный
CI290/290 также проходит. Все8 сценариев дают одинаковые оценки и тип вызова;
малый остаток восстановления точки из проекции укладывается в одинаковый допуск.
Это закрывает только исправление точки выстрела; остальная часть cc0fd988
продолжает проверяться отдельно.


## Проверено: луч должен дойти до выбранного юнита — часть cc0fd988

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf).
При исчерпании обхода луча код засчитывал успех для юнита даже без достижения
его геометрии. Оригинал разрешает такой результат только при стрельбе по
местности; для юнита требуется фактическое достижение цели. Перенесено это
условие. Дальность оружия, коэффициенты, параметры брони и данные не меняются.
Остальные части исходного коммита ещё проверяются отдельно.

Три падающих сценария с настоящими пересечениями геометрии показывают100 при
нуле достижимых пересечений: AI, обычное разрешение и пилот в панцеркляйне.
Тест управляет границей обхода отдельно от дальней плоскости проекции, поэтому
это изолированная проверка игрового обхода, а не воспроизведение неизменённого
игрового ввода. Начальные ошибки подготовки такого сценария сохранены и не
считаются подтверждением бага. После исправления проходят16/16 проверок;
оценки обычных доступных целей87/90 сохраняются, местность даёт100. Для пилота
исходная оценка74 на Windows и77 на Linux; возврат старого условия на той же
Linux-сборке подтверждает77 и до переноса. Исправление сохраняет эти оценки
внутри каждой платформы, а недостигнутую цель отвергает на обеих.
Игровая геометрия цели по-прежнему принадлежит пилоту, декали — его реальному
отдельному корпусу. Наши связи PK/пилота, анимация, лицо, шрифты, HiRes/HD и
сохранения не затронуты; AP/HP/патроны и RNG при проверке не расходуются.
Оригинальное условие подтверждено по именованным методам и фактическому
вызову в Gold/Steam, включая отдельную успешную ветку для местности.


Итог: Windows236/236 связанных и2/2 прежних теста декалей; Linux226/226 прежних
связанных плюс14/14 новых/контекстных/декалей. После синхронизации отдельного
списка Linux локальный CI290/290 проходит. Дополнительная проверка старого
условия на Linux воспроизводит ровно3 ошибки из8; после точного восстановления
исходника и сборки те же сценарии проходят. Общий перенос и окончательная
проверка GitHub Actions остаются незавершёнными.


## Проверено: оригинальный предел дальности оценки AI — часть cc0fd988

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf).
AI мог положительно оценивать выстрел по цели за пределом оригинальных30
игровых единиц. Восстановлен минимум из расстояния до цели по трём координатам
плюс один и существующего предела30. Это исходное ограничение AI, подтверждённое
по фактическим аргументам и расчёту в Gold/Steam. Новое значение баланса не
вводится. Данные оружия/брони, видимость, обычный расчёт стрельбы и наша геометрия
сохраняются. Другие части исходного коммита ещё проверяются отдельно.

Сначала на обеих платформах падает ровно один из8 тестов: до цели33, по
горизонтали19,8, все47 пересечений дальше предела, но AI возвращает40/37.
Семь контролей проходят. После фикса —24/24 на Windows и Linux: дальняя оценка
AI становится0, близкая сохраняется; обычный расчёт сохраняет прежние39/38 и
36/32, заблокированные горизонтальные лучи остаются заблокированными. Различия
исходных оценок между платформами сохраняются; перенос их не нормализует.
Настоящие оружие, модель, карта и пересечения работают без подмены результата.
Начальные ошибки подготовки теста сохранены отдельно и не считаются RED.
Финальный тест не перемещает юнитов и не меняет анимацию, лицо, шрифты, HiRes/HD
или сохранения. Проверки не расходуют AP/HP, патроны и RNG.


Расширенные проверки: Windows244/244 и Linux242/242 связанных, по6/6 контекстных
и декалей на каждой платформе, локальный CI290/290. В каждой платформе изменилась
только дальняя оценка AI; семь остальных результатов и все оценки обычного
расчёта совпали с состоянием до фикса. Шестнадцать прежних результатов проверки
достижения цели на каждой платформе также сохранились. Общий перенос и итоговая
проверка GitHub Actions продолжаются.


## Проверено: дальность луча по юниту и проекция по местности — часть cc0fd988

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf).
Для юнита код растягивал допустимую дальность луча при отклонении от центра
прицеливания. Оригинал использует расстояние вдоль самого луча; проекция нужна
для точки на местности. Восстановлен этот выбор одной формулой. Параметры
оружия/брони, предел AI30, наша геометрия и анимация, лицевая система, шрифты,
HiRes/HD и сохранения сохраняются. Проверка масок трассировки остаётся отдельно.

На Windows и Linux сначала падают ровно два из8 тестов: лучи по юниту получают
100 за пределом радиальной дальности. Остальные шесть контролей проходят.
Тест подаёт замкнутые геометрические объёмы в настоящий растеризатор; он сам
вычисляет пересечения, а игра — итог. Это управляемая проверка геометрической
границы, а не повтор неизменённой модели в обычном игровом вводе. Отдельный
сценарий подтверждает, что союзник на местности по-прежнему блокирует луч при
проекции; применение радиального ограничения к этому случаю было бы ошибкой.
Проверены ближние случаи, штатная связка пилота с отдельным корпусом PK,
неизменность AP/HP, патронов, команд и RNG. Оба разных расчёта подтверждены по
фактическим веткам и аргументам в Gold/Steam.


После фикса проходят32/32 на каждой платформе. Изменились только два ошибочных
результата100→0/-1; шесть контролей и их настоящие геометрические счётчики
сохранились. Восемь результатов предыдущей проверки дальности и16 результатов
достижения цели на каждой платформе также сохранились, включая реальные PK.
Итог: Windows252/252, Linux250/250 связанных, по6/6 контекстных/декалей и
локальный CI290/290. Общий перенос и окончательная проверка GitHub Actions
ещё продолжаются.


## Проверено: препятствие у оружия и исходные категории — часть cc0fd988

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf).
В двух сетках расчёта укрытий юнита отсутствовала категория Item8000.
Из-за этого предмет, блокирующий оружие непосредственно у ствола, мог не
попасть в проверку, и она ошибочно возвращала100. Восстановлена маска8200,
которую оригинал передаёт и в AI, и в предпросмотр. Параметры оружия/брони,
дальность, геометрия, анимация, лицевая система, шрифты, HD и сохранения прежние.

**Не перенесён буквально выбор категорий из upstream.** Его Cover|Weapon
в наших и исходных значениях —600, хотя комментарий утверждает8200. Он начал
бы учитывать невидимые препятствия только для прохода, которые оригинальная
маска8200 пропускает. Добавление Fragment по одному лишь !bAIMode начало бы
блокировать предпросмотр фрагментированной геометрией. Оригинальный GetToHit
использует8200; физический выстрел AttackObjectRanged отдельно использует8210.
Наш общий интерфейс сейчас объединяет эти назначения. Буквальный перенос
изменил бы исходные правила; такое расширение исключено. Разделение назначения
физического выстрела и адаптация его вызывающего кода остаются отдельной работой.
Маску местности и глобальные значения категорий этот фикс не меняет.

На каждой платформе сначала падают три из14 тестов по нужной причине; остальные
11 проходят. После фикса проходят46/46 проверок укрытий. В управляемой геометрии
настоящий растеризатор сам вычисляет пересечения, а игровой код — результат.
Проверены обе сетки, препятствия только для прохода и только для предметов,
исключение самого стрелка, препятствие дальше зоны оружия, неизменность RNG
и игрового состояния. Отдельный сценарий попадает в цель исключительно через
низкую сетку и подтверждает, что исправлены оба вызова трассировки.


Расширенные проверки: Windows266/266, Linux264/264 связанных; по6/6 боевого
контекста/декалей и290/290 локального CI. Все прежние измеренные результаты
укрытий сохранились внутри каждой платформы. Четырнадцать новых управляемых
сценариев совпали между платформами: изменились только три ошибочных оценки
100→0. Общий перенос и окончательная проверка GitHub Actions продолжаются.


## Уточнение cc0fd988: дополнительная категория фрагментов уже эквивалентна у нас

После фикса [cb16ffea](https://github.com/mrartanis/silent-storm-remaster/commit/cb16ffea993635b536acf3b639c3a6b68017dd34)
проверено и оставшееся различие физической маски8210. В нашей реализации
укрытий отдельные фрагменты здания не являются ни препятствиями для оружия,
ни геометрией цели-юнита и пропускаются при расчёте пробивания. Исходный код
хранит сетки для последующего контакта; наш сериализованный объект укрытий
сохраняет лучи и использует отдельный путь контакта. Дополнительное расширение
общего интерфейса для этого пункта не требуется.

Подтверждение на неизменённой геометрии уровня: четыре сценария,1702 настоящих
дополнительных пересечения; совпадают каждый луч, проходимость, остаточная
бронепробиваемость и результаты100/38/90/25. Windows/Linux47/47, локальный
CI290/290. Это проверка эквивалентности без изменения игрового кода и без
заявления нового RED. Сам cc0fd988 закрыт с описанными адаптациями; остальные
изменения предпросмотра/контакта рассматриваются отдельно. Исключение буквальных
600/610 сохраняется. Оригинальный баланс и наши подсистемы не менялись.


## Проверено: AI оценивал выстрел оружием цели — часть66fc9e9a

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718).
Исправлен владелец инвентаря при оценке попадания AI: теперь используется
оружие стрелка. Никакие коэффициенты точности, параметры оружия, дальность,
AP или геометрия не меняются. Наши интерфейсы, целочисленная оценка укрытия,
защита от неконечных значений и отдельная обработка гранатомётов сохраняются.

Обе платформы сначала показывают два нужных падения из4: вооружённый стрелок
получает0 против цели с пустой планируемой рукой, безоружный получает100 за счёт
её оружия. После фикса —51/51 проверок. Физическое снаряжение остаётся настоящим
и прежним; тест меняет только отдельные планируемые руки AI. Расчёт использует
настоящие объекты, оружие, карту и калькулятор. Остальные части source66fc9e9a
ещё рассматриваются отдельно: выбранное снайперское оружие, дробная оценка
укрытия и бюджет движения цели. Возврат исходного NaN не разрешается этим фиксом.


Расширенная проверка этого фикса: Windows271/271, Linux269/269 связанных;
по6/6 боевого контекста и декалей,290/290 локального CI. Предыдущие47 сценариев
укрытий сохранили измеренные результаты. Проверка GitHub Actions на итоговом
HEAD остаётся завершающим этапом общего переноса.


## Проверено: отсутствующая или удалённая цель AI — часть66fc9e9a

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718).
Вооружённый AI мог аварийно завершить расчёт попадания по отсутствующей цели
или по удерживаемому, но уже недействительному объекту цели с очищенным сервером.
Восстановлен исходный ранний возврат0. Оружие, точность, дальность, AP,
поведение действительных целей и наши интерфейсы не меняются.

На Windows и Linux сначала аварийно падают ровно два из5 сценариев; три
контрольных проходят. Отладчик подтверждает чтение пустого указателя в GetToHit
и чтение очищенного сервера из калькулятора для удалённой цели. Проверка
использует настоящие игровые объекты и штатное удаление управляемой ссылки,
удерживая объект от освобождения. Произвольные висячие указатели этот фикс
не обещает исправлять. Другие части source66fc9e9a рассматриваются отдельно.


После фикса проходят56/56 профильных проверок на каждой платформе. Два
прежних падения теперь возвращают0; действительная цель сохраняет100.
Windows276/276, Linux274/274 связанных, по6/6 контекста/декалей и290/290
локального CI. Предыдущие51 сценарий укрытий и выбора оружия сохраняют
измеренные результаты. Общий перенос и финальная проверка GitHub продолжаются.


## Проверено: отсутствующий сервер самого стрелка AI — часть66fc9e9a

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718).
Восстановлена исходная проверка сервера самого стрелка до расчёта попадания.
При пустой или недействительной управляемой ссылке возвращается0. Этот фикс
меняет одно условие AI, сохраняя наши интерфейсы, формат сохранений, собственную
точку выстрела, анимацию, лицевую систему, шрифты и HD. Параметры боя прежние.

Тест моделирует только отсутствующую ссылку через штатное поле сохранения3;
настоящий юнит мира, оружие и цель сохраняются, ссылка восстанавливается после
проверки. Это проверка границы восстановления ссылки, не обычный игровой replay.
На каждой платформе сначала падают два из5 сценариев, три контроля проходят.
Стек подтверждает обращение к позиции пустого сервера и к очищенному серверу
при вычислении точки выстрела. После проверки ссылки проходят61/61 профильных
тестов; действительный стрелок сохраняет результат100. Остальные части source
по выбору снайперского оружия, дробной оценке и движению цели ещё рассматриваются.


Расширенная проверка: Windows281/281, Linux279/279 связанных; по6/6 боевого
контекста/декалей и290/290 локального CI. Предыдущие56 сценариев сохраняют
измеренные результаты. Итоговая проверка GitHub Actions остаётся после всего
переноса полезных исправлений.


## Проверено: выбранная винтовка при оценке снайперского выстрела — часть66fc9e9a

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718).
AI выбирал винтовку, но оценивал шанс по текущей планируемой руке. Теперь
оба расчёта — обычный и в голову — получают выбранное оружие. Старый способ
вызова сохранён, дополнительный метод добавлен в конец интерфейса; поля
сохранений и собственные подсистемы не менялись. Параметры точности, оружия,
брони, дистанции и AP прежние. Поздняя проверка движения цели пока отдельна.

На штатной карте кампании3791 найден настоящий противник в18,25м и положительный
эталон выстрела из выбранной винтовки. Сначала падает ровно один из3 сценариев
на каждой платформе: при пустой планируемой руке AI не выбирает голову, хотя
винтовка подходит. После фикса проходят64/64 профильных проверок. Контроли
проверяют оружие в текущей руке и пустые планируемые магазины; настоящее
снаряжение и патроны сохраняются. Тест проверяет этап выбора прицеливания,
не утверждает разрешение всей снайперской подготовки: последующая видимость
ещё может её запрещать. Исходный маленький учебный уровень для этой проверки
не подходил; ошибки его подготовки не считаются RED бага.


Расширенная проверка: Windows284/284, Linux282/282 связанных; по10/10 контекста,
декалей и прежних снайперских команд,290/290 локального CI. Старые вызовы
снайперской подготовки и прицеливания проходят. Прежние61 сценарий сохраняют
измеренные результаты; новые сравниваются с собственным эталоном запуска.
Общий перенос и финальная проверка GitHub Actions продолжаются.


## 2026-10-28 — four-AP snipe escape budget restores original behavior

Source [66fc9e9ad26497bf41f96a1a8c51c8a03e951718](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718), independent escape-budget hunk.
Imported: BeginSnipe scans target escape positions within4 AP, matching the
actual Gold and Steam callers, rather than the reconstructed full GetMaxAP
turn. This can enable an original snipe decision that our full-turn scan wrongly
rejected. It does not alter the target's AP, movement prices, weapon stats,
sight/FOV or gameplay data. Native positive and blocked controls distinguish
the cases. Own animation/map handling stays in place. The separate upstream
escape-facing change90293ca is still pending; this does not silently import it.
Fractional cover precision remains separately unfinished. Existing finite and
zero-penetration guards must be retained; do not restore upstream NaN behavior.


## 2026-10-28 — fractional AI cover restored; NaN restoration excluded

Source [66fc9e9ad26497bf41f96a1a8c51c8a03e951718](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718).
Imported: native cover percentage and remaining-penetration factor retain
floating precision through the AI hit calculator, matching typed Gold/Steam
math and parameter dataflow. This fixes real sample11->12 and hit prediction
6%->7%; it can change AI evaluations and choices to match the original, without
tuning skills, equipment, armor, AP, vision or data. Existing integer APIs/slots
and constructor are preserved through compatible additions; saved fields and
our weapon origin/animation/PK pilot-shell ownership stay in place.

Excluded literal hunk: upstream deliberately returns NaN for a nonempty ray
set with zero penetrating rays. Our fork previously fixed the resulting UB
when ranking weapons / converting that value to int in mission4522. Retain
empty/zero-penetration/zeroK/nonfinite return0 and bounded integer conversion.
This is an explicit adaptation to the fork's existing correction, not an
unreviewed balance coefficient. It does not justify discarding useful floating
precision. The separate escape-facing source90293ca remains pending.


## 2026-10-28 — first-incapacitation attribution restores original corpse alerts

Source [90293ca892e26576a90c8e6937a886c5c249010b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/90293ca892e26576a90c8e6937a886c5c249010b), independent metadata writer hunk.
Restore pKiller on the first conscious->dead/KO attack (both original builds),
and tDeathTime from the world clock (inspected Steam release; absent in the
older Gold body/PDB). Corpse events can now promote the actual recorded
attacker, and existing corpse-age cleanup can use the correct first tick.
This can change AI alerts after finding an allied body, matching original
behavior. Later corpse hits preserve the first attribution and time.
No damage, XP, critical, AP or difficulty coefficients are tuned. Preserve our
damage interface, PK pilot/shell handling, rewritten death/face animation and
existing32bit serialized fields. Separate escape-facing hunk remains pending;
do not mechanically replace current movement/visibility code for it.


## 2026-10-28 — escape-facing hunk already equivalent in this fork

Source [90293ca892e26576a90c8e6937a886c5c249010b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/90293ca892e26576a90c8e6937a886c5c249010b).
Its useful first-incapacitation metadata writer is imported in f2ee974d. The
remaining target escape-facing assignment is not added to production: actual
returned poses1/2 have direction-independent coordinates and occupied cubes,
and native all-eight-direction positive/blocked audits preserve visibility
and snipe eligibility on both platforms. Actor facing/FOV remains unchanged.
This is a verified equivalent case, not a balance exclusion or a manufactured
RED/fix. Revisit if allowed poses/cube geometry become direction-sensitive.


## 2026-10-28 — restore original scenario discovery, no new tuning

Source [360344ca1679d8bc7f4706d69e1270ee0caa3387](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387), independent visibility hook.
Actual human discovery now reveals a loaded carrier task and completes find
item/person descriptions, matching both inspected original releases. Unseen
clues remain hidden and script-task completion remains script controlled.
This restores original quest bookkeeping using our existing merged sight;
no sight range, AP, damage, XP, task DB values, animation, fonts or HD changes.
Initial hiding in source9ef8ae86 is a separate pending fix, not silently bundled.
Other mixed360344ca hunks, including destruction hooks, remain under review.


## 2026-10-28 — original carrier-task discovery lifecycle restored

Source [9ef8ae8645e1b4c53e4c5f83b1f87add815a7dc5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9ef8ae8645e1b4c53e4c5f83b1f87add815a7dc5).
Hide new carrier tasks until actual carrier discovery, matching the constructor
in both original releases. The existing discovery seam (7f245b09) reveals them
later, including loaded hidden tasks. Other8 task tags retain their defaults;
no task DB data, completion rules, rewards, AP, combat or other tuning changes.
This resolves the separate initial-hiding item noted in the preceding entry.


## 2026-10-28 — original one-AP undock planning bookkeeping

Source [8c6c27b55e74d5fe3c410372f9193c26a90ae174](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8c6c27b55e74d5fe3c410372f9193c26a90ae174), independent Undock.Do hunk.
AI planning to leave a stationary gun spends1 live RPG AP, as verified in both
original releases, enabling the next combat decision after the AP snapshot.
Previously the call was omitted as an allegedly inert flag. Native TDD shows
42->41 and IsNeedToThink0->1; physical exit spends no additional AP. Human/manual
exit executor, docking costs, cached AI AP, animation, PK/face and other tuning
are unchanged. This restores the original numeric behavior rather than choosing
a new balance value. Missing/retired owner references emit no command or cost.
The separate mounted-investigation route fix remains pending.


## 2026-10-28 — mounted investigation routes use normal dismount

Source [8c6c27b55e74d5fe3c410372f9193c26a90ae174](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8c6c27b55e74d5fe3c410372f9193c26a90ae174), independent route bug fix.
AI routes now queue ordinary dismount before a movement/turn/stance request
that the mounted state would reject. Preserve payload, animation wait and live
gun ownership, including restored routes with no transient AI cannon cache.
No new investigation trigger, priority, AP amount, path rule, vision range,
damage or reward coefficient. Native exit remains free; ordinary Look/Goto
keep their measured costs, and the separate original planning SpendAP(1) is
unchanged. Mounted shoot-mode/wait and already-unmounted routes keep behavior.
This resolves the separate route item noted in the preceding entry.


## 2026-10-28 — excluded: disarm radius0.8 ->1.0 (c8c4e225)

Source [c8c4e225df115072ce176598c1324aaf7bd7a7c0](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c8c4e225df115072ce176598c1324aaf7bd7a7c0).
Do not import the new F_DISARM_MINE_DISTANCE1.0 or the approach-helper distance
parameter added to use it. Both original binaries pass0.8 to the reach check
and retain the0.5 exclusion box; original-radius audit records typed mappings
and actual immediate values. The proposed1.0 is a25% horizontal radius increase
for disarming, leaving placement at0.8. It permits extra approach positions:
our real off-grid fixture has closest safe distance0.859001, with4 candidates
under1.0 and none under original0.8. This can make previously inaccessible mines
disarmable. Keep this reach change for the user's review; do not silently tune it.

The useful error-status fix is imported independently with unchanged0.8: an
empty approach list reports PathNotFound rather than internal success/public
general failure. Some off-grid mines therefore remain unreachable under the
original radius. No AP, charges, difficulty or explosive damage changes.
The separate late executor reach-validation hunk is now imported using the
existing original0.8 value. Native RED/GREEN verifies the boundary0.866386 is
rejected; the excluded1.0 radius is not introduced. AP, charges and the roll
remain unchanged. See the 2026-10-28 late-validation entry in UPSTREAM-IMPORTS.md.


### `531f3b7d` — защитный минимум Engineering в знаменателе ремонта

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/531f3b7dcfcda4e16c523e3b9a40b636eed52dd9)
считает стоимость ремонта как `int(deficit / Max(1, Engineering) * 20)`.
В оригинальных Gold `6b4940` и Steam `6baae0` деление выполняется прямо
на Engineering, без этого минимума. Защита меняет обработку запросов с
нулевым или отрицательным навыком; штатные наборы26/27 требуют Engineering100
и перк79, поэтому допустимый игровой ремонт использует положительный навык.

**Решение:** восстановить инженерную формулу, ограничение AP и округление,
но не переносить дополнительный минимум знаменателя. Также не добавлять
минимальную итоговую цену1AP: при реальном Engineering105 ремонт3HP в
оригинале имеет цену0AP. Не добавлять медицинский лимит оставшейся ёмкости
в ремонтный расчёт: исходный repair helper его не применяет.

Подтверждение: два нативных тела203 байта совпадают, кроме адресов двух
констант; тесты настоящего инженера, набора26 и корпуса93 воспроизводят
цены0/19AP,5HP за1AP и поведение при остатке ёмкости1. Это восстановление
оригинала; дополнительная защитная политика upstream исключена.


### `531f3b7d` — минимум знаменателя в расчёте скорости лечения

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/531f3b7dcfcda4e16c523e3b9a40b636eed52dd9)
использует `int(HP / Max(1.f, effectiveSkill * speed) * 60)`.
В оригинальных Gold `6b4ba0` и Steam `6bad40` соответствующий блок165 байт
делит прямо на `effectiveSkill * speed`, без защитного минимума. Сам навык
уже включает штатные поправки пациента, предмета и врача; множитель перка63
применяется только к обычному лечению, а не к critical-first или ремонту.

Дополнительный `Max(1.f, ...)` меняет цену и обработку запросов при нулевом
или отрицательном эффективном навыке, а также при произведении меньше1.
Это дополнительная политика upstream, не восстановление исходной формулы.

**Решение:** восстановить штатный множитель скорости, порядок операций,
ограничение доступными AP и обратное округление количества обработанных HP,
но исключить минимум знаменателя. Минимум итоговой цены1AP также не вводить:
контроль настоящего врача с перками57+63 сохраняет перевязку1HP за0AP.
Без перка63, для critical-first и для уже перенесённых расчётов перка57
контрольные результаты остаются прежними. Числа в базе и параметры перков
не изменены.

Подтверждение: полные165-байтовые блоки обоих оригинальных EXE совпадают,
кроме адресов вызова HasPerk и двух констант; обычное лечение в native TDD
меняет6AP на4AP и34HP на51HP за52AP. Все112 связанных тестов проходят
на Windows и Linux. Прямое деление сохраняет исходную механику; защитная
политика чужого форка остаётся за пределами переноса.

## Новые коммиты о случайных встречах — проверка 29 октября 2026

Оба источника появились после исходного отчёта на281 коммит. Они пока **не
перенесены**. Ниже описаны изменения исходников, а не подтверждённое
сравнение поведения с нашим целевым retail v1.2.

### `5d3fe7df` — появление случайных встреч

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5d3fe7df8e9a3f15f57353d2451df63d85b0306b)
увеличивает минимальную дистанцию появления от партии с8 до32 в единицах
карты и меняет условие отклонения броска0..99 с `< probability` на
`> probability`. То есть меняет вероятность появления с `(100-p)/100` на
`(p+1)/100` для целого p от0 до99. Также скрывает использованный маркер и
немедленно выходит после запроса миссии, а при очередном обновлении очищает
совпадающий nCurrentTemplateID. Это затрагивает частоту, расположение и
повторное появление встреч. Upstream ссылается на retail v1.1; требуется
независимая сверка с нашим целевым v1.2. Независимые lifecycle-ошибки нужно
отделить от дистанции/вероятности. Наш перевод координат HitTestMap сохранять.

### `b709a7dc` — вход только в активную случайную встречу

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b709a7dcb54aec463f2ba85e1cc7b697cc46b27f)
добавляет проверку активности маркера перед входом. Новый маркер с ещё не
назначенной позицией около начала карты и погасший, но ещё отрисовываемый
маркер больше не запускают миссию. Автор прямо называет это исправлением
бага самого оригинала после стоянки лагерем около начала карты. Это изменение
условий входа относительно заявленного поведения оригинала; числового
ребаланса в diff нет. Пока отложено до проверки собственного воспроизведения
и применимости ограничения на оригинальный баланс; автоматически считать
его ни подтверждённым ребалансом, ни восстановлением v1.2 нельзя.

## Проверено: цена выбранной аптечки AI — часть144c7b84

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/144c7b84dc51b2ef80784df6d40ae40dac1889e9).
Оригинальные GetInfo/Do передают выбранный предмет в расчёт стоимости.
Оригинальная ветка FIRSTAID возвращает nAPToUse этой аптечки без дополнительной
скидки или минимума. У нас двухаргументный расчёт смотрел предмет в руках:
при оружии возвращал15AP, хотя выбранная аптечка3 стоит10AP, а аптечка1
бесплатна. Теперь AI проверяет бюджет и списывает в плане цену выбранной
аптечки. DB, общая цена игрока и перки не меняются; бесплатная аптечка остаётся
доступной при нулевом бюджете. Подтверждено оригинальными Gold/Steam EXE и
native TDD4fail/8 ->251/251 на Windows/Linux. Это восстановление исходного
расчёта, без нового ребаланса. Списание здесь относится к плану AI; исполнение
лечения в мире этим тестом не заявлено. Поиск подхода из того же источника
остаётся отдельным неперенесённым изменением.

## Проверено: поиск подхода к лежащему пациенту — часть144c7b84

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/144c7b84dc51b2ef80784df6d40ae40dac1889e9)
и [параметр радиуса помощника](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/afee0c2eb6ee5cc6927eb48d9a273ef2622b6d72).
Оригинальный поиск места для лечения использует радиус1,2 для лежащего
пациента и0,8 для остальных людей. У нас команда всегда искала в0,8 и могла
заставить медика сделать лишний шаг, хотя прямой исполнитель уже разрешал
лечить с текущей клетки. Радиус поиска теперь соответствует оригинальному
ограничению. Сохраняется исходная точка у головы лежащего пациента: допустимая
дистанция до центра тела сама по себе не гарантирует отсутствие подхода.
Подтверждены оригинальные EXE и native TDD1fail/9 ->260/260 Windows/Linux.
Формулы, AP и DB не меняются; собственного нового ребаланса нет. Проверяется
построенная очередь/маршрут, без заявления о проигранном лечении в мире.
Направленные точки ремонта PK из этого источника остаются отдельным переносом.

## Проверено: направленные подходы к ремонту PK — завершение144c7b84

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/144c7b84dc51b2ef80784df6d40ae40dac1889e9).
Оригинальная команда ремонта использует точку в двух клетках сзади PK при
прямом направлении или четыре боковые точки со стоящим инженером лицом к
костюму при диагональном направлении. Наш прежний поиск в0,8 давал точки,
которые затем отбрасывались оригинальным внутренним ограничением ремонта;
команда не строилась даже при наличии подходящих мест. Восстановлены точки
оригинала, подтверждённые Gold/Steam EXE, включая точное50-байтное совпадение
прямого подхода и таблицу всех8 направлений. Native TDD16fail/20 ->280/280
Windows/Linux проверяет все направления пустого PK и костюма с пилотом,
маршрут, конечную позу/направление и четыре отказа. Цена, дальность самого
ремонта, Engineering/перки и DB не меняются. Это восстановление оригинальной
команды, без новых правил баланса. Проверено планирование, без заявления
о проигранном движении или ремонте в мире. Источник144 перенесён полностью.

## Проверено: исходные радиусы подбора — часть70cf40f3

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70cf40f33e68f95da25e8db4865dc86aefb05070).
В оригинале начальный поиск использует0,625 для обычного предмета и1,3 для
IHintItem; при передаче другому бойцу добавляет0,1. Наш общий0,8 отвергал
доступную подсказку и мог завершать подход к обычному предмету слишком далеко.
Полный72-байтный wrapper Gold/Steam, имена RTTI, точные биты констант и передача
параметра в исходный helper сверены; native TDD6fail/12 ->12/12 на обеих
платформах. Обычный предмет с той же DB-записью, что и подсказка, сохраняет
обычный радиус. Близкий подбор1,25 из предыдущего переноса не изменён.
Это восстановление исходного поиска, без новых коэффициентов/DB/AP-формул.
Более длинный корректный маршрут может иметь иную исходную цену движения;
проверки не утверждают неизменность стоимости произвольного маршрута.
Широкий повторный поиск1,6 из того же источника пока остаётся отдельной работой.

## Проверено: повторный поиск подхода — завершение70cf40f3

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70cf40f33e68f95da25e8db4865dc86aefb05070).
Оригинал повторяет подготовку перемещения с абсолютным радиусом1,6, если
начальная подготовка не удалась. Это относится к подбору с земли, передаче
между бойцами и разговору; у передачи сохраняется исходная прибавка0,1.
Старые точки очищаются. Прямой запуск уже готового действия эту ветку обходит,
прочие команды остаются без повтора, обездвиживание не обходится.
Проверены исходные Gold/Steam ветки, константы и вызовы; native TDD3fail/8 ->8/8
на Windows/Linux воспроизводит отсутствие близких точек и ограничение реального
PK-габарита. Начальные0,625/1,3/0,8 и близкий подбор1,25 остаются прежними.
Название DoubleRadius не означает умножение любого радиуса на два. Формулы
AP/предметы/DB не меняются; цена выбранного маршрута следует прежним правилам.
Весь источник70 теперь перенесён; новые правила баланса из него не вводились.

## Проверено: передача зарезервированных AP — cad50790

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cad50790ee66055c8b4d4d66cf16d3f4ae1dfc83).
Очередь передаёт дочернему действию уже занятые AP. Careful/LongBurst считают
оставшийся бюджет, сохраняя минимальную стартовую цену. Без этого reload14
перед Careful при53 AP давал ошибочный прогноз67; теперь53. LongBurst63->46,
реальное движение перед Careful61->53. Недостаток AP не обходится:42<43 и28<29.
Поведение независимо подтверждено оригинальными Gold/Steam методами и native
RED7/18 -> GREEN18/18 на обеих платформах. Цены reload/move/shot, perks, DB и
фактический расход не меняются. Прямые выстрелы и остальные режимы сохраняются.
Нулевая цена узла перемещения при подготовке убранного оружия в этих сценах
сохранена; это не цена RPG-действия AC_PREPARE. Его формула не меняется,
источник08c4 проверяется отдельно. Перенос исправляет прогноз до оригинала,
без нового правила баланса. Проверки не исполняют целую очередь выстрелов.

## Проверено: стоимость подготовки оружия — 08c4bb5a

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/08c4bb5a6b9a94337d88bd8241a2e51b6984cfce).
Оригинал берёт PrepareCost из типа действующего оружия, либо0 без огнестрельного
оружия. Перк22 вычитает параметр из цены; дробная часть усекается, итог ограничен
0..8. Это подтверждено независимыми Gold/Steam dispatch/DB/vtable/x87 ветками.
Заглушка реконструкции2AP/сам параметр перка заменена исходной формулой.
Настоящее оружие с ценой6 теперь расходует6, а не2; при штатном параметре
перка22=0 также6, а не0. Это восстановление retail, не новая настройка баланса.
Умножение цены на параметр, исключённое из7368b4b1, по-прежнему не переносится.
Цены/параметры DB не редактировались; своё преимущество установленного оружия
над оружием в руках сохраняется. Native RED10/12 -> GREEN12/12 обе платформы,
105 настоящих оружий/четыре позы/held+mounted/перк,1760 запросов и реальное
списание AP подтверждают перенос. Полная анимация подготовки/попадание не
заявляются; дробное усечение отдельно подтверждено исходной x87 реализацией.

## Проверено: полный прогноз короткой очереди — часть8cfff49d

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8cfff49d3a409f8514b3b1b78ee9b005ffd311d2).
Оригинальный прогноз резервирует первый выстрел и все оставшиеся пули короткой
очереди по существующей цене AC_BURST. Четыре пули15+3x2 дают21AP, а не15;
при20AP такая очередь отклоняется. Уже наведённое оружие11+3x2 даёт17.
Перк16 использует свой прежний параметр1: пять пуль стоят15+4x2=23.
Реальное списание первой пули и последующих не менялось; native контроль
подтверждает21->6->4->2->0, а при1AP следующая пуля ценой2 запрещена.
Полные оригинальные методы Gold/Steam независимо сопоставлены; native
RED9/18 -> GREEN18/18 обе платформы воспроизводит прогноз и public CanDo.
Цены/длина/DB/параметры perks остаются прежними. Исключённая новая скидка на
дополнительные пули из24a00fd1 по-прежнему не переносится. Однозарядный остаток
не уменьшает резервирование, как и в оригинальном расчёте. Отдельная часть
этого источника про потерю прицеливания при прогнозируемом движении завершена
отдельным TDD-переносом; см. проверенный раздел ниже. Полная анимация/попадания не заявляются.

## Проверено: прицеливание в прогнозе очереди — завершение8cfff49d

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/8cfff49d3a409f8514b3b1b78ee9b005ffd311d2).
После запланированного перемещения, поворота или смены позы оригинал перестаёт
считать оружие уже наведённым для следующего выстрела. Прогноз снова включает
прежнюю цену подготовки. По завершении расчёта исходная позиция и временный
флаг прицеливанияtrue восстанавливаются; реальная анимация остаётся прежней.
Оригинальные Gold/Steam блоки и поля независимо подтверждены. Native
RED11/20 -> GREEN20/20 обе платформы: Snap после движения19->23, поворота11->15,
смены позы15->19; короткая очередь25->29/17->21/21->25. Это восстановление
retail-прогноза, без новых цен или скидок. Даже штатно бесплатный поворот
сохраняет0AP, но снимает прогнозируемое наведение. Возврат в ту же точку не
возвращает временное наведение до завершения прогноза, как в оригинале.
Физическое прицеливание, AP/HP/патроны/позиция/предметы/время/следующее RNG-слово
не меняются; проверены также вложенные очереди, восстановление и контрольные
случаи без изменений. Собственный движок мимики/анимации и формат сохранений
не переписывались. Все части8cfff теперь перенесены; полный маршрут/выстрел/
пиксели и полный save round-trip этими новыми тестами не заявляются.

## Проверено: отказ из-за заполненного инвентаря — 4953e977

[Источник](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4953e977b39ac18b873baf2fd944771a903b6d3e).
Добавлен пропущенный UCR_INVENTORY_NO_PLACE в ветку интерфейса «действие
доступно для выбора, но сейчас выполнить нельзя». Игровой отказ, предметы,
размеры рюкзака, AP и правила размещения не меняются. Оригинальная ветка
Gold/Steam и native RED2/6 -> GREEN6/6 на обеих платформах подтверждены.
Рюкзак действительно заполнен настоящими предметами; после удаления одного
место снова находится. Собственные числовые коды сохранений/LAN, pending-
состояние и адаптер приоритетов сохранены вместо чужой перенумерации. Это
адаптация к нашему форку, не изменение баланса. Остальные hunks разобраны
в UPSTREAM-IMPORTS.md; все известные22 результата теперь обрабатываются.

## Проверено: музыкальные наборы контекстных меню — часть `1c67b8ab`

Восстановлено наследование ambient/combat-наборов при открытии меню из миссии,
карт главы/мира и главного экрана, включая все вкладки настроек. Новая сцена
получает собственные часы и состояние воспроизведения; живые каналы и состояние
исходной сцены не копируются. Это восстановление оригинального владения
аудиосценами, без изменения баланса. Сохранены наш нативный аудиодвижок,
прежнее продолжение текущего трека и существующие теги сохранения. Отдельная
часть о разделении UI-звука и фонового мира пока требует своего теста/аудита.

### Завершение `1c67b8ab`: звук UI отделён от фонового мира

Нативный тест подтвердил отдельную ошибку нашего старого обходного решения:
открытие настроек останавливало уже звучащий UI-сэмпл вместе со звуком мира.
UI теперь использует свою существующую сцену без музыкальных наборов; мир
продолжает владеть своей сценой, музыкой и сценарными звуками. При модальном
меню мир приостанавливается, UI-сэмпл продолжается; при закрытии мир
возобновляется. Поведение оригинала подтверждено независимо. Баланс,
коэффициенты, наш аудиодвижок/рендерер и формат сохранений не меняются.
Обе части источника завершены отдельными RED/GREEN и коммитами.

## Проверено: сценарная анимация общего освещения — часть `cb954e2e`

`SetAmbientEffect` восстановлена до оригинального поведения: вариант выбирается
по флагам мира, только первый источник эффекта анимирует общий свет через
существующий граф, используется текущее время рендерера. Полные частицы и
точечные источники этой командой больше не создаются. Замена, отключение,
окончание анимации и отсутствие подходящего варианта проверены нативно.
Сохранено и поведение оригинала при недействительном первом источнике: более
поздний источник не подставляется. Кэш цвета после удаления связи обновляется
при обычном обновлении базового света; дополнительной инвалидации не добавлено.

Это восстановление оригинального сценарного отображения, без изменения
коэффициентов, AP, навыков или игровых данных. Наш рендерер, граф анимации,
мимика, шрифты и HD-пак сохранены. Остальные части источника ещё не завершены.

### `cb954e2e`: запуск обычного `PlayEffect` и выбор варианта

Восстановлены текущее время начала по часам рендерера и выбор варианта по
флагам мира. Раньше короткий эффект мог появляться уже закончившимся, а
ночной вариант выбирался днём. Состав обычного эффекта, позиция и удаление
через `StopEffect` сохранены; обычный эффект не превращён в общий свет.

Техническая адаптация: при отсутствии подходящего варианта рендерер не
вызывается с нулевым эффектом. Это предотвращение вылета, без изменения
весов вариантов, игровых коэффициентов или данных. Оригинальный рендерер
тоже требовал непустой эффект; его вылет на таком входе не воспроизводится.
Остальные части источника ещё требуют отдельного переноса и проверки.

### `cb954e2e`: немедленная смена света текущей карты

Восстановлен приоритет света текущей зоны для немедленной команды Lua
SetTimeOfDay. Ранее мир менял время суток, а вид выбирал свет из общего
отладочного списка. Native RED2/5 и GREEN5/5 Windows/Linux подтверждают
ночь->день и обычный/запасной путь. Ветка независимо сверена с Gold/Steam EXE.
Временный GetDefaultLight удержан в CPtr для освобождения неиспользованного
результата без повторного выбора. Веса, условия мира и игровые формулы не
менялись; исключения по балансу нет. Фильтрация явного шаблона/отсутствие
варианта остаются отдельной незавершённой частью. Шрифты, HD и собственная
анимация не менялись. Подробности и границы доказательств — UPSTREAM-IMPORTS.md.

### `cb954e2e`: защита при отсутствии варианта освещения

Не переносим падение оригинального GetLight при пустом результате фильтрации.
Основной свет отсутствует — вернуть null; отсутствует только дополнительный
GF2 — сохранить основной без этой дополнительной записи. Это защитная
адаптация для подключения upstream-фильтрации по условиям мира. Native RED2/6
и GREEN6/6 Windows/Linux, GDB и сравнение оригинальных Gold/Steam подтверждают
контракт и причину падения. Для трёх seed сохранены точные0/1/2 вызова SRand,
веса вариантов и обычные результаты. Новых шансов/коэффициентов или замены
оригинальных игровых правил нет. Фильтрация самой сценарной команды пока
остаётся отдельным шагом. Подробные границы — UPSTREAM-IMPORTS.md.

### `cb954e2e`: варианты сценарного света по условиям мира

SetupAmbientLight теперь учитывает условия текущего мира, сохраняет прежний свет
при отсутствии подходящего варианта и продолжает SRand между вызовами, как в
оригинале. Ранее ночной вариант мог примениться днём, а генератор начинал одну
и ту же последовательность заново. Native RED4/6 и GREEN6/6 Windows/Linux
проверяют реальный Lua/мир/вид, день->ночь->день, отсутствие несовместимого GF2
и16 последовательных выборов с прежними весами3/7. Следующее игровое ISAAC
значение при обработке команды сохраняется. Флаги/веса и игровые формулы не
менялись; восстанавливается оригинальный выбор визуального света. Защита
фабрики перенесена отдельно. Подробности и пределы доказательств — UPSTREAM-IMPORTS.md.

### `cb954e2e`: команды освещения и эффектов в главе и фоновых мирах

Глава и главное меню пропускали корректные сценарные команды освещения,
фоновой анимации и обычных эффектов. Подключён общий обработчик с дословно
перенесёнными тактическими ветками; прежний звук остаётся общим. Native RED10/12
и GREEN12/12 Windows/Linux проверяют реальные интерфейсы, Lua, вид и граф сцены.
Общий путь подтверждён независимыми именованными ветками оригинального EXE.
Веса, формулы, данные баланса и схемы сохранений не менялись. Сохранены особые
параметры перехода главы, собственные часы и 2D-глобальная карта без мира.
Это исправление потерянных команд; исключений по балансу нет. Полные пределы
проверок и ещё не перенесённые ветки — UPSTREAM-IMPORTS.md.

### `cb954e2e`: учебный режим и подсказки в общих интерфейсах

Восстановлен показ учебной подсказки при отключённой опции и завершение Lua-
ожидания после скрытой/закрытой подсказки. Перенесены прежние тактические ветки;
экран, одноразовая регистрация и формула XP не менялись. Native RED5/8 и GREEN8/8
обеих платформ проверяют настоящие окна/события. Уже заданная награда1 даёт
прежние1.2 XP персонажу главы один раз; скрытая/повторная подсказка и меню без
кампании XP не добавляют. Это восстановление исходного поведения учебного
режима, включая награду, без новых коэффициентов. Учитывается Lua4: false=nil.
Прежняя проверка опции ==1 сохранена для штатных0/1; оригинальный EXE сравнивает
с0, поэтому эквивалентность произвольных не-булевых настроек здесь не заявлена.

Отдельно найден источник старого падения теста звука: случайная встреча при
отрисовке главы запускает переход в миссию. Это подтверждено native стеком;
правила встреч в игровом коде не изменены. Дальше — отдельная изоляция стенда.
Полные границы и доказательства — UPSTREAM-IMPORTS.md.


### `5f9977b7` / `cb954e2e`: скрипт первого посещения глобальной карты

Восстановлено оригинальное выполнение сценария карты. Подсказка №47 получает
уже заданную в базе награду34 XP; существующая формула интеллекта даёт тестовому
персонажу40.8 XP один раз. Это подтверждено оригинальным вызовом AddXPToAllUnits
и настоящим Lua/UI-проходом. При отключённых подсказках или повторном посещении
XP не добавляется. Значения награды, формулы и вероятность событий не менялись.
Исключённых изменений баланса в этом исправлении нет. Сохранены наши 2D-карта,
шрифты/HD, лицевая и телесная анимация, часы интерфейса и формат сохранений.


### `b709a7dc`: неактивный маркер случайной встречи

Исправлен ложный переход в миссию при попадании отряда в окно ещё не возникшей
или уже завершившейся встречи. Проверяется логическая активность встречи;
видимость её затухающего изображения сама по себе вход не разрешает. Настоящая
активная встреча по-прежнему запускает прежний переход с автосохранением.
Вероятности, дистанции, таймеры и правила появления встреч не менялись.
Это исправление исходной ошибки, без новых настроек баланса. Отдельные
изменения правил появления из `5d3fe7df` пока не перенесены.


### `5d3fe7df`: завершение обработки входа в случайную встречу

Перед постановкой перехода с автосохранением в очередь гасится логический
маркер встречи; после этого обновление сразу завершается. Так автосохранение
не удерживает уже использованный активный маркер, а вход не запускает лишнее
обновление таймера и броски появления встречи. Этот порядок подтверждён
оригинальными Gold/Steam EXE. Вероятности, дистанции и формулы не менялись.
Остальные части `5d3fe7df` пока не перенесены и требуют отдельной проверки.


### `5d3fe7df`: очистка записи о последней случайной миссии

Совпавший с сектором номер последней миссии очищается при наступившем сроке
обновления. До этого срока и для другой миссии запись сохраняется. Это
восстановление исходного учёта состояния; вероятности, дистанции и расписание
не менялись. Новая пауза появления встреч не вводилась: промежуточные записи
таймера в оригинальных EXE перезаписываются в том же обновлении.


### `5d3fe7df`: исходное сравнение вероятности случайной встречи

Восстановлено правило Gold/Steam EXE: бросок0..99 выше порога из данных
отклоняется, равный порогу допускается. Прежний знак работал наоборот:
порог100 блокировал появление. Проверены точные граничные броски, включая
исходный случай «порог0, бросок0». Значения вероятностей в данных, генератор,
число попыток размещения и таймеры сохранены. Это восстановление исходного
правила без новых весов. Минимальная дистанция пока остаётся отдельным пунктом.


### `5d3fe7df`: исходная минимальная дистанция появления встречи

Восстановлен исходный минимум32 вместо старого значения8. В Gold и Steam
проверяется расстояние от32 до256 включительно; значения подтверждены в EXE,
а случаи31/32/256/257 проверены настоящим игровым обновлением. Это единицы
исходной карты: HD-слой заменяет текстуры и не меняет ресурс геометрии Chapters.
Максимум256, данные вероятностей, число попыток и таймеры сохранены.
Новых настроек баланса не введено; все части `5d3fe7df` теперь перенесены.


### `ad118e4a`: начальное пересечение предмета с полом

При включении физики предмет с пересекающими пол сферами приподнимается
вертикально до контакта. Реальная книга438 проверена через игровой переход
из лежащего состояния в физику; исходное пересечение составляло.05 единицы.
Интерполяция начинается с исправленной позиции. Начальные положения гранат
и режима mass-center сохраняются. Коэффициенты трения/сопротивления, массы,
инерция, исходные скорости, генератор и три начальных случайных вызова,
DB-параметры и данные предметов сохранены. Это исправление геометрического
дефекта при пробуждении предметов; новых настроек игрового баланса нет.


### `4949f285`: отмена переноса и сбрасывания тела

Восстановлены исходные условия Gold/Steam: отмена пересоздаёт состояние
бойца только при смене режима переноса. Так отмена сбрасывания сохраняет
тело у бойца, а повторный вход в то же состояние не вызывает его очистку.
Проверены обе стороны операции до и после передачи тела. Очки действий,
здоровье, положение, исходные коэффициенты и данные сохранены; новых
стоимостей или настроек баланса нет. Изменения погоды/точности и прочие
части того же смешанного коммита всё ещё требуют отдельной сверки.


### `4949f285`: стабильный визуальный выбор снаряжения

Выбор geometry/material для привязанного снаряжения теперь использует уже
сохранённый bindSeed бойца вместо clock-seed каждого запроса. Проверены
реальные варианты шапки и полный save/load unit. Gameplay ISAAC не расходуется
этими запросами ни до, ни после исправления. Веса вариантов, статистика,
коэффициенты и существующий tag27 сохранения сохранены; исправлен только
источник локального визуального seed.


### `4949f285`: конечная ячейка луча видимости

Восстановлено правило оригинальных Gold/Steam: луч проверяет препятствия до
конечной ячейки сетки, исключая саму конечную ячейку. Точка у границы занятой
ячейки теперь может быть видна, когда препятствий перед ней нет. Препятствие
внутри луча и твёрдая начальная ячейка продолжают блокировать видимость.
На реальной карте проверены X/Y/Z в обоих направлениях; дистанция/FOV,
zero-length поведение, коэффициенты прозрачности, трава, environment и данные
сохранены. Новых значений или настроек оригинального баланса не введено.
Это восстановление исходной occlusion-политики; подробные native RED/GREEN
и проверка обоих оригиналов: vision-destination-verification.json и
vision-destination-retail-oracle.json в лабораторном каталоге свидетельств.

### `4949f285`: время пропуска невидимых действий

Восстановлен оригинальный бюджет0.03f секунды измеренной работы на вызов
CWorld::UpdateWorld вместо лимита пяти дополнительных hidden segments.
При медленной симуляции это предотвращает длинную пачку скрытых шагов; при
быстрой число шагов ограничивается временем и может отличаться от прежних пяти.
В native regression с50ms работой одного шага было6/300ms world-clock, стало1/50ms;
aim-clock в обоих случаях продвигается на50ms. Каждый следующий update получает
свежий бюджет. Это изменение темпа SP fast-forward, не новые AP/HP/XP/perk
коэффициенты или данные игры. Оригинальный50ms шаг, собственные simulation clocks
и monotonic timer сохранены. LAN host/replica scheduler закрыт прежним guard;
оба контроля проходят. Gold/Steam float0.03 подтверждён в обоих бинарниках.
Доказательства: hidden-skip-budget-verification.json и retail-oracle в лаборатории.

### `5780c0be`: сохранение цели и части тела команды броска

Команда броска в бойца сохраняет native unit target/eHL, а не превращается
в бросок в tile point с eHL=ANY. ANY использует hull center, head/body — свои
native точки; очередь подготавливает поворот. Это восстановление исходного
unit-target dispatch, подтверждённого Gold/Steam. Коэффициенты, стоимость AP,
диапазоны, формулы вероятности и генератор не редактировались. Native unit-cover
validation теперь получает unit metadata вместо прежнего tile запроса.
Фактическая генерация броска/полёт и другие формулы5780c0be ещё требуют отдельного
разбора, поэтому полная эквивалентность итоговых попаданий не заявляется.
Свидетельства: knife-unit-dispatch-verification.json и retail-oracle.

### `5780c0be`: geometry of the thrown-knife unit preview

Preview cover rays now use the ranged throw origin/clear distance instead of
melee height and do not request arm-reach hit locations. This restores original
action-based routing; authored coefficients, ranges, AP costs, skills and the
chance formula remain unchanged. On other geometry the displayed chance can
change because the cover query is now the correct ranged query. In the native
regression both numeric results are clamped to2%, while the wrong origins differ
by up to0.20m. The actual final flight/impact path remains a separate pending audit.

### `5780c0be`: probability from the actual shot origin

The unit preview/helper uses the cover solver's physical shot origin for its
distance and target-area inputs, as both original binaries do. This can correct
the displayed probability at grid-distance boundaries: the native Windows
ANY/head/body fixture changes92/88/89% to87/80/83%. No accuracy coefficient,
weapon range, AP, skill, item data or random rule was changed. The chance calcer
itself is unchanged. This also affects execution callers that reuse the helper.
The numerical example proves the distance correction; the same scene does not
provide an independently observable area-only difference.

### `9235a832`: debug rain/snow placement

The existing test-weather commands create wrapped particles with identity world
placement, matching both original binaries. This removes an extra camera-anchor
translation of the test cloud. The normal mission weather system, particle
wrapping algorithm, authored effect data and all gameplay/balance values are
unchanged. Effect replacement still owns one handle and releases the old one first.

### `974d8543`: existing mission Panzerklein restriction

The chapter marker now enforces the zone's existing bAllowPK flag against living
fielded mercs wearing a suit, matching the original Gold/Steam policy. Such a
party receives the existing localized denial and stays on the chapter map;
it must remove the suit or enter a zone permitting PK. Dead and reserve wearers
do not block entry. This restores authored mission access rules without changing
data, combat coefficients, inventory, costs or script-driven transition paths.

### `898ccb8a`: version footer

The main-menu footer receives the existing localized version text, original
styling and the current build date. This only changes the footer; no gameplay
data or coefficients are changed. Existing remaster/protocol metadata, menu
controls and font backend remain intact. Wide-string concatenation preserves
the full localized text on Windows and Linux.

### `e6a660f7`: authored reserve for person clues

Newly spawned occupied person-clue slots now receive their existing difficulty
death reserve after final level/VP initialization. With the authored coefficient3,
the Windows fixture's VP130/154 yields death thresholds-390/-462 instead of0.
This restores the original survival policy for story targets; it does not add
or change a coefficient or threshold formula. The zero-coefficient and ordinary
non-slot controls retain threshold0. Existing save fields persist the resulting
reserve; this change does not rewrite units already stored in an old save.

### `67ac71a9`: debug explosion command

The existing debug Explosion command uses original grenade record16 instead of21
and has no arbitrary unit owner. Its blast therefore uses the existing neutral
perk context and can be placed when the world's unit roster is empty. This
changes the debug command's authored preset and ownership; normal grenade APIs,
effect algorithms and data values are unchanged. Tests capture the producer's
call boundary, without claiming a full damage/visual comparison.

### `bf54ea44`, first part: overdose on VP drug administration

Restores the Gold/Steam overdose rule at the existing AddVPBoost call boundary:
strict sum(1/duration)>1 or zero denominator causes living units to become
unconscious at VP0 and clears duration tracking. Boost strength, temporary
modifiers, authored drug values and death reserve remain unchanged. Equality
is safe. Already-dead units are not resurrected. Native tests cover small
integer boundary combinations; arbitrary extreme-stack floating precision is
not claimed. Periodic duration countdown/check remains pending separately.

### `bf54ea44`, completed: periodic VP drug duration

Restores the original one-step duration decrement at DoRegenerations and checks
the same overdose rule before critical processing. Previously-safe combinations
can therefore become an overdose as the original duration elapses, with living
VP0/unconsciousness. Equality remains safe. No new strength/threshold/data values
are introduced, modifiers remain installed, and AP refresh does not count down.
The fork's authoritative-world gate prevents duplicate calculation on LAN replicas.
Native tests cover real turn-callback synchronization as well as the RPG consumer.
All source hunks are complete; this supersedes earlier pending notes for bf54ea44.

### `9da280e9`, first part: scenario grenade chance

Script grenade throws honor the explicitly supplied scenario chance, as in
Gold/Steam, rather than silently replacing it with the actor's calculated
accuracy. Scatter retains strict random>chance and the existing0.2 velocity
spread. Equal roll/chance is a hit; out-of-range arguments retain the original
unclamped boundary behavior. Max velocity, skill/fuse, grenade data, inventory
and ordinary player/AI throw calculations remain unchanged. Night-input
forwarding and per-unit turn state are still pending separately.

### Own prerequisite for `9da280e9`: launcher classification after DB load

The portable columnar loader populates RPGWeaponTypes before dependent weapons,
so the existing original type-ID5 rule classifies authored launchers correctly
on Linux. Previously their precreated type shells were read too early and the
launcher flag stayed false. Windows behavior is retained. No weapon values,
type IDs, rules or serialized fields are changed. This is an own-loader fix
required to verify the selected upstream night/launcher behavior, not a copied
upstream hunk. The night inputs remain pending separately.

### `9da280e9`, next part: night preview inputs

Tile, launcher and grenade previews receive the owning mission's actual night
flag, activating the existing original0.9 light multiplier unless perk0x15
bypasses it. The formula and authored values are unchanged; daytime results
and explicit script chance remain intact. This fixes previously optimistic
night chances in the legacy preview APIs. AI constructor and live grenade
executor night/per-unit-first-turn inputs remain pending separately.

### `9da280e9`, AI accuracy input

AI accuracy constructors receive the owning game's night flag and activate the
existing original light rule. Its0.9 multiplier, night-vision bypass, selected
weapon and other accuracy formulas remain unchanged. Native tests exercise both
cover overloads and light evaluation. Live-grenade executor night/per-unit-turn
inputs remain pending separately.

### `9da280e9`: live grenade night input

The native grenade executor receives the owning mission's night state and uses
the existing original light rule when deciding scatter. Previously it used day
accuracy at night. No scatter/velocity/fuse/skill constants or item values are
changed. The independent world-versus-unit first-turn input remains pending;
night tests hold that input constant to isolate this correction.

### `9da280e9`, completed: grenade actor's first turn

Grenade accuracy uses the actor's own first-turn state, as in Gold/Steam.
The existing first-round multiplier and movement suppression therefore receive
the correct input even while world first-turn is true in real time. Authored
coefficients, movement/skill/velocity/fuse formulas and turn counter/save fields
are unchanged. The native98-versus89/roll92 example restores a hit without
erroneous scatter; this is not a blanket decrease in accuracy. All functional
hunks selected from9da280e9 are now complete; this supersedes its prior pending
notes. The own-loader prerequisite f0103180 remains separately identified.

### `037c41d9`, first part: no-first-turn-penalty ability23

Restores the Gold/Steam ability hook: acquired23 returns
Max(existing first-round coefficient,1) before checking the first-round flag.
Default0.9 therefore becomes1; unperked behavior remains unchanged. A runtime
fixture coefficient1.2 verifies the original rule preserves values above1 even
on later turns. That value exists only in the owned test mission and is restored;
no authored coefficient, skill, damage, perk data or save field is changed.
Other independent perk/formula changes from037c41d9 remain pending.

### `037c41d9`: perfect-engineering tool ability50

Restores the Gold/Steam exception that lets acquired50 bypass a charged tool's
ordinary engineering/required-perk gates. A charge is still mandatory; the
outer tool stack count cannot substitute for a charge. Ordinary eligibility,
authored thresholds/charges/perk values, AP and actual charge-spending actions
are unchanged. Native tests isolate the RPG eligibility query and prove skill
bypass plus empty-container controls; no full engineering action is claimed.

### `037c41d9`: always-best-shooting-pose ability12

Acquired12 now applies the original best-pose choice to the active weapon's
stance accuracy bonus without changing the unit's actual pose. Restores the
Gold/Steam strict-comparison quirk: equal positive bonuses and two nonpositive
bonuses both choose ordinary WALK and contribute zero. Ordinary stance bonuses,
integer truncation, weapon data and AP remain unchanged. Native tests cover the
stance calculation; no whole-shot or animation parity claim.

### `037c41d9`: zeroing bonus85

Restores the acquired ability's Param1 addition to the number of completed
repeated shots for accuracy. The original sequence truncates the sum toward
zero, caps it by nMaxShotsRepeat, then doubles it. Keeps the original signed
behavior without adding a lower clamp. Authored bonus2/cap5 and ordinary
no-ability behavior remain unchanged. Native tests isolate this consumer;
exceptional floating values, overflow and whole-shot parity are unclaimed.

### `037c41d9`: difficult-traps ability49

Restores the original acquired49 multiplier for engineering-based difficulty
of mines and grenade traps: int((1 + Param1) * engineering). Retains truncation
toward zero, no lower clamp and ordinary no-ability behavior. Native tests
cover both difficulty APIs. Actual trap placement, spotting and disarm actions
are separate paths and are not claimed by this change.

### `037c41d9`: missing perk-tree lookup

Restores the original false result for classless units with no perk tree.
Prevents null dereference without granting abilities or writing optional
parameters. Actual classed acquired/untaken behavior and authored parameters
are retained. No balance constants or facial/head implementation changes.

### `037c41d9`: disarm-traps ability51

Restores acquired51 Param1 as an engineering addition for the disarm success
query, truncating the sum before the existing probability and 95 upper cap.
Preserves strict success comparison, one random draw, ordinary no-perk behavior
and the original unused second modifier. Native tests isolate the query with
fixed boundary rolls; actual trap removal, practice and AP/item spending are
separate paths. Authored data and current skill are unchanged.

### `037c41d9`: detection distance rounding

**Superseded precision inference:** the earlier double-product claim and
distance15/30 baseline22/0 below are incorrect under observed original PC24/RN.
See the later complete317 original-runtime audit; coefficient/truncation remain.

Restores Gold/Steam intermediate precision while retaining the existing stored
float coefficient44/30 and intercept45. Early float rounding could raise the
detection probability by one point: equal spot/stealth at distance15 was23
instead of22, and at30 was1 instead of0. Native tests confirm both boundaries
and clamp controls. This changes rounding to the original behavior; coefficients,
skills and 0..100 clamp are unchanged. Ability25/75 branches remain separate.

### `037c41d9`: better-spotting ability25

Restores acquired25's original local multiplier of the observer's spotting
term: int((1 + Param1) * spot). The authored .25 turns spot21 into26 inside
the detection query; spot21/target stealth30 at distance0 therefore gives41%
instead of36%. The actual skill is unchanged. Target-only25 does not influence
the result. Native tests cover ownership, truncation and clamp controls; mine
spotting and complete AI visibility are separate consumers.

### `037c41d9`: original ability75 ownership quirk

Restores Gold/Steam behavior even though it is counterintuitive: the observer's
acquired75 scales the target's local stealth term. With the authored positive
.25 this can lower that observer's detection chance; equal spot/stealth100 at
distance0 changes45% to20%. Having75 only on the target leaves45% unchanged.
The audited original query has no night gate. Actual skills and authored data
are unchanged. Native tests confirm ownership, truncation and clamp cases;
full AI/day-night visibility scenarios are not claimed.

### `2f5a3c75`: unsupported PK pose cost fallback (partial)

Restores the original AP-query fallback1 for worn-PK CRAWL/CROUCH in pose-
dependent movement/corpse movement queries. Authored suit1 adds4 AP to valid
movement; these unsupported queries previously returned ordinary costs such
as12/10 for side movement. WALK/RUN and pose-independent actions retain their
costs. Native tests exercise the query, not movement/animation execution.
Original pose-ability recursion precedes this guard; corpse42 ordering remains
pending with that ability's predecessor. No authored AP record changes.

### `037c41d9`: prone-costs-crouch ability34

Restores the acquired flag's original recursive CROUCH cost for pose-dependent
CRAWL queries. Ordinary side/diagonal/corpse-side/corpse-diagonal costs change
8/12/12/18 to6/9/9/13; other poses and pose-independent actions retain their
costs. The recursion precedes the worn-PK unsupported-pose guard. Native tests
preserve PK fallback1 and valid suit movement costs. Actual pose, AP spending,
authored records and movement/animation execution are unchanged by the query.

### `037c41d9`: crouch-costs-walk ability35

Restores the acquired flag's original WALK cost for pose-dependent CROUCH
queries: body side/diagonal/corpse-side/corpse-diagonal6/9/9/13 become4/6/6/9.
Original recursion runs before the PK guard, so worn-PK crouch side/corpse-
diagonal queries with authored suit1/add4 become8/18 rather than fallback1.
Native tests confirm this order. Actual pose and AP spending are unchanged
by the query. The original DB assigns34 and35 to different trees; their generic
recursive composition is retained by code without claiming vanilla joint
learning or a native combined-owner test.

### `037c41d9` / `2f5a3c75`: corpse-tracker42 AP and PK order

Restores fixed corpse-side/diagonal query costs from the acquired ability:
authored Param1=4 gives4/6 AP across valid poses, including valid worn-PK poses.
These fixed costs can increase a RUN query (ordinary3/4 becomes4/6), matching
the original rule. Worn-PK unsupported CRAWL/CROUCH still return1 before42 is
queried. Pose-independent actions and ordinary movement retain their costs.
Fractional parameters multiply before truncation; the API has no lower clamp.
Native tests exercise the cost query and preserve actual state. Authored data,
AP spending and physical carrying/movement/animation are unchanged by the query.
The2f follow-up's two hunks (unsupported-PK guard and corpse42 placement) are
now fully accounted for; the broader037c commit remains partial.

### `037c41d9`: cheap-melee32 AP

Restores the acquired ability's multiplier after the weapon's integer melee
cost: int((1-Param1)*baseAP). Authored .25 reduces item5 natural melee-skill33
cost11 to8 AP, skill0 cost12 to9 and skill140 cost7 to5. Fractional products
truncate; the original query has no lower floor. Thrown-knife and other actions
are outside32's scope. Native tests preserve AP/HP/XP/points, equipment, skills
and RNG. Authored data, actual AP spending and attack animation are unchanged
by this cost query. The broader037c source remains partial.

### `aa031e2e` / `037c41d9`: weapon/throwing-skill knife AP

Replaces fixed16 AP with the original equipped knife's integer min/max cost
scaled by effective throwing skill/140. Authored item5 min7/max12 costs12 at
natural throwing27 or skill0,10 at70 and7 at140. Melee skill and acquired
cheap-melee32 do not affect a thrown knife. The original cheap-throw43 effect
is still pending separately. Native queries preserve actual state and RNG;
projectile execution and AP spending are outside this change.

### `aa031e2e` / `037c41d9`: cheap-throw43 knife AP

Restores int((1-Param1)*integerKnifeBaseAP) for acquired43. Actual person2,
item5, natural throwing27 and authored Param1=0.25 give12->9 AP.
The original API truncates and has no lower floor; temporary Param1=1 yields0,
greater-than-one may yield negative costs. Melee uses32 independently. Native
queries preserve state/RNG; actual AP spending/flight are outside this change.
Grenade43 remains a separate pending cost consumer. The037c reload33 consumer
was already imported with the original Param1 multiplier; no balance rewrite
or duplicate fix is made for that hunk.

### `aa031e2e`: Steam grenade AP table5

Uses the existing authored RPGAP5 price20 instead of hardcoded20; the ordinary
unperked balance is unchanged. Runtime-owned table values are respected without
a lower floor, matching Steam1.2's query. Gold1.1 instead has fixed20; no table
ownership parity is claimed across these versions. Temporary24/7/0/-3 test
values are restored and never written to DB assets. Native query state/RNG
remain stable. Cheap-throw43 is still pending for grenades; actual AP spending
and projectile execution are outside this table lookup change.

### `aa031e2e`: cheap-throw43 grenade AP

Restores acquired43's int((1-Param1)*integerBaseAP) for grenades. Authored.25
reduces the unchanged table5 price20 to15 AP. Products truncate, with no
invented floor: temporary.375 gives12 from20,1 yields0 and1.25 yields-5.
Temporary table7/.25 gives5 and table-3/.25 gives-2. Gold has fixed20 while
Steam uses table5; both have this multiplier. Runtime test values are restored,
DB assets and query state/RNG remain unchanged. Knife43 and melee32 retain
their independent rules. Actual AP spending/flight are outside this hook.

### `037c41d9`: better-evasion40 IC

Restores acquired40's (1+Param1) multiplier to the stored effective IC float,
then rounds through Float2Int with the current mode. Authored Param1=0.15
makes IC21 become24 for actual person41. With normal nearest-even mode,
temporary.5 gives21 ->32 and23 ->34; no floor or upper clamp is invented.
The native query preserves effective skills and state/RNG. Attack-specific39,
aura, CheckIC training/random consumption and incidental damage remain separate
pending consumers; this change does not claim their full restoration.

### `aa031e2e`: explosive modifier min/max truncation

**Precision clarification:** the earlier wide-product inference and large-half
examples below are superseded by the later original-runtime audit. Integer
truncation remains correct (13.5 ->13); product significand is PC24/RN.

Restores original min/max damage multiplication with precision retained until
integer truncation, replacing nearest rounding of a premature float product.
For example9/11 damage with1.5 AE multiplier becomes13/16 instead of14/16;
10/14 with1.25 becomes12/17 instead of12/18. Neutral, zero and exact products
retain results; structure and human-critical rules remain unchanged. Native
typed-function fixtures also cover safe large/negative API inputs, with no
authored-data mutation. No actual explosion trace, damage application or perk
ownership integration claim is added by this arithmetic correction.

### `037c41d9`: live weapon familiarity reaches accuracy

Restores the current item's existing live familiarity bonus to the shared
additive accuracy term. No learning rates/caps or own6/71 rules change. Native
UseWeapon API stimuli creating3.75 live familiarity change isolated firearm
quality1 total to4.75 and melee/unarmed0 to3.75. Another item, even a separate
instance of the same DB weapon, contributes0. The mission's serialized mirror
is not the source of the bonus. Other accuracy factors still transform this
additive term; these figures are not direct final hit-chance percentages.
Native queries preserve state/RNG and do not mutate DB assets. Base sum-order/
precision and targeting73 remain separately pending; no full firing/AI-choice
or complete accuracy-formula equivalence claim is added here.

### `037c41d9`: original additive float order; upstream D2/Stance correction

Restores float stores after SMove+RS, then D2, then Stance; live familiarity
and integer quality follow before the final float store. Both original method
tables prove D2 precedes Stance; the upstream hunk reverses them and is corrected.
Native actual terms show small rounding differences (for example raw float bits
4106bc44 ->4106bc45). Four controls preserve results that the upstream order
would change incorrectly. No term values, authored records, learning rules or
query state/RNG change. These native tests verify the shared additive result;
effects on final hit chances after other factors and actual shooting are not
claimed here. The extra-AP/targeting73 formula remains separately pending.

### `037c41d9`: retail1.2 extra-AP coefficient and movement term

Restores Careful/Snipe int(SMove)-int(extraAP*-1.5), with separate truncations.
For stationary authored person14/firearm17, Careful extra2 gives raw total18
instead of17 and extra3 gives19; Snipe extra2/3 gives8/9. Retail1.2 also adds
the truncated movement penalty again in this term: at authored capped-9,
Careful extra0 raw total6 becomes-3 and Snipe-4 becomes-13. Gold1.1 uses an
older maximum rule; this change follows the project's explicit retail1.2
baseline. Raw negative values remain allowed; later accuracy factors/clamps
and actual hit chances are outside these tests. No DB values, actual AP
spending, skill-training or query state/RNG change. Faster-targeting73 remains
a separate pending multiplier; other calculation terms retain their values.

### `037c41d9`: faster-targeting73 only in Snipe

Restores direct Param1 inside Snipe's int(SMove)-int(extraAP*Param1*-1.5).
Actual person14/firearm17 with authored1.25 and extra10 gives raw additive23
instead of20. Extra1/2 remain6/8 after truncation; the bonus is not a uniform
percentage of the final hit chance. With authored capped movement-9, extra3
raw total-9 becomes-8. Careful/Aimed/SNAP do not receive73's multiplier. Direct
parameter/truncation/no-floor behavior is retained, including temporary zero/
negative boundary tests restored afterward. No authored DB, learning rule or
query state/RNG change. Prepare, actual AP spending, other accuracy factors
and complete shooting outcomes remain outside these native query tests.

### `037c41d9`: late incidental damage evasion and click scope

Restores incidental evasion as late damage reduction after damage calculation,
with max0, rather than rejecting the whole hit. Native base40/IC100 applies1
HP damage, IC20 on a successful roll applies33, and IC140 clamps to0 while
retaining the HUMAN receiver and normal RNG flow. These residual values follow
the original storedfloat.01 (.009999999776482582) and truncation. Intended
targets, worn PK and click-of-death skip the IC roll/practice. The real click
comparison removes an extra shared draw and.1 IC practice for incidental
victims. Existing wound-driven AP recomputation remains (40-damage control
51 ->48 AP); the RNG generator, armor, training formulas and DB assets do not
change. This step verifies native no39/aura0 noncritical RPG application;
typed explosion39/aura/unsigned chance and broader critical/world-trace effects
remain separately pending.

### `037c41d9`: original unsigned evasion chance

CheckIC now preserves the unsigned comparison found in both original binaries.
Negative effective IC therefore passes a percentage roll, as in the target
original. Actual native IC-10/base40 incidental damage becomes43 HP rather
than40: truncating the negative reduction gives -3, which is subtracted.
This numerical quirk is deliberately preserved instead of introducing a
zero clamp. Positive/zero/>100 controls, existing IC practice and exact own
RNG advancement remain verified. The change applies only to CheckIC; the
shared generator and global random comparison helper remain unchanged.
Typed explosion39 and attack-scoped aura still await their separate checks.

### `037c41d9`: explosion evasion39 affects chance and late reduction

Blast and fragment evasion now uses perk39 in both the successful-roll chance
and the late damage percentage. The original stored-float/current-rounding
rule is preserved; normal attacks and the base GetIC query do not gain39.
Actual authored.1 bonus with IC21/base40 yieldsIC23/damage31 (previously32).
Param.5/IC21 givesIC32: a selected newly-successful roll restores40 ->28,
and an already-successful roll restores32 ->28. Rounding examples are
IC1*1.5 ->2 and IC23*1.5 ->34; a negative modifiedIC still follows the original
unsigned chance and may amplify damage (IC-10/base40 ->43). Intended targets
and wornPK continue to skip IC, including practice. Own RNG, existing public
CheckIC, base40 perk, armor and DB remain unchanged. Nonzero aura ownership,
addition and victim application/reset remain separately pending.

### `037c41d9`: aura same-player ownership is independent of alliance

Own aura66 now checks the same player pointer among nearby units independently
of donor28's ally requirement. Two nearby playerless native units therefore
retain own authored10 accuracy/10 evasion, as the original pointer comparison
does; neutral donor28 contributes0. Same nonnull-player squads already count
as allied in this engine and keep their existing aura. Absent/untaken/outside
radius3.125 cases remain0. Both production attack/target preview methods and
native world neighborhoods are verified. Victim damage addition and per-hit
application/reset remain pending. A pre-existing Linux CP1251 perk-label log
exception is isolated in the fixture and is not an aura gameplay fix.

### `037c41d9`: existing aura value is added before final evasion rounding

The stored aura modifier now participates in both evasion chance and the late
damage percentage after any explosion39 float product, before final rounding.
Base GetIC stays unchanged. Native baseIC21/stored+10 givesIC31/damage28 from
40; with authored39 fragment bonus.1 it givesIC33/damage27. A newly-successful
roll independently restores40 ->28. Fractional tests distinguish rounding
the combined value from prematurely rounding the39 product: 1.5+1 rounds2,
and1.5+1.5 rounds3. Negative/zero sums keep original unsigned chance semantics;
intended targets and wornPK continue to skip IC practice. This step verifies
actual native stored state/RPG arithmetic; the real world's per-hit aura
application/reset is still separately pending.

### `037c41d9`: aura is scoped to the actual world damage recipient

World damage now computes the victim's evasion aura, applies it only around
the native RPG damage call and clears it immediately afterwards, including
a blocked or intended hit. A stale modifier cannot leak into later damage.
With actual authored66 +10, IC21/base7 restores5 HP damage instead of6; a
selected newly-successful chance restores7 ->5. Native world HP loss and
hit locator agree; unperked stale+10 restores base4 damage3 ->4. Tests also
distinguish to-hit output10 from evasion output0 and verify negative,
fractional/zero IC values, exact shared RNG and unchanged practice scope.
The source's stale address comments are corrected to mapped Steam7515e6..
751641. No field/schema is added; own world entry, audio/render/network and
public noarg CheckIC remain compatible. This completes the useful037c hunks,
including original-verified corrections to the source's float sum order.
The earlier Linux narrow perk-label logging issue remains separately recorded.

### `aa031e2e`: terrain grass has ground-floor metadata

Terrain grass now receives floor0 explicitly instead of default-100. The
native consumer's floor bits imply that grass is excluded by basement-1
mask7 (ground bit8), instead of remaining included via old bit1. Native
worlds810/4517 verify48 actual grass sectors and85 unchanged terrain controls;
the shared default-100 for other scene nodes remains intact. This is a visual
floor-visibility correction with no combat formula or DB/HD asset change.
Native render dispatch metadata is tested; actual GPU pixels are not claimed
verified here. Fragment/knockout/melee/plain-ray parts of aa remain pending.

### `360344ca`: carrier objectives complete on native death/removal

Death and removal now notify the scenario before inventory/body/state teardown,
completing DESTROY_ITEM_CARRIER tasks for the unit's plot items and any body
it is actually carrying. Actual authored goals now change raw and displayed
objective state from unknown to completed; two items are both handled. Normal
death, silent removal and unconscious->death are covered. A nearby uncarried
body, living carrier or missing item does not complete the task; already-
completed and unrelated task tags remain unchanged. Existing loot/ownership,
combat formulas and data files remain intact. Simple plot-item quantity0 is
preserved as the native input. This closes the360 destruction-notification
hunk; aa's new-attack-knockout notification is still pending.

### `aa031e2e`: new attack knockout completes native scenario tasks

A newly unconscious attack victim now completes its authored STUN_PERSON
task, even when discovery happened earlier or the clue is still unseen.
The same event handles DESTROY_ITEM_CARRIER for plot items in its backpack,
as in the original. Repeated hits on an already unconscious victim do not
notify again. Blocked hits, another persona and direct scripted knockout
do not complete this victim's task. Detection/visibility and unrelated task
states remain intact. This changes objective progression; damage, health,
IC training, combat RNG formulas and database records are unchanged. Original
leave-zone cache invalidation is restored. Actual full leave-zone UI/pixels
and all attack types are not newly claimed by these zero-damage native cases.

### `aa031e2e`: fragment speed metadata restored to3

Targeted and random fragments of ordinary and engineering grenades now pass
the original3.0 speed into real bullet objects instead of0. Counts, random
direction generation, attack values and shooterless particle suppression are
unchanged. This restores metadata/internal velocity scaling. Current fragment
trail models are null, and native travel-time division is guarded by a model,
so no change in visible flight time, damage or hit probability is demonstrated
by this fix. The plain-ray routing hunk is still a separate pending audit.

### `aa031e2e`: melee misses no longer dispatch a strike or success event

A selected melee ray marked as a miss now skips damage/sound dispatch and
the successful-melee event. Actual BODY/ANY misses previously dispatched
both despite chance2/check61; now neither is sent. Real positive controls
still strike and lose health. Existing melee damage/chance/RNG formulas,
weapon data, attack-cost spending at Start and consumed-attack tail remain.

Unverified diagnostic limit: a superseded control with attacker melee skill
2000 and targetVP20000 occasionally returned INT_MIN total health while
IsDead was false. This extreme-input critical/damage/VP path was not isolated
or repaired here and is not claimed to be a supported range or an original
formula. Final acceptance uses ordinary attacker skill0/HP500 and the native
forced-hit flag. Existing zero to-hit draw/infinite-ratio behavior is also
left untouched; final miss inputs explicitly avoid that draw.

### `aa031e2e`: fragment/plain rays use the original loose trace

Plain fragment rays now take TraceLooseRay. The older own cached-hit fallback
already produced a trajectory, but skipped the original loose-ray incidence/
ricochet and incidental-unit checks. Actual zero-damage rays through a human
now consume the original probability0 ricochet draw and chance100 hit draw;
the seeded four-word RNG continuation matches. This corrects the random
sequence seen by later combat and restores existing loose-trace behavior
without changing its formulas. Ignored units/empty rays consume neither draw.
Explicit true/empty-cache callers retain their own existing fallback.

The new checks use zero kinetic and establish routing, native metadata and
random consumption, with HP/AP unchanged. They do not newly establish every
live fragment damage value, all ricochet geometries or visible flight time.
All aa functional hunks are now audited/imported or adapted to original
behavior; the broader upstream queue remains open.

### `c1ab9336`: native melee damage receives world difficulty

Melee executors now pass the real world/map to the existing native damage
helper. The old route passed null, skipping difficulty damage multipliers.
Actual intended native Unit/Tile strikes with base2 and coefficient2 now
cause4 HP loss instead of2; anonymous/coefficient1 controls remain2. Damage
formulas are unchanged; their existing world-dependent inputs now reach them.
Own executor float API, cannon ignore alias and native sound are retained.
Target filtering and knife action classification are still pending.

Unverified state boundary: the selected tactical AI-owned mission still
reports stored IsAIPlayer0; assignment of that flag is not repaired here, so
this is not claimed to complete all enemy-difficulty behavior. Existing
native helper godmode callbacks/traversal also become active; HP protection
remains in RPG, but the four new ordinary cases do not establish every
special callback. These limits remain separate from the proven world fix.

### `c1ab9336`: unit melee only damages its selected receiver

A direct unit melee strike now filters native damage callbacks by its stored
selected target. Two actual bodies on the same ray previously both lost3 HP;
only the selected body now loses3. Unselected godmode bodies also stop getting
phantom -1 hit callbacks. Tile and null-target calls retain all-receiver
behavior. Armor/energy traversal still runs across unselected objects, so
this filter does not turn them into empty space. Damage/chance formulas,
world context, own vector/float API, sound and cannon alias are retained.

The current no-filter godmode path protects HP but produces two callbacks
for the two intersected body pieces. This fix preserves that control rather
than calling it original parity; original valid-unit duplicate suppression
is a separate pending comparison/fix. Attacker ignoring and knife action/
critical restrictions remain pending as well.

### `c1ab9336` / inherited `a8c50bf2`: one melee callback per godmode unit

Original valid-unit duplicate suppression also applies to invulnerable units.
A ray crossing two body parts previously generated two protected -1 hit
callbacks; it now generates one. HP stays protected, the next ordinary unit
still takes the same3 damage, and filtering/armor traversal are retained.
This resolves the separate godmode boundary recorded in the preceding two
melee ports. Canonical attacker ignoring and knife critical restrictions
remain pending. No new damage/chance formula or object validity rule added.

### `c1ab9336`: melee always ignores the canonical attacker

A direct melee ray through a mounted actor could damage that same actor because
only the cannon alias was ignored. Original dispatch always ignores the unit;
we now retain both unit and existing cannon alias. Actual mounted body control
changes self-damage3/hit1 to0/0; unmounted self and cannon protection stay.
This is direct-dispatch coverage, not proof of an ordinary UI route for mounted
melee. No damage/chance formula, sound or animation change. Knife action/critical
classification remains pending.

### `c1ab9336`: knife and fists have different critical restrictions

A nonthrowing equipped blade now has its own action classification. Original
knife attacks use the default command-ban list: unlike fists, MOTIONLESS and
PATIENT do not add a blanket knife-command ban. Other movement/target/critical
checks still apply. Native object/tile lists change5/4 to3/2; blind/object,
idlehand and brokenPK restrictions remain. Fists and throwing/firearm controls
retain prior lists. No damage/chance/AP-cost formula changes; knife cursor
stays correct. New action is appended to preserve all previous own ordinals.
This finishes the c1ab source's UI/dispatch/filter/ignore/classification scope;
the unrelated pending object-melee source4cdc and broader critical formulas
remain separate audits.

### `23630bfd`: audible but unseen attackers retain unaware-target bonuses

Original attack-portion generation checks whether the target sees the attacker;
hearing alone does not remove its unaware-target flag. The actual blade input
now produces38..46 rather than28..46 damage bounds and107 rather than59
critical probability using the existing native formulas. Unheard/visible/null
controls stay as before. A gun control changes the awareness flag with no
numerical damage effect for its current perks. Other weapon/perk combinations
are not newly claimed verified, and existing probability values above100 are
retained. Preview/to-hit awareness is a separate unchanged rule. Tests arrange
actual stored contacts, not natural detection or complete command execution.
Melee called-shot metadata/consumer foundation remains pending under360/236.

### `65558b11`: no extra random severity boost during critical selection

Original critical selection uses supplied severity; current code added0..99
and consumed an extra shared random draw. That addition is removed. This also
restores the shared sequence for later actions. Native critical-disabled
head/body damage stays3/2 HP while RNG now matches zero-critical controls;
zero-energy impacts still do0 damage and consume no draw. Actual injury types
and broader critical damage/cap/resistance/perk formulas remain separate
pending audits. The surrounding attacker-level prefix is retained here.
No new probability clamp, severity limit or formula beyond this extra-draw
removal is introduced.

### `65558b11`: original critical damage by severity and body region

Restores severity times the original float reciprocal300, followed by headx2,
bodyx1.5, handsx0.5 or legsx1. Existing normal location damage is still added.
Controlled actual severity300/base2 hits now lose5 HP at body (was14),7 at
head (13),2 at either hand (7),3 at either leg (10). Severity301/base20
fractional controls lose50/70 at body/head (149/137). These examples use a
native bleeding-only selection and neutral cap1000, not all critical types.
A zero-probability body hit stays2 and zero-energy impact0. Lower excess damage
can also avoid the existing wound/AP penalties; no AP formula changes.
The byte audit follows wide multiply then float store, rather than silently
changing original operation order. Applied injury severity, cap placement,
resistance/perks and forced/difficulty fallback are still pending separate
comparisons. No new clamp or HP/critical chance limit is introduced.

### `65558b11`: injury difficulty is separate from location damage

Actual injury severity now keeps the selected difficulty. A supplied300 became
642 for body/head,428 for hands,535 for legs; all now stay300. Supplied301
body/head became645 and now stay301. These are actual bleeding injuries read
through their normal difficulty interface. The existing treatment rules will
receive those corrected values; no healing, bleeding-rate or duration formula
changes. Actual HP damage stays5/7/2/2/3/3 and fractional50/70 in the controlled
examples from the preceding factor fix. Original intermediate float store and
truncation retained; no new severity limit/clamp. Cap location, resistance/
perks and forced/difficulty fallback remain separate pending work.

### `65558b11`: maximum critical severity constrains selection and damage

The existing maximum-severity setting now applies before selecting the injury
and calculating critical damage. It previously changed only the final stored
difficulty after a stronger injury and excessive damage had already been chosen.
At cap50/input300/base20, actual controlled damage changes body50->25,
head70->30, hand24->15, leg37->20. Native bleeding grades/values now match an
explicit severity50 reference; head has no eligible bleeding-only injury there.
Cap equality/below-cap/zero probability/low input stay unchanged. No new cap,
setter default, script-direct critical rule, injury field or RNG draw added.

Tests verify the actual cached game through a native zero-energy server entry
and existing tag45; previous neutral world-cap factor/severity tests did not
establish that binding and are not cap-order evidence. Main damage remains a
direct native RPG call, not a full UI-command claim. Other enabled injury types,
resistance/perks and forced/difficulty fallback remain separate pending work.

## 2026-10-31: PK resistance in the human critical modifier

Restores the original complementary factor `1 - resistance`. An authored suit
with resistance.7 now contributes.3 to this helper instead of.7. A native mounted
pilot/bypassPK test with base damage20 and severity300 changes human HP damage
body41->29, head58->42, hand21->17 and leg31->23. Injury severity remains300,
shellHP700 and mount/AP/pose/inventory unchanged. Naked50 and zero-critical20
controls are stable. No authored values or earlier penetration gate changed.

Gold/Steam exact typed-field/complement and final-multiply bytes agree. Actual
RED4/6 ->GREEN6/6 each; full Game builds, related101Win/101Linux, CI323. Probability10000
is an explicit isolation input; ordinary critical frequency, non-bypassed shell
penetration and full UI commands are not newly verified. Perks46/47 and
forced/difficulty fallback remain pending within source65558b11.

## 2026-10-31: critical resistance ability46

Restores the original acquired ability46 multiplier `1 - parameter` on the
existing critical factor. The actual authored.1 parameter gives.9 for a human,
or.27 combined with a suit's existing.3 complementary factor. Native base20 /
severity300 damage changes body50->47, head70->66, hand24->23, leg37->35 and
mounted body29->28. Injury severity300, shellHP700, mount/AP/pose/points/inventory
remain stable. Unacquired and zero-critical controls are unchanged.

Gold/Steam HasPerk46/parameter and wide x87 subtract/multiply/final float-store
instructions agree. Actual RED5/8 ->GREEN8/8 each, full Game builds, related
109Win/109Linux, CI323. Probability10000 isolates damage factor; ordinary frequency,
full UI commands and non-bypassed penetration remain unverified by these cases.
Perk47 and forced/difficulty fallback remain pending within source65558b11.

## 2026-10-31: reduced critical severity ability47

Restores original division/truncation before cap, injury selection and damage.
Actual authored1.25 turns severity301 into240. Native body/head/hand/leg damage
50/70/24/37 becomes44/62/22/33; injury DC301 becomes240 and body bleeding value
15 becomes10. At cap270, damage47 becomes44/DC270->240. Bindingcap150 and
unacquired/zero-critical controls remain stable. No authored balance values change.

Gold/Steam exact HasPerk47, wide division and truncation-before-cap agree. Native
RED5/8 ->GREEN8/8 each, full Game builds, related117Win/117Linux, CI323. Four RNG words
and HP/AP/pose/points/input controls match actual explicit-reduced native reference.
Probability10000 isolates this calculation; ordinary frequency/fullUI and
zero/nonfinite divisor or overflow remain unverified. Forced/difficulty fallback
within source65558b11 is still pending.

## 2026-10-31: Steam v1.2 forced human critical flag

Existing bAlwaysHumanCritical now bypasses the probability rejection for nonzero
chance, as target Steam v1.2 does. Chance0 retains its original early return.
Resistance, severity cap, injury selection and random draw are retained; existing
false default/tag14/save/network shape do not change. Gold1.1's differing trueflag
zero-return is recorded and deliberately not copied over the target1.2 behavior.

Native chance1 forced damage body20->50, head30->70, hand14->24, leg17->37 and
injury absent->BLEED/DC300, matching guaranteed native reference. Forced-1 boundary
body20->50; unforced1/forced0/unforced-1 controls stay20/no injury. Four shared RNG
words and AP/pose/life/input flags/cap remain stable. RED5/8 ->GREEN8/8 each, full
Game builds, related125Win/125Linux, CI323. Direct native portions are tested, not full
command/UI producers; world-difficulty fallback remains pending in source65558b11.

## 2026-10-31: failed-roll fallback from actual difficulty AlwaysCritical

Restores target1.2 fallback only after failed nonzero probability, unless forced,
when actual difficulty enables it and resistance is at least1. A valid attacker
REPLACES severity with level*d5 after the normal cap; null attacker retains capped
input and validlevel0 resets0. Existing level*d20 prefix/RNG remains. Native level20
input300 becomes527 by d20, then47 by d5: HP27->33/BLEED DC47. Original cap5 still
allows fallback47; no invented recapping. Nullattacker20->50/DC300, validlevel0
HP27 unchanged but BLEED/DC0. Negative-nonzero boundary follows the same loop.

Actual authored world damage coefficient stays in effect (raw20->base27 for native
attacker). Difficultyoff/resistance46/passed/zero/forcedresisted/nullworld controls
are stable. RED5/11 ->GREEN11/11 each; full Game builds, related136Win/136Linux, CI323,
four shared RNG words and native HP/AP/pose/level/input/cap/difficulty validated.
Legacy null-world calls remain compatible; direct RPG, not full UI command claim.
Pending separate native comparison boundary: chance100 * factor.899999976 must
compare at original wide precision (89.9999976 rather than rounded90). Original
instruction audit identifies it; no native RED/GREEN or fix is claimed yet.

## 2026-10-31: critical chance threshold precision; source65558b11 complete

**Superseded precision inference:** the wide-significand claim and boundary
rejection below are incorrect under the observed original PC24/RN runtime.
See the later original-address critical comparison audit and correction.
Historical measurements are retained; they do not establish original behavior.

Restores original unrounded comparison of integer roll to probability times the
stored resistance factor. With actual46/.1, chance100 has threshold89.9999976,
so draw90 rejects; chance50/draw45 and chance30/draw27 reject likewise. Premature
native FLOAT rounding previously admitted those rolls. Native HP47 becomes20
and the wrong BLEED/DC300 disappears, including AlwaysCritical+resisted control.
Passing89, rejecting91, unacquired, zero-chance and forced controls are unchanged.
No authored values, new clamp or random draws are introduced.

Gold/Steam exact FILD/FMULS/FCOMPP sequences have no intermediate product store.
Native RED4/9 ->GREEN9/9 each; full Game builds, related145Win/145Linux, CI323; four RNG
words and HP/AP/pose/perks/input/cap/difficulty validated. Ordinary authored
boundaries are certified; nonfinite/tiny custom factors, extreme probabilities
and full UI producer flows remain outside this check.

This closes useful functional source65558b11 across the preceding severity RNG,
damage factor, applied severity, cap placement, PK/perks46/47, forced flag and
difficulty fallback fixes. Original post-cap fallback, null/level0 and chance0
quirks remain. All limits/unverified scenarios above remain in force; other
upstream sources and actual final-head hosted CI are still pending.

## 2026-10-31: preserve existing fields at empty mod overlay markers

Restores original target handling of integer/float bitsFDFDFDFD and exact UTF16
`fdfdfdfd`: leave the previous value untouched and report no imported value.
Previously a mod's empty cell could replace AP20 with -33686019, turn a false
flag true, corrupt a damage coefficient, or replace Cyrillic code/localized text
with marker text. All five now retain the existing actual native value. Ordinary
int0/booltrue/float0 and real narrow/wide text overrides continue to apply.

Native actual DB overlay RED5/10 ->GREEN10/10 each; pointer/count/CP1251/UTF16
controls remain exact. Original named storage getter/vtable, sentinel literal/
initializer and exact wcsncmp comparison verified. Full Game builds, related
156Win/155Linux, Windows manager regression and CI323 pass. Own overlay semantics,
codecs, missing-field diagnostics, database files and ADO path preserved.
These are actual data-loader checks, not full UI/script execution claims. The
derived CanSave caller's false-return guard remains separate pending work.

## 2026-10-31: retain save-policy mode when mod field is skipped

The native difficulty importer now respects ImportField's false result before
converting CanSave. A skipped marker preserves actual always-mode2 instead of
replacing it with invalid3. Real chapter/realtime overlays still change2->0/1;
no new save-policy mode, default or authored value is introduced.

Native DB overlay RED1/3 ->GREEN3/3 each, identity/count/CP1251 stable. Exact
original target success branch/member24 and enum/name table verified. Full Game
builds, related159Win/158Linux, Windows manager regression, CI323. These checks certify
native imported policy data; save-menu/UI execution and separate missing-column/
unknown-name cases are not newly certified. Other source360 portions remain.

## 2026-10-31: restore melee bonuses for the requested body part

A melee command now carries its requested part into actual damage calculation.
With HeadshotShouldKill enabled, a ray that hits that part receives original1.2
probability/severity bonuses: body10/20, head90/150, hands35/65, legs25/40. These
are added after existing level*d20 and before resistance/perk47/cap/selection.
Flag-off and HL_ANY attacks retain their previous calculation. Authored values,
defaults and attack enum ordinals are unchanged; own LAN carries optional
transient metadata, ordinary saves retain original tags and clear it on read.

Actual native action/trace/damage and native wire RED9/15 ->GREEN15/15 each;
six controls, all four region groups including low-chance rejection->acceptance.
Head damage122->167/DC184->334; body86->90, hand45->50, leg65->71. Independent
original producer/constructor/flag/match/table/save audit, full Game builds,
related174Win/173Linux and CI323. Checks certify real HP/injuries/actor controls and
native serialization, not complete peer/UI/resume or post-world RNG parity.
Ranged/throwing producers and other360 portions remain separate work. Source
23630bfd is complete together with its earlier visibility-awareness fix.

## 2026-10-31: restore requested-part bonuses for firearm bullets

The native unit-shot ray selector now retains the requested body part on its
generated damage portion before cover/ray construction. Actual bullets can
therefore receive the original1.2 conditional bonuses already restored for
melee: only when the rule is enabled and actual hit matches the requested part.
No authored values, ammo/AP costs, ray preparation or firing timing change.

Actual native command/carrier/trail/bullet-impact RED4/6 ->GREEN6/6 each, two
unchanged controls. Head/body/hand/leg damage67/83/44/63 ->163/87/49/69;
level20*d20 prefix171 and actual ammo8->7 retained. Original named selector,
typed/saved field identity and exact load/store/solver handoff verified.
Full Game builds, related180Win/179Linux, CI323. Tests bound the real trail to one
flesh entry; full firing timing/travel/peer/UI and post-world RNG parity are
not newly certified. Throwing knives and other source360 pieces remain.

## 2026-10-31: retain unaware-target accuracy when an unseen attacker is heard

The native unit to-hit helper now checks target visibility alone before passing
the unaware-target flag. Actual audible/unseen melee chance33->50 and firearm
34->41 now match the original calculator context. This is distinct from the
earlier awareness fix for damage/critical portion generation. Current chance
formulas, cover origin/distance, AP/ammo, defaults and own night API are retained.

Native actual world/cover/to-hit RED2/15 ->GREEN15/15 each,13 controls including
visibility/hearing combinations, null target and throwing knives. Four shared
RNG words and native state unchanged. Original visible-only slot/test/inversion/
native-calculator handoff verified. Full Game builds, related195Win/194Linux, CI323.
Stored awareness was arranged through actual public APIs; natural LOS/full cursor/
command/UI and missing night propagation remain outside these checks.

## 2026-10-31: apply existing night penalty to unit-target accuracy

The native unit to-hit helper now passes its owning mission's IsNight into
the existing calculator. Ordinary night firearm31->28 and throwing knife3->2
now follow that same native calculator; day, melee, actual night-vision21,
script chance37 and null targets remain unchanged. No light/range/velocity
formula, authored values, AP/ammo cost or explicit direct/AI API change.

Native actual world/cover/night RED2/12 ->GREEN12/12 each,10 controls, four RNG
words/state stable. Original typed bool inputs, named owner getters and actual
selector/solver/context/calculator chain verified. Full Game builds, related
233Win/232Linux including earlier night/incidental coverage and CI323. Knife range
remains the current fixed implementation, so full knife accuracy/range/flight
parity is not claimed; full world lighting/LOS/cursor/firing/UI also remains
outside these checks. Other source578 portions remain.

## 2026-10-31: apply owning difficulty to flying-knife impact

Actual knife collision now passes its owning world to the existing native
throwing-hit handler. A base2 attack with difficulty coefficient2 receives4
instead of2; anonymous and coefficient1 controls remain2. No authored values,
range/velocity, saved carrier shape or launch command change. Existing stored
trajectory direction remains, with current-direction reconstruction pending.

Native actual flight/clock/physical body collision RED2/4 ->GREEN4/4 each,
authored model/item, one hit/retirement and actor controls. Original named
knife Segment, complete free147-byte helper and pWorld20 handoff verified.
Full Game builds, related277Win/276Linux, CI323. Tests do not certify OnLabel producer,
full flight/direction/range/velocity/drop behavior, enemy RPG flag classification
or full UI/resume. Those source578 parts remain separate.

## 2026-10-31: current direction for thrown-knife impact effects

Actual knife collision uses its current physical direction, also populating the
retained native attack trajectory reader. Stale upward/sideways launch data no
longer suppresses ground blood after a downward head impact. Damage is unchanged:
15 for base10 and3 for base2 in these controlled cases. Correct downward high/
low damage controls retain one/zero ground decal. No authored values, new formula,
range/velocity or save/LAN field change. An old preview test reference was also
updated to supply actual night and visibility-only awareness from fixes82/83;
the initial related-suite failure is archived and the full suite repeated.

Original Segment current delta/normalization/call verified, complete86-byte
normalizer matched. Native actual physical collision and real world decal
RED2/4 ->GREEN4/4 each. Full Game builds, related281Win/280Linux, CI323. This resolves
the previous direction limitation; full throw command/flight/range/velocity and
full renderer/UI/post-world RNG parity remain outside this check.

## 2026-10-31: intended target survives the knife throw label

Actual knife launch now passes the selected unit to existing attack creation.
Intended hits no longer become incidental hits for the existing IC damage rule;
unseen targets also receive the already established visibility-only backstab
input. No authored numbers or damage/critical/IC formula changed. The earlier
validator correction did not cover this actual launch producer.

Native Start/OnLabel/projectile carrier and native BODY damage RED3/5 ->GREEN5/5
each. Controlled IC100 intended damage1->37; IC0 intended remains37. Unseen but
audible native bounds18..29->24..29, critical52->100. Free-point IC100/IC0 stay1/
37. Cap1, disabled injuries and difficulty flag controls isolate target/damage;
these observations do not certify ordinary critical frequency. AP51->42->42,
native item/actor controls retained. Full Game builds, related286Win/285Linux, CI323.
Original Gold/Steam both forward selected member28. Actual requested part,
range/velocity/full solver/flight/timing/drop/UI remain separate source578 work.

## 2026-10-31: requested part survives actual knife launch

The actual throw label now retains selected eHL in the existing attack metadata.
The already restored target1.2 critical consumer applies its existing bonuses
when HeadshotShouldKill is enabled and actual hit matches: BODY probability+10/
severity+20, HEAD+90/+150 (other regions already certified in the shared consumer).
No new numbers or formulas. ANY, free-point, disabled rule and mismatched actual
part do not gain a bonus. Existing own save/LAN encoding is retained.

Native actual launch carrier/real HEAD-BODY callback/HP/bleeding RED2/6 ->GREEN6/6
each: head90->123/DC185->335, body63->66/DC185->205. Four outcome controls remain
unchanged. Native level20 prefix175, raw chance100/severity10, actual perk37
grant and AP57->48->48/cost9 retained. This deliberately guarantees critical
acceptance; ordinary chance frequency is not inferred. Initial disabled-injury
fixture was unobservable and replaced before accepted RED. Full Game builds,
related292Win/291Linux, CI323. No new full flight/range/velocity/timing/drop/UI, bleeding
duration or post-world RNG parity claim.

## 2026-10-31: original integer conversion for unit-knife accuracy

Unit-target knife chance now returns int(fToHit) after the existing2..100 clamp,
matching original toward-zero conversion. Previously+0.5f could add one
percentage point and accept a boundary roll that should miss. No authored
values, preceding formula, clamp, range/velocity or RNG rule changed.

Actual player preview/unit helper/native CheckToHit RED2/6 ->GREEN6/6 each.
Real day2.73483658/night2.91429806 change3%->2%; native roll3 changes hit ratio1
to2/3 (miss). Four lower-fraction/day-night/minimum2/script37 controls unchanged.
Queries use0 game RNG and Check exactly1, subsequent4 words verified. Both
originals share an identical complete39-byte truncate/save/restore tail; both
41-byte float clamp selectors also match exactly. Full Game builds,
related298Win/297Linux, CI323. Current fixed range and old throw-command velocity/
solver/flight remain separate; no whole accuracy-formula/flight/UI parity claim.

## 2026-10-31: original physical range for knife admission

Range now uses original blade skill/VP/strength/weight/gravity and perk88 velocity
multiplier instead of STR*1.5; the unused constant15 marker was removed. Actual
blade remains available to old post-tear-off calcers. No authored numbers changed.
Raw skill+perk89 (or melee70), multiplied by VP factor, contributes1/18 to STR;
actual worn PK uses its grenade strength. Power=(base+STR*coefficient)*grams^
double(float.74); velocity=power/(grams*.001f), then perk88. Range=(v/gravity)*v*
1.6f, truncated to native admission tiles. Original intermediate float stores
and wide denominator/final range preserved. VP factor uses unmodified XP+base
maximum; an altered current cap does not falsely remove the health penalty.

Native getter/real admission FINAL RED7/10 ->GREEN10/10 each. THROW0->100:11->19
tiles; perk88:15->18; perk89:32->33; healing125->500:15->19; STR10->20:15->32.
Actual PK pilotSTR12->22 stays14; currentVP300 with cap reduced500->300 and
theory500 stays18. Night/cover/actual launch15 controls remain unchanged. Real
label creates one knife, AP57->46/cost11, coherent legacy deadline; queries0RNG.
Full Game builds, related308Win/307Linux, CI323. Original complete285/48/49 and typed
blade/VP/item/PK contexts verified. Old speed5, fixed1..15 accuracy range and
cached solver/full flight still require separate work. No cross-CRT pow/extended80
bit identity or full continuous-distance/flight/UI parity claim.

## 2026-10-31: physical knife range now feeds accuracy

Knife accuracy uses its original physical range instead of fixed1..15. Minimum
is the truncated physical range; maximum is int(minimum*1.5f), so19->28 and
33->49. Skill, strength and authored perks88/89 can affect accuracy through
range as in the original. Existing quality0 and other chance arithmetic kept.

Real native cover/distance8 fixtures: ordinary29->69%; STR20/THROW50 2->75%;
actual perk89 2->76%; actual perk88 2->64%; THROW0 2->29%. Script37 stays37%.
At roll equal to corrected chance, affected cases become hits. RED5/6 ->GREEN6/6
each, query0RNG/check1draw/AP/HP/item/pose/grant point controls. COMPLETE original
171 bytes and exact117 conversion helper verified; min truncation precedes1.5.
Full Game builds, related314Win/313Linux, CI323. Actual velocity/solver/full flight/drop
and shared source9f3 accuracy skill remain unfinished; no full flight/UI claim.

## 2026-10-31: actual knife speed uses original physical velocity

Real knife launch now uses original physical velocity*1.5 instead of fixed5m/s.
The existing blade/skill/VP/strength/weight/perk calculation feeds the actual
projectile. Perk88 product remains wide until after1.5; existing range calls
retain their float store. No authored numbers or damage/save/LAN fields changed.

Actual native labels seeded with real GameRandom120 and skill/perk/healing/STR/night/cover inputs deliver
16.61/16.21/21.85/16.61/21.48/14.74/14.74/14.74m/s versus5. Final RED7/8 ->GREEN8/8
each, one unchanged lifetime/AP/item condition control. One real transferred
blade, AP cost9 or11, native query RNG/HP/pose/grant controls. Original named
producer, actual velocity call,1.5f then float store and typed world handoff
verified; speed norm tolerance1e-5 relative. Full Game builds, related322Win/321Linux,
CI323. Original absolute lifetime distance, ranged solver/cache and complete
collision/flight/frame/drop/UI still require separate work; no exhaustive x87/CRT
or post-world RNG claim.

## 2026-10-31: knife admission rejects fractional excess range

The original compares fractional target distance to truncated maximum range.
Removed premature target-distance truncation from actual CanUnitThrowKnife:
at15-tile range,15.125/15.875/15.0009766 are denied;14.875 and15 remain allowed,
16.125 remains denied. Distance components/sqrt/grid scaling keep double width
until original distancefloat store. Own command errors and data unchanged.

Initial native RED3/6 ->GREEN6/6 each. Correct two earlier perk89 test points
from33.125 (outside range33) to32.5 (between original before32/after33). Restore
ONLY92 production and repeat FINAL RED5/8 ->GREEN8/8 each including those two
real native consumers. At32.5, gate0->0 becomes11->0; original range32/33 kept.
Three identical controls; query0RNG/AP/HP/pose/
item/perk controls. COMPLETE original175 and direct23-byte current-position
helper verified, only decoded calls/verified grid pointer masked in175. Full
Game builds, related328Win/327Linux, CI323. No exhaustive extended80 identity or full
solver/absolute lifetime/collision-flight/drop/UI claim.

## 2026-10-31: original knife flight distance and lifetime

Actual point/unit launches pass the original integer physical range directly
to the flight timer. Removed an extra*.625 grid conversion that shortened the
distance represented by the deadline. At range15 the timer now spans about15
world distance units instead9.375; selected-unit fixtures range18 now18 instead
11.25. Original constructor uses distance/speed*1000, stores float duration,
rounds with current FPU mode and adds actual throw time. Speed/AP/item flow kept.

Actual labels FINAL RED14/14 ->GREEN14/14 each: nine point/range cases, three
selected-unit and two freepoint cases. Native saved clock/velocity/item/carrier,
point speed literals and original target damage controls; tolerance1.1ms*speed.
Named original producer/world/factory/ctor and complete44-byte clock block
verified; real factory allocationF8/FC version difference not masked. Full Game
builds, related328Win/327Linux, CI323. Existing norm/float clock adaptation retained;
no exhaustive clock-bit identity or full solver/collision-flight/drop/UI claim.

## 2026-10-31: knife wound penalty keeps original maximum health

Knife accuracy now measures remaining health against original base+XP maximum,
even if an injury lowered the current cap. Current300/cap300/original500 remains
60% health rather than falsely100%; raw throwing100 receives*.9 ->skill90.
Native fixture chance68->67%; at roll68 this changes hit->miss. Ordinary wounded
cap500, healing, healthy, actual perk89 and script37 controls unchanged. The perk
continues to act through physical range; no new direct flat accuracy bonus.

FINAL native RED1/6 ->GREEN6/6 each; query0/check1draw/AP/HP/item/pose/grant controls.
Exact original104 and actual main-thread original104/182 kernel prove90 under
observed runtime PC24/RN (CW007f). Forced diagnosticPC53 alone yields89, so the
extra-wide89 hypothesis was rejected, and native float product retained. Owned
original test exited normally; full thread context and temporary cfg restored.
Full Game builds, related334Win/333Linux, CI323. No authored/save/LAN changes.

Precision audit remains open for earlier imported formulas: x87 registers alone
do not imply wide arithmetic when PC24 is active, and RC placement also matters.
No broad numerical parity claim until those native original PC/RC cases are
checked. Existing physical adapters unchanged here; no full shared9f3/solver/
flight/drop/UI claim.

## 2026-10-31: correct critical chance rounding to observed original runtime

Corrects earlier wide-precision adaptation: actual original uses single
significand precision at this comparison under observed CW007f/PC24/RN.
With real resistance46/.1, probabilities100/50/30 round to90/45/27 and equal
roll90/45/27 admits critical. Fresh native boundary cases damage20->47 and
BLEED/DC300 restored, including resisted AlwaysCritical. Five controls unchanged;
four RNG words and actor/perk/input/cap/difficulty controls preserved. No new
authored values, clamps or random draws. Exact RC chronology +actual original
comparison/branches at original addresses prove this kernel behavior; isolated
chooser stops before original injury selection. Context/config restored, normal
owned-process exit. Main-menu/kernel scope, no live-gameplay breakpoint claim.
RED4/9 ->GREEN9/9 each, full Game builds related334Win/333Linux, CI323. Prior precision
claim above explicitly superseded; other precision adapters audited separately.
Source: [65558b11623917d5634c55a1b718fa2cb588ef66](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/65558b11623917d5634c55a1b718fa2cb588ef66).

## 2026-10-31: correct late incidental damage to observed original precision

Earlier descriptions of IC100 leaving1 damage and IC20/base40 leaving33 are
superseded. Actual original multiplies in observed PC24/RN BEFORE switching to
integer truncation: IC100/base40 leaves0; IC20/base40 leaves32. For base100,
effectiveIC22 leaves78 and IC2 leaves98. Signed custom aura IC-10/base40 leaves
44; existing final max0 clamp remains. Click/intended/PK/failed-roll gates,
shared RNG, aura reset and original late receiver order are retained.

Fresh native RED13/56 ->GREEN56/56 each with43 controls (39 matching core
measurements and four unchanged actual knife labels), actual HP/hit/
receiver/perks/aura and4RNGwords. Actual original69-byte block+callback dispatch
at original addresses and30 CW/input records confirm arithmetic; isolated final
getter stops before original health application. Complete owned probe normal
exit/config/context restore; incomplete attempts excluded. Main-menu/kernel
scope, no full gameplay breakpoint claim. Full Game builds related396Win/395Linux,
CI323. Derived AP follows resulting HP. This corrects original parity; no new
authored bonus, guard or random draw. Earlier source86 free-point damage1 is a
historical consequence of old shared receiver and is superseded by this fix.
Source: [037c41d91ec01f29848f346787aa7a8a1d471c43](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/037c41d91ec01f29848f346787aa7a8a1d471c43).

## 2026-10-31: explosive product precision before integer truncation

Original observed PC24/RN rounds the float product BEFORE truncating to integer.
Thus10/30 damage*.7 gives7/21 rather than6/20;*.9 gives9/27 rather than8/26.
Signed -30/-10*.7 gives-21/-7. The final integer conversion still truncates:
9/11*1.5 remains13/16. Existing33-million half-product API tests now match
original float precision; earlier wide/large-half explanations are superseded.
Structure/critical flags, other fields, authored data and random calls unchanged.

Native RED5/18 ->GREEN18/18 both,13 identical controls. COMPLETE original87,
actual-address36 input/CW records and six runtimeCW007f samples; owned guarded
probe restores context/cfg and exits normally with0, incomplete attempts excluded.
Full Game builds, related27Win/26Linux, CI323. Native modifier/known grenade consumer
scope; no full explosion propagation/authored-perk ownership/gameplay breakpoint
claim. Extreme arbitrary integer/float, nonfinite and overflow remain uncertified.
Source: [aa031e2e636629f83c4b86eb5b73dc9b4fa76918](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa031e2e636629f83c4b86eb5b73dc9b4fa76918).

## 2026-10-31: detection baseline uses original float-operation precision

Corrects earlier31 wide intermediate assumption. Original PC24/RN rounds the
distance product and subtraction separately before integer truncation. Equal
spot/stealth at distance15 gives23 instead of22, and at30 gives1 instead of0;
spot70/stealth30 at15 gives63 instead of62. Actual25 fixture changes18->19.
Stored coefficient44/30/intercept45, ability ownership/scalars, ignored night
argument and0..100 clamp stay unchanged. No new authored bonus or query RNG draw.

Native RED4/36 ->GREEN36/36 each with32 identical controls. COMPLETE original317,
actual-address72 operand/CW records; separate24 actual mission RPG getter/AP/
weapon entries confirmPC24/RN on the primary thread. That live run does not
capture perception itself. Owned probes restore config/context and exit normally
with0. Full Game builds, related71Win/71Linux, CI323. Direct probability API/finite
native input scope; full visibility/AI frequency and other caller modes unclaimed.
Earlier wide/22/0 baseline explanations are explicitly superseded.
Source: [037c41d91ec01f29848f346787aa7a8a1d471c43](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/037c41d91ec01f29848f346787aa7a8a1d471c43).

## 2026-10-31: fractional-cap skill training core

Restores record63 per-skill training rates and fractional XP capacity. Used
capacity includes both accumulated skill points and fractional progress. The
next increment is remainingCapacity*learningFactor/rate, clamped to .00001..1/10;
the existing dynamic-cell upgrade guard still stops exhausted skills. LEVEL
cannot train. The obsolete unit PK/add-value offset is absent from this original
formula. Existing live unit entry currently delegates with factor1; associated
attribute practice and learning-perk multipliers remain separate pending work.
Authored records and total XP awards are unchanged.

At totalXP.01, melee cap100/rate20 gains .000104997285 instead of zero; burst
cap50/rate60 .0000174995475, medicine cap25/rate10 .0000524986426. At XP10,
melee XPpart1/progress.25 advances to .283210069 instead of stopping. LEVEL's
incorrect .1 progress is removed. These are direct training-core examples,
not a claim of whole-mission experience balance parity.

Native RED14/24 ->GREEN24 each,10 identical controls; fixed progress tolerance
1e-7. Complete original Steam151 plus cap helper75/actual initialized DB rates/
Upgrade,48 operand/CW records; actual mainCW007f. Gold210 is a real different
version and not forcibly equated; Steam adds LEVEL guard/extracts cap helper.
Owned contexts/config restore and normal exit0. Full Game builds,
related112Win/112Linux, CI323. Source remains PARTIAL for attributes/perks/caller cleanup.
Source: [2893fa8470599905a5de221be3d8057a52c6d479](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2893fa8470599905a5de221be3d8057a52c6d479).

## 2026-10-31: training progress and cap truncate to whole points

Corrects an inherited reconstruction error: a skill point now requires a full
accumulated point of progress. Progress.5/.75 no longer grants an early point.
For direct increment2.5 onto.25, the result is two points plus.75 remainder,
instead of three points and-.25. Fractional XP capacity also truncates: cap1.75
permits at most one XP-earned point. Existing eligibility checks and original
return behavior remain, including true after a positive increment even if the
cap clamp removes it. Authored rates/XP awards/base curve are unchanged. Saves
retain their stored progress; no retroactive migration of earlier negative
remainders is introduced. Other methods' rounding remains separate.

Native RED8/40 ->GREEN40 both with32 identical controls. COMPLETE142 original
method exactly identical Gold/Steam, zero masks;32 actual original operand/CW
records prove both casts use truncation and all raw results match literal tests.
Hurt/frozen controls and four RNG continuation words pass. Owned context/config
restore, normalexit0. Full Game builds, related128Win/128Linux, CI323. Direct-cell/base
training scope; associated attributes/learning perks still pending in source2893.
Reconstruction origin: [a8c50bf29e70191077257cca1a7fbd4cae01b731](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a8c50bf29e70191077257cca1a7fbd4cae01b731).
Source: [2893fa8470599905a5de221be3d8057a52c6d479](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2893fa8470599905a5de221be3d8057a52c6d479).

## 2026-10-31: associated primary-attribute practice

Melee/throwing/burst practice also trains Strength; shooting/sniping/stealth trains
Dexterity; spotting/medicine/engineering trains Intelligence. The associated attribute
updates first, then the selected skill always practices. Either whole-point improvement
reports success, including attribute-only growth. Passive/direct attribute skills have no
extra association. Base rates, corrected whole-progress threshold and authored records are
unchanged. Learning perks11/60 are the next separate part.

Native RED: 14 failures among 250 Windows / 249 Linux tests; GREEN: 318 Windows / 317
Linux tests passed,62 identical measured controls; all17 selected skills and three
integer-growth boundaries. Actual original complete unit code156+table36/
core/DB/derived-stat updates,44 operand/CW records/all17 cells; exact integer/
current/cap/bool, progress1e-7. Low-XP associated progress previously0 now.00001.
Controlled XP1m/progress.999999 with authored STRcap1/rate100 crosses original threshold1:
Strength's VP update follows original, success is returned and melee still practices. This
is an API boundary test, not new authored XP/balance data. TotalXP/perk points/RNG
unchanged. Owned context/config restore/normalexit0, full Game builds, related 318 Windows
/ 317 Linux, CI323. Direct-unit scope; perk/injured/full mission frequency parity
unclaimed. Source remains PARTIAL for learning perks. Source:
[2893fa8470599905a5de221be3d8057a52c6d479](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2893fa8470599905a5de221be3d8057a52c6d479).

## 2026-10-31: authored learning-perk factors

Taken perk 11 applies its authored learning factor to Strength/Dexterity/Intelligence;
perk 60 applies it to secondary skills, including Vitality. The captured authored
parameter is 1.1 (110%). A secondary skill's associated attribute and the selected skill
each use their own category's perk. Absent/unowned perks use 1.0; LEVEL remains
untrainable. The factor changes the incremental practice amount before the existing rate
division and minimum/maximum clamps. It adds no total XP or perk points and changes no
authored database values.

Native RED: 4 among 266 Windows / 265 Linux; full GREEN 334 Windows / 333 Linux, 88
identical measured controls. 16 actual grants/category cases/all 17 cells; actual original
real-tree/full-unit 32 records, progress 1e-7 and exact integer/cap/current/freeze/bool.
Original points/ownership and actual destructor cleanup verified; native RNG unchanged.
Original stack/context/config restored and normal exit 0. Full Game builds and CI 323.
Fractional-progress scope; combined perks, injured promotions and full mission frequency
are not claimed by this matrix. Source 2893 is now complete after its separately recorded
rate/Upgrade/association fixes; other remaining commits and whole-task final-head hosted
CI are still pending.

Source: [2893fa8470599905a5de221be3d8057a52c6d479](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2893fa8470599905a5de221be3d8057a52c6d479).

## 2026-10-31: nearest-even rounding of modified skill caps

Modified skill caps now round exact halves to the nearest even integer in the observed
game environment, matching original Update: `5 * 0.5` becomes 2 rather than 3; `30 - 7.5`
becomes 22 rather than 23; a controlled -2.5 cap becomes -2 rather than -3. Damage to the
raw current value is preserved; frozen skills still expose the cap. XP parts, stored
practice progress and authored modifiers/rates/data do not change. Upgrade still truncates
whole progress/caps; difficulty scaling uses its existing helper pending a separate
original caller audit.

Native RED: 8 among 282 Windows / 281 Linux; full GREEN 547 Windows / 546 Linux, 100
identical measured controls. 16 real-cell/modifier fixtures and 32 complete
actual-original 178 input/CW records, with exact cap/raw/base/XP/progress/freeze and
unchanged native RNG. Effective original view is derived from the captured fields, without
executing its getter. Original stack/context/config restored and normal exit 0. Full Game
builds, wounds/perception/to-hit/AP/grenade consumers and CI 323 pass. Exact finite binary
boundary scope; altered FPU mode/exception flags, nonfinite/overflow, all non-binary FMA
and dead-modifier/World difficulty behavior are unclaimed here. Remaining source count 57
and whole-task final-head hosted CI are still pending.

Source: [a8c50bf29e70191077257cca1a7fbd4cae01b731](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a8c50bf29e70191077257cca1a7fbd4cae01b731).

## 2026-10-31: medical and engineering action practice

Starting first aid now practices Medicine; trapping/disarming objects, setting/disarming
mines, picking locks and repairing Panzerkleins practice Engineering. Each also practices
Intelligence through the already-restored original training rules. Fractional progress
accumulates normally; a whole-point improvement emits one existing skill notification.
This restores original action-to-skill dispatch, without changing rates, caps, perk
factors, XP rewards, tool charges or AP costs. Source156's mounted-weapon reload selection
was already restored and is rechecked here.

Native RED14/91 Windows/Linux ->full GREEN 374 Windows / 375 Linux;76 identical measured
controls and all476 new initial/reference cell rows preserved.28 actual-original
fractional dispatch records at2 floating-point precision modes match all17 cells; original
true acknowledgment/log path and command completion frequency are outside that sparse
runtime matrix. Native whole-point tests check existing real acknowledgment behavior
against the previously original-audited Unit growth rules. Full Game builds,
treatment/tools/reload regressions and CI323 pass. Source156 is complete; ordinary
pending/partial sources56 and whole-task final-head hosted CI remain.

The NPC difficulty exploratory test did not establish a fix: original World scales before
SetXPLevel too, and its level routine recomputes caps. No NPC ordering/coefficient/balance
change was made. Earlier difficulty ROUND comments are unsupported by the newly observed
explicit CHOP conversion, kept as a separate unresolved helper audit.

Source: [156ccd173ff6c9671904e6d94e709397fc701d66](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/156ccd173ff6c9671904e6d94e709397fc701d66).

## 2026-10-31: reconcile deferred original-balance entries

The older excluded-balance bucket mixed one deliberate original-balance change with four
unverified deferrals. User scope is ALL useful original-balance fixes, so an unverified
original claim is not a completed exclusion. This is a source-ledger correction with no
engine/test/data change and no fabricated new RED.

Sourceeb45f18a is now imported: its entire patch changes BOTH CDynamicSkill.Upgrade
conversions from half-away rounding to truncation.
Commit3f491d04725045f3ecffa7896e7e8dacbdc101c6 already implements exactly those hunks with
native RED8/40both ->GREEN40/40,32 unchanged controls/related128both/CI323. Actual
namedGold6b5a70/142 maps toSteam6bbc90 uniquely EXACTZERO masks;32 complete
original-address native operand/CW records and explicit FLDCW CHOP before both conversions
prove original behavior. Current104 full gates include all16 Upgrade cases unchanged. The
old note that independent original verification was absent is superseded, not a claim of a
new code fix.

Sources2e15a102 (Hide->Stealth practice),24a00fd1 (additional short-burst AP) andaa724eca
(fatal-bleeding hostile-roster XP) return to pending until actual original
audits/nativeRED prove their behavior. The first dispatch is now observed in the original
action table but still needs real Hide entry/exit/idempotency tests. The short-burst
implementation is still absent in currentGetWeaponBurstAP. Fatal bleeding needs actual
reward recipient/timing/empty/single-payment verification. No engine change for any of
these3 is implied by reopening.

Source405ed861 remains excluded: its stated purpose floors paid costs at1 AP despite the
original, an intentional action-economy change incompatible with preserving original
balance. Own renderer/audio/fonts/faces/HD/LAN exclusions are unchanged.

After source156 completion the ordinary queue had56. Reopening3 earlier uncounted audit
deferrals makes the active unfinished total59 (pending43 +partial16). Imported188,
excluded-balance1. Earlier57 referred only to ordinary pending/partial rows and did not
include these held entries; the current total is an honest expanded audit queue, not3
newly introduced defects. Hosted actual final-head CI is still required after the entire
task.

Source: [eb45f18a9787d7994bcdb224b198818009786445](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/eb45f18a9787d7994bcdb224b198818009786445).

Source: [2e15a10299e0d5e4a90558e39b34cc150ce51382](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2e15a10299e0d5e4a90558e39b34cc150ce51382).

Source: [24a00fd151a39d6c696c2ee8ab6deeb9d862f875](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24a00fd151a39d6c696c2ee8ab6deeb9d862f875).

Source: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed).

Source: [405ed86144c0340c1ac36f0775d989123e7d8010](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/405ed86144c0340c1ac36f0775d989123e7d8010).

## 2026-10-31: Stealth practice on real Hide entry

Actually entering Hide now practices Stealth and its associated Dexterity, matching the
original action dispatch. Exiting Hide, requesting an unchanged state or a rejected
cooldown order does not train; repeating an explicit Hide request trains only on the first
entry. The existing command styles, AP pricing/charge order, visibility/cooldown checks
and save/network fields remain intact. A whole-point Stealth improvement emits one
existing skill notification. Learning rates, caps, perk factors and authored data do not
change.

Native actual-world RED5/43 Windows/Linux ->full GREEN 384 Windows / 385 Linux;33
identical measured controls,170 new initial/reference rows and476 prior
medical/engineering rows unchanged. Two wholepoint cases test selected Stealth; Dexterity
stays fractional here. Original completeRun123/DoAction160 control flow is independently
statically bound, and6 actual original pureRegister records at2CW check all17 cells.
Original pureRegister AP remains52; native full command spends14 and ends38, an explicitly
checked scope boundary. Original world/executor/trueACK frequency is not claimed
runtime-proved by the sparse kernel. Original stack/context/config restore/readback and
normalexit0; full Game builds and CI323 pass. Source2e15 complete,58 pending/partial
sources remain; whole-task hosted final-head CI still pending.

Source: [2e15a10299e0d5e4a90558e39b34cc150ce51382](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2e15a10299e0d5e4a90558e39b34cc150ce51382).

## 2026-10-31: Original additional-round AP and short-burst perk

The short burst perk now reduces the AP cost of additional rounds, matching original Steam
1.2. First divide maximum AP by the weapon rate using integer division. For a short burst
with the perk, subtract the authored 15% discount. Round the product and subtraction to
single precision before truncating the result. A price of 20 becomes 17, 3 becomes 2, and
1 stays 1. Other firing modes and units without the perk receive no discount. The minimum
additional-round cost is 1 AP; no weapon or a zero rate returns the unavailable-cost value
10000. Spent current AP does not replace maximum AP in this formula. This applies to
additional rounds only. Authored values, ammunition, equipment, XP, skills and gameplay
randomness remain unchanged.

Native tests reproduced 9 failures out of 54 on both Windows and Linux before the fix. All
555 Windows and 556 Linux related checks now pass, along with full Game builds and 323
asset-free checks. The 45 measured controls and 170 Hide / 476 medical reference cell rows
are unchanged. The original method was executed 28 times with real original weapon and
perk lifecycle calls. The observed original single-precision mode returns 17; a diagnostic
double-precision mode returns 16 and is not used as the reference. Inventory fallback was
inspected in original code and exercised through real native inventory; the original probe
does not claim to execute a complete mission, ammunition lifecycle or firing sequence. The
original process stack, context and configuration were restored and it exited normally.
The baseline database is unchanged. Source 24a is complete; 57 pending or partial sources
remain. Hosted CI for the final commit of the entire task is still pending.

Source: [24a00fd151a39d6c696c2ee8ab6deeb9d862f875](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24a00fd151a39d6c696c2ee8ab6deeb9d862f875).

## 2026-10-31: Original single-precision XP threshold calculation

XP thresholds now round the power-minus-one intermediate to single precision and multiply
by exactly 50, as original Steam 1.2 does. Dividing by the inexact float 0.02 could raise
a threshold by one representable float step. With authored cap 15, the threshold queried
for level 6 changes from 2243.253662109375 to the original 2243.25341796875. Class caps,
growth parameters and the existing strict level-promotion comparison remain unchanged. The
reward coefficient and recipient-distribution fixes are still pending separately.

Native tests reproduced one failure out of 54 on both systems before the fix. All 573
Windows / 574 Linux related checks, full Game builds and 323 asset-free checks now pass.
The 53 measured controls are unchanged. Twelve original executions of the complete 64-byte
function verify all six reference values. Observed original single precision is the
reference; diagnostic double precision is recorded separately. This pure math probe does
not claim to run original unit construction, live progression or a complete mission. The
original stack, context and configuration were restored and the process exited normally.
The database is unchanged. This independent correction was found during the aa724 audit
and does not complete that source: 57 pending or partial sources remain. Hosted CI on the
final commit of the entire task remains pending.

Audit context: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed).

## 2026-10-31: Original base XP reward coefficient

The base XP reward now matches original Steam 1.2. Subtract the victim's current XP
threshold from its next threshold, divide by the eligible party size clamped to 1..6, then
multiply by the original float coefficient 0.185185179. Replacing the old division by 8.1
restores about 50% more base XP. With authored cap 15 at level 0, one fighter's base share
changes from about 5.505972 to 8.258958; a share in a three-person party changes from
about 1.835324 to 2.752986. The recipient's existing intelligence and perk modifiers apply
separately and remain unchanged. Class caps, authored growth rates and the party-size
clamp are unchanged. Reward distribution is still being restored separately.

Exact native tests reproduced 12 failures out of 47 on both systems before the coefficient
change. GREEN includes 587 related plus 11 supplemental checks on Windows and 588 related
plus 11 supplemental checks on Linux: 598 / 599 unique passing cases, covering all 47 RED
names. The supplemental gate covers periodic-bleeding cases omitted from the initial
selection; no source changed between the gates. Full Game builds and 323 asset-free checks
also pass. The 24 measured query controls and 11 periodic scenarios are unchanged. Actual
original executions verify all 14 reference reward bit patterns and thresholds under the
observed single-precision mode. Diagnostic double precision is recorded separately. This
pure calculation probe does not claim to run original missions, constructors, deaths or
recipient distribution. Original stack, context and configuration were restored; the
process exited normally and the database is unchanged. This independent correction does
not complete source aa724: 57 pending or partial sources remain. Hosted CI on the final
commit of the entire task remains pending.

Audit context: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed).

## 2026-10-31: XP distribution after a fatal periodic pass

When the periodic regeneration/damage pass leaves a unit dead, each hostile party now
receives the original XP reward through its combat-capable members. Dead and unconscious
members receive no share; each hostile party is handled separately. Neutral or allied
parties, a surviving victim, an already-dead callback and a network replica receive no new
award. Repeating the callback after death does not pay twice. Existing party-size
clamping, recipient intelligence/perk modifiers and authoritative-world rules are
preserved. Periodic timing, damage and authored growth parameters do not change. The
earlier threshold and base-reward corrections supply the original amount.

Native World tests reproduced four failures out of 58 on both systems. All 609 Windows /
610 Linux full checks and 323 asset-free checks pass. Seven new scenario controls, 36
query controls and 11 earlier periodic scenarios are unchanged. The four repaired cases
pay exactly 3/2/3/4 eligible recipients, matching independent native AddXP references.
HP/death, inputs, expected results and four subsequent RNG words remain unchanged. Typed
original code verifies the periodic control flow and per-player/CanFight delegation. Its
actual reward function was executed separately 28 times; matching native cap/level/party
inputs produce the same reference bits. This does not claim execution of the original
World/death/constructor loop. Source aa724 is complete; 56 pending or partial sources
remain. Direct-attack first-incapacitation XP is next. Hosted CI on the final commit of
the entire task remains pending.

Source: [aa724eca2dd4b71a630bbdb63992f8bd8f3752ed](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/aa724eca2dd4b71a630bbdb63992f8bd8f3752ed).

## 2026-11-01: hostile-party XP at direct-attack incapacitation

Direct attacks now award the original experience to the hostile attacker's combat-capable
party on the first knockout or death. The roster and reward are captured before damage.
Allied, neutral, anonymous and unregistered attackers receive no award; dead/KO party
members are excluded. A later hit that kills an already unconscious but retained body does
not award experience again. When damage removes the target entirely, the prepared award is
paid, including the original exception for an already unconscious target. Existing
party-size limits, intelligence/perk modifiers and reward formulas are retained.

Nineteen real native scenarios reproduced10 failures out of85 on both systems, including a
null-lookup crash after attacker removal. Full GREEN 636 Windows / 637 Linux, full Game
builds and323 asset-free checks pass. All66 old controls/nine passing new controls remain
unchanged; the20 comparable post-hit phase records preserve damage, HP, state, killer/time
and four following RNG words. Each phase's experience matches an independent native Unit
clone using existing AddXP. Original control flow and dispatch are statically audited;
separate actual original reward math matches captured native inputs. Original
World/constructor/recipientAddXP execution is not claimed. Sourcec923 is complete;55
pending or partial sources remain. Hosted CI on the final commit of the entire task
remains pending.

Source: [c923fe9eaa63ed62aee9178976661a74d4daefd9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c923fe9eaa63ed62aee9178976661a74d4daefd9).

## 2026-11-01: movement-immunity and range accuracy perks

Accuracy now honors perk19 by removing the additive movement penalty, while retaining the
existing first-round gate and movement cap for units without the perk. Perk86 applies its
authored range multiplier to the firearm's maximum range, with integer truncation and
unchanged minimum range. In the tested stock gun, factor1.25 changes maximum50 to62; the
corresponding effective accuracy-distance term for skill30 changes28 to30.5714283. The
original skill scale140 is retained. Skill values, authored parameters, AP spending and
other accuracy terms do not change in this fix.

Native RED3/60 both ->full GREEN 674 Windows / 675 Linux, full Game builds and323
asset-free checks. All60 RED names are selected;50 old measured controls and seven new
numeric controls remain valid. A perk-owned zero-movement case now returns original
positive zero. Actual typed original Prepare256/SMove105/D1 38 control flow and constants
are statically audited; no new original object or perk-grant runtime execution is claimed.
A rejected scale100 assumption in the added D1 test was corrected from the actual original
reciprocal before production. Source9f3 remains partial: full shared weapon
skill/burst/aim/grenade behavior still needs work. An unproved D1 rounding candidate is
retained for later original-precision audit. Remaining55 sources; hosted CI on the final
commit of the entire task remains pending.

Source: [9f3c03712f2a86f83fb7739891827847b2f9f645](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9f3c03712f2a86f83fb7739891827847b2f9f645).

## 2026-11-01: shared item skill and original accuracy arithmetic

Weapon class proficiency now adds authored +10 to the corresponding shooting skill before
aiming/burst/VP; grenade91 and melee70/throwing-blade89 apply in the shared item query.
Burst30 uses int(bullet/parameter) as the exponent; at stock parameter2, second-bullet
skill improves12 ->24 for the tested SMG recoil35/burst70. Nonbinary third-bullet burst99
changes12 ->17 with the perk. Fractional aiming is retained through the final VP
multiplication: raw70 plus2/3 at VP.75 now truncates53 rather than52. Actual movement
coefficient is added to extra aiming AP before truncation; stock gun17 coefficient-.9,
moves3, extra5 now gives70 rather than72; perk19 does not cancel this aiming term.
First-turn gating remains. A cap-only VP injury current300/cap300/theoretical500 gives
factor.9, hence63 rather than70. Grenade91 gives80 rather than70, wounded60 rather than52,
and cap-only72 rather than70. Melee direct shared query70/80 is checked without changing
the knife-specific raw throwing override. Existing range86 and additive movement19 from
the preceding component remain intact. No authored values, AP expenditure, knife physics
or new grenade range/stance formula is introduced.

Original PC24/RN arithmetic and final truncation are verified against68 real original
records in34 contexts. Genuine native RED17/90 both ->full708 Windows /709 Linux/Game
builds and323 asset-free checks; all90 RED names included and60 old measured controls
unchanged. All34 raw return words match both platforms. Explicit backpack grenade
metadata/active firearm are retained through the necessary native adapter. Source9f3 is
complete;54 source commits remain unfinished. Original grenade physics/world construction
and whole-task final hosted CI are not claimed by these checks.

Source: [9f3c03712f2a86f83fb7739891827847b2f9f645](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9f3c03712f2a86f83fb7739891827847b2f9f645).

## 2026-11-01: original distance-skill interpolation precision

Distance interpolation now uses the original float reciprocal and rounding order:
(maxRange-minRange)*skill*float(1/140)+minRange. Authored ranges, skill values/scale, AP
spending and other terms are unchanged. At tested skill13/min22/max62, the term
changes25.714284896850586 ->25.714286804199219, exactly one float ULP. This preserves
original precision rather than introducing a new balance parameter; no broader change to
final hit percentages is asserted.

Actual native RED1/98 both and8 real original Prepare/D1 queries establish the correction;
original observed PC24/RN produces41cdb6dc, diagnostic PC53 produces old41cdb6db. Full712
Windows /713 Linux/Game builds/CI323 pass, all98 RED names included,94 old measured
controls and3 new controls unchanged. Source226's range/stance/final truncation hunks and
source3cfc physical throw work remain separate;54 sources unfinished. Original
D2/world/final shots and whole-task final hosted CI are not runtime claims here.

Source: [226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4).

## 2026-11-01: grenade accuracy interval, stance perk and truncation

Grenade accuracy now interpolates from the existing physical base distance to
int(1.5*baseDistance), using the original skill scale140 and float/truncation order. This
changes accuracy weighting within the existing throw solver's limits. Tested regular
base21/28 gains accuracy endpoint31/42; engineer base30/41 gains45/61. Actual D1 values at
tested skill70/140 become26/42/37.5/61. Authored ability92 selects the best stance bonus
without changing the unit's pose. With stock crawl-4/crouch-2 it removes either penalty,
while standing stays0 and unowned penalties remain. Final probability truncates after the
existing2..100 clamp. At fixed distances8/9, native chance changes74 ->79 and70 ->76 with
the interval change and truncation; same-interval pre-fix floats73.7984/69.5231 had
rounded upward, and post-range floats79.7057/76.7160 now truncate. No AP spending,
authored values or physical launch velocity/limit is changed here.

Actual native RED10/114 both ->full728 Windows /729 Linux/Game builds/CI323, all114 RED
names included.91 prior measured controls stay exact;7 prior shared grenade controls
change only maxRange to the new contract, preserving raw
skill/minRange/quality/weight/item/ammo/RNG. All16 new min/max/stance/D1 values match32
actual original records; original finalGetToHit conversion is independently statically
verified, but original live-world chance/flight is not executed. Source226 complete,53
unfinished. Source3cfc still owns physical throw/perk88/90/PK/Hide changes. Whole-task
final hosted CI remains pending.

Source: [226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/226b2ddb63dce7dcffa22e1b12c7c73c4a81f4f4).

## 2026-11-01: live grenade velocity and physical distance

Physical speed now honors authored88 multiplier1.1 and current fractional shared skill
with original rounding. Tested88 ranges change21 ->25 for regular70,28 ->34 for regular140
and30 ->36 for engineer70. Wounded52.5 remains fractional, with velocity10.9886255264.
Damage after preparation now changes physical range to current19 instead of cached21.
Cap-only/healed VP retain the restored theoretical denominator. PK strength overrides
human strength and its measured control is unchanged. Several bare regular velocities
correct one float ULP. Accuracy1.5/92/truncation, AP, authored values, fuse rules, PK
origins, hide behavior and save/network layouts remain unchanged here.

Native RED13/129 both ->185 Windows /182 Linux affected tests/full Game builds/CI323;
all129 RED names,114 old measurements and2 new controls are included/exact.15 references
match30 original records under observed PC24. Original world/PK-wear/cache/flight
execution and exhaustive cross-CRT parity are outside the bounded proof. Source3cfc still
needs fuse90/fractional threshold, PK origins and Hide/vision.53 sources remain; final
whole-task hosted CI is pending.

Source: [3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a).

## 2026-11-01: grenade fuse, PK launch and concealment completion

Perfect-fuse perk90 now preserves the ordinary fuse and skips both its random roll and any
misfire scaling. Without90, fractional71.0999985 does not fail at roll71, and52.5 does not
fail at roll52; roll99 retains the original forced-failure rule even at140 skill. This
changes the subsequent gameplay RNG sequence exactly where the original skips calls.
Script chance stays caller-supplied and its physical spin retains3 ordinary later draws.
Worn-PK grenades launch from the left side with animation side0. Throwing while hidden now
exits concealment and requests deferred vision refresh, including completion after load,
with a single throw AP payment and existing save/network layouts.

Canonical native RED12/145 +4/6 both (union16/151) ->207 Windows /204 Linux affected
tests, full Game builds/CI323.129 previous measured controls and6 new controls are
exact.64 actual original957 records verify delay words and full ISAAC state on unchanged
hypotheses, with complete temporary global RNG/context/stack/config restoration. Original
PK/Hide branch proofs are static; complete original World/trajectory/Run/physics execution
is outside scope. Source3cfc is complete after prior shared-skill/physical/lifetime
imports;52 sources remain. Whole-task final hosted CI is pending.

Source: [3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3cfccbee6ac4c5a231a5a6f7ec4230342c82bc1a).

## 2026-11-01: held grenade priority and engineering fallback

An engineering grenade in a hand now allows UnitSwitchToGrenade to select a regular
grenade from the backpack. Ordinary held grenades take priority: SLOT_1 first, then
SLOT_2, even if the other ordinary grenade was active. The queued backpack exchange
targets the opposite active slot and retains owner/items/activation/ordered Continue. This
restores script selection only; AP/HP/inventory/position/time/RNG are unchanged before
queued execution, and authored costs, physical throw formulas and save/network layouts
stay intact.

Native RED5/13 both ->79 Windows /76 Linux selected tests, full Game builds/CI323. Fixed9
cases and exact queue payloads, prior ordinary/draw controls, plus12 actual original
classifier records/6 smoke establish DB-only held classification. Actual original
Lua/World/queue/exchange execution is not claimed; these are native producer tests and
bounded original getter/RTTI runtime with complete read-only RNG/context/stack/config
restoration. Source14ff completes after prior queue/draw/Hide work;51 sources remain and
final whole-task hosted CI is pending.

Source: [14ffbcc83ea0fff503aa22386c98274a5aadd9ca](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/14ffbcc83ea0fff503aa22386c98274a5aadd9ca).

## 2026-11-01: camera framing floor gates and authored PK concealment

Two-point framing keeps an already-visible pose only below rod35, with equality searching.
Framing now honors frozen/clamped camera floors and publishes the camera's accepted floor
to the real render bridge. Tested request9 becomes2, frozen1 stays1, released negative
request clamps0. Authored stealth-capable PK5/persona318 no longer fails Hide availability
solely for wearing a suit; forbidden records/cooldown still reject, while existing
visibility checks/costs remain. Actual unhide/toggle/no-op cases complete without extra
AP/HP/item/position change. Hide-entry LOS/AP and complete original framing-pose parity
are outside these bounded tests; candidate ordering/local-RNG restoration remains
source1b85. Public API/save/network/renderer/authored DB values stay unchanged.

Native RED union14/21 both ->86 Windows /85 Linux selected camera/Hide tests, full Game
builds/CI323;7 controls and inputs fixed. Actual original base-floor10 records/4 smoke,
complete read-only RNG/stack/context/CW/config restoration and separate static
ordered35/DB-bool predicates support the changes. No original complete
World/framing/LOS/bridge/PKCanDo runtime is claimed. Sourcea675 is complete;50 sources
remain and whole-task final hosted CI is pending.

Source: [a6753bf56be2d09c3b84412b4285db2ff54d6306](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a6753bf56be2d09c3b84412b4285db2ff54d6306).

## 2026-11-01: camera candidate order and local probability window

Camera framing tries the current viewing angle before rotating to alternatives and keeps
base priority on equal angular distance. Cheat-gated cinematic probability uses a
truncated window instead of adding0.5: tested count3/divisor1.6 window2->1 changes a
deterministic missed activation to count4/4000ms retained lease. The tested unframeable
target subsequently restores final ratio1; no final active slowing or world-time change is
claimed. Forced fallback consumes the original local draw before selecting its supplied
rod; local state is discarded and no gameplay ISAAC shift is implied. Own
APIs/save/network/render-floor bridge remain.

Native canonical RED5/14 both ->100 Windows /99 Linux selected regressions, full Game
builds/CI323,9 exact controls and inputs fixed. Original typed static
candidate/tie/truncate/local-roll audit supports the producer; no new full original
World/framing/LOS runtime claim. Checkpoint119's three search-yaw expectations change
intentionally; its floor/rod/freeze results remain exact. Source1b85 complete,49 sources
remain; whole-task final hosted CI pending.

Source: [1b85f23ac337fb5f2f4d6acde5341cfe66bb757a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1b85f23ac337fb5f2f4d6acde5341cfe66bb757a).

## 2026-11-01: PK pilot exit owns the forward placement

Leaving a PK now moves its pilot to the snapped point1.45 units ahead, keeps the released
shell at the former wearer position and re-seats both actual animators. Exit passability
is checked in the current pose/facing before detaching; a rejected point uses the existing
failure state. The eight tested passable directions now have zero simulation/animation
error. AP, healed wounds, shell HP, inventory identity and world time stay fixed in these
callback cases; complete action AP spending and a separate blocked-abort runtime are
outside the claim. Two ordinary eligibility controls stay exact. Own packed
positions/enums/API/save/network/helpers remain.

Canonical native RED8/10 both ->116 Windows /117 Linux selected related regressions, full
Game builds/CI323. Typed actual original1058 callback/static branch/argument audit
supports the placement; no original full World/exit/animation runtime or full RNG parity
is claimed. Sourced44 complete,48 sources remain; whole-task final hosted CI pending.

Source: [d44bba0e1f5ecb91f589ef70e205eb0cacfc6285](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/d44bba0e1f5ecb91f589ef70e205eb0cacfc6285).

## 2026-11-01: PK collapse, stun and ordinary corpse refresh gate

Dead, unconscious and stunned PK wearers use authored collapse2484/2466 with the existing
hold-last-frame policy and impact sound16584. Their logical placement no longer enters
ordinary human-corpse hip snapping/hit-location refresh, preserving the worn shell in
place. Five actual collapse cases now emit their clip/hold policy/sound; two downed-wearer
cache cases keep0 human corpse points and unchanged position. Four ordinary controls and
captured AP/HP/consciousness outcomes remain exact. This restores Steam behavior; Gold's
smaller old helper only placed a prone pose. Own direction-aware placement checks, run
flag, packed types/API/save/network remain.

Canonical native RED7/11 both ->127 Windows /128 Linux selected regressions, full Game
builds/CI323; prior121 pilot-exit10 measurements exact. Original Steam223
helper/caller/whole615 segment audited statically; no original World/audio/animation
runtime or sampled future held bone frames is claimed. Sourcec941 complete,47 sources
remain; whole-task final hosted CI pending.

Source: [c94140922b034b3e11dd9e2a1d740d32c559e488](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c94140922b034b3e11dd9e2a1d740d32c559e488).

## 2026-11-01: hearing probability, quiet practice and crawling noise

Guaranteed loud sounds no longer grant SPOT/INT practice; quiet successful detection
retains its existing fractional practice and d99 draw. Tested loud
progress.00779656228/.0000207908342 becomes0/0. Perk20's fractional SPOT product truncates
(tested probability71->70); human loud hearing no longer receives PK-only sensor perk76
(100->0 at tested boundary). A quiet sound without a unit source now uses hard-range
hearing instead of dereferencing null. Crawling SetPosition no longer emits its extra
AI-noise event; walk/stance/turn controls remain. Positive quiet-step77 coefficients
use1/coef as in the original, with that edge supported statically rather than a separately
isolated custom-data runtime. Authored records/provider/training formulas/AP/HP/XP pool
and save/network types remain.

Canonical native RED5/14 both (4 assertions +null-source crash) ->220 Windows /221 Linux
selected regressions, full Game builds/CI323,9 exact controls/inputsfixed and21 prior PK
measurements exact. Typed original whole517/285/742 and separately decoded179 caller
support the changes; no original full World/hearing/RNG runtime claim. Source242
complete,46 sources remain; whole-task final hosted CI pending.

Source: [242f6dfd827d93014c4210980f9d1c4af2a45b67](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/242f6dfd827d93014c4210980f9d1c4af2a45b67).

## 2026-11-01: heard unseen AI camera and live RPG ownership identity

Successful heard AI sounds in turn-based play can focus the camera when the human hearer
cannot see their source. Repeat already-audible sounds still qualify. Priority0, source
floor, no zoom, no slow motion and zero dwell match the original. Visible sources, human
sources, AI hearers, ownerless sequences, deaf hearers, null sound records and dead
sources produce no focus; two qualifying listeners each produce their own command.

Restore the saved owner-AI snapshot when creating or assigning a unit. This also makes the
already-present flesh-damage difficulty branch choose the authored enemy multiplier for
AI-owned attackers, instead of always choosing the player multiplier. Commander-only
changes keep the snapshot until another owner assignment. Null/anonymous attackers retain
unscaled damage, and human attackers retain the player multiplier. The original
flesh-material gate and damage/training/AP/HP/RNG formulas are unchanged; no authored
difficulty record or save/network tag changes. Existing saved flag values remain
authoritative; no legacy save migration is performed. Native fixed100 damage with distinct
authored coefficients isolates four genuine RED cases, while six damage outcome controls
pass.

Canonical15/33 RED ->33 new tests both plus416Win/414Linux related passes, full Game
both/CI323,35 prior hearing/PK measurements exact. Original typed139/1316,
setter/classifier/ctor-call and exact62 difficulty gate are static evidence; zero new
original runtime records. Source171 complete,45 sources remain.

Source: [1715d48307f8282d20192425599c5b4253c335ea](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/1715d48307f8282d20192425599c5b4253c335ea).

## 2026-11-01: whole-hull cover projection, extended raster bounds and normal range caps

Cover and hit-location queries frame the whole hull while retaining the requested part
filter. Rasterization extends beyond the target so geometry is not cut off at its center;
primary and secondary sampling squares scale with the extended projection. Penetration
remains capped at30 world units on both AI and normal player cover paths. Tile cover
stops0.625 before the aim point instead of0.25 and also respects30. This can change
preview/AI cover weights and sampled ray distributions near body parts, distant targets
and tile-adjacent allies. No independent accuracy, damage, training, AP or weapon formula
changes. Unit walks remain radial, tile walks retain projected-plane distance, and trace
masks/part filters/save/network types are unchanged.

The original generic routines take an explicit range; normal AI/human unit/tile callers
independently pass30. Keep that existing public adapter rather than adding another
public/save parameter; custom splinter ranges elsewhere remain. Native RED15/22 ->22 new
cases pass,7 outcome controls and all captured query-state/RNG invariants. Final38
affected tests both plus371Win/372Linux other related passes, Game both/CI323,33 prior124
samples exact. Original complete933/540 and independently bound tangent/callers are static
evidence; zero new original runtime records. Earlier tests' uncapped33 reference and0.25
tile plane were updated to the confirmed rules, with old failed runs archived. Source01eb
complete,44 sources remain.

Source: [01eb873b41cd2c8b9ce2d7b81835d2e00d492118](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/01eb873b41cd2c8b9ce2d7b81835d2e00d492118).

## 2026-11-01: particle half-texel margins and grass cell insets

Rendering-only change: particle texture coordinates add/subtract0.5 physical atlas texel
before existing packing. Grass splits an uninset sheet, then applies its own per-cell
half-texel margin while retaining V orientation and sprite order. This restores original
atlas-edge sampling where coordinates can represent the margin. Native geometry,
color/depth, counts, gameplay RNG, texture ownership, authored assets, shaders and
save/network formats remain unchanged; no AP/damage/accuracy/training rule changes.

Own HD4096 atlases remain RGBA8 with2048 UV packing. Even HD boundaries may round a half
texel away; five exact HD controls intentionally keep that result. NPOT19 rain edges still
change through original nearest-even rounding. No wider global UV format or pixel-perfect
HD/seam-removal claim. Native real bgfx rain/grass RED9/14 ->32Win/32Linux/Game
both/CI323,5 exact controls and unchanged geometry hashes. Original complete481/678 plus
named consumer/default0.5/grass0 calls support the port statically; zero new original
runtime records. Source24c1 complete,43 sources remain.

Source: [24c1eca77dbcf2eff5feaa8ef603f390df93f7fb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/24c1eca77dbcf2eff5feaa8ef603f390df93f7fb).

## 2026-11-01: retain RGBA8 and HD atlas policy (source67d excluded)

Intentionally exclude the complete legacy16-bit atlas restoration. Keep the existing
remaster maximum-quality/RGBA8 policy and own1024/HD4096 transparent atlas; do not
reintroduce the A4R4G4B4 conversion preset or lower color precision. All source hunks are
mode-specific; the current RGBA8 fallback already preserves usage/ownership. No game code,
renderer output, assets, gameplay coefficient, RNG, AP or save/network format changes.
Documentation only; reuse unchanged-production checkpoint126 Game both/32Win/32Linux/CI323
including actual graphics-options/HD-cache tests. No fresh run or original parity/runtime
claim.42 sources remain.

Source: [67d766bc791c9207624bce8753fbe7b56a56e704](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/67d766bc791c9207624bce8753fbe7b56a56e704).

## 2026-11-01: global-map desktop ownership, draw refresh and globe selection

UI lifecycle restoration only: the strategic map owns a real desktop/client, refreshes
native polygon hover and recommendation flashing in Draw, and offers the original unsaved
optional globe selector with default false. Keep authored recommendation logic, campaign
XP, world/script execution, adaptive layout, shared audio and2D rendering. No damage,
accuracy, AP, movement, training, RNG or authored database tuning.

New UI save writes use native nested desktop-base state; backward-compatible reads accept
old remaster flat CWindow saves and restore only missing list/client ownership after
loading the complete graph. Class IDs and LAN registration remain. Native RED7/10 both
->10/10 both then208Win/181Linux related/full Game both/CI323; all14 prior map lifecycle
probes retained. Linux coverage combines179 initial passes and two targeted unchanged-code
retry passes after startup/resource timeouts, not a single all-green181 batch. Actual
original named VTs/callers/opcodes/registration support the change statically; Gold/Steam
whole-body layout equivalence is rejected, zero new original runtime records.41 sources
remain.

Source: [b91ea228f2a5443af74d0ae728e24410f7d90fad](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b91ea228f2a5443af74d0ae728e24410f7d90fad).

## 2026-11-01: shared fixed raster edges with original projective guard planes

Restore original geometric sampling rather than introduce a combat coefficient: common
triangle edges use16.16 arithmetic, including original half-pixel boundary behavior. The
perspective path first clips against near0.01 and four0.001 guard planes, bounding
projected coordinates near+/-1000 before integer conversion. Pixel coverage, cover rays,
AI visibility/occlusion and resulting hit previews can change at edges; large off-screen
geometry no longer creates negative/extra intervals. Keep the original near distance and
existing native fixed helpers, unsigned32 wrap handling, span/depth implementations,
projection matrices and public/packed/save/network layouts. Original temporary
clip-distance storage is recomputed within the private algorithm. The separate
integer-shadow-depth restoration remains pending.

Native generic RED6/16 and projective guard RED15/20 both ->combined36/36 both,
final439Win/435Linux/Game both/CI359. Ten generic controls are exact; guard
source/interval-chain, geometric slab/count and direct4-RNG continuation invariants pass.
Old night/awareness/knife/called-shot/AI-cover cases recover without fixture changes when
both dependent sources are present. No AP/damage/accuracy/training parameter, authored
database, HD texture, font/face, SDL/bgfx backend or LAN type change. Original typed whole
bodies and bound local switch tables support this statically; zero new original runtime
records.39 sources remain.

Source: [70371f39f6fb4a8cc7cdc15b117f1c704cc81aeb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/70371f39f6fb4a8cc7cdc15b117f1c704cc81aeb), [3bd9a53bf0cdd3a2e70c6418029c80f8e11325f5](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/3bd9a53bf0cdd3a2e70c6418029c80f8e11325f5).

## 2026-11-01: original AI cover trace and transient-lock-free flanking

Restore original geometric AI policy: projection trace is0 for AI probes and30 for human
shots, independently of the unchanged physical penetration cap30. The projection remains
max(0.1,max(trace,targetDistance+2)); distant allies beyond a nearby target no longer
enter that AI frustum merely because of the bullet maximum. Original flank search ignores
transient unit locks with suicide checks disabled, retaining its8 AP wave cap,
perpendicular0.1737 threshold, action/pose costs and14 AP constructor default. Candidate
places and AI cover scores can therefore change; authored coefficients, AP/damage/training
formulas, tile framing and save/network types do not.

Exclude the donor's shared-wrapper IsBigLocker insertion: actual Steam77df57..77df80
preserves the original bool-shift defect, including pose-lock not reaching moveOnly. Keep
that original behavior and add an explicit regression control. This corrects a misleading
old source comment, not the gameplay rule. Also correct checkpoint125's written AI range
interpretation: bullet maximum30 and trace0 are separate original arguments; historical
raw evidence is unchanged. Canonical RED4/16 ->16/16 both, final455Win/451Linux in
separate447/443+8 all-green batches/Game both/CI359; all12 outcome controls and
AP/HP/position/4 RNG invariants exact. No new original runtime claim.38 sources remain.

Source: [9cf3fd3dae155de3b0c0e1de443cf54a7f494426](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/9cf3fd3dae155de3b0c0e1de443cf54a7f494426).

## 2026-11-01: original ally-ray classification and AI geometry safety

Restore original geometric friendly-fire policy: an ally-blocked ray is loose, not a
blocked target-hit ray; AI safety uses actual weapon-origin hit/loose-cell geometry and
the tactical state's live non-enemies. The private collector includes every cell interval,
even beyond a reached target or penetration stop, and excludes obstacle-only rays. AI
places and cover/accuracy outcomes can change where the former one-metre centreline missed
collision geometry or where ally rays incorrectly remained in hitRays. No
AP/damage/accuracy/training coefficients or weapon data change; authored attack generation
spends no ammunition. Keep physical range30, AI trace0, human/tile framing, finite score
guards and the corrected130 flank caller.

Adapt donor guards to actual original: no enemy is false, invalid actor/state true before
the weapon check, enemy/server validity is not CanFight. Exclude the donor no-enemy true
and extra enemy CanFight behavior. Preserve legacy CCoverInfo tags2..6, three SRay fields
and class ID; the new private object union is transient, cleared onREAD, rebuilt only by
fresh CalcCovers. No serialized grids/cell coordinates, LAN types, authored
assets/HD/fonts/faces/backend changes. Canonical RED7/15 both ->17GREEN with8 exact
outcome controls, full506Win/502Linux/Game both/CI359; saved bytes/fields and
AP/HP/ammo/position/4 RNG invariants verified. Original complete523/234/717/505/99 methods
plus independently decoded actual safety calls/branches support this statically; whole1099
comparison rejected.0 new original runtime records.37 sources remain.

Source: [fc2836a521913dafcb7737659c6912f178dc5ef1](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fc2836a521913dafcb7737659c6912f178dc5ef1).

## 2026-11-01: original integer shadow and occlusion depths

Restore original rendering/occlusion precision: ushort depths use10000 shadow-part scale
or40000 camera scale,8 fractional iterator bits, nearest-pixel gradient anchoring and
signedSAR8. Equal quantized depths keep the first submitted owner. Span depth writes
intentionally wrap to a16-bit word; HZ sphere queries above65535 return visible
immediately, while ordinary queries truncate to a word and compare strictly. Spherical
initialization is capped65535; hierarchy padding is65535 and2x2 reduction is conservative
minimum. These can change hidden-part lists, shadows and near-surface rendering relative
to the prior float approximation. Retain original overflow/quantization policy, not a new
precision preference.

Default non-shadow consumers keep float depth formulas; own16.16 edge wrap,0.01/four0.001
clipping,24-byte projected point, raster public APIs and saved/LAN types stay. Native2D
arrays replace original private pointer/row caches, avoiding32-bit ABI copying. Full
native RED10/19 ->19GREEN both,9 exact controls, fractional-gradient/frontbehind and
persistent ownership checks; old16 generic raster outputs exact/all20 guards pass,
final527Win523Linux/Game both/CI378. No authored AP/damage/accuracy/training values or
assets/HD/font/face/backend/audio changes. Static whole213/281/314/721/151/399/278/913
plus prior1248/769 and independently decoded Generate/Fast scale inputs support this;
unknown-global whole comparisons rejected.0 new original runtime records.36 sources
remain.

Source: [acf8dc8198a56959358dc0f226cf2613d8d56b4e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/acf8dc8198a56959358dc0f226cf2613d8d56b4e).

## 2026-11-01: original game icon in the existing SDL window

Visual branding only: replace A5 with the original seven-frame game icon in existing
resource101. The live SDL loader already selects this application icon. Preserve own SDL
startup/lifecycle; historical GDI splash/DC/bitmap changes and deleted WinFrame
implementation are explicitly excluded, not reported as an already-shown logo. No
AP/damage/accuracy/training, gameplay, renderer algorithms, font/face, HD textures, data,
audio or wire changes. Exact source asset copy, no image generation or PNG rebuild.

Native actual SDL window RED1/1 ->GREEN1/1 with loaded/selection controls; actual final
Game resource101 matches all7 original payloads and8973e7b08c5fa1e4 native32px
fingerprint. Windows14 related single all-green, Linux12 distinct split11+1 proper-env
retry, full Game both/CI378. Original module was data-only,0 new original-game runtime
records.35 sources remain.

Source: [65b2d080d3a95f9bf713fbda814c2b39c68c74a4](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/65b2d080d3a95f9bf713fbda814c2b39c68c74a4).

## 2026-11-01: wounded AI fear and live nearby support

Original AI behavior changes: unsupported actors become scared at VP<0.5*maximum or
confirmed enemy count>2, replacing the incomplete >8-only heuristic. Fight-capable
non-enemy units within strict9 support the actor even across neutral owners or stale side
caches. Self, enemies, unconscious units and support at/above9 do not prevent fear.
Panzerklein pilots are exempt; valid contacts remain usable despite stale planningHP0. No
extra chance/difficulty coefficient or AP cost is introduced. Existing help/scared latch
semantics remain, first fear marks modified. GetKnownEnemies and retreat place/timing are
still pending donor work.

Actual original399/402 methods, namedPK54/skill19 and independently decoded
constants/guards; canonical native RED8/18 ->GREEN18/18 both with10 exact control rows;
physical/planned AP/HP/positions/contacts/fourRNG preserved. 389 Windows/392 Linux related
cases, full Game both/CI378 pass. Runtime support controls are below/above9, not exact9.
No data/HD/font/face/audio/wire changes;0 new original-game process/runtime records;35
source commits remain.

Source: [98871d0f0def4f966e097b984c18e57a8f437d7b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98871d0f0def4f966e097b984c18e57a8f437d7b).

## 2026-11-01: combined threats and deferred reachable retreat

Original retreat behavior: combine suspected then confirmed enemies; choose among
reachable RUN places using the existing weighted-away/cone/random helper with minimum5
units and maximum30 movement AP. This replaces a deterministic farthest passable point
near an8-unit offset that did not establish reachability. The search uses the fixed30
budget even at currentAP0. Keep empty-search Guard radius12. Install the next reaction and
update it later through the existing pump, preserving the current route until then.
Existing helper internals, movement prices and paidAP are unchanged. Contact-list copy
preserves order/duplicates/nulls and does not change saved/wire fields. The random
selection intentionally restores its original draw instead of preserving the
approximation's no-draw behavior.

Canonical native RED12/14 ->GREEN14/14 both, actual cost29Windows/26Linux<=30 and
distance>=5, suspected-contact destination deflection, deferred route/timing and four
seeded RNG-word checks. Physical/planned AP/HP/position/contacts/wish pose unchanged. 403
Windows/406 Linux related cases, full Game both/CI378; all18 prior fear rows exact, all12
prior alarm cases pass after old deterministic-fallback test expectation is updated.
Original static99/58/67/549 and independently decoded typed Normal callsites; no
unknown-global/control masks or original game process.34 sources remain.

Source: [98871d0f0def4f966e097b984c18e57a8f437d7b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98871d0f0def4f966e097b984c18e57a8f437d7b).

## 2026-11-01: tooltip options and original hover deadline

UI timing only: ui_showtooltips now controls hover claims; ui_tooltipdelay is scaled
by10000, so .1 gives1000ms. Keep the same-owner deadline, reset on owner changes/re-entry,
and show strictly after the deadline. Existing no-claim Step cleanup remains. The
portable53-bit deadline math follows the observed original x86 default027F context: .7 at
time100 gives7099; original signedFISTP32 out-of-range result80000000 is preserved.33
bounded original calls characterize alternate precision contexts separately; no
all-mode/full-original-UI identity is claimed. This supersedes the earlier provisional
immediate display retained during sourcea2's oversized clamp partial. No
gameplay/AP/damage, save/wire, fonts/faces/HD/PNG, owned sound/input/render lifecycle
changes. Width400 and old tooltip measurement/Draw/clamp remain until the separate
sourcea2 stage.

Native RED10/12 ->GREEN12/12 both, two exact controls, owner geometry/fourRNG/shared sound
preserved; 141 Windows/133 Linux related cases/full Game both/CI378.12 packaged
vector-font rows match across platforms and old ordinary/oversized metrics remain intact.0
original Game.exe processes;33 bounded native original-function calls. Both donor sources
remain partial;34 unfinished.

Source: [360344ca1679d8bc7f4706d69e1270ee0caa3387](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/360344ca1679d8bc7f4706d69e1270ee0caa3387).

## 2026-11-01: measured tooltips and authored screen anchors

Tooltip layout only: maximum logical text width400->512, measure before placement, use
authored four screen-anchor corners and update for moved/changed owners. Unanchored
tooltips keep initial cursorX and normally sit above the owner, with ownerHeight+32 below
fallback. Clamp to current logical canvas including oversized tips, wide display and
HiDPI. Use inherited standard Frame drawing and Window positioning; no per-draw size-reset
override. Preserve136 hover deadline/options, actual font family/size/texture pipeline,
save/wire IDs, audio/input/SDL/bgfx and HD/face resources.

Native RED17/20 ->GREEN20/20 both/three exact controls. Genuine wrap
boundary408x44->448x26; the existing long caption intentionally changes408x1808 at616,0
to518x1214 at506,0, while ordinary138x26 at886,741 and true oversized48x1826 at30,0
remain. All12 vector-font compatibility rows and12 clock traces are exact versus136;
owner/input/shared-sound/fourRNG preserved. Full Game both/161 Windows/153 Linux
related/CI378; no gameplay or original-game process/runtime changes. Sourcea2 complete;33
donor sources remain.

Source: [a2bba0ad86c91dcf5a239adda24792664abf1e43](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a2bba0ad86c91dcf5a239adda24792664abf1e43).

## 2026-11-01: AI-controlled demonstration squads and level preparation

Explicit autoplay demonstration squads now use the original AI-controlled flag and prepare
every merc to level30 before mission creation. This changes the demo's unit skills/perks
through the existing SetXPLevel routine as the original does; ordinary human creation and
campaign preparation retain their previous behavior. Keep existing four-squad/random4..7
composition, seed selection, parser quirks and map selection. Automatic idle entry, demo
watchdog, flash overlay and mission lifecycle are not enabled by this partial import. No
global level rule, paidAP, damage, price or combat formula changes.

Actual Steam four AddPlayer calls and post-createAI byte54, typed saved flag8/mercvector5
and nested merc-level30 loop are statically confirmed. Native RED2/2->GREEN2/2 both using
actual console command and real world links; every demo merc reaches30 and the separate
ordinary human remains unchanged. Random counts intentionally vary; no claim of full demo
battle/watchdog validation. Full Game both/CI378; related 416/417 Windows and 386/386
Linux, including one recorded pre-existing intermittent Windows campaign movement timeout
reproduced on previous-head source with byte-exact native .text. Current source
restored/rebuilt and demo cases retested;33 donor sources unfinished.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: one-shot demonstration map transitions

Explicit autoplay demonstrations can now advance to a new random map on any winning squad,
world turn id>100, or UI watchdog>20minutes, once per current modal. Exact boundaries100
and20minutes do not transition. Dismissal input retains its one-shot latch. Keep ordinary
mission loss/trace/action/state/save policy and LAN restrictions verbatim in the default
callback. Preserve existing random map/squad selection and138 level preparation,
class/save/wire identifiers and shared renderer/audio backend.
Initialization/overlay/sequence/fullscreen and automatic idle-menu entry still await
separate hunks.

Reject the donor's common combat/ambient UpdateSound move: both actual Steam and Gold keep
it in interactive CMission::GameStep; demo must not run that policy. Native music input
remains combat in demo (enum2; old behavior reset it to ambient enum0), while ordinary
mission sound logic is unchanged. Original GetTurnID is the world counter, not game
progress; actual getter and strict signed/unsigned branches are decoded without
field/frame/virtual-slot masks. Native RED4/9->GREEN9/9 both/five exact controls; actual
transition container replaces its modal once. Full Game both/425 Windows/395 Linux
selected related all pass/CI378; known pre-existing Windows campaign movement flake is
explicitly excluded and not claimed fixed.33 donor sources remain.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: publisher-logo demonstration and UI-clock rendering

Explicit demonstrations now show the original blinking publisher logo and hide the
ordinary mission HUD, with sequence1, pre-init lose latch and scripted turn-based request
as original. Both overlay Step and Draw use the live UI clock even while simulation is
paused. The fork's HUD-reserving client view is overridden only for demo rendering with a
full normalized canvas; native wide/HiDPI projection and fonts are retained. Shared scene
sound is not pumped twice. Keep the original flash class ID/five fields and ordinary menu
implementation, extracted into a shared header.

Preserve the SDL pointer position instead of restoring the original cursor-center warp;
the demo cursor is hidden. No ordinary mission AP/damage/skills/prices, LAN protocol,
saved field numbers, audio backend, asset sampling or HD/face pipeline changes.138
original demo level preparation and139 transition thresholds/latch/interactive-only music
remain intact. Automatic menu idle entry still awaits its separate port.

Native RED6/6 ->GREEN6/6 both, combined17/17; actual draw time4000->UI123 and flash
morph123, original state flags, full ordinary/wide1792/200percent canvases and shared
audio verified. Full Game both/178 Windows/170 Linux selected related/CI378;9 prior
transition traces and12 vector-font traces exact. Sourcefdd partial,33 donor sources
remain; known prior-head campaign movement flake not claimed fixed.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: main-menu idle demonstration, input and return focus

Main menu starts original AI demo after strictly more than two UI-clock minutes. Real
activity resets before UI can consume it; time-only events do not. Returning focus starts
fresh interval on same clock. Restore original menu scalar timestamp field2 instead of
incorrect UI reference; base-owned UI reference becomes transient. Ordinary mission saved
tags and LAN wire unchanged.

Exclude donor extra UI-counter ResetTiming on focus: original refreshes/stamps existing UI
time then inherits focus, preserving150ms catch-up. Retain138-140 demo
AI/levels/lifecycle/logo, live UI drawing, interactive-only music, own full demo canvas
adapter and SDL pointer position. Native inactive menu paint/drawable gate,
startup/LAN/HiDPI/fonts/audio/HD/face pipelines remain. No ordinary mission
AP/damage/skills/prices changes.

Native RED6/8 ->GREEN8/8 both, combined25/25, Gameboth/186 Windows/178 Linux
related/CI378;9 prior lifecycle+6 presentation+12 font rows exact. Actual demo
entry/return, UI-consumed input reset and real minimized guard verified. Sourcefdd
complete,32 remaining. Known prior campaign movement flake remains outside UI suite.

Source: [fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fdd3e76a911cd34238d37a4a1a36dd8aacbf6bc9).

## 2026-11-01: cached point lights honor their shadow flag

Cached point lights with their existing bCastShadow=false now illuminate through the
existing no-shadow color path and complete their cache slot without cubemap occluder
passes. Shadow-casting lights and later mixed neighbors keep their existing depth work. No
source radius/color/attenuation, damage/AP/skills/prices or gameplay balance changes.
Existing32-byte light-state/save flags and LAN wire remain unchanged.

Adapt original no-shadow branch to own sequential four-stage cache scheduler. Keep native
bgfx backend, stable sky/cache ownership, texture/shader/register choices and HD/face/font
pipelines; donor mixed screen-history/reprojection/register/shader hunks remain pending.
No new PNG/assets or original runtime. Whole420 helper equivalence is explicitly rejected
because actual float operands lie in writable-marked rdata; no readonly-constant
normalization claim.

Native canonicalRED5/8 ->GREEN8/8 both, three exact controls, actual no-shadow depth
views2/4->0, mixed neighbor2 and positive receiver color traversal verified. Full Game
both/269 Windows/260 Linux selected related/CI378;23 prior demo/menu and12 font rows
exact. Sourcef6 partial,32 donor sources unfinished. Known prior campaign movement flake
remains outside selected suite.

Source: [f6f6011fc301ace82317b9007ec6dd36b57f3369](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f6f6011fc301ace82317b9007ec6dd36b57f3369).

## 2026-11-01: selected-camera floor ownership and initial handoff

Tactical floor controls now use the selected camera's existing saved floor, clamp and
freeze. The render scene receives that floor on the normal native frame, initial common
subset and snapshot restoration. Framing no longer publishes another camera's render floor
immediately. Level-bar buttons/drawing and floor keys/script requests use the selected
owner. Initial all-AI demo selection now follows its actual acting player and copies the
selected deploy pose/floor to an idle cinematic camera when no locator owns it, avoiding
the old map-edge/ceiling starting view.

Restore the original initialization's existing player/locator/camera subset without extra
world ticks or per-turn pose copies. Remove the runtime-only provider, retaining saved
floor3/ranges11/12/class IDs and LAN wire. Non-tactical menu/globalmap floor/lock policy
remains owned by the scene through the shared dispatcher. Interactive-only music already
matched at139. No AP/damage/skill/price changes, SDL cursor warp, renderer/audio/font
backend or frozen HD/face/PNG changes.

Native RED13/14 ->GREEN14 both/control0, combined50 and final51 with a dedicated real
virtual non-active-winner consumer. Actual initialized
camera/API/frame/buttons/freeze/clamp/handoff verified. Historical bridge-only immediate
scene assertions are replaced by original deferred semantics and actual frame proof. Full
Game both/717 Windows/682 Linux related covered/CI378, with original716/717 Windows
and681/682 Linux root-map failures retained, then Windows6/6 and Linux9/9 affected reruns
after separate diagnostic-only validation143. All52 raw map geometry/non-item/gameplay
fields stay exact; no physical coordinates changed to repair a diagnostic hash.23 prior
demo/menu+8 cached-point+12 font rows exact. Known prior campaign movement flake remains
outside selected suite. Source6b complete,31 remaining.

Source: [6b4f7b8f67f26adb20506bef1015f89aba9f29dd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6b4f7b8f67f26adb20506bef1015f89aba9f29dd).

## 2026-11-01: saved object-melee executor foundation (145)

The native save decoder can now restore the original object-melee executor ID01353130, its
remembered point, extra AP and optional target. Loaded attacks use real inherited
action/AP handling, current-hull or saved-point aim, normal hit/miss and saved-target
damage filtering. Default/null filter remains unfiltered; saved unit targets keep original
HL_ANY defaults. This class was absent; no fresh command producer, parameter constructor
or current unit/tile/live object route changes here.

Native RED7/14 ->GREEN14 both with seven exact controls, actual AP53->32 for
cost14+extra7, real target/pocket/null decoding and native two-human selected0/3 versus
null3/3 damage/callbacks. Correct the test fixture's artificial VP500 cap only after
reproducing six old failures with byte-exact parent production; retain authored health and
strict measured damage, callbacks and godmode. No production
health/chance/AP-price/RPG/balance change, save/LAN backend change or frozen asset
generation. Game both/372 Windows/361 Linux related/CI378,12 font rows exact. Parameter
constructor/routing and original null-coordinate issue remain pending;31 unfinished
sources.

Source: [4cdc1eada6c182c0a9093364e95c750a6166d4c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4cdc1eada6c182c0a9093364e95c750a6166d4c8).

## 2026-11-01: fresh object-melee routing and requested AP (complete146)

New melee/knife commands against non-unit world objects preserve requested extra AP
instead of losing it during point-command conversion. In actual native tests extra7
changes the unarmed query14->21 and a real knife18->25; direct Start spends base+extra.
AP-price tables, skills, damage and to-hit formulas are unchanged. These commands now run
the original object executor's existing native one-shot point selection/hit-miss path,
with the SS NULL filter. Direct point, unit and missing-target controls keep their
existing paths.

Retain the fork's previous finite current-hull center as the runtime remembered point.
Original SS passedNULL and ignored a failed coordinate lookup; reproducing those
uninitialized coordinates would introduce undefined input. This explicit own constructor
adaptation preserves existing geographic input and does not claim an original zero point.
Donor Sentinels retained-target/frame-aim producer is omitted as an independently verified
version difference; non-null loaded states still use145's saved-target support. Native
path/facing/equipment, named own enum ordinals, saved fields/classID and LAN/native
backend/assets stay unchanged.

RED5/7 ->GREEN7 both, combined21 and two exact direct-point/targetless controls. Native
fresh identity/extraAP/Start/finite-point/null-filter roundtrip and real nonthrowing knife
covered; prior14 cases plus godmode-dedup and12 font rows exact. Game both/379 Windows/368
Linux related/CI378 pass. Earlier door lookup +145 saved executor +146 SS
routing/constructor adaptation complete this source;30 unfinished. Prior campaign movement
flake remains tracked and unrelated, not fixed here.

Source: [4cdc1eada6c182c0a9093364e95c750a6166d4c8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/4cdc1eada6c182c0a9093364e95c750a6166d4c8).

## 2026-11-01: initial cinematic floor range (147)

Scripted sequences now start with the map's floor limits on their cinematic camera,
matching the already bounded player cameras and view. Previously an actual single-floor
view0 could have cinematic floor4; initial range assignment now clamps it to0.
Bounded[-1,1] and unset/default[-3,4] cases are independently checked. One initialization
line; no AP/damage/price formula, pose, timing, frame count, saved range field or snapshot
policy changes.

Native RED2/3 ->GREEN3 both, combined54 with51 existing cases green and fixed46
rows/non-active-winner marker exact. Real startup, selected-cinematic frame and graph
identities checked; temporary metadata input restored and game.db file unchanged. Game
both/278 Windows/251 Linux related/CI378, prior melee/godmode and12 font rows exact.
Native backends, own HiDPI/font/HD/face assets and LAN remain intact. e825 deployment/foot
support/light remainder stays pending; source partial30unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: live and saved foot support (148)

Unit foot placement now uses the actual movement floors and the same supporting points
before and after loading. Excluded geometry no longer supplies another floor's height;
missing support keeps the unit height. Constructor and actual movement publish the
existing saved fields; native cached and freshly loaded surfaces agree at every sample.
Restore original nearest non-door support, relocation bounds and same-center guard while
retaining native gradient smoothing. Three production files, no damage/AP/price/timing
changes or new save tags.

Canonical native RED6/7->GREEN7/combined61 both, including full second-floor
inclusion/exclusion surfaces, actual PlaceUnit/Move and fresh registered-node reload.
Initial diagnostic peak beyond existing smoothing slope was corrected only in input after
preserving failed evidence, then final tests replayed against exact parent source. All54
earlier camera/demo/light cases pass,49 fixed rows plus non-active-winner marker exact.
Full Game both/376 Windows/348 Linux related/CI378; prior melee/godmode/font12 exact,
database hash unchanged. Existing native backends, fonts/HD/face/PNG and LAN unchanged.
e825 still partial for deployment/light propagation;30 unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: authored point-light shadow propagation (149)

Authored point lights now retain their shadow setting through world rendering, scene
creation and saved light-node reload. The setting remains independent of lightmap-only
eligibility, color, position and radius. Actual44-lamp true-shadow route changes44 lost
flags to0; false-shadow and zero-radius controls remain exact. The142 cached renderer
already honors the delivered setting. Mandatory explicit argument restored across existing
interfaces, no new virtual slot or save tag, no AP/damage/price/timing change.

Native RED3/7 ->GREEN7/combined68 both, all61 prior foot/camera/demo/cached-point cases
and fixed58 rows exact. Real native light factory/radiance/fresh saved-node reload tested,
no fake light or world reload. Game both/452 Windows/423 Linux related/CI378, prior
melee/godmode/font12 exact, database unchanged. Existing backends, assets and LAN
retained. e825 remains partial for deployment;30 unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: authored subzone passage deployment (150)

Campaign ETStore underground arrivals now use their authored crouched unit markers,
selected by passage side. Ordinary passage arrivals receive native final facing and
formation after their passage approach has been selected. Unit geometry snapping is
unchanged. Own LAN placement call sequence is preserved and12 existing network gates pass
per platform, including preparation, combat movement, stop/resume and all encounter
deployments. No AP/damage/price/timing rule or save/API changes.

Native canonical RED4/5 ->GREEN5/combined73 both; exact ordinary non-passage control,
all68 prior cases and65 fixed rows retained. Initial diagnostic raw wall-target mismatch
is retained and corrected only by independently resolving existing native nearest geometry
before locks; final full position bits remain strict. Game both/471 Windows/442 Linux
related/CI378, previous melee/godmode/fonts12 exact, database unchanged. All15 e825 source
files accounted for across144/147/148/149/150; source imported,29 unfinished.

Source: [e825c21b229ebf5c302b2361daf49166db50407e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/e825c21b229ebf5c302b2361daf49166db50407e).

## 2026-11-02: native Lua implicit-end scheduling (151)

Match actual Steam1.2 script update scheduling: implicit function completion yields the
caller until its next native script update; explicit return continues immediately. This is
verified Steam behavior; Gold1.1 differs and is not claimed as matching. Four-line loop
break, existing native return and save handling retained. No AP/damage/price rules, extra
world frames or artificial clock changes.

Native RED3/4 ->GREEN4 both, explicit-return control exact, two nested yields and
independent saved suspended Script resume verified. The old5247 test assumes flyer state
at50; actual Steam scheduling puts it at52. Only the diagnostic phase gate now waits for
the native event within its unchanged220 updates, retaining native grid/survival/0.001
position/fullsave/endturn/secondsave checks. Parent phase50 andGREEN52 both pass; initial
full624/625 failure retained, refined5/5 after freshALL/CI382,625 aggregate coverage
without a single625/625 claim. Linux590/590 complete. Game both/625 Windows/590 Linux
distinct related coverage/CI382, previous/new77 cases,71 prior fixed
rows/melee/godmode/fonts12 exact;12 LAN gates and361 encounter maps each platform pass.
Database unchanged. Other6face mission/route/deployment changes remain pending;29
unfinished.

Source: [6facee6dbea4189b0d817f37d4f62f9318212152](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6facee6dbea4189b0d817f37d4f62f9318212152).

## 2026-11-02: deferred outgoing subzone termination (152)

Within-zone passage transitions now finish the outgoing mission before constructing its
replacement. The old party is removed from the old world and its linked-zone snapshot is
written through existing native Terminate. This happens in the deferred main-loop command,
preserving the active world throughout UI/script processing. Real ETStore forward/reverse
and exact first-entry control verified, RED2/3 ->GREEN3 both. No AP/damage/price changes,
artificial frames/clock, save tags or LAN protocol. Existing zone/template, loading image,
bool behavior and prior constructor callers retained via one optional outgoing-mission
input.

Snapshot file creation is tested; snapshot reentry and historical/stale footprint cleanup
are separate. Game both/640 Windows/604 Linux related/CI382, prior/new80 cases and80 prior
fixed rows, melee/godmode/fonts12 exact, database unchanged.12 LAN gates/361 encounter
maps each platform pass. Source6face remains partial for route order/deployment
reservations;29 unfinished.

Source: [6facee6dbea4189b0d817f37d4f62f9318212152](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6facee6dbea4189b0d817f37d4f62f9318212152).

## 2026-11-02: authored unaware-critical parameters (153)

Restore original Steam perk74 arithmetic. Chance uses1+Param1 capped100; difficulty uses
original difficulty times Param2; both truncate. Authored original/baseline DB
parameters0.25/1.5 are identical. This corrects exaggerated critical input: native melee
difficulty4900->73 and11300->169; gun chance49->48/severity878->27. Ordinary damage bounds
and unconscious input stay unchanged, while the corrected critical inputs intentionally
affect later original injury calculations. No-perk/non-backstab controls remain exact. No
new damage rule, AP price, item data or LAN protocol.

Native canonical RED3/6 ->GREEN6 both with independent fixed literals, native perk-tree
acquisition and actual melee/gun CreateAttack. Queries preserve
RNG/AP/HP/pose/item/ammo/perk points; no fake combat/world reload or added frames/clock.
Game both/1377 Windows/1340 Linux related/CI382, prior/new86 cases and83 fixed
rows/melee/godmode/fonts12 exact;12 LAN gates/361 maps each pass, DB unchanged. Other884
healed health/depletion/KO changes remain pending;29 unfinished.

Source: [884f91f42db3aa840f729fb5196a18133d49917b](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/884f91f42db3aa840f729fb5196a18133d49917b).

## 2026-11-02: native health depletion and knockout probability (154)

Restore original Steam damage and knockout arithmetic. Bandaged health cannot become
negative; zero total health avoids NaN-to-int corruption; raw and bandaged damage shares
remain separate. Knockout probability uses post-hit total health/max health, original
clamped factor2.500000238418579 and rounding; above half health the ordinary probability
is0. Draws are1..99, removing a spurious zero-roll knockout at probability0. Nonpositive
total and click-of-death still force unconsciousness. This intentionally corrects injury
outcomes; no authored health, weapon, AP, difficulty or perk values are changed.

Actual native cap147/HealVP/death critical/party reserve and final17 unchanged tests give
canonical RED10/17 both ->GREEN17 both, including exact64YES/65NO at total36. Seven
controls exact. Original actual Steam blocks/helper identities independently audited, no
unsafe operand masks or original execution. Game both/1400 Windows/1363
Linux/CI382/prior103cases89fixedrows/melee/fonts12/12 LAN gates with361maps each pass, DB
identical to Steam. Entire source884 now imported;28 unfinished.

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

## 2026-11-02: world placement reservations and waypoint route order (155)

Restore original reservation scope and waypoint command ordering. New actors no longer
overlap units excluded by a temporary path query, placement restores all reservations, and
pending dead reservations use existing native eligibility cleanup. Existing own
anchor/floor and seven-cell diagonal footprint checks remain. Waypoint routes are
installed after synchronous teleport for AI or script sequences. No weapon, health, AP,
item, difficulty, resource or network protocol values change.

Native canonical RED4/7 ->GREEN7 both with three exact controls and all7 RNG tails
matching normal placement; four actual Lua route controls are exact seeded
parent/production, not a fabricated asynchronous bug. Game both/1411 Windows/1374
Linux/CI382/114cases106oldrows/XP-AI-melee-fonts exact;12 LAN gates/361 maps each, DB
unchanged. Source6face all5files now imported;27 unfinished.

Source: [6facee6dbea4189b0d817f37d4f62f9318212152](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/6facee6dbea4189b0d817f37d4f62f9318212152).

## 2026-11-02: explosive target groups and protected-unit clearance (156)

Restore original AI explosive-target decisions. Groups use live fightable world units,
strict1.5m proximity and original pre-insertion centers. Protect the thrower, allies and
neutral fightable units using the nearest linear distance: ordinary/engineer grenades
compare the unchanged authored range5 directly; rockets require more than3m. Existing AP
affordability, feasible trajectory and fallback checks remain. No item, blast radius,
damage, health, AP cost, difficulty or network command value changes. Own null-player and
older save fields remain supported.

Phased group RED5/7 ->GREEN7 and consumer RED3/8 ->combined15 both, with exact
controls/RNG. Full Game both/877 Windows/850 Linux plus14 supplemental melee cases
each/CI382,129 cases/117 old rows/13 tactical controls/12 fonts,12 LAN gates and361 maps
each pass; original static proof and all discovery limitations are recorded in
UPSTREAM-IMPORTS.md. Source069 all3files now imported;26 unfinished.

Source: [069451e7e61dc0c720812b2f939339f7732fdaf3](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/069451e7e61dc0c720812b2f939339f7732fdaf3).

## 157. Unit-aware waypoint and deployment placement

Restore original nearby placement for ordinary blocked script teleports and authored
deployment markers. Prepare standing pose before querying, require free connected tiles
within3m and rank them using native facing and source height. Keep panzerklein placement
with its existing full-footprint policy and route/command/same-point semantics. These
changes affect where obstructed units appear; no combat statistics, costs, equipment
records, save or network command fields change.

Native canonical RED3/8 ->GREEN8 both with five exact controls including empty/worn PKs.
Full Game both/single899 Windows872 Linux/CI382,137 focused cases/117 prior fixed rows/13
tactical controls/12 fonts and12 LAN gates/361 maps each pass. Original static proof,
distinct numeric literals, native blocked-deploy RNG change and archived invalid input are
detailed in UPSTREAM-IMPORTS.md. All9 source files accounted,25 unfinished.

Source: [a8311049890a7fa91a1a439a43fdfc2da9619a3e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/a8311049890a7fa91a1a439a43fdfc2da9619a3e).

The full/confirmation worn-PK setup HP varies137/157 on both platforms; each unchanged
native test proves before/after HP preservation. Canonical small five controls are exact;
full/confirmation raw HP inequality is retained explicitly, not normalized or called
equality. Extra unchanged8 confirmations each overlap899/872 totals. Exact setup cause
remains unproven; see UPSTREAM-IMPORTS.md.

## 158. Door-triggered forced relocation and animation timing

Restore nearby obstruction handling for ordinary units and start forced movement at the
current animation clock. The world receives the door grid notification after the real
flipper change; destination position is published before visibility refresh. Ordinary
search uses the native connected/facing/height criterion already restored in157. Keep the
existing large-unit candidate/footprint policy as an own adaptation and the existing
fall/death behavior. The original queues its grid event, whereas the established own world
adapter delivers synchronously; no original same-tick timing equivalence is claimed. No
combat values, costs, item records or save/network fields change.

Canonical native RED2/5 ->GREEN5 both with three exact controls; full Game both/single927
Windows900 Linux/CI382,142 focused cases/23 extra controls/117 prior fixed rows/12 fonts
and12 LAN gates/361 maps each pass.13 tactical views pass. Native input/bootstrap health
inequalities remain visible in reports; no value normalization. All3 source files now
accounted,24 unfinished. Full original proof and discovery limitations are recorded in
UPSTREAM-IMPORTS.md.

Source: [71bf7730c33bf37b19beaab21042f6a39e5b3285](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/71bf7730c33bf37b19beaab21042f6a39e5b3285).

The initial Linux transport-only NetworkSessionTests compound assertion failed once; its
exact cause remains unproven. Five LAB diagnostic replays did not reproduce it, and a
fresh unchanged full12-gate Linux run passes all361 maps. Initial logs are retained, no
repository/assertion/timeout edits or new exclusions, diagnostic probes not counted as LAN
gates. See UPSTREAM-IMPORTS.md.

## 159. Ordinary hover attack admission for world objects

Allow ordinary attack hover to enter its existing state for actual IObject targets,
including world props and doors, as the original type admission does. Preserve current
forced attack, own-unit restrictions, sound-marker handling, command feasibility and
caption policy. This changes target admission; no damage, hit chance, AP costs, item
records, save or network values change. No dead-panzerklein physics change is included.

Real native mission RED2/5 ->GREEN5 both, three exact control rows/all five RNG tails and
actor/target controls preserved. Full Game both/scoped UI558 Windows550 Linux/CI382,147
focused cases/117 previous fixed rows/12 fonts and12 LAN gates/361 maps each pass;13
tactical views pass. Full original bounds and unclaimed owner102/whole802 scope are
recorded in UPSTREAM-IMPORTS.md. UI1 of10 source files imported, remaining9 dead-PK files
pending;24 unfinished.

Source: [b3bd4703140eedb548126513d32cc16ad5c06b2a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b3bd4703140eedb548126513d32cc16ad5c06b2a).

An auxiliary old smooth-mode1 full-row comparison found one fewer RNG draw in Linux; all
physical/time/HP/AP fields matched. One unchanged-binary confirmation each returned the
previous tail, SHA fixed before/after. Full unequal RNG fields remain reported, not
normalized or called equal; exact cause unproven. These overlapping confirmations are not
added to558/550 counts; new hover5 RNG tails remain exact. See UPSTREAM-IMPORTS.md.

## 160. Preserve collapsed dead panzerkleins when their floor is destroyed

When a supporting floor disappears, a dead panzerklein wearer now drops vertically with
its collapsed pose preserved. Another unit's dynamic lock is treated as occupied physical
support, rather than a reason to slide the corpse sideways. The actual physical fall-ray
limit remains in force. Existing live-unit movement and ordinary corpse processing retain
their paths. This is an own compatibility correction around the original frozen-collapse
path; original CPlayerBase dead-event filtering is preserved because the current world
GRID adapter already delivers support changes to dead units.

The new path bypasses living movement callbacks and fall processing for dead wearers. It
changes corpse placement/animation and removes four old handler RNG draws in the isolated
long-fall test. This may affect the later RNG sequence when that bug is triggered; no
all-case paired RNG-equivalence claim is made. Six other paired tails remain exact, all
seven new operation queries consume none. No AP costs, damage/hit-chance formulas, item
records, save/wire fields or live-unit combat values change.

Final corrected native RED3/7 ->GREEN7 both with whole frozen pose and real floor/lock
witnesses. Four physical controls retained, job-budget iteration counts reported honestly
as raw inequalities. Full Game both/related1156 Windows1129 Linux/CI382/154 focused/117
old fixed rows/12 fonts/13 tactical views/12 LAN gates and361 maps each pass. Database
unchanged; one pre-existing nested-save exclusion. Full
original/static/runtime-scope/discovery details are in UPSTREAM-IMPORTS.md. Source b3bd470
is complete including159 UI:23 unfinished.

An auxiliary old door mode0 full-row comparison found one fewer RNG draw on Windows with
identical physical results. One unchanged-binary confirmation each returned the canonical
tail, with executable SHA fixed before/after. Raw inequality and unproven cause remain
reported, without normalization or extra test counts; see UPSTREAM-IMPORTS.md.

Source: [b3bd4703140eedb548126513d32cc16ad5c06b2a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b3bd4703140eedb548126513d32cc16ad5c06b2a).

## 161. Query the target hull for melee hit-chance previews

Melee unit previews now measure reachable body parts on the target rather than the
attacking unit. A real target with no body part within reach previously displayed100 in
the isolated query; it now returns the existing empty-set value0. Near target queries use
the correct three locations instead of six actor locations, while the tested ANY/head/body
percentages100/86/92 remain unchanged. Real attack execution and its chance/damage/AP
formulas retain their paths. Own first-turn/logging, Lua and LAN interfaces remain
unchanged.

This is a preview-input correction, not an authored value or combat-formula change. Final
native RED4/6 ->GREEN6 both, two complete scripted ranged controls and all six RNG tails
exact. Actor AP/HP, target HP, position/item/command/time controls pass. Full Game
both/scoped query663 Windows/655 Linux/CI382/160 focused/117 old rows/12 fonts/13 tactical
views/12 LAN gates and361 maps each pass. Database/save/wire fields unchanged; only the
existing nested-save exclusion. Raw prior setup/job/RNG inequalities are retained if seen;
full original/runtime limitations are in UPSTREAM-IMPORTS.md. Remaining source fcc994bc
directed attack poses/inactive door turns are still pending,23 unfinished.

Source: [fcc994bc87c9dad2c90e6c3fd4288bbce281b094](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fcc994bc87c9dad2c90e6c3fd4288bbce281b094).

## 162. Preserve the current pose in directed attack planning

Directed tile and object attacks retain the actor's current pose while selecting a facing.
A blocked prone facing no longer creates an automatic crouch movement prefix. Six valid
standing/crouching/prone controls retain their full results and AP start11; the rejected
prefix formerly reported4, which was preparation cost rather than total shot AP. Weapon,
damage, chance, movement cost and authored database values remain unchanged.

Final native RED2/8 ->unchanged GREEN8 both; all eight physical witnesses and RNG tails
exact, HP/AP/ammo/position/item/command/time preserved. Full Game both/scoped768
Windows758 Linux/CI382/168 focused/117 old rows/13 tactical/12 LAN gates and361 maps each
pass. Prior raw setup/job/RNG inequalities remain explicit if seen. Save/wire schema
unchanged. Original bounded limits appear in UPSTREAM-IMPORTS.md; source inactive door
turn remains pending,23 unfinished.

Source: [fcc994bc87c9dad2c90e6c3fd4288bbce281b094](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fcc994bc87c9dad2c90e6c3fd4288bbce281b094).

## 163. Skip ordinary door turns at inactive path points

Inactive movement points no longer receive an ordinary turn before a queued door action.
The native ladder fixture previously inserted a heading1 point that the physical network
rejected; it now retains heading6. A real standing door approach still turns6->7. This is
a queue-preparation correction; authored values and combat/AP formulas remain unchanged,
and these focused tests do not execute movement or open/close the door.

Final native RED1/3 ->unchanged GREEN3 both, two complete controls/all three RNG tails/six
input witnesses exact; actual HP/AP/position/time/door state retained. Full Game
both/scoped771 Windows761 Linux/CI382/171 focused/117 old rows/13 tactical/12 LAN gates
and361 variants each pass. Prior raw native job/setup/RNG inequalities remain explicit.
Save/wire schema and baseline data unchanged. Lower-level requested-action and original
static proof limits are in UPSTREAM-IMPORTS.md. Source fcc994bc complete,22 unfinished
donor sources remain.

Source: [fcc994bc87c9dad2c90e6c3fd4288bbce281b094](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/fcc994bc87c9dad2c90e6c3fd4288bbce281b094).

## 164. Measure tile-shot distance from the actual cover origin

Tile/non-unit shot chance now measures range from the actual traced shot origin instead of
the unit's feet; the calculator also receives that same origin. Real day-controlled native
inputs correct standing62->54 and crouched55->63. Equal-origin and blocked-cover controls
stay62 and0. Existing chance, damage, AP and weapon formulas and authored values are
unchanged; corrected geometric input can change the resulting percentage. Point-only
arithmetic changes are not claimed from the current constant-area tile calculator. The
legacy direct tile preview dispatcher remains a separate pending317 hunk.

Final native RED3/5 ->unchanged GREEN5 both, two complete query/integration controls and
all five zero-draw query/one-draw roll78 tails exact. HP/AP/ammo/position/item/time/day
preserved; integration uses CheckTileToHit but does not fire a projectile or cause damage.
Full Game both/scoped1048 Windows1038 Linux/CI382/176 focused/117 old rows/13 tactical/12
LAN gates and361 maps each pass. Raw previous native setup/job/RNG differences remain
explicit. Linux initial disconnect and separate original compound-event failures are
retained; final LAN gates pass after one complete unchanged retry, with unchanged
binaries/library and no test/assertion change; cause unproven. Save/wire and baseline data
unchanged. Source317 is partial;22 unfinished sources remain. Original proof and fixture
discovery limits are in UPSTREAM-IMPORTS.md.

Source: [31725f2d03de84b3f3ec1972cc521db60eb5428e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/31725f2d03de84b3f3ec1972cc521db60eb5428e).

## 165. Return the unavailable sentinel for unreachable melee previews

Unreachable current-pose unit melee preview returns-1 rather than0 to both Lua and network
consumers, matching the original unavailable contract and existing AP-only cursor display.
Reach is measured to native target HL_ANY center independently of called shot.
Authoritative replica delegation stays first; formulas, AP, damage, helper, save and
packet formats remain unchanged. Four native game+Lua cases, actual signed TCP
reply/cache, old hull/caption transitions and ten complete controls establish the
behavior. Original Lua arg3 is HL, while our existing argument API remains retained in
this narrow change. Canonical RED7/17 ->GREEN17 both;
full1058W1048L/186focused/CI382/12LANeach361 pass; measured pure query RNG tails exact.
Sourcef645 closes;21 unfinished sources remain. Proof bounds and raw legacy inequalities
are in UPSTREAM-IMPORTS.md.

Source: [f6455c9f92eb6d4c66efebc74face61d1e1065bb](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f6455c9f92eb6d4c66efebc74face61d1e1065bb).

## 166. Use directed shot origins and per-bullet tile previews

Tile shot previews now reject a pose that cannot face its commanded point, cast from that
facing's actual origin and average existing per-bullet chances. Native
examples77->-1,69->62,63->70, short68->53 and long72->58; controls69/0 remain unchanged.
HP/AP/ammo/item/position/time and query RNG are preserved, with no executed shot, damage,
action or packet change. Seven real native cases RED5->GREEN7 both; Game503 targeted
each/CI382/12LANeach361 pass. Raw cross-platform positive-cover inequalities are retained
with unchanged-binary confirmations, not normalized; cause unproven and no full cover
parity claim. Firstturn/logging/generalobject/grenade-rocket/UI-Lua routing remains
pending317;21 unfinished sources. Original proof bounds and exact scope are in
UPSTREAM-IMPORTS.md.

Source: [31725f2d03de84b3f3ec1972cc521db60eb5428e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/31725f2d03de84b3f3ec1972cc521db60eb5428e).

## 167. Give previews native unit turn state and correct Lua hit locations

Unit/tile preview percentages now use the attacking unit's native first turn,
independently of query logging: controlled unit75(first)/84(later), tile76/84. Real Lua
numeric argument selects hit location: defaultANY/legs86->100, body86->92,
head/hands/invalidHEAD86 controls. Existing waypoint numeric compatibility argument is
accepted and ignored. HP/AP/ammo/item/position/time and query RNG remain unchanged.
Existing network FIRST field retains its role and layout; four native-counter requests
corrected without new fields. Native RED9/53 ->GREEN53 both; Game521 scoped
each/CI382/12LANeach361 pass. No action, damage, database or save-format change. General
object/grenade-rocket/full command routing remains pending317;21unfinished sources.
Initial Linux disconnect/other diagnostic failures remain recorded; original
executable/library hashes unchanged and one full unchanged LAN retry passes, cause
unproven. Original proof bounds and fixture corrections are recorded in
UPSTREAM-IMPORTS.md.

Source: [31725f2d03de84b3f3ec1972cc521db60eb5428e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/31725f2d03de84b3f3ec1972cc521db60eb5428e).

## 168. Complete command-based attack previews for objects, grenades and rockets

Attack preview routing now follows the actual object/tile command. Prop90->84, door0->16;
grenade tile0->100/unit0->70; real rocket cover coefficient1 retains clear55 and changes
weak-obstacle tile7->0. Positive fractional rocket cover remains positive through the
retained integer adapter; no all-type float/formula rewrite. Prone nonunit attacks reject
native impossible-facing1/2 with-1/AP-only7, clear18 unchanged. Actual TCP/UI retains
fields, IDs, actor ownership, Script37 and per-query purity. Native phaseA
RED9/64->GREEN64, phaseB RED1/2->final GREEN66 both;
HP/AP/ammo/quantity/position/time/query RNG exact. No database/action/damage/save format
change. Fresh Gameboth546/CI382/LAN12each361 pass; initial interim full544 failures and
unequal native cover inputs are retained with unproven causes. Source317 complete
after164/166/167/168,20 unfinished; f638 unit pose/selector and combat/contact remain
separate.

Source: [31725f2d03de84b3f3ec1972cc521db60eb5428e](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/31725f2d03de84b3f3ec1972cc521db60eb5428e). Dependencies only: [f638aca9a410eef82afb20e926c97716dc448a90](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f638aca9a410eef82afb20e926c97716dc448a90).

## 169. Restore native unit composite preview callers

Unit composite previews now check the current pose facing the actual hull center, reject
native impossible direction1/2 with-1, and use the adjusted ranged origin. Existing
hit-location selection is passed through even when it reports false; no new calcer
formula. Blocked prone Script37->-1, clear standing/crouched/prone37 unchanged; empty-list
game/Lua0->37 with nonempty37 control. Geometric reach is not a claim of full melee order
legality. HP/AP/ammo/quantity/position/time/query RNG unchanged. Parent stages RED1/70 and
extraRED1/2 ->final GREEN78 both; Game218 each/CI382/LAN12each361 pass. No damage, IC
reduction, bullet, packet, action, database or save-format implementation change. All
legacy hull observer assertions are retained via public real-world-map forwarding,
including six prior rows; initial Win218 observer failures archived, no production change
for observer. Sourcef638 cached-hit fallback remains pending;226 imported/13 pending/7
partial/20 unfinished.

Source: [f638aca9a410eef82afb20e926c97716dc448a90](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/f638aca9a410eef82afb20e926c97716dc448a90).
