#!/usr/bin/env python3
"""gen_i18n.py - writes settings_app/storageturbo_i18n_table.h from the table below.

Columns follow the Vita system-language order (TS_LANG_*). English (US/UK) is
authoritative; the others are machine translations, open to correction.
Run from the repo root:  python3 tools/gen_i18n.py
"""
LANGS = ["ja", "en", "fr", "es", "de", "it", "nl", "pt", "ru", "ko", "zht", "zhs",
         "fi", "sv", "da", "no", "pl", "ptbr", "engb", "tr"]

T = {}
def s(id_, **kw):
    T[id_] = kw

s("TS_I18N_TITLE", **{l: "Turbo Storage" for l in LANGS})
s("TS_I18N_ROW_SD2VITA", en="Faster SD2Vita", engb="Faster SD2Vita", ja="SD2Vitaを高速化",
  fr="SD2Vita plus rapide", es="SD2Vita más rápido", de="Schnellere SD2Vita", it="SD2Vita più veloce",
  nl="Snellere SD2Vita", pt="SD2Vita mais rápido", ptbr="SD2Vita mais rápido", ru="Ускорить SD2Vita",
  ko="SD2Vita 가속", zht="加速 SD2Vita", zhs="加速 SD2Vita", fi="Nopeampi SD2Vita",
  sv="Snabbare SD2Vita", da="Hurtigere SD2Vita", no="Raskere SD2Vita", pl="Szybsze SD2Vita",
  tr="Daha hızlı SD2Vita")
s("TS_I18N_ROW_INTERNAL", en="Faster internal storage", engb="Faster internal storage",
  ja="本体ストレージを高速化", fr="Stockage interne plus rapide", es="Almacenamiento interno más rápido",
  de="Schnellerer interner Speicher", it="Memoria interna più veloce", nl="Snellere interne opslag",
  pt="Armazenamento interno mais rápido", ptbr="Armazenamento interno mais rápido",
  ru="Ускорить встроенную память", ko="내장 저장소 가속", zht="加速內建儲存空間", zhs="加速内置存储",
  fi="Nopeampi sisäinen tallennustila", sv="Snabbare intern lagring", da="Hurtigere intern lagring",
  no="Raskere intern lagring", pl="Szybsza pamięć wewnętrzna", tr="Daha hızlı dahili depolama")
s("TS_I18N_ROW_HUD", en="Storage HUD", engb="Storage HUD", ja="ストレージHUD", fr="HUD de stockage",
  es="HUD de almacenamiento", de="Speicher-HUD", it="HUD memoria", nl="Opslag-HUD", pt="HUD de armazenamento",
  ptbr="HUD de armazenamento", ru="HUD памяти", ko="저장소 HUD", zht="儲存 HUD", zhs="存储 HUD",
  fi="Tallennus-HUD", sv="Lagrings-HUD", da="Lager-HUD", no="Lagrings-HUD", pl="HUD pamięci",
  tr="Depolama HUD")
s("TS_I18N_ROW_HUD_POS", en="HUD position", engb="HUD position", ja="HUDの位置", fr="Position du HUD",
  es="Posición del HUD", de="HUD-Position", it="Posizione HUD", nl="HUD-positie", pt="Posição do HUD",
  ptbr="Posição do HUD", ru="Положение HUD", ko="HUD 위치", zht="HUD 位置", zhs="HUD 位置",
  fi="HUD:n sijainti", sv="HUD-position", da="HUD-placering", no="HUD-plassering", pl="Położenie HUD",
  tr="HUD konumu")
s("TS_I18N_ROW_DEVMODE", en="Developer mode", engb="Developer mode", ja="開発者モード",
  fr="Mode développeur", es="Modo desarrollador", de="Entwicklermodus", it="Modalità sviluppatore",
  nl="Ontwikkelaarsmodus", pt="Modo de programador", ptbr="Modo de desenvolvedor", ru="Режим разработчика",
  ko="개발자 모드", zht="開發者模式", zhs="开发者模式", fi="Kehittäjätila", sv="Utvecklarläge",
  da="Udviklertilstand", no="Utviklermodus", pl="Tryb programisty", tr="Geliştirici modu")
s("TS_I18N_HINT_SD2VITA",
  en="Speeds up reading and writing a microSD card in an SD2Vita adapter. Your files and how they are stored do not change. Turn off if you suspect a problem. Takes effect immediately; no reboot needed.",
  engb="Speeds up reading and writing a microSD card in an SD2Vita adapter. Your files and how they are stored do not change. Turn off if you suspect a problem. Takes effect immediately; no reboot needed.",
  ja="SD2Vitaアダプターに入れたmicroSDカードの読み書きを高速化します。ファイルや保存方法は変わりません。問題が疑われる場合はオフにしてください。すぐに反映され、再起動は不要です。",
  fr="Accélère la lecture et l'écriture d'une carte microSD dans un adaptateur SD2Vita. Vos fichiers et leur mode d'enregistrement ne changent pas. Désactivez en cas de doute. Prend effet immédiatement, sans redémarrage.",
  es="Acelera la lectura y escritura de una tarjeta microSD en un adaptador SD2Vita. Tus archivos y la forma de guardarlos no cambian. Desactívalo si sospechas un problema. Se aplica al instante; no hace falta reiniciar.",
  de="Beschleunigt Lesen und Schreiben einer microSD-Karte in einem SD2Vita-Adapter. Deine Dateien und ihre Speicherung bleiben unverändert. Bei Verdacht auf ein Problem ausschalten. Wirkt sofort, ohne Neustart.",
  it="Velocizza la lettura e la scrittura di una microSD in un adattatore SD2Vita. I file e il modo in cui sono salvati non cambiano. Disattiva se sospetti un problema. Ha effetto subito, senza riavviare.",
  nl="Versnelt het lezen en schrijven van een microSD-kaart in een SD2Vita-adapter. Je bestanden en de manier van opslaan veranderen niet. Zet uit als je een probleem vermoedt. Werkt meteen; opnieuw opstarten is niet nodig.",
  pt="Acelera a leitura e a escrita de um cartão microSD num adaptador SD2Vita. Os seus ficheiros e a forma como são guardados não mudam. Desligue se suspeitar de um problema. Tem efeito imediato; não é preciso reiniciar.",
  ptbr="Acelera a leitura e a gravação de um cartão microSD em um adaptador SD2Vita. Seus arquivos e a forma como são salvos não mudam. Desligue se suspeitar de um problema. Tem efeito imediato; não precisa reiniciar.",
  ru="Ускоряет чтение и запись карты microSD в адаптере SD2Vita. Ваши файлы и способ их хранения не меняются. Выключите, если подозреваете проблему. Действует сразу, перезагрузка не нужна.",
  ko="SD2Vita 어댑터에 넣은 microSD 카드의 읽기와 쓰기를 빠르게 합니다. 파일과 저장 방식은 바뀌지 않습니다. 문제가 의심되면 끄세요. 즉시 적용되며 재시작할 필요가 없습니다.",
  zht="加快 SD2Vita 轉接器中 microSD 記憶卡的讀寫速度。檔案及其儲存方式不會改變。若懷疑有問題，請關閉。立即生效，無需重新啟動。",
  zhs="加快 SD2Vita 转接器中 microSD 存储卡的读写速度。文件及其存储方式不会改变。若怀疑有问题，请关闭。立即生效，无需重启。",
  fi="Nopeuttaa SD2Vita-sovittimessa olevan microSD-kortin lukemista ja kirjoittamista. Tiedostosi ja niiden tallennustapa eivät muutu. Kytke pois, jos epäilet ongelmaa. Tulee voimaan heti; uudelleenkäynnistystä ei tarvita.",
  sv="Snabbar upp läsning och skrivning på ett microSD-kort i en SD2Vita-adapter. Dina filer och hur de lagras ändras inte. Stäng av om du misstänker ett problem. Gäller direkt; ingen omstart behövs.",
  da="Gør læsning og skrivning på et microSD-kort i en SD2Vita-adapter hurtigere. Dine filer og måden, de gemmes på, ændres ikke. Slå fra, hvis du har mistanke om et problem. Træder i kraft med det samme; ingen genstart nødvendig.",
  no="Gjør lesing og skriving på et microSD-kort i en SD2Vita-adapter raskere. Filene dine og måten de lagres på endres ikke. Slå av hvis du mistenker et problem. Trer i kraft umiddelbart; ingen omstart nødvendig.",
  pl="Przyspiesza odczyt i zapis karty microSD w adapterze SD2Vita. Twoje pliki i sposób ich zapisu się nie zmieniają. Wyłącz, jeśli podejrzewasz problem. Działa od razu, bez ponownego uruchamiania.",
  tr="SD2Vita adaptöründeki microSD kartın okuma ve yazma hızını artırır. Dosyalarınız ve saklanma biçimleri değişmez. Bir sorundan şüphelenirseniz kapatın. Hemen etkili olur; yeniden başlatma gerekmez.")
