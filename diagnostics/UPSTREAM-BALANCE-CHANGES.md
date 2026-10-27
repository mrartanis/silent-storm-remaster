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
полезна; реализацию нужно строить на исходной retail формуле, не переносить
скидку и минимум из чужой текущей версии.

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
| [2de89e04](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/2de89e04ae2713bd7fbd267b5b3a96b60feb810d) Restore incidental bullet impacts and periodic integer bleeding | Попадания по случайным целям, ночная точность и кровотечение меняют боевые расчёты; нужен oracle. |
| [5780c0be](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/5780c0be34f000c316e1239cd749ae96ccbea027) Restore retail throwing knife targeting, flight and facing | Метательные ножи: штатная дальность, скорость, попадания, направление. Это восстановление формул, не готовый безопасный cherry-pick. |
| [c1ab9336](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/c1ab93366661d3a4e25a0effe62625bae92c2d0c) Restore retail melee damage dispatch and armed attack cursor | Native UI-различение кулака/экипированного оружия адаптировано в GetState; action dispatch и critical bans сохранены. Target-filter/damage dispatch остаётся для проверки реальных попаданий и original damage. |
| [7f4f0ea8](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/7f4f0ea872937e4dcb4a887ff6a136d7dce1fcd5) Match retail nearest AI contact selection | Nearest-contact selection использует другие позиции/фильтры; отдельно сверять поведение AI. |
| [98af43bd](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/98af43bdaf252e5a2eac1ad047c92511adb8e565) Restore retail global-game difficulty initialization | Передача выбранной сложности и default difficulty требуют сверки; не менять default незаметно. |
| [cc0fd988](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/cc0fd9888357c7a9f5d47fec2c22f3073eb5e7cf) Match retail AI firing range and target reach checks | AI range/target-reached/attack-origin — восстановление retail боевого trace, нужны сравнительные тесты. |
| [66fc9e9a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/66fc9e9ad26497bf41f96a1a8c51c8a03e951718) Match retail AI snipe weapon and cover evaluation | AI snipe должен считать выбранным оружием и не терять дробную cover precision. |
| [0ffb568d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/0ffb568d67bc6ca389d7ca63a9da4f3978c801a7) Restore retail level XP persistence and fresh skill maximum flags | Повторная native-проверка нового репозитория выявила потерю общего XP в SetXPLevel; сохранение существующего расчёта исправляется отдельно. FreezeToMax для новых skills независимо сверён с original Steam EXE и перенесён: см. fresh-skill-modifier-values в UPSTREAM-IMPORTS.md. |
| [08c4bb5a](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/08c4bb5a6b9a94337d88bd8241a2e51b6984cfce) Restore retail weapon preparation AP costs | AP подготовки оружия из weapon type/перков вместо stub; сверять original, не считать автоматически ребалансом. |
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
| [b875e43d](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/b875e43ddec65e0375d1335eb8d8726e38b4934a) Use perk-adjusted selected-weapon cost for AI reload actions | AI reload должен использовать стоимость выбранного оружия с теми же perks; AP формулы проверить original и согласовать с игроком. |
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

## Распределение повреждений PK и пилота — `432b64c8`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/432b64c8bbe97a9f347a6c9468e7c69173d68394)
смешивает независимый crash отступления AI, интерфейс и боевые расчёты.
Crash после замены реакции на Guard перенесён отдельно. Следующие PK-hunks
пока не перенесены без проверки соответствия оригиналу:

- Вместо вероятности критического попадания в пилота, умноженной на
  fCriticalResist, используется (1 − fCriticalResist). Меняются как вероятность,
  так и масштаб min/max повреждения пилота; безусловное значение critical=2000
  заменяется рассчитанным значением.
- После повреждения корпуса остаток пробития уменьшается на фиксированные 1590
  с нижней границей 0. Он определяет продолжение обработки человеческого
  повреждения. Это заменяет прежнюю обработку material threshold и критики.
- Возвращается отдельная классификация отказа/пилота/корпуса CReceivedDmg;
  продолжение человеческого damage зависит от RD_HUMAN. Добавляются ветки
  GodMode, bypass PK, живого корпуса и обнуление отрицательного PK VP.

Upstream помечает это восстановлением retail, а не новым балансом. До native
сравнения с оригиналом одних комментариев недостаточно для переноса расчётов.
Локализованный текст hit labels перенесён отдельным native HUD-тестом; цвета
и fade сохранены. Цветовые изменения и group Hide требуют отдельных проверок
и не являются частью исправления времени жизни реакции.

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

## Разрешения сохранения и выбор сложности — оставшиеся части `02d85e5b`

[Исходный коммит](https://github.com/met-nikita/Silent-Storm-Reconstruction/commit/02d85e5b939ac897f726293b37f30b55d0f2ff72)
смешивает применение готового permission в интерфейсе и изменение вычисления
самого permission. Независимый UI-фикс кнопки Save переносится с native RED/GREEN;
он не меняет, когда миссия разрешает сохраняться. Следующие правила пока не перенесены:

- Пересчёт bCanSave на каждом игровом шаге. HQ всегда разрешает Save, режим
  E_CS_CHAPTER запрещает его в тактической миссии, E_CS_REALTIME допускает только
  real-time вне sequence, E_CS_ALWAYS разрешает всегда. Это влияет на доступность
  сохранений по сложности и режиму; нужно подтвердить соответствие оригиналу
  и текущим сценарным ограничениям, прежде чем менять вычисление флага.
- Замена прямого difficulty enum -> DB id на EASY=0 -> DB1, HARD=1 -> DB3,
  NORMAL=2 -> DB2. Коэффициенты DB сами по себе не переписываются, но в двух
  позициях меняется выбранная запись, а значит применённая сложность. Требуется
  сверка видимых подписей с эталоном retail и текущими записями нашего форка;
  менять default или фактическую сложность незаметно нельзя.

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