s("TS_I18N_HINT_INTERNAL",
  en="Speeds up reading and writing the console's built-in storage. Off by default as a precaution, because this storage also holds system data and plugin settings. As an extra precaution, turn it off before updating or downgrading the firmware. Takes effect immediately; no reboot needed.",
  engb="Speeds up reading and writing the console's built-in storage. Off by default as a precaution, because this storage also holds system data and plugin settings. As an extra precaution, turn it off before updating or downgrading the firmware. Takes effect immediately; no reboot needed.",
  ja="本体内蔵ストレージの読み書きを高速化します。このストレージにはシステムデータやプラグインの設定も保存されているため、念のため初期設定はオフです。念のため、ファームウェアのアップデートやダウングレードの前にはオフにしてください。すぐに反映され、再起動は不要です。",
  fr="Accélère la lecture et l'écriture du stockage intégré de la console. Désactivé par défaut par précaution, car ce stockage contient aussi des données système et les réglages des plugins. Par précaution supplémentaire, désactivez-le avant de mettre à jour ou de rétrograder le firmware. Prend effet immédiatement, sans redémarrage.",
  es="Acelera la lectura y escritura del almacenamiento integrado de la consola. Desactivado por defecto como precaución, porque este almacenamiento también guarda datos del sistema y la configuración de los plugins. Como precaución adicional, desactívalo antes de actualizar o volver a una versión anterior del firmware. Se aplica al instante; no hace falta reiniciar.",
  de="Beschleunigt Lesen und Schreiben des eingebauten Speichers der Konsole. Vorsichtshalber standardmäßig aus, da dieser Speicher auch Systemdaten und Plugin-Einstellungen enthält. Schalte es zur zusätzlichen Sicherheit aus, bevor du die Firmware aktualisierst oder downgradest. Wirkt sofort, ohne Neustart.",
  it="Velocizza la lettura e la scrittura della memoria integrata della console. Disattivato per impostazione predefinita per precauzione, perché questa memoria contiene anche dati di sistema e impostazioni dei plugin. Come ulteriore precauzione, disattivalo prima di aggiornare o fare il downgrade del firmware. Ha effetto subito, senza riavviare.",
  nl="Versnelt het lezen en schrijven van de ingebouwde opslag van de console. Standaard uit uit voorzorg, omdat deze opslag ook systeemgegevens en plugininstellingen bevat. Zet het voor de zekerheid uit voordat je de firmware bijwerkt of downgradet. Werkt meteen; opnieuw opstarten is niet nodig.",
  pt="Acelera a leitura e a escrita do armazenamento integrado da consola. Desligado por predefinição por precaução, porque este armazenamento também guarda dados do sistema e definições de plugins. Como precaução adicional, desligue-o antes de atualizar ou reverter o firmware. Tem efeito imediato; não é preciso reiniciar.",
  ptbr="Acelera a leitura e a gravação do armazenamento integrado do console. Desligado por padrão por precaução, porque esse armazenamento também guarda dados do sistema e configurações de plugins. Como precaução extra, desligue antes de atualizar ou fazer downgrade do firmware. Tem efeito imediato; não precisa reiniciar.",
  ru="Ускоряет чтение и запись встроенной памяти консоли. По умолчанию выключено из предосторожности: в этой памяти также хранятся системные данные и настройки плагинов. В качестве дополнительной меры предосторожности выключайте его перед обновлением или понижением версии прошивки. Действует сразу, перезагрузка не нужна.",
  ko="본체 내장 저장소의 읽기와 쓰기를 빠르게 합니다. 이 저장소에는 시스템 데이터와 플러그인 설정도 있으므로 만일을 위해 기본값은 꺼짐입니다. 추가 예방 조치로, 펌웨어를 업데이트하거나 다운그레이드하기 전에 끄세요. 즉시 적용되며 재시작할 필요가 없습니다.",
  zht="加快主機內建儲存空間的讀寫速度。由於此儲存空間也存放系統資料與外掛程式設定，為求謹慎，預設為關閉。為求更加謹慎，更新或降級韌體前請先關閉。立即生效，無需重新啟動。",
  zhs="加快主机内置存储的读写速度。由于该存储还保存系统数据和插件设置，为谨慎起见，默认关闭。为进一步谨慎起见，更新或降级固件前请先关闭。立即生效，无需重启。",
  fi="Nopeuttaa konsolin sisäisen tallennustilan lukemista ja kirjoittamista. Oletuksena pois päältä varotoimena, koska tässä tallennustilassa on myös järjestelmän tietoja ja liitännäisten asetuksia. Lisävarotoimena kytke se pois ennen laiteohjelmiston päivittämistä tai alentamista. Tulee voimaan heti; uudelleenkäynnistystä ei tarvita.",
  sv="Snabbar upp läsning och skrivning på konsolens inbyggda lagring. Av som standard av försiktighetsskäl, eftersom lagringen även innehåller systemdata och plugininställningar. Som en extra försiktighetsåtgärd, stäng av det innan du uppdaterar eller nedgraderar firmwaren. Gäller direkt; ingen omstart behövs.",
  da="Gør læsning og skrivning på konsollens indbyggede lager hurtigere. Slået fra som standard af forsigtighed, fordi lageret også rummer systemdata og plugin-indstillinger. Som en ekstra forholdsregel skal du slå det fra, før du opdaterer eller nedgraderer firmwaren. Træder i kraft med det samme; ingen genstart nødvendig.",
  no="Gjør lesing og skriving på konsollens innebygde lagring raskere. Av som standard av forsiktighet, fordi lagringen også inneholder systemdata og plugininnstillinger. Som et ekstra forholdsregel bør du slå det av før du oppdaterer eller nedgraderer fastvaren. Trer i kraft umiddelbart; ingen omstart nødvendig.",
  pl="Przyspiesza odczyt i zapis wbudowanej pamięci konsoli. Domyślnie wyłączone dla bezpieczeństwa, ponieważ ta pamięć przechowuje też dane systemowe i ustawienia wtyczek. Dla dodatkowego bezpieczeństwa wyłącz je przed aktualizacją lub obniżeniem wersji oprogramowania. Działa od razu, bez ponownego uruchamiania.",
  tr="Konsolun dahili depolamasının okuma ve yazma hızını artırır. Bu depolama sistem verilerini ve eklenti ayarlarını da tuttuğu için önlem olarak varsayılan olarak kapalıdır. Ek bir önlem olarak, yazılımı güncellemeden veya eski sürüme düşürmeden önce kapatın. Hemen etkili olur; yeniden başlatma gerekmez.")
s("TS_I18N_ROW_MEMCARD", en="Faster Vita memory card", engb="Faster Vita memory card", ja="Vitaメモリーカードを高速化",
  fr="Carte mémoire Vita plus rapide", es="Tarjeta de memoria Vita más rápida", de="Schnellere Vita-Speicherkarte",
  it="Scheda di memoria Vita più veloce", nl="Snellere Vita-geheugenkaart", pt="Cartão de memória Vita mais rápido",
  ptbr="Cartão de memória Vita mais rápido", ru="Ускорить карту памяти Vita", ko="Vita 메모리 카드 가속",
  zht="加速 Vita 記憶卡", zhs="加速 Vita 存储卡", fi="Nopeampi Vita-muistikortti", sv="Snabbare Vita-minneskort",
  da="Hurtigere Vita-hukommelseskort", no="Raskere Vita-minnekort", pl="Szybsza karta pamięci Vita", tr="Daha hızlı Vita hafıza kartı")
s("TS_I18N_HINT_MEMCARD",
  en="Speeds up reading and writing the official PS Vita memory card. Your files and how they are stored do not change. Turn off if you suspect a problem. Takes effect immediately; no reboot needed.",
  engb="Speeds up reading and writing the official PS Vita memory card. Your files and how they are stored do not change. Turn off if you suspect a problem. Takes effect immediately; no reboot needed.",
  ja="PS Vita専用メモリーカードの読み書きを高速化します。ファイルや保存方法は変わりません。問題が疑われる場合はオフにしてください。すぐに反映され、再起動は不要です。",
  fr="Accélère la lecture et l'écriture de la carte mémoire officielle PS Vita. Vos fichiers et leur mode d'enregistrement ne changent pas. Désactivez en cas de doute. Prend effet immédiatement, sans redémarrage.",
  es="Acelera la lectura y escritura de la tarjeta de memoria oficial de PS Vita. Tus archivos y la forma de guardarlos no cambian. Desactívalo si sospechas un problema. Se aplica al instante; no hace falta reiniciar.",
  de="Beschleunigt Lesen und Schreiben der offiziellen PS Vita-Speicherkarte. Deine Dateien und ihre Speicherung bleiben unverändert. Bei Verdacht auf ein Problem ausschalten. Wirkt sofort, ohne Neustart.",
  it="Velocizza la lettura e la scrittura della scheda di memoria ufficiale PS Vita. I file e il modo in cui sono salvati non cambiano. Disattiva se sospetti un problema. Ha effetto subito, senza riavviare.",
  nl="Versnelt het lezen en schrijven van de officiële PS Vita-geheugenkaart. Je bestanden en de manier van opslaan veranderen niet. Zet uit als je een probleem vermoedt. Werkt meteen; opnieuw opstarten is niet nodig.",
  pt="Acelera a leitura e a escrita do cartão de memória oficial da PS Vita. Os seus ficheiros e a forma como são guardados não mudam. Desligue se suspeitar de um problema. Tem efeito imediato; não é preciso reiniciar.",
  ptbr="Acelera a leitura e a gravação do cartão de memória oficial do PS Vita. Seus arquivos e a forma como são salvos não mudam. Desligue se suspeitar de um problema. Tem efeito imediato; não precisa reiniciar.",
  ru="Ускоряет чтение и запись официальной карты памяти PS Vita. Ваши файлы и способ их хранения не меняются. Выключите, если подозреваете проблему. Действует сразу, перезагрузка не нужна.",
  ko="정품 PS Vita 메모리 카드의 읽기와 쓰기를 빠르게 합니다. 파일과 저장 방식은 바뀌지 않습니다. 문제가 의심되면 끄세요. 즉시 적용되며 재시작할 필요가 없습니다.",
  zht="加快 PS Vita 專用記憶卡的讀寫速度。檔案及其儲存方式不會改變。若懷疑有問題，請關閉。立即生效，無需重新啟動。",
  zhs="加快 PS Vita 专用存储卡的读写速度。文件及其存储方式不会改变。若怀疑有问题，请关闭。立即生效，无需重启。",
  fi="Nopeuttaa virallisen PS Vita -muistikortin lukemista ja kirjoittamista. Tiedostosi ja niiden tallennustapa eivät muutu. Kytke pois, jos epäilet ongelmaa. Tulee voimaan heti; uudelleenkäynnistystä ei tarvita.",
  sv="Snabbar upp läsning och skrivning på det officiella PS Vita-minneskortet. Dina filer och hur de lagras ändras inte. Stäng av om du misstänker ett problem. Gäller direkt; ingen omstart behövs.",
  da="Gør læsning og skrivning på det officielle PS Vita-hukommelseskort hurtigere. Dine filer og måden, de gemmes på, ændres ikke. Slå fra, hvis du har mistanke om et problem. Træder i kraft med det samme; ingen genstart nødvendig.",
  no="Gjør lesing og skriving på det offisielle PS Vita-minnekortet raskere. Filene dine og måten de lagres på endres ikke. Slå av hvis du mistenker et problem. Trer i kraft umiddelbart; ingen omstart nødvendig.",
  pl="Przyspiesza odczyt i zapis oficjalnej karty pamięci PS Vita. Twoje pliki i sposób ich zapisu się nie zmieniają. Wyłącz, jeśli podejrzewasz problem. Działa od razu, bez ponownego uruchamiania.",
  tr="Resmi PS Vita hafıza kartının okuma ve yazma hızını artırır. Dosyalarınız ve saklanma biçimleri değişmez. Bir sorundan şüphelenirseniz kapatın. Hemen etkili olur; yeniden başlatma gerekmez.")
s("TS_I18N_ST_SLOW",
  en="Note: the system's slow mode for memory cards is on, which deliberately slows every memory card transfer.",
  engb="Note: the system's slow mode for memory cards is on, which deliberately slows every memory card transfer.",
  ja="注意：システムのメモリーカード低速モードがオンになっており、すべての転送が意図的に遅くなっています。",
  fr="Remarque : le mode lent de la carte mémoire du système est activé ; il ralentit volontairement chaque transfert.",
  es="Nota: el modo lento de la tarjeta de memoria del sistema está activado y ralentiza a propósito cada transferencia.",
  de="Hinweis: Der Langsam-Modus des Systems für Speicherkarten ist aktiv und bremst absichtlich jede Übertragung.",
  it="Nota: la modalità lenta della scheda di memoria del sistema è attiva e rallenta di proposito ogni trasferimento.",
  nl="Let op: de trage modus van het systeem voor geheugenkaarten staat aan en vertraagt bewust elke overdracht.",
  pt="Nota: o modo lento do cartão de memória do sistema está ligado e abranda propositadamente cada transferência.",
  ptbr="Observação: o modo lento do cartão de memória do sistema está ligado e deixa cada transferência mais lenta de propósito.",
  ru="Примечание: системный медленный режим карты памяти включён и намеренно замедляет каждую передачу.",
  ko="참고: 시스템의 메모리 카드 저속 모드가 켜져 있어 모든 전송을 의도적으로 느리게 합니다.",
  zht="注意：系統的記憶卡慢速模式已開啟，會刻意減慢每次傳輸。",
  zhs="注意：系统的存储卡慢速模式已开启，会刻意减慢每次传输。",
  fi="Huomio: järjestelmän muistikortin hidas tila on päällä ja hidastaa tarkoituksella jokaista siirtoa.",
  sv="Obs! Systemets långsamma läge för minneskort är på och gör avsiktligt varje överföring långsammare.",
  da="Bemærk: systemets langsomme tilstand for hukommelseskort er slået til og gør bevidst hver overførsel langsommere.",
  no="Merk: systemets trege modus for minnekort er på og gjør bevisst hver overføring tregere.",
  pl="Uwaga: systemowy tryb wolnej karty pamięci jest włączony i celowo spowalnia każdy transfer.",
  tr="Not: sistemin hafıza kartı yavaş modu açık; her aktarımı bilerek yavaşlatır.")
s("TS_I18N_HINT_HUD",
  en="Show live storage speed over every app. Compact: one line per storage in use. Extended: adds totals and errors.",
  engb="Show live storage speed over every app. Compact: one line per storage in use. Extended: adds totals and errors.",
  ja="すべてのアプリ上にストレージの速度をリアルタイム表示します。コンパクト：使用中のストレージごとに1行。拡張：合計とエラーも表示。",
  fr="Affiche la vitesse du stockage en direct par-dessus toutes les applications. Compact : une ligne par stockage utilisé. Étendu : ajoute les totaux et les erreurs.",
  es="Muestra la velocidad del almacenamiento en directo sobre cualquier aplicación. Compacto: una línea por almacenamiento en uso. Extendido: añade totales y errores.",
  de="Zeigt die Speichergeschwindigkeit live über jeder App. Kompakt: eine Zeile pro genutztem Speicher. Erweitert: zusätzlich Summen und Fehler.",
  it="Mostra la velocità della memoria in tempo reale sopra ogni app. Compatto: una riga per memoria in uso. Esteso: aggiunge totali ed errori.",
  nl="Toont de opslagsnelheid live boven elke app. Compact: één regel per opslag in gebruik. Uitgebreid: voegt totalen en fouten toe.",
  pt="Mostra a velocidade do armazenamento em tempo real sobre qualquer aplicação. Compacto: uma linha por armazenamento em uso. Alargado: acrescenta totais e erros.",
  ptbr="Mostra a velocidade do armazenamento ao vivo sobre qualquer app. Compacto: uma linha por armazenamento em uso. Estendido: adiciona totais e erros.",
  ru="Показывает скорость хранилища в реальном времени поверх любого приложения. Компактно: строка на каждое используемое хранилище. Расширенно: добавляет итоги и ошибки.",
  ko="모든 앱 위에 저장소 속도를 실시간으로 표시합니다. 간단히: 사용 중인 저장소마다 한 줄. 자세히: 합계와 오류 추가.",
  zht="在所有應用程式上即時顯示儲存速度。精簡：每個使用中的儲存空間一行。詳細：加入總計與錯誤。",
  zhs="在所有应用上实时显示存储速度。精简：每个使用中的存储一行。详细：加入总计与错误。",
  fi="Näyttää tallennuksen nopeuden reaaliajassa jokaisen sovelluksen päällä. Tiivis: rivi jokaista käytössä olevaa tallennustilaa kohden. Laaja: lisää summat ja virheet.",
  sv="Visar lagringshastigheten live över alla appar. Kompakt: en rad per lagring som används. Utökad: lägger till summor och fel.",
  da="Viser lagerhastigheden live over alle apps. Kompakt: én linje pr. lager i brug. Udvidet: tilføjer totaler og fejl.",
  no="Viser lagringshastigheten live over alle apper. Kompakt: én linje per lagring i bruk. Utvidet: legger til totaler og feil.",
  pl="Pokazuje prędkość pamięci na żywo nad każdą aplikacją. Kompaktowy: jedna linia na używaną pamięć. Rozszerzony: dodaje sumy i błędy.",
  tr="Depolama hızını her uygulamanın üzerinde canlı gösterir. Kompakt: kullanılan her depolama için bir satır. Genişletilmiş: toplamları ve hataları ekler.")
s("TS_I18N_HINT_HUD_POS", en="Which corner of the screen the HUD sits in.", engb="Which corner of the screen the HUD sits in.",
  ja="HUDを表示する画面の隅。", fr="Le coin de l'écran où s'affiche le HUD.", es="En qué esquina de la pantalla aparece el HUD.",
  de="In welcher Bildschirmecke das HUD erscheint.", it="In quale angolo dello schermo appare l'HUD.",
  nl="In welke hoek van het scherm de HUD staat.", pt="Em que canto do ecrã aparece o HUD.",
  ptbr="Em qual canto da tela o HUD aparece.", ru="В каком углу экрана отображается HUD.",
  ko="HUD가 표시될 화면 모서리.", zht="HUD 顯示在螢幕的哪個角落。", zhs="HUD 显示在屏幕的哪个角落。",
  fi="Missä näytön kulmassa HUD näkyy.", sv="I vilket hörn av skärmen HUD:en visas.",
  da="Hvilket hjørne af skærmen HUD'en vises i.", no="Hvilket hjørne av skjermen HUD-en vises i.",
  pl="W którym rogu ekranu pojawia się HUD.", tr="HUD'nin ekranın hangi köşesinde görüneceği.")
s("TS_I18N_HINT_DEVMODE",
  en="Opens a command port (TCP 1340) on your local network for diagnostics. Leave off unless you are testing.",
  engb="Opens a command port (TCP 1340) on your local network for diagnostics. Leave off unless you are testing.",
  ja="診断用にローカルネットワーク上でコマンドポート（TCP 1340）を開きます。テスト時以外はオフにしてください。",
  fr="Ouvre un port de commande (TCP 1340) sur votre réseau local pour le diagnostic. Laissez désactivé sauf pour des tests.",
  es="Abre un puerto de comandos (TCP 1340) en tu red local para diagnóstico. Déjalo desactivado salvo para pruebas.",
  de="Öffnet einen Befehlsport (TCP 1340) im lokalen Netzwerk für die Diagnose. Nur zum Testen einschalten.",
  it="Apre una porta comandi (TCP 1340) sulla rete locale per la diagnostica. Lascia disattivato se non stai testando.",
  nl="Opent een opdrachtpoort (TCP 1340) op je lokale netwerk voor diagnose. Laat uit tenzij je test.",
  pt="Abre uma porta de comandos (TCP 1340) na sua rede local para diagnóstico. Deixe desligado exceto para testes.",
  ptbr="Abre uma porta de comandos (TCP 1340) na sua rede local para diagnóstico. Deixe desligado, a não ser para testes.",
  ru="Открывает командный порт (TCP 1340) в локальной сети для диагностики. Включайте только для тестирования.",
  ko="진단을 위해 로컬 네트워크에 명령 포트(TCP 1340)를 엽니다. 테스트할 때가 아니면 끄세요.",
  zht="在區域網路上開啟命令連接埠（TCP 1340）以供診斷。除非正在測試，請保持關閉。",
  zhs="在局域网上打开命令端口（TCP 1340）以供诊断。除非正在测试，请保持关闭。",
  fi="Avaa komentoportin (TCP 1340) lähiverkkoon vianmääritystä varten. Pidä pois päältä, ellet testaa.",
  sv="Öppnar en kommandoport (TCP 1340) i ditt lokala nätverk för felsökning. Låt vara av om du inte testar.",
  da="Åbner en kommandoport (TCP 1340) på dit lokale netværk til fejlfinding. Lad den være slået fra, medmindre du tester.",
  no="Åpner en kommandoport (TCP 1340) på det lokale nettverket for feilsøking. La være av med mindre du tester.",
  pl="Otwiera port poleceń (TCP 1340) w sieci lokalnej do diagnostyki. Zostaw wyłączone, jeśli nie testujesz.",
  tr="Tanılama için yerel ağınızda bir komut bağlantı noktası (TCP 1340) açar. Test etmiyorsanız kapalı bırakın.")
def items(id_, d):
    s(id_, **d)
items("TS_I18N_ITEM_HUD_OFF", dict(en="Off", engb="Off", ja="オフ", fr="Désactivé", es="Desactivado", de="Aus", it="Disattivato", nl="Uit", pt="Desligado", ptbr="Desligado", ru="Выкл.", ko="끔", zht="關閉", zhs="关闭", fi="Pois", sv="Av", da="Fra", no="Av", pl="Wyłączony", tr="Kapalı"))
items("TS_I18N_ITEM_HUD_COMPACT", dict(en="Compact", engb="Compact", ja="コンパクト", fr="Compact", es="Compacto", de="Kompakt", it="Compatto", nl="Compact", pt="Compacto", ptbr="Compacto", ru="Компактно", ko="간단히", zht="精簡", zhs="精简", fi="Tiivis", sv="Kompakt", da="Kompakt", no="Kompakt", pl="Kompaktowy", tr="Kompakt"))
items("TS_I18N_ITEM_HUD_EXTENDED", dict(en="Extended", engb="Extended", ja="拡張", fr="Étendu", es="Extendido", de="Erweitert", it="Esteso", nl="Uitgebreid", pt="Alargado", ptbr="Estendido", ru="Расширенно", ko="자세히", zht="詳細", zhs="详细", fi="Laaja", sv="Utökad", da="Udvidet", no="Utvidet", pl="Rozszerzony", tr="Genişletilmiş"))
items("TS_I18N_ITEM_POS_TL", dict(en="Top left", engb="Top left", ja="左上", fr="En haut à gauche", es="Arriba a la izquierda", de="Oben links", it="In alto a sinistra", nl="Linksboven", pt="Canto superior esquerdo", ptbr="Canto superior esquerdo", ru="Слева сверху", ko="왼쪽 위", zht="左上", zhs="左上", fi="Ylävasen", sv="Uppe till vänster", da="Øverst til venstre", no="Øverst til venstre", pl="Lewy górny", tr="Sol üst"))
items("TS_I18N_ITEM_POS_TR", dict(en="Top right", engb="Top right", ja="右上", fr="En haut à droite", es="Arriba a la derecha", de="Oben rechts", it="In alto a destra", nl="Rechtsboven", pt="Canto superior direito", ptbr="Canto superior direito", ru="Справа сверху", ko="오른쪽 위", zht="右上", zhs="右上", fi="Yläoikea", sv="Uppe till höger", da="Øverst til højre", no="Øverst til høyre", pl="Prawy górny", tr="Sağ üst"))
items("TS_I18N_ITEM_POS_BL", dict(en="Bottom left", engb="Bottom left", ja="左下", fr="En bas à gauche", es="Abajo a la izquierda", de="Unten links", it="In basso a sinistra", nl="Linksonder", pt="Canto inferior esquerdo", ptbr="Canto inferior esquerdo", ru="Слева снизу", ko="왼쪽 아래", zht="左下", zhs="左下", fi="Alavasen", sv="Nere till vänster", da="Nederst til venstre", no="Nederst til venstre", pl="Lewy dolny", tr="Sol alt"))
items("TS_I18N_ITEM_POS_BR", dict(en="Bottom right", engb="Bottom right", ja="右下", fr="En bas à droite", es="Abajo a la derecha", de="Unten rechts", it="In basso a destra", nl="Rechtsonder", pt="Canto inferior direito", ptbr="Canto inferior direito", ru="Справа снизу", ko="오른쪽 아래", zht="右下", zhs="右下", fi="Alaoikea", sv="Nere till höger", da="Nederst til højre", no="Nederst til høyre", pl="Prawy dolny", tr="Sağ alt"))
s("TS_I18N_ST_FALLBACK",
  en="Switched back to standard after an error. Turn off and on to retry; please report it.",
  engb="Switched back to standard after an error. Turn off and on to retry; please report it.",
  ja="エラーのため通常に戻りました。オフにしてからオンにすると再試行します。報告をお願いします。",
  fr="Repassé en mode standard après une erreur. Désactivez puis réactivez pour réessayer ; merci de le signaler.",
  es="Volvió al modo estándar tras un error. Desactívalo y actívalo para reintentar; por favor, infórmalo.",
  de="Nach einem Fehler auf Normalbetrieb zurückgeschaltet. Aus- und wieder einschalten zum erneuten Versuch; bitte melden.",
  it="Tornato alla modalità standard dopo un errore. Disattiva e riattiva per riprovare; per favore segnalalo.",
  nl="Na een fout teruggeschakeld naar standaard. Zet uit en weer aan om opnieuw te proberen; meld het alsjeblieft.",
  pt="Voltou ao modo padrão após um erro. Desligue e ligue para tentar de novo; por favor, reporte.",
  ptbr="Voltou ao modo padrão após um erro. Desligue e ligue para tentar de novo; por favor, reporte.",
  ru="После ошибки возвращён обычный режим. Выключите и включите, чтобы повторить; пожалуйста, сообщите об этом.",
  ko="오류로 일반 모드로 돌아갔습니다. 껐다 켜면 다시 시도합니다. 신고해 주세요.",
  zht="發生錯誤後已切回標準模式。關閉再開啟即可重試；請回報此問題。",
  zhs="发生错误后已切回标准模式。关闭再开启即可重试；请报告此问题。",
  fi="Palasi tavalliseen tilaan virheen jälkeen. Kytke pois ja takaisin yrittääksesi uudelleen; ilmoitathan siitä.",
  sv="Gick tillbaka till standard efter ett fel. Slå av och på för att försöka igen; rapportera gärna.",
  da="Skiftede tilbage til standard efter en fejl. Slå fra og til for at prøve igen; rapportér det gerne.",
  no="Gikk tilbake til standard etter en feil. Slå av og på for å prøve igjen; rapporter det gjerne.",
  pl="Po błędzie przełączono z powrotem na tryb standardowy. Wyłącz i włącz, aby spróbować ponownie; zgłoś to.",
  tr="Bir hatadan sonra standarda dönüldü. Yeniden denemek için kapatıp açın; lütfen bildirin.")
s("TS_I18N_ST_NO_BUFFER", en="Unavailable: not enough memory at boot. Standard speed.", engb="Unavailable: not enough memory at boot. Standard speed.",
  ja="利用不可：起動時のメモリ不足。通常の速度。", fr="Indisponible : mémoire insuffisante au démarrage. Vitesse standard.",
  es="No disponible: memoria insuficiente al arrancar. Velocidad estándar.", de="Nicht verfügbar: zu wenig Speicher beim Start. Normale Geschwindigkeit.",
  it="Non disponibile: memoria insufficiente all'avvio. Velocità standard.", nl="Niet beschikbaar: te weinig geheugen bij het opstarten. Standaardsnelheid.",
  pt="Indisponível: memória insuficiente no arranque. Velocidade padrão.", ptbr="Indisponível: memória insuficiente na inicialização. Velocidade padrão.",
  ru="Недоступно: не хватило памяти при запуске. Обычная скорость.", ko="사용 불가: 부팅 시 메모리 부족. 일반 속도.",
  zht="無法使用：開機時記憶體不足。標準速度。", zhs="不可用：启动时内存不足。标准速度。",
  fi="Ei käytettävissä: muisti ei riittänyt käynnistyksessä. Tavallinen nopeus.", sv="Ej tillgängligt: för lite minne vid start. Normal hastighet.",
  da="Utilgængelig: for lidt hukommelse ved opstart. Normal hastighed.", no="Utilgjengelig: for lite minne ved oppstart. Normal hastighet.",
  pl="Niedostępne: za mało pamięci przy uruchomieniu. Standardowa prędkość.", tr="Kullanılamıyor: açılışta bellek yetersiz. Standart hız.")
s("TS_I18N_ST_FIRMWARE", en="Inactive: this firmware is not supported. Standard speed.", engb="Inactive: this firmware is not supported. Standard speed.",
  ja="無効：このファームウェアには対応していません。通常の速度。", fr="Inactif : ce firmware n'est pas pris en charge. Vitesse standard.",
  es="Inactivo: este firmware no es compatible. Velocidad estándar.", de="Inaktiv: Diese Firmware wird nicht unterstützt. Normale Geschwindigkeit.",
  it="Inattivo: questo firmware non è supportato. Velocità standard.", nl="Inactief: deze firmware wordt niet ondersteund. Standaardsnelheid.",
  pt="Inativo: este firmware não é suportado. Velocidade padrão.", ptbr="Inativo: este firmware não é suportado. Velocidade padrão.",
  ru="Неактивно: эта прошивка не поддерживается. Обычная скорость.", ko="비활성: 이 펌웨어는 지원되지 않습니다. 일반 속도.",
  zht="未啟用：不支援此韌體。標準速度。", zhs="未启用：不支持此固件。标准速度。",
  fi="Ei käytössä: tätä laiteohjelmistoa ei tueta. Tavallinen nopeus.", sv="Inaktiv: den här firmwaren stöds inte. Normal hastighet.",
  da="Inaktiv: denne firmware understøttes ikke. Normal hastighed.", no="Inaktiv: denne fastvaren støttes ikke. Normal hastighet.",
  pl="Nieaktywne: to oprogramowanie nie jest obsługiwane. Standardowa prędkość.", tr="Etkin değil: bu yazılım desteklenmiyor. Standart hız.")
s("TS_I18N_ST_HOOKS", en="Inactive: could not start. Standard speed.", engb="Inactive: could not start. Standard speed.",
  ja="無効：開始できませんでした。通常の速度。", fr="Inactif : impossible de démarrer. Vitesse standard.", es="Inactivo: no se pudo iniciar. Velocidad estándar.",
  de="Inaktiv: Start fehlgeschlagen. Normale Geschwindigkeit.", it="Inattivo: impossibile avviare. Velocità standard.", nl="Inactief: kon niet starten. Standaardsnelheid.",
  pt="Inativo: não foi possível iniciar. Velocidade padrão.", ptbr="Inativo: não foi possível iniciar. Velocidade padrão.",
  ru="Неактивно: не удалось запустить. Обычная скорость.", ko="비활성: 시작할 수 없습니다. 일반 속도.", zht="未啟用：無法啟動。標準速度。", zhs="未启用：无法启动。标准速度。",
  fi="Ei käytössä: käynnistys epäonnistui. Tavallinen nopeus.", sv="Inaktiv: kunde inte starta. Normal hastighet.", da="Inaktiv: kunne ikke starte. Normal hastighed.",
  no="Inaktiv: kunne ikke starte. Normal hastighet.", pl="Nieaktywne: nie udało się uruchomić. Standardowa prędkość.", tr="Etkin değil: başlatılamadı. Standart hız.")
s("TS_I18N_ST_SAFE", en="Inactive: safe mode after an incomplete start. Standard speed.", engb="Inactive: safe mode after an incomplete start. Standard speed.",
  ja="無効：前回の起動が完了しなかったためセーフモード。通常の速度。", fr="Inactif : mode sans échec après un démarrage incomplet. Vitesse standard.",
  es="Inactivo: modo seguro tras un inicio incompleto. Velocidad estándar.", de="Inaktiv: abgesicherter Modus nach unvollständigem Start. Normale Geschwindigkeit.",
  it="Inattivo: modalità provvisoria dopo un avvio incompleto. Velocità standard.", nl="Inactief: veilige modus na een onvolledige start. Standaardsnelheid.",
  pt="Inativo: modo de segurança após um arranque incompleto. Velocidade padrão.", ptbr="Inativo: modo de segurança após uma inicialização incompleta. Velocidade padrão.",
  ru="Неактивно: безопасный режим после незавершённого запуска. Обычная скорость.", ko="비활성: 시작이 완료되지 않아 안전 모드. 일반 속도.",
  zht="未啟用：啟動未完成，已進入安全模式。標準速度。", zhs="未启用：启动未完成，已进入安全模式。标准速度。",
  fi="Ei käytössä: vikasietotila keskeytyneen käynnistyksen jälkeen. Tavallinen nopeus.", sv="Inaktiv: felsäkert läge efter en ofullständig start. Normal hastighet.",
  da="Inaktiv: sikker tilstand efter en ufuldstændig start. Normal hastighed.", no="Inaktiv: sikker modus etter en ufullstendig oppstart. Normal hastighet.",
  pl="Nieaktywne: tryb awaryjny po niepełnym uruchomieniu. Standardowa prędkość.", tr="Etkin değil: tamamlanmayan başlatma sonrası güvenli mod. Standart hız.")
s("TS_I18N_ST_NO_KERNEL", en="Inactive: the kernel plugin is not loaded.", engb="Inactive: the kernel plugin is not loaded.",
  ja="無効：カーネルプラグインが読み込まれていません。", fr="Inactif : le plugin noyau n'est pas chargé.", es="Inactivo: el plugin de kernel no está cargado.",
  de="Inaktiv: Das Kernel-Plugin ist nicht geladen.", it="Inattivo: il plugin kernel non è caricato.", nl="Inactief: de kernelplugin is niet geladen.",
  pt="Inativo: o plugin de kernel não está carregado.", ptbr="Inativo: o plugin de kernel não está carregado.", ru="Неактивно: плагин ядра не загружен.",
  ko="비활성: 커널 플러그인이 로드되지 않았습니다.", zht="未啟用：未載入核心外掛程式。", zhs="未启用：未加载内核插件。",
  fi="Ei käytössä: ydinliitännäistä ei ole ladattu.", sv="Inaktiv: kärnpluginet är inte laddat.", da="Inaktiv: kerne-pluginet er ikke indlæst.",
  no="Inaktiv: kjerne-pluginen er ikke lastet.", pl="Nieaktywne: wtyczka jądra nie jest załadowana.", tr="Etkin değil: çekirdek eklentisi yüklenmedi.")
def esc(t):
    return t.replace("\\", "\\\\").replace('"', '\\"')

s("TS_I18N_ROW_HUD_GAME", ja="ゲーム中のHUD", en="In-game HUD", fr="HUD en jeu", es="HUD durante el juego", de="HUD im Spiel", it="HUD in gioco", nl="HUD tijdens spelen", pt="HUD durante o jogo", ru="HUD в играх", ko="게임 중 HUD", zht="遊戲中顯示 HUD", zhs="游戏中显示 HUD", fi="HUD pelin aikana", sv="HUD i spel", da="HUD i spil", no="HUD i spill", pl="HUD w grach", ptbr="HUD durante o jogo", engb="In-game HUD", tr="Oyun içi HUD")
s("TS_I18N_HINT_HUD_GAME", ja="ゲームやアプリの実行中もHUDを表示します。オフにすると、ホーム画面、設定、クイックメニュー（PSボタン）でのみ表示されます。", en="Also show the HUD while a game or app is running. When off, it only shows on the Home screen, in Settings and in the quick menu (press the PS button).", fr="Affiche aussi le HUD pendant un jeu ou une application. Désactivé, il n'apparaît que sur l'écran d'accueil, dans les Paramètres et dans le menu rapide (touche PS).", es="Muestra también el HUD mientras se ejecuta un juego o una aplicación. Si está desactivado, solo aparece en la pantalla de inicio, en Ajustes y en el menú rápido (botón PS).", de="Zeigt das HUD auch während eines Spiels oder einer App. Wenn aus, erscheint es nur auf dem Startbildschirm, in den Einstellungen und im Schnellmenü (PS-Taste).", it="Mostra l'HUD anche durante un gioco o un'applicazione. Se disattivato, appare solo nella schermata Home, nelle Impostazioni e nel menu rapido (tasto PS).", nl="Toont de HUD ook tijdens een game of app. Als dit uit staat, verschijnt de HUD alleen op het startscherm, in Instellingen en in het snelmenu (PS-toets).", pt="Mostra também o HUD durante um jogo ou aplicação. Desativado, só aparece no ecrã inicial, nas Definições e no menu rápido (botão PS).", ru="Показывать HUD и во время игры или приложения. Если выключено, HUD виден только на главном экране, в настройках и в быстром меню (кнопка PS).", ko="게임이나 앱 실행 중에도 HUD를 표시합니다. 끄면 홈 화면, 설정, 퀵 메뉴(PS 버튼)에서만 표시됩니다.", zht="執行遊戲或應用程式時也顯示 HUD。關閉時，只在主畫面、設定及快速選單（PS 按鈕）中顯示。", zhs="运行游戏或应用程序时也显示 HUD。关闭时，仅在主屏幕、设置和快捷菜单（PS 按钮）中显示。", fi="Näyttää HUD:n myös pelin tai sovelluksen aikana. Pois päältä se näkyy vain aloitusnäytössä, asetuksissa ja pikavalikossa (PS-näppäin).", sv="Visar HUD:en även när ett spel eller en app körs. Av: den visas bara på hemskärmen, i Inställningar och i snabbmenyn (PS-knappen).", da="Viser også HUD'en, mens et spil eller en app kører. Fra: den vises kun på startskærmen, i Indstillinger og i hurtigmenuen (PS-knappen).", no="Viser også HUD-en mens et spill eller en app kjører. Av: den vises bare på startskjermen, i Innstillinger og i hurtigmenyen (PS-knappen).", pl="Pokazuje HUD także podczas gry lub aplikacji. Gdy wyłączone, HUD jest widoczny tylko na ekranie głównym, w Ustawieniach i w szybkim menu (przycisk PS).", ptbr="Mostra o HUD também durante um jogo ou aplicativo. Desligado, ele só aparece na tela inicial, em Configurações e no menu rápido (botão PS).", engb="Also show the HUD while a game or app is running. When off, it only shows on the Home screen, in Settings and in the quick menu (press the PS button).", tr="HUD'yi bir oyun veya uygulama çalışırken de gösterir. Kapalıyken yalnızca ana ekranda, Ayarlar'da ve hızlı menüde (PS düğmesi) görünür.")
s("TS_I18N_ROW_HUD_RECENT", ja="使用中のみ表示", en="Recent activity only", fr="Activité récente uniquement", es="Solo actividad reciente", de="Nur aktive Speicher", it="Solo attività recente", nl="Alleen recente activiteit", pt="Apenas atividade recente", ru="Только активные", ko="최근 사용만 표시", zht="僅顯示最近使用", zhs="仅显示最近使用", fi="Vain viimeaikainen käyttö", sv="Endast senaste aktivitet", da="Kun seneste aktivitet", no="Bare nylig aktivitet", pl="Tylko ostatnia aktywność", ptbr="Somente atividade recente", engb="Recent activity only", tr="Yalnızca son etkinlik")
s("TS_I18N_HINT_HUD_RECENT", ja="読み書きしたストレージだけを表示し、30秒間使われないと消えます。何も使っていないときはHUDも消えます。", en="Only shows a storage while it's in use: it appears when it's read or written and drops off after 30 seconds without activity. With nothing in use, the HUD hides.", fr="N'affiche un stockage que pendant son utilisation : il apparaît lors d'une lecture ou d'une écriture et disparaît après 30 secondes sans activité. Si rien n'est utilisé, le HUD se masque.", es="Solo muestra un almacenamiento mientras se usa: aparece al leer o escribir y desaparece tras 30 segundos sin actividad. Si no se usa ninguno, el HUD se oculta.", de="Zeigt einen Speicher nur während der Nutzung: Er erscheint beim Lesen oder Schreiben und verschwindet nach 30 Sekunden ohne Aktivität. Wird keiner genutzt, wird das HUD ausgeblendet.", it="Mostra una memoria solo mentre è in uso: appare durante una lettura o una scrittura e scompare dopo 30 secondi di inattività. Se nessuna è in uso, l'HUD si nasconde.", nl="Toont een opslag alleen terwijl die in gebruik is: hij verschijnt bij lezen of schrijven en verdwijnt na 30 seconden zonder activiteit. Als er niets in gebruik is, verdwijnt de HUD.", pt="Só mostra um armazenamento enquanto está em uso: aparece ao ler ou escrever e desaparece após 30 segundos sem atividade. Se nada estiver em uso, o HUD esconde-se.", ru="Показывает память, только пока она используется: строка появляется при чтении или записи и исчезает через 30 секунд без активности. Если ничего не используется, HUD скрывается.", ko="사용 중인 저장소만 표시합니다. 읽거나 쓸 때 나타나고 30초 동안 사용하지 않으면 사라집니다. 아무것도 사용하지 않으면 HUD가 숨겨집니다.", zht="僅在儲存空間使用中時顯示：讀取或寫入時出現，30 秒無活動後消失。皆未使用時，HUD 會隱藏。", zhs="仅在存储使用中时显示：读取或写入时出现，30 秒无活动后消失。均未使用时，HUD 会隐藏。", fi="Näyttää tallennustilan vain käytön aikana: se tulee näkyviin lukiessa tai kirjoittaessa ja katoaa 30 sekunnin käyttämättömyyden jälkeen. Kun mitään ei käytetä, HUD piiloutuu.", sv="Visar bara en lagring medan den används: den visas vid läsning eller skrivning och försvinner efter 30 sekunder utan aktivitet. När inget används döljs HUD:en.", da="Viser kun et lager, mens det er i brug: det vises ved læsning eller skrivning og forsvinder efter 30 sekunder uden aktivitet. Når intet er i brug, skjules HUD'en.", no="Viser bare en lagring mens den er i bruk: den vises ved lesing eller skriving og forsvinner etter 30 sekunder uten aktivitet. Når ingenting er i bruk, skjules HUD-en.", pl="Pokazuje pamięć tylko podczas używania: pojawia się przy odczycie lub zapisie i znika po 30 sekundach bez aktywności. Gdy nic nie jest używane, HUD się ukrywa.", ptbr="Só mostra um armazenamento enquanto está em uso: aparece ao ler ou gravar e some após 30 segundos sem atividade. Se nada estiver em uso, o HUD se esconde.", engb="Only shows a storage while it's in use: it appears when it's read or written and drops off after 30 seconds without activity. With nothing in use, the HUD hides.", tr="Bir depolamayı yalnızca kullanılırken gösterir: okunduğunda veya yazıldığında görünür, 30 saniye işlem olmazsa kaybolur. Hiçbiri kullanılmıyorsa HUD gizlenir.")
s("TS_I18N_ROW_SAFE", ja="セーフモード", en="Safe mode", fr="Mode sans échec", es="Modo seguro", de="Abgesicherter Modus", it="Modalità provvisoria", nl="Veilige modus", pt="Modo de segurança", ru="Безопасный режим", ko="안전 모드", zht="安全模式", zhs="安全模式", fi="Vikasietotila", sv="Felsäkert läge", da="Fejlsikret tilstand", no="Sikker modus", pl="Tryb awaryjny", ptbr="Modo de segurança", engb="Safe mode", tr="Güvenli mod")
s("TS_I18N_HINT_SAFE", ja="前回の起動が完了しなかったため（起動中の再起動やクラッシュ）、今回は何も有効になっていません。チェックを外してVitaを再起動すると、Turbo Storageが元に戻ります。", en="On because the last start didn't finish (the Vita restarted or crashed while Turbo Storage was starting), so nothing is active this boot. Untick, then restart the Vita to turn Turbo Storage back on.", fr="Activé car le dernier démarrage ne s'est pas terminé (la Vita a redémarré ou planté pendant le démarrage de Turbo Storage) : rien n'est actif cette fois. Décochez, puis redémarrez la Vita pour réactiver Turbo Storage.", es="Activado porque el último inicio no terminó (la Vita se reinició o falló mientras Turbo Storage arrancaba), así que nada está activo en este arranque. Desmárcalo y reinicia la Vita para volver a activar Turbo Storage.", de="Aktiv, weil der letzte Start nicht abgeschlossen wurde (die Vita wurde während des Starts von Turbo Storage neu gestartet oder ist abgestürzt), daher ist diesmal nichts aktiv. Deaktivieren und die Vita neu starten, um Turbo Storage wieder einzuschalten.", it="Attiva perché l'ultimo avvio non è stato completato (la Vita si è riavviata o bloccata mentre Turbo Storage si avviava), quindi in questo avvio non è attivo nulla. Deseleziona, poi riavvia la Vita per riattivare Turbo Storage.", nl="Aan omdat de laatste start niet is voltooid (de Vita herstartte of liep vast terwijl Turbo Storage startte), dus deze keer is niets actief. Vink uit en herstart de Vita om Turbo Storage weer in te schakelen.", pt="Ativo porque o último arranque não terminou (a Vita reiniciou ou bloqueou enquanto o Turbo Storage arrancava), por isso nada está ativo agora. Desmarque e reinicie a Vita para voltar a ativar o Turbo Storage.", ru="Включён, потому что прошлый запуск не завершился (Vita перезагрузилась или зависла во время запуска Turbo Storage), поэтому сейчас ничего не активно. Снимите флажок и перезагрузите Vita, чтобы снова включить Turbo Storage.", ko="지난번 시작이 완료되지 않아(Turbo Storage 시작 중 재시작 또는 충돌) 이번 부팅에서는 아무것도 활성화되지 않았습니다. 체크를 해제한 후 Vita를 재시작하면 Turbo Storage가 다시 켜집니다.", zht="因上次啟動未完成（Turbo Storage 啟動時 Vita 重新啟動或當機）而開啟，本次開機未啟用任何功能。取消勾選後重新啟動 Vita，即可重新啟用 Turbo Storage。", zhs="因上次启动未完成（Turbo Storage 启动时 Vita 重启或崩溃）而开启，本次开机未启用任何功能。取消勾选后重启 Vita，即可重新启用 Turbo Storage。", fi="Päällä, koska edellinen käynnistys ei valmistunut (Vita käynnistyi uudelleen tai kaatui Turbo Storagen käynnistyessä), joten mikään ei ole nyt käytössä. Poista valinta ja käynnistä Vita uudelleen ottaaksesi Turbo Storagen taas käyttöön.", sv="På eftersom den senaste starten inte slutfördes (Vitan startades om eller kraschade medan Turbo Storage startade), så inget är aktivt nu. Avmarkera och starta om Vitan för att slå på Turbo Storage igen.", da="Slået til, fordi den sidste start ikke blev fuldført (Vitaen genstartede eller gik ned, mens Turbo Storage startede), så intet er aktivt nu. Fjern markeringen, og genstart Vitaen for at slå Turbo Storage til igen.", no="På fordi forrige oppstart ikke ble fullført (Vitaen startet på nytt eller krasjet mens Turbo Storage startet), så ingenting er aktivt nå. Fjern merket og start Vitaen på nytt for å slå på Turbo Storage igjen.", pl="Włączony, bo ostatnie uruchomienie się nie zakończyło (Vita zrestartowała się lub zawiesiła podczas uruchamiania Turbo Storage), więc teraz nic nie jest aktywne. Odznacz i uruchom Vitę ponownie, aby znów włączyć Turbo Storage.", ptbr="Ativo porque a última inicialização não terminou (o Vita reiniciou ou travou enquanto o Turbo Storage iniciava), então nada está ativo agora. Desmarque e reinicie o Vita para ativar o Turbo Storage de novo.", engb="On because the last start didn't finish (the Vita restarted or crashed while Turbo Storage was starting), so nothing is active this boot. Untick, then restart the Vita to turn Turbo Storage back on.", tr="Son başlatma tamamlanmadığı için açık (Turbo Storage başlarken Vita yeniden başladı veya çöktü), bu yüzden şu an hiçbir şey etkin değil. İşareti kaldırıp Vita'yı yeniden başlatarak Turbo Storage'ı tekrar açın.")
s("TS_I18N_HINT_SAFE_LEFT", ja="セーフモードを解除しました。Vitaを再起動すると、Turbo Storageが元に戻ります。", en="Safe mode left. Restart the Vita to turn Turbo Storage back on.", fr="Mode sans échec quitté. Redémarrez la Vita pour réactiver Turbo Storage.", es="Modo seguro desactivado. Reinicia la Vita para volver a activar Turbo Storage.", de="Abgesicherter Modus verlassen. Starte die Vita neu, um Turbo Storage wieder einzuschalten.", it="Modalità provvisoria disattivata. Riavvia la Vita per riattivare Turbo Storage.", nl="Veilige modus verlaten. Herstart de Vita om Turbo Storage weer in te schakelen.", pt="Modo de segurança desativado. Reinicie a Vita para voltar a ativar o Turbo Storage.", ru="Безопасный режим отключён. Перезагрузите Vita, чтобы снова включить Turbo Storage.", ko="안전 모드를 해제했습니다. Vita를 재시작하면 Turbo Storage가 다시 켜집니다.", zht="已離開安全模式。重新啟動 Vita 即可重新啟用 Turbo Storage。", zhs="已退出安全模式。重启 Vita 即可重新启用 Turbo Storage。", fi="Vikasietotilasta poistuttu. Käynnistä Vita uudelleen ottaaksesi Turbo Storagen taas käyttöön.", sv="Felsäkert läge avslutat. Starta om Vitan för att slå på Turbo Storage igen.", da="Fejlsikret tilstand forladt. Genstart Vitaen for at slå Turbo Storage til igen.", no="Sikker modus avsluttet. Start Vitaen på nytt for å slå på Turbo Storage igjen.", pl="Tryb awaryjny wyłączony. Uruchom Vitę ponownie, aby znów włączyć Turbo Storage.", ptbr="Modo de segurança desativado. Reinicie o Vita para ativar o Turbo Storage de novo.", engb="Safe mode left. Restart the Vita to turn Turbo Storage back on.", tr="Güvenli moddan çıkıldı. Turbo Storage'ı tekrar açmak için Vita'yı yeniden başlatın.")
ORDER = ['TS_I18N_TITLE', 'TS_I18N_ROW_SD2VITA', 'TS_I18N_ROW_INTERNAL', 'TS_I18N_ROW_HUD', 'TS_I18N_ROW_HUD_POS', 'TS_I18N_ROW_DEVMODE', 'TS_I18N_ROW_MEMCARD', 'TS_I18N_ROW_HUD_GAME', 'TS_I18N_ROW_HUD_RECENT', 'TS_I18N_HINT_SD2VITA', 'TS_I18N_HINT_INTERNAL', 'TS_I18N_HINT_HUD', 'TS_I18N_HINT_HUD_POS', 'TS_I18N_HINT_DEVMODE', 'TS_I18N_HINT_MEMCARD', 'TS_I18N_HINT_HUD_GAME', 'TS_I18N_HINT_HUD_RECENT', 'TS_I18N_ITEM_HUD_OFF', 'TS_I18N_ITEM_HUD_COMPACT', 'TS_I18N_ITEM_HUD_EXTENDED', 'TS_I18N_ITEM_POS_TL', 'TS_I18N_ITEM_POS_TR', 'TS_I18N_ITEM_POS_BL', 'TS_I18N_ITEM_POS_BR', 'TS_I18N_ST_FALLBACK', 'TS_I18N_ST_NO_BUFFER', 'TS_I18N_ST_FIRMWARE', 'TS_I18N_ST_HOOKS', 'TS_I18N_ST_SAFE', 'TS_I18N_ST_NO_KERNEL', 'TS_I18N_ST_SLOW', 'TS_I18N_ROW_SAFE', 'TS_I18N_HINT_SAFE', 'TS_I18N_HINT_SAFE_LEFT']
assert set(ORDER) == set(T), set(ORDER) ^ set(T)
out = ["/* Generated by tools/gen_i18n.py - edit that, not this file. */",
       "/* clang-format off */"]
for k in ORDER:
    row = T[k]
    missing = [l for l in LANGS if l not in row]
    assert not missing, (k, missing)
    out.append(f"[{k}] = {{")
    for l in LANGS:
        out.append(f'\t"{esc(row[l])}",   /* {l} */')
    out.append("},")
open("settings_app/storageturbo_i18n_table.h", "w").write("\n".join(out) + "\n")
print("wrote", len(ORDER), "strings x", len(LANGS), "languages")
